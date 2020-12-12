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
/* USER CODE END Header_VoltageMeasurement_Task */
void VoltageMeasurement_Task(void *argument)
{
  /* USER CODE BEGIN VoltageMeasurement_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END VoltageMeasurement_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
