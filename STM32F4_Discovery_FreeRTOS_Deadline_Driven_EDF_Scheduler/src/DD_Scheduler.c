/*
 * dd_scheduler.c
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#include "DD_Scheduler.h"

TaskHandle_t DD_Task_Create(DD_TaskHandle_t create_task)
{
	/*
	  This primitive, creates a deadline driven scheduled task. It follows the steps outlined below
	  1. Opens a queue
	  2. Creates the task specified and assigns it the minimum priority possible
	  3. Composes a create_task_message and sends it to the DD-scheduler
	  4. Waits for a reply at the queue it created above
	  5. Once the reply is received, it obtains it
	  6. Destroys the queue
	  7. Returns to the invoking task
	*/

	// Create a queue of size with the parameters (could be smaller since includes pointers to next and prev) with size 1.
	DD_Message = xQueueCreate( 1, sizeof(DD_Task_t));

	// Task create: xTaskCreate( TaskFunction_t Function ,  const char * const pcName, configMINIMAL_STACK_SIZE ,NULL ,UBaseType_t uxPriority , TaskHandle_t *pxCreatedTask);
	xTaskCreate( create_task->task_function , "Created Task", configMINIMAL_STACK_SIZE ,NULL , DD_TASK_PRIORITY_MINIMUM , &(create_task->task_handle));

	// Send message to DD_Scheduler
	xQueueSend(DD_Message, &create_task, (TickType_t)10 );

	// Wait for read - Queue is empty means data is read.
	while (uxQueueMessagesWaiting(DD_Message) == 1);

	// TODO: Verify if DD_scheduler sends a reply, and if it does, change the wait logic.

	// Delete the message queue
	vQueueDelete(DD_Message);

	return &(create_task->task_handle); // or just create_task.task_handle
}


uint32_t DD_Task_Delete(DD_TaskHandle_t delete_task)
{
	// Create a queue of size with the parameters (could be smaller since includes pointers to next and prev) with size 1.
	DD_Message = xQueueCreate( 1, sizeof(DD_Task_t));

	// Deletes the task passed into the function
	vTaskDelete( &(delete_task->task_handle) );

	xQueueSend(DD_Message, &delete_task, (TickType_t)10 );

	// Wait for read - Queue is empty means data is read.
	while (uxQueueMessagesWaiting(DD_Message) == 1);

	// TODO: Verify if DD_scheduler sends a reply, and if it does, change the wait logic.

	// Delete the message queue
	vQueueDelete(DD_Message);

	return 0;
}


DD_TaskListHandle_t DD_Return_Active_List()
{
	return &active_list;
}


DD_TaskListHandle_t DD_Return_Overdue_List()
{
	return &overdue_list;
}
