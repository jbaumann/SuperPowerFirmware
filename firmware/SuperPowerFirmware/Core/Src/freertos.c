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
#include <string.h>
#include "adc.h"
#include "i2c.h"
#include "ch_bq25895.h"
#include "rtc.h"
#include "task_communication.h"

// JB TODO move to external impl.
#include "rtc.h"
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
  .stack_size = 2048 * 4
};
/* Definitions for RTC */
osThreadId_t RTCHandle;
const osThreadAttr_t RTC_attributes = {
  .name = "RTC",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 2048 * 4
};
/* Definitions for StateMachine */
osThreadId_t StateMachineHandle;
const osThreadAttr_t StateMachine_attributes = {
  .name = "StateMachine",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 2048 * 4
};
/* Definitions for VoltageMeasurem */
osThreadId_t VoltageMeasuremHandle;
const osThreadAttr_t VoltageMeasurem_attributes = {
  .name = "VoltageMeasurem",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 2048 * 4
};
/* Definitions for LED */
osThreadId_t LEDHandle;
const osThreadAttr_t LED_attributes = {
  .name = "LED",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 2048 * 4
};
/* Definitions for Test */
osThreadId_t TestHandle;
const osThreadAttr_t Test_attributes = {
  .name = "Test",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 1024 * 4
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
/* Definitions for LED_R_Queue */
osMessageQueueId_t LED_R_QueueHandle;
const osMessageQueueAttr_t LED_R_Queue_attributes = {
  .name = "LED_R_Queue"
};
/* Definitions for Test_R_Queue */
osMessageQueueId_t Test_R_QueueHandle;
const osMessageQueueAttr_t Test_R_Queue_attributes = {
  .name = "Test_R_Queue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void I2C_Task(void *argument);
void RTC_Task(void *argument);
void StateMachine_Task(void *argument);
void VoltageMeasurement_Task(void *argument);
void LED_Task(void *argument);
void Test_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */



/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t *ulExpectedIdleTime)
{
/* place for user code */
}

__weak void PostSleepProcessing(uint32_t *ulExpectedIdleTime)
{
/* place for user code */
}
/* USER CODE END PREPOSTSLEEP */

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
  RTC_R_QueueHandle = osMessageQueueNew (2, sizeof(Task_Data), &RTC_R_Queue_attributes);

  /* creation of Statemachine_R_Queue */
  Statemachine_R_QueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &Statemachine_R_Queue_attributes);

  /* creation of LED_R_Queue */
  LED_R_QueueHandle = osMessageQueueNew (16, sizeof(LED_QueueMsg_t*), &LED_R_Queue_attributes);

  /* creation of Test_R_Queue */
  Test_R_QueueHandle = osMessageQueueNew (16, sizeof(Task_Data), &Test_R_Queue_attributes);

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

  /* creation of LED */
  LEDHandle = osThreadNew(LED_Task, NULL, &LED_attributes);

  /* creation of Test */
  TestHandle = osThreadNew(Test_Task, NULL, &Test_attributes);

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

uint8_t buffer[SLAVE_BUFFER_SIZE];
uint8_t size;
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
			debug_print("I2C_Task receive, ");
			//HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		}
	}
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
	Task_Data msg;
	osStatus_t status;
	for(;;)
	{
		status = osMessageQueueGet(RTC_R_QueueHandle, &msg, NULL, osWaitForever);
		if(status ==osOK){
			debug_print("RTC_Task receive, ");
			rtc_msg_decode(msg.data_size, msg.data);
		}
	}
  /* USER CODE END RTC_Task */
}

/* USER CODE BEGIN Header_StateMachine_Task */
/**
* @brief Function implementing the StateMachine thread.
* @param argument: Not used
* @retval None
*/
//extern UBaseType_t uxTaskGetStackHighWaterMark( TaskHandle_t xTask );
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

/* USER CODE END Header_VoltageMeasurement_Task */
void VoltageMeasurement_Task(void *argument)
{
  /* USER CODE BEGIN VoltageMeasurement_Task */
	HAL_StatusTypeDef ret_val;


	// on first execution
	//ret_val = ch_init(&hi2c1);
	while(1){
		osDelay(100);
	}
	/* Infinite loop */
	for (;;) {

		// Turn the I2C slave functionality off
		ret_val = HAL_I2C_DisableListen_IT(&hi2c1);

		if(hi2c1.State == HAL_I2C_STATE_READY) {
			// start ADC conversion
			ret_val = ch_transfer_byte_to_register(&hi2c1, CH_CONV_ADC, CH_CONV_ADC_START);

			if(ret_val == HAL_OK) {
				// We are waiting for the ADC to finish its conversion and
				// switch back to listen mode for until it is done
				HAL_I2C_EnableListen_IT(&hi2c1);
				osDelay(ch_conv_delay); // time for conversion, see 8.2.8 Battery Monitor on p.24
				ret_val = HAL_I2C_DisableListen_IT(&hi2c1);
				if(ret_val == HAL_OK) {
					// Read values from charger
					uint8_t reg = CH_STATUS;
					ret_val = HAL_I2C_Master_Transmit(&hi2c1, CHARGER_ADDRESS, &reg, 1, ch_i2c_master_timeout);
					if(ret_val == HAL_OK) {
						ret_val = HAL_I2C_Master_Receive_IT(&hi2c1, CHARGER_ADDRESS, i2c_ch_BQ25895_register.reg, sizeof(I2C_CH_BQ25895_Register));
					} else if(ret_val == HAL_ERROR) { // Master Transmit Address
						// This should never happen because we just did a successful
						// transmit a second ago. We have to ignore this and hope for
						// the next time.
					}
				} else if(ret_val == HAL_ERROR) { // Disable Listen
					// if we can't turn off the listen mode then there
					// is an ongoing communication between RPi and us.
					// We'll try again next time
				}
			} else if(ret_val == HAL_ERROR) { // Master_Transmit ADC
				// cannot transmit data to the charger, means the device
				// is not reachable. We simply ignore this and wait for
				// it to come online.
			}
		}
		if(ret_val != HAL_OK) {
			HAL_I2C_EnableListen_IT(&hi2c1);
		}

		osDelay(ch_update_interval);

	}
  /* USER CODE END VoltageMeasurement_Task */
}

/* USER CODE BEGIN Header_LED_Task */
/**
* @brief Function implementing the LED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LED_Task */
void LED_Task(void *argument)
{
  /* USER CODE BEGIN LED_Task */
	LED_QueueMsg_t *msg;
	LED_QueueMsg_t *current = NULL, *background = blink_second_background;
	osStatus_t status;
	uint32_t waiting_time = 0;

    for(;;)
    {
	if(background == NULL) {
		waiting_time = osWaitForever; // wait for message
	} else {
		waiting_time = 0; // do not wait for message
	}
		status = osMessageQueueGet(LED_R_QueueHandle, &msg, NULL, waiting_time);
		if (status == osOK) {
			if(msg->iterations == 0) {
				background = NULL;
				current = NULL;
			} else if(msg->iterations == 255) {
				background = msg;
				current = background;
			} else {
				current = msg;
			}
		} else {
			current = background;
		}

		if(current != NULL) {
			uint8_t iterations = current->iterations;
			if(iterations == 0xFF) iterations = 1;

			for(uint8_t i = 0; i < iterations; i++) {
				for(uint8_t s = 0; s < current->number_steps; s++) {
					LED_Step step = ((LED_Step *)(current->steps))[s];
					uint8_t repeat = step.repeat;
					if(repeat == 0) repeat = 1;

					for(uint8_t r = 0; r < repeat; r++) {
						if(step.ontime != 0)
						{
							HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
							osDelay(step.ontime);
						}
						if(step.offtime != 0) {
							HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
							osDelay(step.offtime);
						}
					}
				}
			}
			if(current->final_delay != 0) {
				osDelay(current->final_delay);
			}
		}
		current = NULL;
    }
  /* USER CODE END LED_Task */
}

/* USER CODE BEGIN Header_Test_Task */
/**
* @brief Function implementing the Test thread.
* @param argument: Not used
* @retval None
*/
uint8_t test_task_data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

/* USER CODE END Header_Test_Task */
void Test_Task(void *argument)
{
  /* USER CODE BEGIN Test_Task */
	Task_Data msg;
	osStatus_t status;
  /* Infinite loop */
  for(;;)
  {
		status = osMessageQueueGet(Test_R_QueueHandle, &msg, NULL, osWaitForever); // wait for message
		if (status == osOK) {
			// copy msg to test_task_data
			uint8_t len = (msg.data_size > sizeof(test_task_data)) ? sizeof(test_task_data) : msg.data_size;
			memcpy(test_task_data, msg.data, len);

			// Here comes the business logic
			for (uint8_t i = 0; i < len; i++) {
				test_task_data[i] *= 2;
			}
		}

  }
  /* USER CODE END Test_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

uint8_t test_callback(uint8_t transfer[]) {
	memcpy(transfer, test_task_data, sizeof(test_task_data));
	return sizeof(test_task_data);
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
