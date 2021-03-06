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

#include "ups_state.h"
#include "main.h"

uint8_t ups_state;

//temp
uint32_t seconds;
uint32_t timeout;
uint8_t primed;
uint8_t should_shutdown;
uint16_t bat_voltage;
uint16_t warn_voltage;
uint16_t restart_voltage;
uint16_t ups_shutdown_voltage;
_Bool force_shutdown;
//static const int BLINK_TIME = 100;
_Bool vext_off_is_shutdown;
static const int MIN_POWER_LEVEL = 4700;
uint16_t ext_voltage;

/*
   Change the state dependent on the freshly read battery voltage
*/
void voltage_dependent_state_change() {

  if (bat_voltage <= ups_shutdown_voltage) {
    if(ups_state < ups_warn_to_shutdown) {
      ups_state = ups_warn_to_shutdown;
    }
  } else if (bat_voltage <= warn_voltage) {
    if(ups_state < ups_warn_state) {
      ups_state = ups_warn_state;
      should_shutdown |= shutdown_cause_bat_voltage;
    }
  } else if (bat_voltage <= restart_voltage) {
    if (ups_state == ups_unclear_state && seconds > timeout) {
      // the RPi is not running, even after the timeout, so we assume that it
      // shut down, this means we come from a WARN_STATE or SHUTDOWN_STATE
      ups_state = ups_warn_state;
    }
  } else { // we are at a safe voltage

    switch (ups_state) {
      case ups_shutdown_state:
        ups_state = ups_shutdown_to_running;
        break;
      case ups_warn_to_shutdown:
        ups_state = ups_shutdown_to_running;
        break;
      case ups_warn_state:
        ups_state = ups_warn_to_running;
        break;
      case ups_unclear_state:
        ups_state = ups_running_state;
        break;
    }
  }
}

/*
   Act on the current state
 */
void act_on_state_change() {
  // This is placed before the general check of all stages as to
  // not duplicate the code of the shutdown_state
  if (ups_state == ups_warn_to_shutdown) {
    // immediately turn off the system if force_shutdown is set
    if (primed == 1) {
      if (force_shutdown != 0) {
//        ups_off();
      }
    }
    ups_state = ups_shutdown_state;
  }

  if (ups_state == ups_shutdown_state) {
//    ledOff_buttonOff();
  } else if (ups_state == ups_warn_state) {
    // The RPi has been warned using the should_shutdown variable
    // we simply let it shutdown even if it does not set SL_INITIATED

//    reset_counter_Safe();
  } else if (ups_state == ups_shutdown_to_running) {
    // we have recovered from a shutdown and are now at a safe voltage
    if (primed == 1) {
//      ups_on();
    }
//    reset_counter_Safe();
    ups_state = ups_running_state;
    should_shutdown = shutdown_cause_none;
  } else if (ups_state == ups_warn_to_running) {
    // we have recovered from a warn state and are now at a safe voltage
    // we switch to State::running_state and let that state (below) handle
    // the restart
    ups_state = ups_running_state;
    should_shutdown = shutdown_cause_none;
  } else if (ups_state == ups_unclear_state) {
    // we do nothing and wait until either a timeout occurs, the voltage
    // drops to warn_voltage or is higher than restart_voltage (see handle_state())
  }

  if (ups_state == ups_running_state) {
    _Bool should_restart = false;

    if(vext_off_is_shutdown) {
      should_restart = ext_voltage < MIN_POWER_LEVEL;
    } else {
        should_restart = seconds > timeout;
    }

    if (should_restart) {
      if (primed == 1) {
        // RPi has not accessed the I2C interface for more than timeout seconds.
        // We restart it. Signal restart by blinking ten times
//        blink_led(10, BLINK_TIME / 2);
//        restart_raspberry();
      }

//      reset_counter_Safe();
    }
  }
}

void handle_state() {
  // Turn the LED on
  if (ups_state <= ups_warn_state) {
    if (primed == 1 || (seconds < timeout) ) {
      // start the regular blink if either primed is set or we are not yet in a timeout.
//      ledOn_buttonOff();
    }
  }

  // change the state depending on the current battery voltage
  voltage_dependent_state_change();

  // If the button has been pressed or the bat_voltage is lower than the warn voltage
  // we blink the LED 5 times to signal that the RPi should shut down, if it has not
  // already signalled that it is doing so
  if (ups_state <= ups_warn_state) {
    // we first check whether the Raspberry is already in the shutdown process
    if(!(should_shutdown & shutdown_cause_rpi_initiated)) {
      if (should_shutdown > shutdown_cause_rpi_initiated && (seconds < timeout)) {
        // RPi should take action, possibly shut down. Signal by blinking 5 times
//        blink_led(5, BLINK_TIME);
      }
    }
  }

  act_on_state_change();

  // Turn LED off
  if (ups_state <= ups_warn_to_shutdown) {
    // allow the button functionality as long as possible and even if not primed
//    ledOff_buttonOn();
  }
}


/*
   When we get an I2C communication then this might change our state (because now we
   know the RPi is alive). Called only during an interrupt.
 */
void i2c_triggered_state_change() {
  // If we are in an unclear state, then a communication from the RPi moves us to running state
  if (ups_state == ups_unclear_state) {
    ups_state = ups_running_state;
  }
}
