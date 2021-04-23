/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 0x1F;
  hrtc.Init.SynchPrediv = 0x03FF;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /*
   * The values for AsyncHPrediv and SynchPrediv have been taken from
   * https://stm32f4-discovery.net/2014/07/library-19-use-internal-rtc-on-stm32f4xx-devices/
   */

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
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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

uint8_t rtc_get_RTC_register(uint8_t reg, uint8_t tdata[]){
	//char* ptr = NULL;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BCD);

	tdata[0] = time.Seconds;
	tdata[1] = time.Minutes;
	tdata[2] = time.Hours;
	tdata[3] = date.WeekDay;
	tdata[4] = date.Date;
	tdata[5] = date.Month;
	tdata[6] = date.Year;
	tdata[7] = 0;

	return 8;
}


void rtc_msg_decode(uint8_t cmd_size, uint8_t data[]){

	// TODO Refactor

	uint8_t aux = cmd_size;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BCD);
	switch(data[0]){
	case 0:
		if(aux-- > 0){
			time.Seconds = data[1];
			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 1:
		if(aux-- > 0){
			time.Minutes = data[2];
			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 2:
		if(aux-- > 0){
			time.Hours = data[3];
			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 3:
		if(aux-- > 0){
			date.WeekDay = data[4];
			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 4:
		if(aux-- > 0){
			date.Date = data[5];
			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 5:
		if(aux-- > 0){
			date.Month = data[6];
			__attribute__ ((fallthrough));
		}else{
			break;
		}
	case 6:
		if(aux-- > 0){
			date.Year = data[7];
			__attribute__ ((fallthrough));
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
	if( (0xFF & first_value) == BACKUP_INIT_VALUE) {

		config_registers.reg[0] = first_value;
		for(int i = 1; i < (sizeof(Config_Registers) / 4); i++) {
			config_registers.reg[i] = rtc_read_backup_reg(i);
		}
	}
}


/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
