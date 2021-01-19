/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
#include "i2c.h"

/* USER CODE BEGIN 0 */
/*
 * Our own includes have to be placed below the user code line as well
 */
#include <stdbool.h>
#include <string.h>

#include "rtc.h"
#include "crc_8bit.h"
#include "cmsis_os.h"
#include "queue_handles.h"
#include "ch_bq25895.h"



/*
 * Communication data for I2C
 */
uint8_t i2c1_buffer[I2C_BUFFER_SIZE + 1]; // worst case size including the crc
_Bool i2c_primary_address = true;
enum I2C_Register i2c_register;
_Bool i2c_in_progress = false;

// TODO Refactor
uint8_t slaveReceiveBuffer[32];
uint8_t *slaveTransmitBuffer;
__IO uint16_t sizeOfData;


/*
 * We use 24 bit for the prog_version, this should be enough.
 */
uint32_t prog_version = (MAJOR << 16) | (MINOR << 8) | PATCH;

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 128;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_ENABLE;
  hi2c1.Init.OwnAddress2 = 130;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */
  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

    /* I2C1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/*
 * Determine the correct struct for the register, check
 * the bounds and if everything is ok, copy the value
 * to the buffer
 *
 *  register_number the register value sent by the RPi
 */
void i2c_writeRegisterToBuffer(enum I2C_Register register_number) {
	// identify the addressed struct and copy the value
	if (register_number < (enum I2C_Register)STATUS_8BIT_OFFSET) {

		// access to the CONFIG_8BIT struct
		uint8_t reg = register_number - CONFIG_8BIT_OFFSET;
		if(reg < i2c_config_reg_8bit_size) {
			i2c1_buffer[0] = i2c_config_register_8bit->reg[reg];
		}
	} else if (register_number < (enum I2C_Register)CONFIG_16BIT_OFFSET) {

		// access to the STATUS_8BIT struct
		uint8_t reg = register_number - STATUS_8BIT_OFFSET;
		if(reg < i2c_status_reg_8bit_size) {
			i2c1_buffer[0] = i2c_status_register_8bit->reg[reg];
		}
	} else if (register_number < (enum I2C_Register)STATUS_16BIT_OFFSET) {

		// access to the CONFIG_16BIT struct
		uint8_t reg = register_number - CONFIG_16BIT_OFFSET;
		if(reg < i2c_config_reg_16bit_size) {
			uint16_t *val = (uint16_t*) i2c1_buffer;
			val[0] = i2c_config_register_16bit->reg[reg];
		}
	} else if (register_number < (enum I2C_Register)SPECIAL_16BIT_OFFSET) {

		// access to the STATUS_16BIT struct
		uint8_t reg = register_number - STATUS_16BIT_OFFSET;
		if(reg < i2c_status_reg_16bit_size) {
			uint16_t *val = (uint16_t*) i2c1_buffer;
			val[0] = i2c_status_register_16bit->reg[reg];
		}
	} else {
		// access to the SPECIAL_16BIT struct

		// Special register requesting the version number
		if(register_number == i2creg_version) {
			uint32_t *val = (uint32_t*) i2c1_buffer;
			val[0] = prog_version;
		}
	}
}

/*
 * Determine the correct struct for the register, check
 * the bounds and if everything is ok, copy the value
 * from the buffer to the register
 *
 *  register_number the register value sent by the RPi
 */
void i2c_writeBufferToRegister(uint8_t register_number) {
	uint8_t reg_has_changed = false;

	// identify the addressed struct and copy the value
	if (register_number < STATUS_8BIT_OFFSET) {

		// access to the CONFIG_8BIT struct
		uint8_t reg = register_number - CONFIG_8BIT_OFFSET;
		if(reg < i2c_config_reg_8bit_size) {
			i2c_config_register_8bit->reg[reg] = i2c1_buffer[0];
			reg_has_changed = true;
		}
	} else if (register_number < CONFIG_16BIT_OFFSET) {
		/* the RPi does not set values in the STATUS_8BIT struct */
	} else if (register_number < STATUS_16BIT_OFFSET) {

		// access to the CONFIG_16BIT struct
		uint8_t reg = register_number - CONFIG_16BIT_OFFSET;
		if(reg < i2c_config_reg_16bit_size) {
			uint16_t *val = (uint16_t*) (i2c1_buffer);
			i2c_config_register_16bit->reg[reg] = val[0];

			I2C_QueueMsg_t msg;
			msg.id = register_number;
			msg.big_val = val[0];

			osMessageQueuePut(I2C_R_QueueHandle, &msg, 0, 0);

			reg_has_changed = true;
		}
	} else if (register_number < (enum I2C_Register)SPECIAL_16BIT_OFFSET) {
		/* the RPi does not set values in the STATUS_16BIT struct */

	} else {
		// access to the SPECIAL_16BIT struct
	}
	if(reg_has_changed) {
		backup_registers();
	}
}

/*
 * Determine the correct transfer size depending on the current value
 * of the i2c_register
 */
uint8_t i2c_calc_transfer_size( ) {
	uint8_t len = 1 + 1; // 8bit by default + crc
	if (i2c_register >= (enum I2C_Register) SPECIAL_16BIT_OFFSET) {
		switch (i2c_register) {
		case i2creg_version:
			// 3 byte + crc
			len = 3 + 1;
			break;
		case i2creg_write_to_eeprom:
			// 1 byte + crc
			len = 1 + 1;
			break;
		default:
			break;
		}
	} else if (i2c_register >= (enum I2C_Register) CONFIG_16BIT_OFFSET) {
		// We have a 16bit register + crc
		len = 2 + 1;
	}
	return len;
}

/*
 * We use the primary address for the UPS and the secondary address for
 * the RTC
 */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection,
		uint16_t AddrMatchCode) {

	i2c_primary_address = (hi2c->Init.OwnAddress1 == AddrMatchCode);

	if (i2c_primary_address) {
		// the UPS is accessed

		uint8_t len;
		switch (TransferDirection) {
		case I2C_DIRECTION_TRANSMIT:
			// we receive the register and later its contents in
			// HAL_I2C_SlaveRxCpltCallback(). We use 8-bit registers,
			// thus the length is 1.
			HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, i2c1_buffer, 1, I2C_FIRST_FRAME); // read the register
			break;
		case I2C_DIRECTION_RECEIVE:
			// we copy the data to the buffer and send it to the RPi
			i2c_register = hi2c->Instance->DR;
			len = i2c_calc_transfer_size();
			i2c_writeRegisterToBuffer(i2c_register);
			i2c1_buffer[len - 1] = calcCRC(i2c_register, i2c1_buffer, len - 1);
			HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, i2c1_buffer, len, I2C_LAST_FRAME);
			break;
		default:
			break;
		}
	} else {
		// the RTC is accessed

		// TODO Refactor
		switch(TransferDirection){
		case I2C_DIRECTION_TRANSMIT:
			//addr = AddrMatchCode;
			test.address = AddrMatchCode;
			HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, slaveReceiveBuffer, 32, I2C_FIRST_FRAME);
			break;
		case I2C_DIRECTION_RECEIVE:
			switch(AddrMatchCode >> 1){
			case 20:
				slaveTransmitBuffer = (uint8_t*)rtc_get_RTC_register(slaveReceiveBuffer[0]);
				sizeOfData = 6;
				HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, slaveTransmitBuffer, sizeOfData, I2C_LAST_FRAME);
				break;
			case 0x21:
				break;
			default:
				break;
			}
			break;
			default:
				break;
		}
	}
}

/*
 * This callback is called when the RPi wants to send data
 * to us. The first call with i2c_in_progress == false
 * receives the i2c register, sets i2c_in_progress to true
 * and starts a second call to receive the actual data.
 */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {

	if(i2c_primary_address) {
		// UPS code
		if(i2c_in_progress) {
			// This is the second call triggered by the
			// "else"-path of this statement, in which we
			// have extracted the register number.
			// We now have received the data, check the CRC
			// and save it to the correct register.
			i2c_in_progress = false;

			uint8_t len = hi2c->XferSize - 1;
			// check crc
			uint8_t crc = calcCRC(i2c_register, i2c1_buffer, len);

			// copy value if crc is correct
			if(crc == i2c1_buffer[len]) {
				i2c_writeBufferToRegister(i2c_register);
			}
		}
		else {
			// This is the first call, triggered by
			// HAL_I2C_AddrCallback(). We extract the
			// i2c_register to use it in the next round.
			i2c_in_progress = true;
			i2c_register = i2c1_buffer[0];
			uint8_t len	= i2c_calc_transfer_size();
			HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, i2c1_buffer, len, I2C_LAST_FRAME);
		}
	} else {
		// RTC code
	}


	HAL_I2C_EnableListen_IT(&hi2c1);

}

/*
 * This callback is called when the data from the charger
 * has been successfully received. We copy the relevant
 * information and turn the listen mode back on
 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	i2c_status_register_8bit->val.charger_status = i2c_ch_BQ25895_register.val.ch_status;
	uint16_t batv = ch_convert_batv(i2c_ch_BQ25895_register.val.ch_bat_voltage);
	i2c_status_register_16bit->val.bat_voltage = batv;
	uint16_t vbus_v = ch_convert_vbus(i2c_ch_BQ25895_register.val.ch_vbus_voltage);
	i2c_status_register_16bit->val.vbus_voltage = vbus_v;
	uint16_t ch_current = ch_convert_charge_current(i2c_ch_BQ25895_register.val.ch_charge_current);
	i2c_status_register_16bit->val.charge_current = ch_current;

	// Turn the slave functionality on again
	// We don't need to check the return value because it
	// can only be HAL_OK or HAL_BUSY, either way I2C is
	// listening after this call
	HAL_I2C_EnableListen_IT(&hi2c1);

}

/*
 * We restart the I2C listening mode
 */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){
	// TODO Check whether address can be determined from hi2c
	if(i2c_primary_address) {
		i2c_in_progress = false;
	} else {
		// RTC Code
		if(hi2c->XferCount == 32){
			test.cmd_size = 0;
		}else{
			test.cmd_size = (uint8_t)(32 - hi2c->XferCount - 1);
			memcpy(test.data, slaveReceiveBuffer+1, test.cmd_size);
		}
		memset(slaveReceiveBuffer, 0, 31);
		if(test.cmd_size > 0){
			rtc_msg_decode(test);
		}
	}
	HAL_I2C_EnableListen_IT(&hi2c1); // Restart
}

/*
 * The following callbacks simply set the i2c_in_progress
 * value back to false in case something goes wrong.
 */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
	i2c_in_progress = false;
	HAL_I2C_EnableListen_IT(&hi2c1);
}
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
	i2c_in_progress = false;
	HAL_I2C_EnableListen_IT(&hi2c1);

}
void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c) {
	i2c_in_progress = false;
	HAL_I2C_EnableListen_IT(&hi2c1);
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
