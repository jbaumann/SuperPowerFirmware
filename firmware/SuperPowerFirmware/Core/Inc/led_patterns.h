/*
 * led_patterns.h
 *
 *  Created on: May 19, 2021
 *      Author: jbaumann
 */

#ifndef INC_LED_PATTERNS_H_
#define INC_LED_PATTERNS_H_

/*****************************************************************************
 * LED Queue Message definitions
 * Here are all definitions used to generate LED patterns signaling different
 * conditions to the user.
 *****************************************************************************/

typedef struct {
	union {
		uint8_t small_val;
		uint16_t big_val;
	};
	uint32_t id;
} I2C_QueueMsg_t;



typedef struct {
	uint32_t color;
	uint16_t ontime;
	uint16_t offtime;
	uint8_t intensity;
	uint8_t effect;
	uint8_t repeat;
} LED_Step;

typedef struct {
	uint8_t iterations;
	uint8_t number_steps;
	uint16_t final_delay;
	LED_Step (*steps)[];

} LED_QueueMsg_t;

extern LED_QueueMsg_t *rpi_shuts_down;
extern LED_QueueMsg_t *reboot_rpi;
extern LED_QueueMsg_t *background_off;
extern LED_QueueMsg_t *blink_second_background;
extern LED_QueueMsg_t *blink_SOS_3;

#endif /* INC_LED_PATTERNS_H_ */
