/*
 * utility_functions.c
 *
 *  Created on: Nov 22, 2020
 *      Author: jbaumann
 */

#include "main.h"

/*
 * Programmatically jump into the bootloader to accept a new firmware
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
 * Implement debug output using SWO
 */
/*
//#ifdef DEBUG
int _write(int file, char *ptr, int len) {
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++) {
		ITM_SendChar(*ptr++);
	}
	return len;
}
//#endif // DEBUG
*/
