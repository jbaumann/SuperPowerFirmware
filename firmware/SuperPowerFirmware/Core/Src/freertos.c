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
#include <stdbool.h>

#include <string.h>
#include "i2c.h"
#include "ch_bq25895.h"
#include "rtc.h"
#include "task_communication.h"
#include "ups_state.h"
#include "rtc.h"
#include "display.h"
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
uint8_t registers[10];
void get_charger_registers() {
	HAL_StatusTypeDef ret_val;
	uint8_t reg = 0;
	ret_val = HAL_I2C_Master_Transmit(&hi2c3, CHARGER_ADDRESS, &reg, 1,
			ch_i2c_master_timeout);
	if (ret_val == HAL_OK) {
		ret_val = HAL_I2C_Master_Receive(&hi2c3, CHARGER_ADDRESS, registers,
				sizeof(registers), ch_i2c_master_timeout);
	}
}



/* USER CODE END Variables */
/* Definitions for Display */
osThreadId_t DisplayHandle;
const osThreadAttr_t Display_attributes = {
  .name = "Display",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RTC */
osThreadId_t RTCHandle;
const osThreadAttr_t RTC_attributes = {
  .name = "RTC",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for StateMachine */
osThreadId_t StateMachineHandle;
const osThreadAttr_t StateMachine_attributes = {
  .name = "StateMachine",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LED */
osThreadId_t LEDHandle;
const osThreadAttr_t LED_attributes = {
  .name = "LED",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Test */
osThreadId_t TestHandle;
const osThreadAttr_t Test_attributes = {
  .name = "Test",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Display_R_Queue */
osMessageQueueId_t Display_R_QueueHandle;
const osMessageQueueAttr_t Display_R_Queue_attributes = {
  .name = "Display_R_Queue"
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

void Display_Task(void *argument);
void RTC_Task(void *argument);
void StateMachine_Task(void *argument);
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
  /* creation of Display_R_Queue */
  Display_R_QueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &Display_R_Queue_attributes);

  /* creation of RTC_R_Queue */
  RTC_R_QueueHandle = osMessageQueueNew (16, sizeof(Task_Data), &RTC_R_Queue_attributes);

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
  /* creation of Display */
  DisplayHandle = osThreadNew(Display_Task, NULL, &Display_attributes);

  /* creation of RTC */
  RTCHandle = osThreadNew(RTC_Task, NULL, &RTC_attributes);

  /* creation of StateMachine */
  StateMachineHandle = osThreadNew(StateMachine_Task, NULL, &StateMachine_attributes);

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

/* USER CODE BEGIN Header_Display_Task */

/**
  * @brief  Function implementing the Display thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Display_Task */
void Display_Task(void *argument)
{
  /* USER CODE BEGIN Display_Task */
	uint8_t display_type = i2c_config_register_8bit->display_type;
	if(display_type != 0 && display_type <= num_display_definitions) {
		init_display();
		/* Infinite loop */
		for (;;) {
			osStatus_t status;
			uint16_t msg;

			update_display();
			status = osMessageQueueGet(Display_R_QueueHandle, &msg, NULL, 1000);
		}
	}
	// Clean up if no display is configured
	osThreadExit();
  /* USER CODE END Display_Task */
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
	Task_Data msg;
	osStatus_t status;
	for(;;)
	{
		status = osMessageQueueGet(RTC_R_QueueHandle, &msg, NULL, osWaitForever);
		if(status ==osOK){
//			debug_print("RTC_Task receive, ");
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

	HAL_StatusTypeDef ret_val;

	get_charger_registers();

 	// on first execution
	ret_val = ch_init(&hi2c3);

	/* Infinite loop */
	for (;;) {
		uint16_t msg = 0;

		if (hi2c3.State == HAL_I2C_STATE_READY) {
			get_charger_registers();

			// start ADC conversion
			ret_val = ch_transfer_byte_to_register(&hi2c3, CH_CONV_ADC,
					CH_CONV_ADC_START);

			get_charger_registers();

			if (ret_val == HAL_OK) {
				osDelay(ch_conv_delay); // time for conversion, see 8.2.8 Battery Monitor on p.24
				// Read values from charger
				uint8_t reg = CH_STATUS;
				ret_val = HAL_I2C_Master_Transmit(&hi2c3, CHARGER_ADDRESS, &reg, 1, ch_i2c_master_timeout);
				if (ret_val == HAL_OK) {
					// we now use blocking I2C communication since we are the master
					ret_val = HAL_I2C_Master_Receive(&hi2c3, CHARGER_ADDRESS, i2c_ch_BQ25895_register_reg,
							sizeof(I2C_CH_BQ25895_Register), ch_i2c_master_timeout);
					if(ret_val == HAL_OK) {
						i2c_status_register_8bit->charger_status = i2c_ch_BQ25895_register.ch_status;
						uint16_t batv = ch_convert_batv(i2c_ch_BQ25895_register.ch_bat_voltage);
						i2c_status_register_16bit->ups_bat_voltage = batv;
						uint16_t vbus_v = ch_convert_vbus(i2c_ch_BQ25895_register.ch_vbus_voltage);
						i2c_status_register_16bit->vbus_voltage = vbus_v;
						uint16_t ch_current = ch_convert_charge_current(i2c_ch_BQ25895_register.ch_charge_current);
						i2c_status_register_16bit->charge_current = ch_current;

						// ok, contact has been established, we can use the values
						i2c_status_register_8bit->charger_contact = true;
					}

					get_charger_registers();
				} else if (ret_val == HAL_ERROR) { // Master Transmit Address
					// This should never happen because we just did a successful
					// transmit a second ago. We have to ignore this and hope for
					// the next time.
				}
			} else if (ret_val == HAL_ERROR) { // Master_Transmit ADC
				// cannot transmit data to the charger, means the device
				// is not reachable. We simply ignore this and wait for
				// it to come online.
			}
		}
		handle_state();

		osMessageQueuePut(RTC_R_QueueHandle, &msg, 0, 0);

		//osDelay(ups_update_interval);

	}
  /* USER CODE END StateMachine_Task */
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
	uint32_t waiting_time = 1000;
	uint8_t len = 0;
  /* Infinite loop */
  for(;;)
  {
		status = osMessageQueueGet(Test_R_QueueHandle, &msg, NULL, waiting_time); // wait for message
		if (status == osOK) {
			// copy msg to test_task_data
			len = (msg.data_size > sizeof(test_task_data)) ? sizeof(test_task_data) : msg.data_size;
			memcpy(test_task_data, msg.data, len);
		}
		// Here comes the business logic
		for (uint8_t i = 0; i < len; i++) {
			test_task_data[i] += 1;
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
