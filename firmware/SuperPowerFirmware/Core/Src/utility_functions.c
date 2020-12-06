/*
 * utility_functions.c
 *
 *  Created on: Nov 22, 2020
 *      Author: jbaumann
 */

#include <errno.h>

#include "main.h"
#include "usart.h"

/*
 * Programmatically jump into the bootloader to accept a new firmware
 * Version for the STM32F411
 * TODO Check whether this works with the STM32F412 as well
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
#endif // DEBUG

