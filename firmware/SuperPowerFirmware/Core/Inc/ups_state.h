/*
 * state.h
 *
 *  Created on: 17 Jan 2021
 *      Author: jbaumann
 */

#ifndef INC_UPS_STATE_H_
#define INC_UPS_STATE_H_

static const uint16_t ups_update_interval    = 10000;  // update interval for the charger in milliseconds

#define bit(b) (1UL << (b))

void i2c_triggered_ups_state_change();
void handle_state() ;
void reset_timeout();
/*
   Values modelling the different states the system can be in
*/
enum UPS_State {
  ups_running_state                 = 0,       // the system is running normally
  ups_unclear_state                 = bit(0),  // the system has been reset and is unsure about its state
  ups_warn_to_running               = bit(1),  // the system transitions from warn state to running state
  ups_shutdown_to_running           = bit(2),  // the system transitions from shutdown state to running state
  ups_warn_state                    = bit(3),  // the system is in the warn state
  ups_warn_to_shutdown              = bit(4),  // the system transitions from warn state to shutdown state
  ups_shutdown_state                = bit(5),  // the system is in the shutdown state
};

/*
 * These values are used for the communication between UPS and RPi.
 * Bit 0-4 have informational character.
 * Bit 5-7 definitely trigger a shutdown
 */
enum Shutdown_Cause {
  shutdown_cause_none               = 0,
  shutdown_cause_i2c_has_been_reset = bit(0),
  shutdown_cause_rpi_initiated      = bit(1),
  shutdown_cause_ext_voltage        = bit(2),
  shutdown_cause_button             = bit(3),
  shutdown_cause_reserved_4         = bit(4),
  // the following levels definitely trigger a shutdown
  shutdown_cause_reserved_5         = bit(5),
  shutdown_cause_reserved_6         = bit(6),
  shutdown_cause_bat_voltage        = bit(7),
};

extern uint8_t ups_state_should_shutdown;

#endif /* INC_UPS_STATE_H_ */
