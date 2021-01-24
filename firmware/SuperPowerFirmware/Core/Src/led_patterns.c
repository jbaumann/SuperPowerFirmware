/*
 * led_patterns.c
 *
 *  Created on: Jan 21, 2021
 *      Author: jbaumann
 */

// TODO extract the LED_Pattern definition
#include "main.h"
#include "cmsis_os.h"
#include "queue_handles.h"

LED_QueueMsg_t blink_5 = {
		.iterations = 1,
		.steps = {
				{
						.color = 0xFFFFFF,
						.intensity = 0xFF,
						.effect = 0,
						.ontime = 100,
						.offtime = 100,
						.repeat = 5,
				},
		},
};

LED_QueueMsg_t background_off = {
		.iterations = 0,
};

LED_QueueMsg_t blink_second_background = {
		.iterations = 255,
		.steps = {
				{
						.color = 0xFFFFFF,
						.intensity = 0xFF,
						.effect = 0,
						.ontime = 100,
						.offtime = 900,
				},
		},
};

