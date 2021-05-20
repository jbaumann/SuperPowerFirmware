/*
 * queue_handles.h
 *
 *  Created on: Dec 11, 2020
 *      Author: jbaumann
 *
 * This file contains the message queue definitions and
 * the structure definitions for everything having to do
 * with task communication via I2C
 */

#ifndef INC_TASK_COMMUNICATION_H_
#define INC_TASK_COMMUNICATION_H_

#include "cmsis_os.h"


/*****************************************************************************
 * Task communication definitions
 *****************************************************************************/

enum task_communication_consts {
	TASK_MAX_DATA_SIZE      =   32,                      // _EXTRACT_TASK_COMM_
};

/*
 * The external declarations for all Queue handles and callback functions
 */
osMessageQueueId_t Display_R_QueueHandle;
osMessageQueueId_t LED_R_QueueHandle;
osMessageQueueId_t RTC_R_QueueHandle;
osMessageQueueId_t Test_R_QueueHandle;


/*
 * The callbacks provided by the tasks for the I2C communication
 */
uint8_t test_callback(uint8_t transfer[]);

/*
 * The following structure contains the information needed to let the I2C
 * code and the user tasks communicate with each other.
 */
typedef struct {
	osMessageQueueId_t *queue;
	uint8_t (*callback) (uint8_t *tdata);

} Task_Communication;

/*
 * The following array of Task_Communication entries is the lookup table
 * used by the I2C code to identify the queue to send data to a task and
 * the callback used to generate data that is to be sent back to the RPi.
 * Both can be set to NULL, in which case the functionality is not used.
 * The function name of the callback is used to generate a name for the
 * task communication in the generated Python interface.
 * Since we use simple pattern matching it is extremely important that
 * every callback has the comment _EXTRACT_TASK_COMM_ and that this
 * comment is used without any changes.
 * Otherwise recognition of tasks and their correct numbering will not
 * work.
 */
static const Task_Communication task_communication[] = {
	{
		.queue = &Test_R_QueueHandle,
		.callback = test_callback,                   // _EXTRACT_TASK_COMM_
	},
};

/*
 * Helper value that guarantees that we stay within the boundaries of the array.
 */
static const uint8_t task_comm_array_size = sizeof(task_communication) / sizeof(Task_Communication);

/*
 * This struct is used to transfer the data to the respective task, whenever the RPi
 * sends something to the registers between 0xF0-0xFF.
 */
typedef struct {
	uint8_t data_size;
	uint8_t data[TASK_MAX_DATA_SIZE];
} Task_Data;


#endif /* INC_TASK_COMMUNICATION_H_ */
