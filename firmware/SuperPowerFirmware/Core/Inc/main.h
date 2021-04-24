/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/*
 * Our version number - used by the daemon to ensure that the major number is equal between firmware and daemon
 */
enum VersionNumber {
	MAJOR = 1, MINOR = 0, PATCH = 7,
};

/*
   We create an init value from the lower bits of the minor version number (BITS_FOR_MINOR)
   and use the remaining bits for the lower bits of the major number (BITS_FOR_MAJOR).
   This way, whenever the minor or major version changes, the eeprom will be initialized again to
   ensure that everything works without problems.
*/
static const uint8_t BITS_FOR_MINOR    = 5;
static const uint8_t BITS_FOR_MAJOR    = CHAR_BIT - BITS_FOR_MINOR;
static const uint8_t MINOR_PART        = MINOR & ((1<<BITS_FOR_MINOR)-1);
static const uint8_t MAJOR_PART        = (MAJOR & BITS_FOR_MAJOR) << BITS_FOR_MINOR;
static const uint8_t BACKUP_INIT_VALUE = MAJOR_PART | MINOR_PART;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

void jumpToBootloader();
/*
#define debug_print(fmt, ...) \
	    do { if (DEBUG) fprintf(stdout, fmt, ##__VA_ARGS__); } while (0)
 */

#define debug_print(fmt, ...) \
	do { if (DEBUG) fprintf(stdout, "%s:%d:%s(): " fmt, __FILE__, \
				__LINE__, __func__, ##__VA_ARGS__); } while (0)
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define nOP_Enable_Pin GPIO_PIN_0
#define nOP_Enable_GPIO_Port GPIOC
#define USART_TX_LED0_Pin GPIO_PIN_2
#define USART_TX_LED0_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define User_Button_Pin GPIO_PIN_10
#define User_Button_GPIO_Port GPIOB
#define User_Button_EXTI_IRQn EXTI15_10_IRQn
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
