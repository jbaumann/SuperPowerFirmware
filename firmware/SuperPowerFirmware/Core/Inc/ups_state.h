/*
 * state.h
 *
 *  Created on: 17 Jan 2021
 *      Author: jbaumann
 */

#ifndef INC_UPS_STATE_H_
#define INC_UPS_STATE_H_

#define bit(b) (1UL << (b))

/*
   Values modelling the different states the system can be in
*/
enum UPS_State {
  ups_running_state            = 0,       // the system is running normally
  ups_unclear_state            = bit(0),  // the system has been reset and is unsure about its state
  ups_warn_to_running          = bit(1),  // the system transitions from warn state to running state
  ups_shutdown_to_running      = bit(2),  // the system transitions from shutdown state to running state
  ups_warn_state               = bit(3),  // the system is in the warn state
  ups_warn_to_shutdown         = bit(4),  // the system transitions from warn state to shutdown state
  ups_shutdown_state           = bit(5),  // the system is in the shutdown state
};

enum Shutdown_Cause {
  shutdown_cause_none          = 0,
  shutdown_cause_reserved_0    = bit(0),
  shutdown_cause_rpi_initiated = bit(1),
  shutdown_cause_ext_voltage   = bit(2),
  shutdown_cause_button        = bit(3),
  shutdown_cause_reserved_4    = bit(4),
  // the following levels definitely trigger a shutdown
  shutdown_cause_reserved_5    = bit(5),
  shutdown_cause_reserved_6    = bit(6),
  shutdown_cause_bat_voltage   = bit(7),
};

#endif /* INC_UPS_STATE_H_ */
