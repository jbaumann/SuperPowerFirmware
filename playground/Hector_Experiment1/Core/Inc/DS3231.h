/*
 * DS3231.h
 *
 *  Created on: 8 nov. 2020
 *      Author: hector
 */

#ifndef DS3231_H_
#define DS3231_H_
#include <stdint.h>
typedef struct {
	uint8_t arr[6];
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
	union{
		uint8_t byte;
		struct {
			uint8_t seconds: 4;
			uint8_t tSeconds: 3;
			uint8_t a1m1: 1;
		};
	}alarm1Seconds;
	union{
		uint8_t byte;
		struct {
			uint8_t minutes: 4;
			uint8_t tMinutes: 3;
			uint8_t a1m2: 1;
		};
	}alarm1Minutes;
	union {
		uint8_t byte;
		struct {
			uint8_t hour: 4;
			uint8_t tHour: 1;
			uint8_t am_pm_20Hour: 1;
			uint8_t format_12_24: 1;
			uint8_t a1m3: 1;
		};
	}alarm1Hours;
	union {
		uint8_t byte;
		struct {
			uint8_t day_date: 4;
			uint8_t tDate: 2;
			uint8_t dy_dt: 1;
			uint8_t a1m4: 1;
		};
	}dayDate;
}ds3231;

#define DS3231_SECONDS			0
#define DS3231_MINUTES			1
#define DS3231_HOURS				2
#define DS3231_DAY				3
#define DS3231_DATE				4
#define DS3231_MONTH_CENTURY		5
#define DS3231_YEAR				6

#endif /* DS3231_H_ */
