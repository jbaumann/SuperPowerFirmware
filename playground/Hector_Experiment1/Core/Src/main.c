/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "DS3231.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

__IO uint8_t transferDirection = 0;
#define SLAVE_BUFFER_SIZE 32
uint8_t slaveReceiveBuffer[SLAVE_BUFFER_SIZE];
uint8_t* slaveTransmitBuffer;
__IO uint16_t sizeOfData;

RTC_TimeTypeDef time;
RTC_DateTypeDef date;
char timebuffer[] = {1,2,3,4,5,6,7,8,0,0,0};
uint16_t addr = 0;
//uint8_t cmdSize = 0;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void jumpToBootloader(){
	void (*sysMemBootJump)(void);
	volatile uint32_t addr = 0x1FFF0000;
	HAL_RCC_DeInit();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
	__disable_irq();
	__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
	sysMemBootJump = (void(*)(void))(*((uint32_t *)(addr + 4)));
	__set_MSP(*(uint32_t *)addr);
	sysMemBootJump();
}


typedef struct {
	uint16_t addres;
	uint8_t cmd_size;
	uint8_t data[SLAVE_BUFFER_SIZE + 1];
}i2c_cmd;

i2c_cmd test;

char* getRegister(uint8_t reg){
	//ds3231 rt;
	//rt.seconds.seconds = 0;
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

uint8_t ds3231_cmd_decode(i2c_cmd msg){
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	//ds3231 data;
	uint8_t size = msg.cmd_size;
	//memcpy(data.array, msg.data, size);
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



void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode){
	UNUSED(hi2c);
	switch(TransferDirection){
	case I2C_DIRECTION_TRANSMIT:
		addr = AddrMatchCode;
		test.addres = addr;
		HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, slaveReceiveBuffer, SLAVE_BUFFER_SIZE, I2C_FIRST_FRAME);
		break;
	case I2C_DIRECTION_RECEIVE:
		slaveTransmitBuffer = (uint8_t*)getRegister(slaveReceiveBuffer[0]);
		sizeOfData = 8;
		HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, slaveTransmitBuffer, sizeOfData, I2C_LAST_FRAME);

		break;
	default:
		break;
	}
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){
	HAL_I2C_EnableListen_IT(&hi2c1); // Restart
		test.cmd_size = (uint8_t)(SLAVE_BUFFER_SIZE + 1 - hi2c->XferCount);
		if(test.cmd_size > 0 && test.cmd_size <= SLAVE_BUFFER_SIZE){
			memcpy(test.data, slaveReceiveBuffer, test.cmd_size);
			ds3231_cmd_decode(test);
			memset(slaveReceiveBuffer, 0, SLAVE_BUFFER_SIZE);
		}

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	//jumpToBootloader();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_I2C_EnableListen_IT(&hi2c1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	 /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	 /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	 UNUSED(file);
	 UNUSED(line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
