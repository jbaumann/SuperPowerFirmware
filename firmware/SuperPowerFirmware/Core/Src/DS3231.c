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
#include "cmsis_os.h"
#include "queue.h"
#include <string.h>

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
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
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

extern osMessageQueueId_t I2C_R_QueueHandle;

void RTC_Task(void *argument)
{
	i2c_cmd cmd;
	//i2c_cmd_dynamic response;
	//response.data = pvPortMalloc(10 * sizeof(uint8_t));
  for(;;)
  {
	 if(pdTRUE == xQueueReceive(RTC_R_QueueHandle, &cmd, 100)){
		 //cmd recevived by the task
		 if(cmd.cmd_size == 2){
			 memcpy(cmd.data, (uint8_t*)getRegister(cmd.data[1]),8);
			 //cmd.data = (uint8_t*)getRegister(cmd.data[1]);
			 cmd.cmd_size = 10;
			 xQueueSend(I2C_R_QueueHandle, &cmd,100);
			 //HAL_I2C_Slave_Seq_Transmit_DMA(&hi2c1, cmd.data, cmd.cmd_size, I2C_LAST_FRAME);
		 }else{
			 ds3231_cmd_decode(cmd);
		 }
	 }
  }
}
