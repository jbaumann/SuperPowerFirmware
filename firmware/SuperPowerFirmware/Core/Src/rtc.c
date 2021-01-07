/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include "DS3231.h"
#include "i2c_register.h"

RTC_TimeTypeDef time;
RTC_DateTypeDef date;
char timebuffer[] = {0,0,0,0,0,0,0,0,0,0,0}; // is the initialization necessary?
//uint16_t addr = 0;

I2C_Cmd test;

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

char* rtc_get_RTC_register(uint8_t reg){
	// TODO Review lines
	//ds3231 rt;
	//rt.seconds.seconds = 0;

	//TODO Refactor
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


void rtc_msg_decode(I2C_Cmd msg){

	// TODO Refactor

	uint8_t aux = msg.cmd_size;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BCD);

	switch(msg.data[0]){
	case 0:
		if(aux-- > 0){
			time.Seconds = msg.data[0];
//			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 1:
		if(aux-- > 0){
			time.Minutes = msg.data[1];
//			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 2:
		if(aux-- > 0){
			time.Hours = msg.data[2];
//			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 3:
		if(aux-- > 0){
			date.WeekDay = msg.data[3];
//			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 4:
		if(aux-- > 0){
			date.Date = msg.data[4];
//			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 5:
		if(aux-- > 0){
			date.Month = msg.data[5];
//			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 6:
		if(aux-- > 0){
			date.Year = msg.data[6];
//			__attribute__ ((fallthrough));
		}else{
			break;
		}
	default:
		break;
	}
	HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BCD);
	HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BCD);
}


/*
 * Basic read and write from backup register.Based on
 * https://os.mbed.com/users/gregeric/notebook/using-stm32-rtc-backup-registers/
 */


uint32_t rtc_read_backup_reg(uint32_t backup_register) {
	if(backup_register > 19) {
		return 0xFFFFFFFF;
	}
    return HAL_RTCEx_BKUPRead(&hrtc, backup_register);
}

void rtc_write_backup_reg(uint32_t backup_register, uint32_t data) {
	if(backup_register > 19) {
		return;
	}
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, backup_register, data);
    HAL_PWR_DisableBkUpAccess();
}

/*
 * The following function writes the current config data to the
 * backup registers in the RTC chip
 */
void backup_registers() {
	for(int i = 0; i < (sizeof(Config_Registers) / 4); i++) {
		rtc_write_backup_reg(i, config_registers.reg[i]);
	}
}

/*
 * The following function reads the current config data from the
 * backup registers in the RTC chip if the version number matches
 */
void restore_registers() {
	// read the first register
	uint32_t first_value;
	first_value = rtc_read_backup_reg(0);
	// check the version number first
	if( (0xFF &first_value) == BACKUP_INIT_VALUE) {

		config_registers.reg[0] = first_value;
		for(int i = 1; i < (sizeof(Config_Registers) / 4); i++) {
			config_registers.reg[i] = rtc_read_backup_reg(i);
		}
	}
}


/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
