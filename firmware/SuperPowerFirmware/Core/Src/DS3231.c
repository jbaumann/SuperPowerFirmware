/*
 * DS3231.c
 *
 *  Created on: Jan 11, 2021
 *      Author: hector
 */
#include "DS3231.h"
#include "rtc.h"
#include "i2c_register.h"
#include "stdint.h"

RTC_TimeTypeDef time;
RTC_DateTypeDef date;
char timebuffer[] = {1,2,3,4,5,6,7,8,0,0,0};

uint8_t ds3231_cmd_decode(i2c_cmd msg){
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	uint8_t size = msg.cmd_size;
	uint8_t i = 1;
	switch (msg.data[0]) {
	case 0:
		if(size-- > 0){
			time.SecondFraction = 0;
			time.Seconds = msg.data[i];
			i++;
		}else{
			break;
		}
	case 1:
		if(size-- > 0){
			time.Minutes = msg.data[i];
			i++;
		}else{
			break;
		}
	case 2:
		if(size-- > 0){
			time.Hours = msg.data[i];
			i++;
		}else{
			break;
		}
	case 3:
		if(size-- > 0){
			date.WeekDay = msg.data[i];
			i++;
		}else{
			break;
		}
	case 4:
		if(size-- > 0){
			date.Date= msg.data[i];
			i++;
		}else{
			break;
		}
	case 5:
		if(size-- > 0){
			date.Month= msg.data[i];
			i++;
		}else{
			break;
		}
	case 6:
		if(size-- > 0){
			date.Year= msg.data[i];
			i++;
		}else{
			break;
		}
	default:
		if(size < 0)
		return HAL_ERROR;
	}
	HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BCD);
	HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BCD);
	return HAL_OK;
}

char* getRegister(uint8_t reg){
	char* ptr = NULL;

	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BCD);

	timebuffer[0] = time.Seconds;
	timebuffer[1] = time.Minutes;
	timebuffer[2] = time.Hours;
	timebuffer[3] = date.WeekDay;
	timebuffer[4] = date.Date;
	timebuffer[5] = date.Month;
	timebuffer[6] = date.Year;

	if(reg <= 6){
	  ptr = &timebuffer[(uint8_t)reg];
	}
	return ptr;
}
