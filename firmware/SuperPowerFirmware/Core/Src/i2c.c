/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
#include "i2c.h"

/* USER CODE BEGIN 0 */
/*
 * Our own includes have to be placed below the user code line as well
 */
#include <stdbool.h>

#include "rtc.h"
#include "crc_8bit.h"
#include "cmsis_os.h"
#include "queue_handles.h"

/*
 * Initialization of the register structures
 */
I2C_Config_Register_8Bit i2c_config_register_8bit = {
	.val.primed                  =    0,   // 1 if the uC should control the system
	.val.force_shutdown          =    0,   // 1 if the uC should shutdown the UPS if the voltage is too low (hard shutdown)
	.val.enable_bootloader       =    0,   // 1 if the bootloader is enabled
};

I2C_Status_Register_8Bit i2c_status_register_8bit = {
	.val.should_shutdown         =  0x0,   // if != 0 contains the motivation for why the RPi should shutdown
	.val.charger_status          =  0x0,   // contains the contents of the status register 0x0E
};

I2C_Config_Register_16Bit i2c_config_register_16bit = {
	.val.timeout                 =  120,   // the timeout for the reset, should cover shutdown and reboot
	.val.restart_voltage         = 3900,   // the battery voltage at which the RPi will be started again
	.val.warn_voltage            = 3400,   // the battery voltage at which the RPi should should down
	.val.ups_shutdown_voltage    = 3200,   // the battery voltage at which a hard shutdown is executed
};

I2C_Status_Register_16Bit i2c_status_register_16bit = {
	.val.bat_voltage             =    0,   // the battery voltage, 3.3 should be low and 3.7 high voltage
	.val.charge_current          =    0,   // the battery charge current
	.val.vbus_voltage            =    0,   // the primary power voltage
	.val.ext_voltage             =    0,   // external voltage measured on PA0
	.val.seconds                 =    0,   // seconds since last i2c access
	.val.temperature             =    0,   // the on-chip temperature
};


/*
 * Communication buffers for I2C
 */
uint8_t i2c1_buffer[I2C_BUFFER_SIZE + 1]; // worst case size including the crc

_Bool i2c_in_progress = false;

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
			i2c1_buffer[0] = i2c_config_register_8bit.reg[reg];
		}
	} else if (register_number < (enum I2C_Register)CONFIG_16BIT_OFFSET) {

		// access to the STATUS_8BIT struct
		uint8_t reg = register_number - STATUS_8BIT_OFFSET;
		if(reg < i2c_status_reg_8bit_size) {
			i2c1_buffer[0] = i2c_status_register_8bit.reg[reg];
		}
	} else if (register_number < (enum I2C_Register)STATUS_16BIT_OFFSET) {

		// access to the CONFIG_16BIT struct
		uint8_t reg = register_number - CONFIG_16BIT_OFFSET;
		if(reg < i2c_config_reg_16bit_size) {
			uint16_t *val = (uint16_t*) i2c1_buffer;
			val[0] = i2c_config_register_16bit.reg[reg];
		}
	} else if (register_number < (enum I2C_Register)SPECIAL_16BIT_OFFSET) {

		// access to the STATUS_16BIT struct
		uint8_t reg = register_number - STATUS_16BIT_OFFSET;
		if(reg < i2c_status_reg_16bit_size) {
			uint16_t *val = (uint16_t*) i2c1_buffer;
			val[0] = i2c_status_register_16bit.reg[reg];
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
	// identify the addressed struct and copy the value
	if (register_number < STATUS_8BIT_OFFSET) {

		// access to the CONFIG_8BIT struct
		uint8_t reg = register_number - CONFIG_8BIT_OFFSET;
		if(reg < i2c_config_reg_8bit_size) {
			i2c_config_register_8bit.reg[reg] = i2c1_buffer[0];
		}
	} else if (register_number < CONFIG_16BIT_OFFSET) {
		/* the RPi does not set values in the STATUS_8BIT struct */
	} else if (register_number < STATUS_16BIT_OFFSET) {

		// access to the CONFIG_16BIT struct
		uint8_t reg = register_number - CONFIG_16BIT_OFFSET;
		if(reg < i2c_config_reg_16bit_size) {
			uint16_t *val = (uint16_t*) (i2c1_buffer + 1); // reg is [0]
			i2c_config_register_16bit.reg[reg] = val[0];

			I2C_QueueMsg_t msg;
			msg.id = register_number;
			msg.big_val = val[0];

			osMessageQueuePut(I2C_R_QueueHandle, &msg, 0, 0);

		}
	} else if (register_number < (enum I2C_Register)SPECIAL_16BIT_OFFSET) {
		/* the RPi does not set values in the STATUS_16BIT struct */

	} else {
		// access to the SPECIAL_16BIT struct
	}
}

/*
 * We use the primary address for the UPS and the secondary address for
 * the RTC
 */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection,
		uint16_t AddrMatchCode) {

	_Bool primary_address = (hi2c->Init.OwnAddress1 == AddrMatchCode);
	enum I2C_Register register_number = hi2c->Instance->DR;

	if (primary_address) {
		// the UPS is accessed

		uint8_t len = 1; // 8bit by default
		if(register_number >= (enum I2C_Register)CONFIG_16BIT_OFFSET) {
			// We have a 16bit register
			len = 2;
		}
		if(register_number >= (enum I2C_Register)SPECIAL_16BIT_OFFSET) {
			switch(register_number) {
			case i2creg_version:
				len = 3;
			default:
				break;
			}
		}
		// We need the sequential I2C methods
		switch (TransferDirection) {
		case I2C_DIRECTION_TRANSMIT:
			// TODO check the results
			HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, i2c1_buffer, len + 2, I2C_FIRST_FRAME); //len + reg + crc
			break;

		case I2C_DIRECTION_RECEIVE:
			// identify the addressed struct and copy the value
			i2c_writeRegisterToBuffer(register_number);

			i2c1_buffer[len] = calcCRC(register_number, i2c1_buffer, len);
			// TODO check the results
			HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, i2c1_buffer, len + 1, I2C_LAST_FRAME);
			break;

		default:
			break;
		}
	} else {
		// the RTC is accessed
		// TODO Hector this is the part where you tie in the RTC
		switch (TransferDirection) {
		case I2C_DIRECTION_TRANSMIT:
			// TODO check the results
			HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, i2c1_buffer,
					I2C_BUFFER_SIZE, I2C_FIRST_FRAME);
			break;
		case I2C_DIRECTION_RECEIVE:
//			i2c1_slave_transmit_buffer = (uint8_t*) rtc_get_register(
//					i2c1_slave_receive_buffer[0]);
//			i2c1_size_of_data = 6;
//			HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, i2c1_slave_transmit_buffer,
//					i2c1_size_of_data, I2C_LAST_FRAME);
			break;
		default:
			break;
		}
	}
}

/*
 * This callback is called when data from the RPi has been
 * successfully received. The buffer contains the register,
 * then the value(s) and finally the CRC. Depending on the
 * size of the transmitted data we have 3 bytes (for 8bit)
 * or 4 bytes (for 16bit) values.
 */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	i2c_in_progress = false;

	uint8_t len = hi2c->XferSize - 2;
	uint8_t register_number = i2c1_buffer[0];

	// check crc
	uint8_t crc = calcCRC(register_number, (i2c1_buffer + 1), len);

	// copy value if crc is correct
	if(crc == i2c1_buffer[len + 1]) {
		i2c_writeBufferToRegister(register_number);
	}

}

/*
 * We restart the I2C listening mode
 */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){
	HAL_I2C_EnableListen_IT(&hi2c1); // Restart
}

/*
 * The following callbacks simply set the i2c_in_progress
 * value back to false in case something goes wrong.
 */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
	i2c_in_progress = false;

}
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
	i2c_in_progress = false;

}
void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c) {
	i2c_in_progress = false;

}


/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
