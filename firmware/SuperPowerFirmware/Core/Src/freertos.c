/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "queue_handles.h"
#include "adc.h"
#include "i2c.h"
#include "i2c_register.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for I2C */
osThreadId_t I2CHandle;
const osThreadAttr_t I2C_attributes = {
  .name = "I2C",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 4096 * 4
};
/* Definitions for RTC */
osThreadId_t RTCHandle;
const osThreadAttr_t RTC_attributes = {
  .name = "RTC",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 4096 * 4
};
/* Definitions for StateMachine */
osThreadId_t StateMachineHandle;
const osThreadAttr_t StateMachine_attributes = {
  .name = "StateMachine",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 4096 * 4
};
/* Definitions for VoltageMeasurem */
osThreadId_t VoltageMeasuremHandle;
const osThreadAttr_t VoltageMeasurem_attributes = {
  .name = "VoltageMeasurem",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 4096 * 4
};
/* Definitions for I2C_R_Queue */
osMessageQueueId_t I2C_R_QueueHandle;
const osMessageQueueAttr_t I2C_R_Queue_attributes = {
  .name = "I2C_R_Queue"
};
/* Definitions for RTC_R_Queue */
osMessageQueueId_t RTC_R_QueueHandle;
const osMessageQueueAttr_t RTC_R_Queue_attributes = {
  .name = "RTC_R_Queue"
};
/* Definitions for Statemachine_R_Queue */
osMessageQueueId_t Statemachine_R_QueueHandle;
const osMessageQueueAttr_t Statemachine_R_Queue_attributes = {
  .name = "Statemachine_R_Queue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

uint16_t ch_convert_batv(uint8_t raw);
uint16_t ch_convert_vbus(uint8_t raw);
uint16_t ch_convert_charge_current(uint8_t raw);

/* USER CODE END FunctionPrototypes */

void I2C_Task(void *argument);
void RTC_Task(void *argument);
void StateMachine_Task(void *argument);
void VoltageMeasurement_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of I2C_R_Queue */
  I2C_R_QueueHandle = osMessageQueueNew (128, sizeof(uint16_t), &I2C_R_Queue_attributes);

  /* creation of RTC_R_Queue */
  RTC_R_QueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &RTC_R_Queue_attributes);

  /* creation of Statemachine_R_Queue */
  Statemachine_R_QueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &Statemachine_R_Queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of I2C */
  I2CHandle = osThreadNew(I2C_Task, NULL, &I2C_attributes);

  /* creation of RTC */
  RTCHandle = osThreadNew(RTC_Task, NULL, &RTC_attributes);

  /* creation of StateMachine */
  StateMachineHandle = osThreadNew(StateMachine_Task, NULL, &StateMachine_attributes);

  /* creation of VoltageMeasurem */
  VoltageMeasuremHandle = osThreadNew(VoltageMeasurement_Task, NULL, &VoltageMeasurem_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_I2C_Task */
/**
  * @brief  Function implementing the I2C thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_I2C_Task */
void I2C_Task(void *argument)
{
  /* USER CODE BEGIN I2C_Task */
	I2C_QueueMsg_t msg;
	osStatus_t status;


	/* Infinite loop */
	for (;;) {
		status = osMessageQueueGet(I2C_R_QueueHandle, &msg, NULL, osWaitForever); // wait for message
		if (status == osOK) {
			printf("Hello receive, ");
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		}
	}
	osDelay(1);
  /* USER CODE END I2C_Task */
}

/* USER CODE BEGIN Header_RTC_Task */
/**
* @brief Function implementing the RTC thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RTC_Task */
void RTC_Task(void *argument)
{
  /* USER CODE BEGIN RTC_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END RTC_Task */
}

/* USER CODE BEGIN Header_StateMachine_Task */
/**
* @brief Function implementing the StateMachine thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StateMachine_Task */
void StateMachine_Task(void *argument)
{
  /* USER CODE BEGIN StateMachine_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StateMachine_Task */
}

/* USER CODE BEGIN Header_VoltageMeasurement_Task */
/**
* @brief Function implementing the VoltageMeasurem thread.
* @param argument: Not used
* @retval None
*/
// TODO correct size
#define CH_BUF_SIZE 8
uint8_t ch_buf[CH_BUF_SIZE];
static const uint16_t i2c_master_timeout =  1000;  // timeout in milliseconds
static const uint16_t bq_update_interval = 10000;  // update interval for the charger in milliseconds

/* USER CODE END Header_VoltageMeasurement_Task */
void VoltageMeasurement_Task(void *argument)
{
  /* USER CODE BEGIN VoltageMeasurement_Task */
	HAL_StatusTypeDef ret_val;

	// on first execution

	//	BYTE_WATCHDOG_STOP = 0b10001101 #Stop Watchdog timer
	//	bus.write_byte_data(BQ25895_ADDRESS, REG_WATCHDOG, BYTE_WATCHDOG_STOP)

	//	BYTE_ILIM 		= 0b01111111 #3.25A input current limit
	//	bus.write_byte_data(BQ25895_ADDRESS, REG_ILIM, BYTE_ILIM)

	//	BYTE_ICHG 		= 0b01111111 #.5A charging current limit
	//	bus.write_byte_data(BQ25895_ADDRESS, REG_ICHG, BYTE_ICHG)

	//	BYTE_BATFET 	= 0b01001000 #delay before battery == disconnected
	//	bus.write_byte_data(BQ25895_ADDRESS, REG_BATFET, BYTE_BATFET)

	//BYTE_CONV_ADC_START = 0b10011101
	//BYTE_CONV_ADC_STOP  = 0b00011101

	/* Infinite loop */
	for (;;) {

//		HAL_ADCEx_InjectedStart_IT(&hadc1);


		// Turn the I2C slave off...
		ret_val = HAL_I2C_DisableListen_IT(&hi2c1);

		if(ret_val == HAL_OK) {
			// start ADC conversion
			ch_buf[0] = CH_CONV_ADC;
			ch_buf[1] = 0b10011101;
			ret_val = HAL_I2C_Master_Transmit(&hi2c1, CHARGER_ADDRESS, ch_buf, 2, i2c_master_timeout);
			if(ret_val == HAL_OK) {
				// osDelay(1200);

				// Read values from charger
				ch_buf[0] = CH_STATUS;
				ret_val = HAL_I2C_Master_Transmit(&hi2c1, CHARGER_ADDRESS, ch_buf, 1, i2c_master_timeout);
				if(ret_val == HAL_OK) {
					ret_val = HAL_I2C_Master_Receive(&hi2c1, CHARGER_ADDRESS, ch_buf, CH_BUF_SIZE, i2c_master_timeout);
					if(ret_val == HAL_OK) {
						// 0 + Status
						// 1 - Faults
						// 2 - VINDPM Threshold
						// 3 + Battery Voltage
						// 4 - System Voltage
						// 5 - TS Voltage (Battery Sensing Thermistor - Temperature Sensing)
						// 6 + VBus-Voltage
						// 7 + Charge Current

						i2c_status_register_8bit.val.charger_status = ch_buf[0];
						uint16_t batv = ch_convert_batv(ch_buf[3]);
						i2c_status_register_16bit.val.bat_voltage = batv;
						uint16_t vbus_v = ch_convert_vbus(ch_buf[6]);
						i2c_status_register_16bit.val.vbus_voltage = vbus_v;
						uint16_t ch_current = ch_convert_charge_current(ch_buf[7]);
						i2c_status_register_16bit.val.bat_current = ch_current;
					} else if(ret_val == HAL_ERROR) { // Master Receive
						// TODO check the results
					}
				} else if(ret_val == HAL_ERROR) { // Master Transmit Address
					// TODO check the results
				}
			} else if(ret_val == HAL_ERROR) { // Master_Transmit ADC
				// TODO check the results
			}
		}


		// ... and turn it on again
		// We don't need to check the return value because it
		// can only be HAL_OK or HAL_BUSY, either way I2C is
		// listening after the call
		HAL_I2C_EnableListen_IT(&hi2c1);


		osDelay(bq_update_interval);

	}
  /* USER CODE END VoltageMeasurement_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/*
 * Table 22. REG0E
 * ADC conversion of Battery Voltage (VBAT)
 * Offset: 2.304V
 * Range: 2.304V (0000000) – 4.848V (1111111)
 * Default: 2.304V (0000000)
 *
 * BATV[6] 1280mV
 * BATV[5]  640mV
 * BATV[4]  320mV
 * BATV[3]  160mV
 * BATV[2]   80mV
 * BATV[1]   40mV
 * BATV[0]   20mV
 */
uint16_t ch_convert_batv(uint8_t raw) {
	uint16_t result = 2304;
	uint16_t bit_val = 20;
	for(int i = 0; i < 7; i++) {
		if(raw & 0x1) {
			result += bit_val;
		}
		bit_val <<= 1;
		raw >>= 1;
	}
	return result;
}


/*
 * Table 25 REG11
 * ADC conversion of VBUS voltage (VBUS)
 * Offset: 2.6V
 * Default: 2.6V (0000000)
 *
 *  VBUS[7] VBUS_GD - 0 no VBUS - 1 VBUS attached
 * VBUSV[6] 6400mV
 * VBUSV[5] 3200mV
 * VBUSV[4] 1600mV
 * VBUSV[3]  800mV
 * VBUSV[2]  400mV
 * VBUSV[1]  200mV
 * VBUSV[0]  100mV
 */

uint16_t ch_convert_vbus(uint8_t raw) {
	uint16_t result = 2600;
	if(raw & 0b10000000) { // VBUS good
		uint16_t bit_val = 100;
		for(int i = 0; i < 7; i++) {
			if(raw & 0x1) {
				result += bit_val;
			}
			bit_val <<= 1;
			raw >>= 1;
		}
		return result;
	}
	return 0;
}

/*
 * Table 26 REG12
 * ADC conversion of Charge Current (IBAT) when VBAT > VBATSHORT
 * Offset: 0mA
 * Range 0mA (0000000) – 6350mA (1111111) Default: 0mA (0000000)
 * Note: This register returns 0000000 for VBAT < VBATSHORT
 * ICHGR[6] 3200mA
 * ICHGR[5] 1600mA
 * ICHGR[4]  800mA
 * ICHGR[3]  400mA
 * ICHGR[2]  200mA
 * ICHGR[1]  100mA
 * ICHGR[0]   50mA
 */

uint16_t ch_convert_charge_current(uint8_t raw) {
	uint16_t result = 0;
	uint16_t bit_val = 50;
	for(int i = 0; i < 7; i++) {
		if(raw & 0x1) {
			result += bit_val;
		}
		bit_val <<= 1;
		raw >>= 1;
	}

	return result;
}


/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
