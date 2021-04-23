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

#include "task_communication.h"
#include "rtc.h"
#include "crc_8bit.h"
#include "cmsis_os.h"
#include "ch_bq25895.h"
#include "ups_state.h"

#include "DS3231.h"


/*
 * Communication data for I2C
 */
_Bool i2c_primary_address = true;

/*
 * The following structure holds the information needed for a transfer.
 * The data array is the I2C buffer for the respective address.
 * It contains the following data:
 * - On Transmit
 *   - Data to be sent
 *   - CRC
 * - On Receive
 *   - Register
 *   - Data received
 *   - CRC
 *
 * The variable data_size contains the size of the data without register and crc
 */
typedef struct {
	uint8_t data_size;
	union {
		uint8_t rawdata[I2C_BUFFER_SIZE];
		uint8_t tdata[I2C_BUFFER_SIZE];
		struct {
			enum I2C_Register i2c_register;
			uint8_t rdata[I2C_BUFFER_SIZE - 1];
		};
	};
} I2C_Transaction;

I2C_Transaction rtc_transaction, ups_transaction;


/*
 * We use 24 bit for the prog_version, this should be enough.
 */
uint32_t prog_version = (MAJOR << 16) | (MINOR << 8) | PATCH;

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
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
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}
/* I2C3 init function */
void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 400000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

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

    /* I2C1 DMA Init */
    /* I2C1_RX Init */
    hdma_i2c1_rx.Instance = DMA1_Stream0;
    hdma_i2c1_rx.Init.Channel = DMA_CHANNEL_1;
    hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_i2c1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_i2c1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_i2c1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(i2cHandle,hdmarx,hdma_i2c1_rx);

    /* I2C1_TX Init */
    hdma_i2c1_tx.Instance = DMA1_Stream1;
    hdma_i2c1_tx.Init.Channel = DMA_CHANNEL_0;
    hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_i2c1_tx.Init.Mode = DMA_CIRCULAR;
    hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_i2c1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_i2c1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(i2cHandle,hdmatx,hdma_i2c1_tx);

    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
  else if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspInit 0 */

  /* USER CODE END I2C3_MspInit 0 */

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**I2C3 GPIO Configuration
    PC9     ------> I2C3_SDA
    PA8     ------> I2C3_SCL
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* I2C3 clock enable */
    __HAL_RCC_I2C3_CLK_ENABLE();

    /* I2C3 interrupt Init */
    HAL_NVIC_SetPriority(I2C3_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
    HAL_NVIC_SetPriority(I2C3_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);
  /* USER CODE BEGIN I2C3_MspInit 1 */

  /* USER CODE END I2C3_MspInit 1 */
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

    /* I2C1 DMA DeInit */
    HAL_DMA_DeInit(i2cHandle->hdmarx);
    HAL_DMA_DeInit(i2cHandle->hdmatx);

    /* I2C1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
  else if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspDeInit 0 */

  /* USER CODE END I2C3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C3_CLK_DISABLE();

    /**I2C3 GPIO Configuration
    PC9     ------> I2C3_SDA
    PA8     ------> I2C3_SCL
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8);

    /* I2C3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
  /* USER CODE BEGIN I2C3_MspDeInit 1 */

  /* USER CODE END I2C3_MspDeInit 1 */
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
uint8_t i2c_writeRegisterToBuffer(enum I2C_Register register_number, uint8_t tdata[]) {
	uint8_t len = 1; // 8bit by default

	// identify the addressed struct and copy the value
	if (register_number < (enum I2C_Register)STATUS_8BIT_OFFSET) {

		// access to the CONFIG_8BIT struct
		uint8_t reg = register_number - CONFIG_8BIT_OFFSET;
		if(reg < i2c_config_reg_8bit_size) {
			tdata[0] = i2c_config_register_8bit->reg[reg];
		}
	} else if (register_number < (enum I2C_Register)CONFIG_16BIT_OFFSET) {

		// access to the STATUS_8BIT struct
		uint8_t reg = register_number - STATUS_8BIT_OFFSET;
		if(reg < i2c_status_reg_8bit_size) {
			tdata[0] = i2c_status_register_8bit->reg[reg];
		}
	} else if (register_number < (enum I2C_Register)STATUS_16BIT_OFFSET) {

		// access to the CONFIG_16BIT struct
		len = 2; // 16bit

		uint8_t reg = register_number - CONFIG_16BIT_OFFSET;
		if(reg < i2c_config_reg_16bit_size) {
			uint16_t *val = (uint16_t*) tdata;
			val[0] = i2c_config_register_16bit->reg[reg];
		}
	} else if (register_number < (enum I2C_Register)SPECIAL_16BIT_OFFSET) {

		// access to the STATUS_16BIT struct
		len = 2; // 16bit

		uint8_t reg = register_number - STATUS_16BIT_OFFSET;
		if(reg < i2c_status_reg_16bit_size) {
			uint16_t *val = (uint16_t*) tdata;
			val[0] = i2c_status_register_16bit->reg[reg];
		}
	} else if (register_number < (enum I2C_Register) TASK_COMMUNICATION) {
		/* access to the SPECIAL_16BIT struct */

		switch (register_number) {
		case i2creg_version:
			// Special register requesting the version number
			len = 3;  // 3 byte
			uint32_t *val = (uint32_t*) tdata;
			val[0] = prog_version;
			break;
		case i2creg_write_to_eeprom:
			len = 1;  // 1 byte
			break;
		case i2creg_should_shutdown:
			len = 1; // 1 byte
			tdata[0] = ups_state_should_shutdown;
			break;
		default:
			break;
		}

	} else {
		/* access to the task communication */
		uint8_t task_number = register_number - TASK_COMMUNICATION;
		if (task_number < task_comm_array_size) {
			uint8_t (*callback) (uint8_t *tdata) = task_communication[task_number].callback;
			if (callback != NULL) {
				len = callback(tdata);
			}
		}
	}
	return len + 1;  // add the space for the crc
}

/*
 * Determine the correct struct for the register, check
 * the bounds and if everything is ok, copy the value
 * from the buffer to the register
 *
 * register_number the register value sent by the RPi
 * data the data sent by the RPi
 * len the length of the data
 *
 */
void i2c_writeBufferToRegister(uint8_t register_number, uint8_t data[], uint8_t len) {
	uint8_t reg_has_changed = false;

	// identify the addressed struct and copy the value
	if (register_number < STATUS_8BIT_OFFSET) {

		// access to the CONFIG_8BIT struct
		uint8_t reg = register_number - CONFIG_8BIT_OFFSET;
		if(reg < i2c_config_reg_8bit_size) {
			i2c_config_register_8bit->reg[reg] = data[0];
			reg_has_changed = true;
		}
	} else if (register_number < CONFIG_16BIT_OFFSET) {
		/* the RPi does not set values in the STATUS_8BIT struct */
	} else if (register_number < STATUS_16BIT_OFFSET) {

		// access to the CONFIG_16BIT struct
		uint8_t reg = register_number - CONFIG_16BIT_OFFSET;
		if(reg < i2c_config_reg_16bit_size) {
			uint16_t *val = (uint16_t*) (data);
			i2c_config_register_16bit->reg[reg] = val[0];

			reg_has_changed = true;
		}
	} else if (register_number < (enum I2C_Register)SPECIAL_16BIT_OFFSET) {
		/* the RPi does not set values in the STATUS_16BIT struct */

	} else if (register_number < (enum I2C_Register)TASK_COMMUNICATION) {
		/* access to the SPECIAL_16BIT struct */
		switch (register_number) {
		case i2creg_jump_to_bootloader:
			if(i2c_config_register_8bit->val.enable_bootloader != 0) {
				// we are jumping into the bootloader
				jumpToBootloader();
			}
			break;
		case i2creg_should_shutdown:
			// TODO Sync this with UPS State
			if(data[0] == 0) {
				ups_state_should_shutdown = 0;
			} else {
				ups_state_should_shutdown |= data[0];
			}
			break;
		default:
			break;
		}


	} else {
		/* access to the task communication */
		uint8_t task_number = register_number - TASK_COMMUNICATION;
		if (task_number < task_comm_array_size) {
			// send data to Queue
			osMessageQueueId_t *queue = task_communication[task_number].queue;
			if (queue != NULL) {
				/*
				 * The data is already in ups_transaction.rdata, we map the rawdata
				 * to the Task_Data structure and thus have to write the length
				 * into the i2c_register. This trick saves us from copying the data
				 */
				ups_transaction.i2c_register = len;
				osMessageQueuePut(*queue, &ups_transaction.rawdata, 0, 0);
			}
		}
	}
	if(reg_has_changed) {
		backup_registers();
	}
	// check for RTC register change and trigger re-init
	if(register_number == i2creg_rtc_async_prediv || register_number == i2creg_rtc_sync_prediv) {
		// TODO Check whether this enough or whether the RTC has to
		// be de-initialized first
		//MX_RTC_Init();
	}
}

/*
 * We use the primary address for the UPS and the secondary address for
 * the RTC
 */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection,
		uint16_t AddrMatchCode) {

	i2c_triggered_ups_state_change();

	i2c_primary_address = (hi2c->Init.OwnAddress1 == AddrMatchCode);

	if (i2c_primary_address) {
		// the UPS is accessed

		uint8_t len, i2c_register;
		switch (TransferDirection) {
		case I2C_DIRECTION_TRANSMIT:
			HAL_I2C_Slave_Seq_Receive_DMA(&hi2c1, ups_transaction.rawdata, I2C_BUFFER_SIZE, I2C_LAST_FRAME);
			break;
		case I2C_DIRECTION_RECEIVE:
			// we copy the data to the buffer and send it to the RPi
			i2c_register = hi2c->Instance->DR;
			len = i2c_writeRegisterToBuffer(i2c_register, ups_transaction.tdata);
			ups_transaction.tdata[len - 1] = calcCRC(i2c_register, ups_transaction.tdata, len - 1);
			HAL_I2C_Slave_Seq_Transmit_DMA(&hi2c1, ups_transaction.tdata, len, I2C_LAST_FRAME);
			break;
		default:
			break;
		}
	} else {
		// the RTC is accessed
		uint8_t sizeOfData;
		switch (TransferDirection) {
		case I2C_DIRECTION_TRANSMIT:
			//HAL_I2C_Slave_Seq_Receive_DMA(&hi2c1, slaveReceiveBuffer, I2C_BUFFER_SIZE, I2C_FIRST_FRAME);
			HAL_I2C_Slave_Seq_Receive_DMA(&hi2c1, rtc_transaction.rawdata,
					I2C_BUFFER_SIZE, I2C_FIRST_FRAME);
			break;
		case I2C_DIRECTION_RECEIVE:
			sizeOfData = rtc_get_RTC_register(rtc_transaction.rdata[0],
					rtc_transaction.tdata);
			HAL_I2C_Slave_Seq_Transmit_DMA(&hi2c1, rtc_transaction.tdata,
					sizeOfData, I2C_LAST_FRAME);
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
	HAL_I2C_EnableListen_IT(&hi2c1);
}


/*
 * This callback is called when the data from the charger
 * has been successfully received. We copy the relevant
 * information and turn the listen mode back on
 */
/*
 * We now do a blocking call in the FreeRTOS task
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	if(hi2c == &hi2c3) {
		i2c_status_register_8bit->val.charger_status = i2c_ch_BQ25895_register.val.ch_status;
		uint16_t batv = ch_convert_batv(i2c_ch_BQ25895_register.val.ch_bat_voltage);
		i2c_status_register_16bit->val.bat_voltage = batv;
		uint16_t vbus_v = ch_convert_vbus(i2c_ch_BQ25895_register.val.ch_vbus_voltage);
		i2c_status_register_16bit->val.vbus_voltage = vbus_v;
		uint16_t ch_current = ch_convert_charge_current(i2c_ch_BQ25895_register.val.ch_charge_current);
		i2c_status_register_16bit->val.charge_current = ch_current;

		// ok, contact has been established, we can use the values
		i2c_status_register_8bit->val.charger_contact = true;
	}
}
 */


/*
 * We restart the I2C listening mode
 */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){

	if(i2c_primary_address)  {
		uint8_t len = (uint8_t)(I2C_BUFFER_SIZE - hi2c->XferCount);
		if(len != 0) {
			uint8_t data_len = len - 2;
			ups_transaction.data_size = data_len;

			// check crc and copy value if crc is correct
			uint8_t crc = calcCRC(ups_transaction.i2c_register, ups_transaction.rdata, data_len);
			if(crc == ups_transaction.rdata[data_len]) {
				i2c_writeBufferToRegister(ups_transaction.i2c_register, ups_transaction.rdata, data_len);
			}
		}
	} else {
		rtc_transaction.data_size = (hi2c->XferCount == 0) ? 0 : (uint8_t)(I2C_BUFFER_SIZE - hi2c->XferCount);
		if(rtc_transaction.data_size > 0 && rtc_transaction.data_size < I2C_BUFFER_SIZE) {
			Task_Data cmd;
			cmd.data_size = rtc_transaction.data_size;
			memcpy(cmd.data, rtc_transaction.rawdata, cmd.data_size);
			osMessageQueuePut(RTC_R_QueueHandle, &cmd, 0, 0);
		}
	}
	HAL_I2C_EnableListen_IT(&hi2c1); // Restart
}


/*
 * The following callbacks simply set the i2c_in_progress
 * value back to false in case something goes wrong.
 */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
//	ups_transaction.i2c_in_progress = false;
	HAL_I2C_EnableListen_IT(&hi2c1);
}
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
//	ups_transaction.i2c_in_progress = false;
	HAL_I2C_EnableListen_IT(&hi2c1);

}
void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c) {
//	ups_transaction.i2c_in_progress = false;
	HAL_I2C_EnableListen_IT(&hi2c1);
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
