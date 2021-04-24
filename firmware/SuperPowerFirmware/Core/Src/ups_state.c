/*
 * ups_state.c
 *
 *  Created on: 17 Jan 2021
 *      Author: jbaumann
 */


/*
   The state variable encapsulates the all-over state of the system (ATTiny and RPi
   together).
   The possible states are:
    RUNNING_STATE       -  0 - the system is running normally
    UNCLEAR_STATE       -  1 - the system has been reset and is unsure about its state
    WARN_TO_RUNNING     -  2 - the system transitions from warn state to running state
    SHUTDOWN_TO_RUNNING -  4 - the system transitions from shutdown state to running state
    WARN_STATE          -  8 - the system is in the warn state
    WARN_TO_SHUTDOWN    - 16 - the system transitions from warn state to shutdown state
    SHUTDOWN_STATE      - 32 - the system is in the shutdown state
    They are ordered in a way that allows to later check for the severity of the state by
    e.g., "if(state <= WARN_STATE)"
    This function implements the state changes between these states, during the normal
    execution but in the case of a reset as well. For this we have to take into account that
    the only information we might have is the current voltage and we are in the RUNNING_STATE.
*/

#include <stdbool.h>

#include "main.h"
#include "ups_state.h"
#include "i2c_register.h"
#include "task_communication.h"
#include "gpio.h"

uint32_t millis_last_contact      = 0;
uint8_t ups_state_should_shutdown = shutdown_cause_none;


/*
 * Change the state dependent on the freshly read battery voltage
 */
void voltage_dependent_state_change(uint16_t seconds_since_last_contact) {
	// check first whether charger contact is established
	if (i2c_status_register_8bit->val.charger_contact == true) {
		if (i2c_status_register_16bit->val.ups_bat_voltage
				<= i2c_config_register_16bit->val.ups_shutdown_voltage) {
			if (i2c_status_register_8bit->val.ups_state
					< ups_warn_to_shutdown) {
				i2c_status_register_8bit->val.ups_state = ups_warn_to_shutdown;
			}
		} else if (i2c_status_register_16bit->val.ups_bat_voltage
				<= i2c_config_register_16bit->val.warn_voltage) {
			if (i2c_status_register_8bit->val.ups_state < ups_warn_state) {
				i2c_status_register_8bit->val.ups_state = ups_warn_state;
				ups_state_should_shutdown |= shutdown_cause_bat_voltage;
			}
		} else if (i2c_status_register_16bit->val.ups_bat_voltage
				<= i2c_config_register_16bit->val.restart_voltage) {
			if (i2c_status_register_8bit->val.ups_state == ups_unclear_state
					&& seconds_since_last_contact
							> i2c_config_register_16bit->val.timeout) {
				// the RPi is not running, even after the timeout, so we assume that it
				// shut down, this means we come from a WARN_STATE or SHUTDOWN_STATE
				i2c_status_register_8bit->val.ups_state = ups_warn_state;
			}
		} else { // we are at a safe voltage

			switch (i2c_status_register_8bit->val.ups_state) {
			case ups_shutdown_state:
				i2c_status_register_8bit->val.ups_state =
						ups_shutdown_to_running;
				break;
			case ups_warn_to_shutdown:
				i2c_status_register_8bit->val.ups_state =
						ups_shutdown_to_running;
				break;
			case ups_warn_state:
				i2c_status_register_8bit->val.ups_state = ups_warn_to_running;
				break;
			case ups_unclear_state:
				i2c_status_register_8bit->val.ups_state = ups_running_state;
				break;
			case ups_running_state:
			case ups_warn_to_running:
			case ups_shutdown_to_running:
				break;
			}
		}
	}
}

/*
 * Act on the current state
 */
void act_on_state_change(uint16_t seconds_since_last_contact) {
	// This is placed before the general check of all stages as to
	// not duplicate the code of the shutdown_state
	if (i2c_status_register_8bit->val.ups_state == ups_warn_to_shutdown) {
		// immediately turn off the system if force_shutdown is set
		if (i2c_config_register_8bit->val.primed == 1) {
			if (i2c_config_register_8bit->val.force_shutdown != 0) {
				ups_off();
				osMessageQueuePut(LED_R_QueueHandle, &background_off, 0, 0);
			}
		}
		i2c_status_register_8bit->val.ups_state = ups_shutdown_state;
	}

	if (i2c_status_register_8bit->val.ups_state == ups_shutdown_state) {
		// Nothing to do
	} else if (i2c_status_register_8bit->val.ups_state == ups_warn_state) {
		// The RPi has been warned using the should_shutdown variable
		// we simply let it shutdown even if it does not set SL_INITIATED

		reset_timeout();
	} else if (i2c_status_register_8bit->val.ups_state
			== ups_shutdown_to_running) {
		// we have recovered from a shutdown and are now at a safe voltage
		if (i2c_config_register_8bit->val.primed == 1) {
			ups_on();
			osMessageQueuePut(LED_R_QueueHandle, &blink_second_background, 0, 0);
		}
		reset_timeout();
		i2c_status_register_8bit->val.ups_state = ups_running_state;
		ups_state_should_shutdown = shutdown_cause_none;
	} else if (i2c_status_register_8bit->val.ups_state == ups_warn_to_running) {
		// we have recovered from a warn state and are now at a safe voltage
		// we switch to State::running_state and let that state (below) handle
		// the restart
		i2c_status_register_8bit->val.ups_state = ups_running_state;
		ups_state_should_shutdown = shutdown_cause_none;
	} else if (i2c_status_register_8bit->val.ups_state == ups_unclear_state) {
		// we do nothing and wait until either a timeout occurs, the voltage
		// drops to warn_voltage or is higher than restart_voltage (see handle_state())
	}

	if (i2c_status_register_8bit->val.ups_state == ups_running_state) {
		_Bool should_restart = false;

		should_restart = seconds_since_last_contact
				> i2c_config_register_16bit->val.timeout;

		if (should_restart) {
			if (i2c_config_register_8bit->val.primed > 0) {
				// RPi has not accessed the I2C interface for more than timeout seconds
				// or the button has beend pressed (primed == 2).
				// We restart it. Signal restart by blinking ten times
				osMessageQueuePut(LED_R_QueueHandle, &reboot_rpi, 0, 0);

				restart_raspberry();

				// Primed has been set by the button press, we use the restart time
				// for debouncing
				if(i2c_config_register_8bit->val.primed == 2) {
					i2c_config_register_8bit->val.primed = 0;
				}
				reset_timeout();
			}
		}
	}
}

void handle_state() {
	uint16_t seconds_since_last_contact = (HAL_GetTick() - millis_last_contact)
			/ 1000;

	i2c_status_register_16bit->val.seconds = seconds_since_last_contact;

	// change the state depending on the current battery voltage
	voltage_dependent_state_change(seconds_since_last_contact);

	// If the button has been pressed or the bat_voltage is lower than the warn voltage
	// we blink the LED 5 times to signal that the RPi should shut down, if it has not
	// already signalled that it is doing so
	if (i2c_status_register_8bit->val.ups_state <= ups_warn_state) {
		// we first check whether the Raspberry is already in the shutdown process
		if (!(ups_state_should_shutdown & shutdown_cause_rpi_initiated)) {
			if (ups_state_should_shutdown > shutdown_cause_rpi_initiated
					&& (seconds_since_last_contact
							< i2c_config_register_16bit->val.timeout)) {
				// RPi should take action, possibly shut down. Signal by blinking 5 times
				osMessageQueuePut(LED_R_QueueHandle, &rpi_shuts_down, 0, 0);
			}
		}
	}

	act_on_state_change(seconds_since_last_contact);
}

/*
 * Reset the internal counter to 0.
 * We fetch the current tick counter and store it to later calculate the seconds
 * since the reset. In addition we set the I2C register value to 0
 */
void reset_timeout() {
	i2c_status_register_16bit->val.seconds = 0;
	millis_last_contact = HAL_GetTick();
}

/*
 * When we get an I2C communication then this might change our state. We also know
 * that the RPi is still alive.
 */
void i2c_triggered_ups_state_change() {
	// If we are in an unclear state, then a communication from the RPi moves us to running state
	if (i2c_status_register_8bit->val.ups_state == ups_unclear_state) {
		i2c_status_register_8bit->val.ups_state = ups_running_state;
	}
	reset_timeout();
}

