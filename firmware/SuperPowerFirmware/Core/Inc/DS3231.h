/*
 * DS3231.h
 *
 *  Created on: 8 nov. 2020
 *      Author: hector
 */

#ifndef DS3231_H_
#define DS3231_H_

#include "stdint.h"
#include "i2c_register.h"
#include "cmsis_os.h"


typedef struct {
	uint8_t array[6];
	union{
		uint8_t byte;
		struct {
			uint8_t seconds: 4;
			uint8_t tSeconds: 3;
		};
	}seconds;
	union{
			uint8_t byte;
			struct {
				uint8_t minutes: 4;
				uint8_t tMinutes: 3;
			};
		}minutes;
	union {
			uint8_t byte;
			struct {
				uint8_t hour: 4;
				uint8_t tHour: 1;
				uint8_t am_pm_20Hour: 1;
				uint8_t format_12_24: 1;
			};
		}hours;
	uint8_t day: 3;
	union {
		uint8_t byte;
		struct {
			uint8_t date: 4;
			uint8_t tDate: 2;
		};
	}date;
	union {
		uint8_t byte;
		struct {
			uint8_t month: 4;
			uint8_t tMonth: 1;
			uint8_t unused: 2;
			uint8_t century: 1;
		};
	}month;
	union {
		uint8_t byte;
		struct {
			uint8_t year: 4;
			uint8_t tYear: 4;
		};
	}year;

}ds3231;

extern osMessageQueueId_t RTC_R_QueueHandle;

uint8_t ds3231_cmd_decode(i2c_cmd msg);
char* getRegister(uint8_t reg);
#endif /* DS3231_H_ */
