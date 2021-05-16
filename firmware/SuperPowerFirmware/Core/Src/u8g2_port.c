/*
 * u8g2_port.c
 *
 *  Created on: Apr 24, 2021
 *      Author: hector
 */
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include "u8g2.h"
#include "i2c.h"

extern uint8_t *i2c_buffer;

uint8_t u8x8_stm32_gpio_and_delay(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr){
  switch (msg)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
    	osDelay(pdMS_TO_TICKS(1));
      //vTaskDelay(pdMS_TO_TICKS(1));
      break;
    case U8X8_MSG_DELAY_MILLI:
      osDelay(pdMS_TO_TICKS(arg_int));
      break;
    case U8X8_MSG_GPIO_DC:
      break;
    case U8X8_MSG_GPIO_RESET:
      break;
    }
  return 1;
}


uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  //  static uint8_t buffer[32];  /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
  static uint8_t buf_idx;
  uint8_t *data;
  uint8_t res = 1;

  switch(msg)
    {
    case U8X8_MSG_BYTE_SEND:
      data = (uint8_t *)arg_ptr;
      while( arg_int > 0 )
	{
	  i2c_buffer[buf_idx++] = *data;
	  data++;
	  arg_int--;
	}
      break;
    case U8X8_MSG_BYTE_INIT:
      /* add your custom code to init i2c subsystem */
      break;
    case U8X8_MSG_BYTE_SET_DC:
      /* ignored for i2c */
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      buf_idx = 0;
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      res = HAL_I2C_Master_Transmit(&hi2c3, u8x8_GetI2CAddress(u8x8) << 1, i2c_buffer, buf_idx, 100);
      res = (res == HAL_OK) ? 1 : 0;
      break;
    default:
      return 0;
    }
  return res;
}
