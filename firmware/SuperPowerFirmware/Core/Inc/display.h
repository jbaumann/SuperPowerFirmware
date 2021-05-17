/*
 * display.h
 *
 *  Created on: May 15, 2021
 *      Author: jbaumann
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

void init_display(uint8_t display_type);
void deinit_display();
void update_display(uint8_t display_type);

extern uint8_t num_display_definitions;


#endif /* INC_DISPLAY_H_ */
