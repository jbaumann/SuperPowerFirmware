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
#include "rtc.h"

/*
 * Initialization of the register structures
 */
I2C_Config_Register_8Bit i2c_config_register_8bit = {
	.val.primed                  =    0,   // 1 if the uC should control the system
	.val.force_shutdown          =    0,   // 1 if the uC should shutdown the UPS if the voltage is too low (hard shutdown)
};

I2C_Status_Register_8Bit i2c_status_register_8bit = {
	.val.should_shutdown         =  0x0,   // if != 0 contains the motivation for why the RPi should shutdown
};

I2C_Config_Register_16Bit i2c_config_register_16bit = {
	.val.timeout                 =  120,   // the timeout for the reset, should cover shutdown and reboot
	.val.bat_voltage_coefficient = 1000,   // the multiplier for the measured battery voltage * 1000, integral non-linearity
	.val.bat_voltage_constant    =    0,   // the constant added to the measurement of the battery voltage * 1000, offset error
	.val.ext_voltage_coefficient = 2000,   // the multiplier for the measured external voltage * 1000, integral non-linearity
	.val.ext_voltage_constant    =  700,   // the constant added to the measurement of the external voltage * 1000, offset error
	.val.restart_voltage         = 3900,   // the battery voltage at which the RPi will be started again
	.val.warn_voltage            = 3400,   // the battery voltage at which the RPi should should down
	.val.ups_shutdown_voltage    = 3200,   // the battery voltage at which a hard shutdown is executed
	.val.temperature_coefficient = 1000,   // the multiplier for the measured temperature * 1000, the coefficient
	.val.temperature_constant    = -270,   // the constant added to the measurement as offset
};

I2C_Status_Register_16Bit i2c_status_register_16bit = {
	.val.bat_voltage             =    0,   // the battery voltage, 3.3 should be low and 3.7 high voltage
	.val.ext_voltage             =    0,   // the external voltage from Pi or other source
	.val.seconds                 =    0,   // seconds since last i2c access
	.val.temperature             =    0,   // the on-chip temperature
};


/*
 * Communication buffers for I2C
 */
uint8_t slave_receive_buffer[I2C_BUFFER_SIZE];
uint8_t* slave_transmit_buffer;
__IO uint8_t size_of_data;

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
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_ENABLE;
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
 * We use the primary address for the UPS and the secondary address for
 * the RTC
 */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection,
		uint16_t AddrMatchCode) {

	_Bool primary_address = (hi2c->Init.OwnAddress1 == hi2c->Devaddress);

	if (primary_address) {
		// the UPS is accessed
		debug_print("i2c addr UPS");
	} else {
		// the RTC is accessed
		switch (TransferDirection) {
		case I2C_DIRECTION_TRANSMIT:
			HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, slave_receive_buffer,
					I2C_BUFFER_SIZE, I2C_FIRST_FRAME);
			break;
		case I2C_DIRECTION_RECEIVE:
			slave_transmit_buffer = (uint8_t*)rtc_get_register(slave_receive_buffer[0]);
			size_of_data = 6;
			HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, slave_transmit_buffer, size_of_data, I2C_LAST_FRAME);
			break;
		default:
			break;
		}
	}
}


void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){
	/*
	if(hi2c->XferCount == I2C_BUFFER_SIZE){
		test.cmd_size = 0;
	}else{
		test.cmd_size = (uint8_t)(32 - hi2c->XferCount - 1);
		memcpy(test.data, slaveReceiveBuffer+1, test.cmd_size);
	}
	memset(slaveReceiveBuffer, 0, 31);
	if(test.cmd_size > 0){
		RTC_msg_decode(test);
	}
	*/
	HAL_I2C_EnableListen_IT(&hi2c1); // Restart
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
