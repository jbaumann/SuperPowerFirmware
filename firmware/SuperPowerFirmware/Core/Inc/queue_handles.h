/*
 * queue_handles.h
 *
 *  Created on: Dec 11, 2020
 *      Author: jbaumann
 */

#ifndef INC_QUEUE_HANDLES_H_
#define INC_QUEUE_HANDLES_H_

// TODO find a better name for the include file

osMessageQueueId_t I2C_R_QueueHandle;
osMessageQueueId_t LED_R_QueueHandle;


typedef struct {                                // object data type
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
	LED_Step steps[];

} LED_QueueMsg_t;


LED_QueueMsg_t blink_5;
LED_QueueMsg_t background_off;
LED_QueueMsg_t blink_second_background;

#endif /* INC_QUEUE_HANDLES_H_ */
