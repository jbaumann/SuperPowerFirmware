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

/*
 * The Step definitions describe a single LED pattern
 */
LED_Step _blink_short[] = {
		{
		.color     = 0xFFFFFF,
		.intensity = 0xFF,
		.effect    = 0,
		.ontime    = 100,
		.offtime   = 100,
		.repeat    = 1,
		},
};

LED_Step _blink_medium[] = {
		{
		.color     = 0xFFFFFF,
		.intensity = 0xFF,
		.effect    = 0,
		.ontime    = 500,
		.offtime   = 500,
		.repeat    = 1,
		},
};

LED_Step _blink_long[] = {
		{
		.color     = 0xFFFFFF,
		.intensity = 0xFF,
		.effect    = 0,
		.ontime    = 100,
		.offtime   = 900,
		.repeat = 1,
		},
};

LED_Step _blink_SOS[] = {
		{
		.color     = 0xFFFFFF,
		.intensity = 0xFF,
		.effect    = 0,
		.ontime    = 200,
		.offtime   = 200,
		.repeat    = 3,
		},
		{
		.color     = 0xFFFFFF,
		.intensity = 0xFF,
		.effect    = 0,
		.ontime    = 400,
		.offtime   = 400,
		.repeat    = 3,
		},
		{
		.color     = 0xFFFFFF,
		.intensity = 0xFF,
		.effect    = 0,
		.ontime    = 200,
		.offtime   = 200,
		.repeat    = 3,
		},
};

/*
 * The full messages describe a number of patterns over which
 * a number of iterations should be made
 */
LED_QueueMsg_t blink_5 = {
		.iterations = 5,
		.number_steps = 1,
		.steps = &_blink_short,
};

LED_QueueMsg_t background_off = {
		.iterations = 0,
};


LED_QueueMsg_t blink_second_background = {
		.iterations = 255,
		.number_steps = 1,
		.steps = &_blink_long,
};

LED_QueueMsg_t blink_SOS = {
		.iterations = 3,
		.number_steps = 3,
		.steps = &_blink_SOS,
		.final_delay = 400,
};
