/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "ups_state.h"
#include "i2c_register.h"
#include "cmsis_os.h"

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

// The time that is need to guarantee that the RPi is powered down
const int switch_recovery_delay = 500;

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(nOP_Enable_GPIO_Port, nOP_Enable_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = nOP_Enable_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(nOP_Enable_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = User_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(User_Button_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	// This is either the blue button on the Nucleo board
	// or the GPIO5/pin 29 on the RPi
	if (GPIO_Pin == B1_Pin) {
		// TODO Bootloader on button press remove?
		if (i2c_config_register_8bit->val.enable_bootloader != 0) {
			// TODO remove
			// jumpToBootloader();
		}
		else {
			// Raspberry signals shutdown?
		}
	}
	// The user button can be pressed to restart the RPi if turned off
	// or to signal an action if it is running
	if (GPIO_Pin == User_Button_Pin) {

		/*
		uint8_t should_restart = i2c_status_register_16bit->val.seconds
				> i2c_config_register_16bit->val.timeout;
		should_restart = should_restart && i2c_config_register_8bit->val.user_button_restart;

		if (should_restart && i2c_config_register_8bit->val.primed == 0) {
			i2c_config_register_8bit->val.primed = 2;
			// could be set during the shutdown while the timeout has not yet been exceeded. We reset it.
			ups_state_should_shutdown = shutdown_cause_none;
		} else {
			// signal the Raspberry that the button has been pressed.
			ups_state_should_shutdown |= shutdown_cause_button;
		}
		*/
		ups_state_should_shutdown |= shutdown_cause_button;
	}
}

void ups_off() {
	// set ups pin low
	HAL_GPIO_WritePin(nOP_Enable_GPIO_Port, nOP_Enable_Pin, GPIO_PIN_RESET);
}
void ups_on() {
	// set ups pin high
	HAL_GPIO_WritePin(nOP_Enable_GPIO_Port, nOP_Enable_Pin, GPIO_PIN_SET);
}

void restart_raspberry() {

	ups_state_should_shutdown = shutdown_cause_none;

	ups_off();

	osDelay(switch_recovery_delay); // wait for the switch circuit to revover

	ups_on();
}
/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
