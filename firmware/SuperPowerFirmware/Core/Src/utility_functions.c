/*
 * utility_functions.c
 *
 *  Created on: Nov 22, 2020
 *      Author: jbaumann, Hector Manuel
 */

#include <errno.h>

#include "main.h"
#include "usart.h"

/*
 * Programmatically jump into the bootloader to accept a new firmware
 * Version for the STM32F411
 * Taken from https://stm32f4-discovery.net/2017/04/tutorial-jump-system-memory-software-stm32/
 */
void jumpToBootloader() {
	void (*sysMemBootJump)(void);
	volatile uint32_t addr = 0x1FFF0000;
	HAL_RCC_DeInit();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
	__disable_irq();
	__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
	sysMemBootJump = (void (*)(void)) (*((uint32_t*) (addr + 4)));
	__set_MSP(*(uint32_t*) addr);
	sysMemBootJump();
}

/*
 * Implement debug output
 * On Windows, use Putty
 * On Unix-based systems you can use 'screen' or something else
 * Here an example for 'screen' on OSX:
 * screen /dev/cu.usbmodem14103 115200,cs8
 */
#ifdef DEBUG
//TODO beter config for FreeRTOS debugging
/*
 * Use debug_print to print debug messages, see main.h
 */
/*
 * Use SWO for the debug output
 * Currently not used, we are using the UART instead
 */
//int _write(int file, char *ptr, int len) {
//	int DataIdx;
//	for (DataIdx = 0; DataIdx < len; DataIdx++) {
//		ITM_SendChar(*ptr++);
//	}
//	return len;
//}

/*
 * Use the UART for debug messages
 */

int _write(int fd, char *ptr, int len) {
	HAL_StatusTypeDef hstatus;

	hstatus = HAL_UART_Transmit(&huart2, (uint8_t*) ptr, len, HAL_MAX_DELAY);
	if (hstatus == HAL_OK)
		return len;
	else
		return EIO;
}

#ifdef FREERTOS_TOTAL_RUNTIME_TIMER
/*
* by default the timer 10 is used to measure the run time percentage of each task
* TIM_HandleTypeDef htim10;
*/

TIM_HandleTypeDef htim10;
void initializeTimerForRunTimeStats(void)
{
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 65535;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 65535;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  __HAL_RCC_TIM10_CLK_ENABLE();
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
}

void configureTimerForRunTimeStats(void)
{

	HAL_TIM_Base_Start(&htim10);
}

unsigned long getRunTimeCounterValue(void)
{
	return __HAL_TIM_GetCounter(&htim10);
}
#else
__weak void configureTimerForRunTimeStats(void){}

__weak unsigned long getRunTimeCounterValue(void){ }
#endif // FREERTOS_TOTAL_RUNTIME_TIMER
#endif // DEBUG




