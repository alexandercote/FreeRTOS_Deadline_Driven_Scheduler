/*
 * dd_scheduler.c
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#include "DD_Scheduler.h"



static DD_TaskList_t active_list;
static DD_TaskList_t overdue_list;

// Queues for Task_Create and Task_Delete
QueueHandle_t DD_Message_Queue = NULL;
//QueueHandle_t DD_Create_Message = 0;
//QueueHandle_t DD_Delete_Message = 0;



/* Pseudo-Code
 * Receive request from one of 4 helper functions
 * Before anything, clear overdue tasks as a requirement before any task is handled. (quick if no tasks are overdue)
 * Figure out message type (switch statement)
 * Task_Create
 *  -> add new element to active list
 * Task_Delete
 *  -> remove the selected task from the active list (and maybe overdue list????)
 *  Active_List & Overdue_List data requests
 *  -> provide data for the monitor task (string name and deadline)
 */

// ISSUES
// -> Overdue task could eliminate items, and then later on DD_Message_Delete could be called...
//      -> Solved by checked if item is in the active list first

void DD_Scheduler(  void *pvParameters )
{
	DD_Message_t received_message;

	while(1)
	{
		if( xQueueReceive( DD_Message_Queue, &received_message, portMAX_DELAY ) ) // wait until an item is present on the queue.
		{
			// going through DD_Message_Type_t enum
			switch(received_message.message_type)
			{
				case(DD_Message_Create):
					// Need to pass in the DD_TaskHandle_t and the list to work on.
				    DD_TaskList_Deadline_Insert( received_message.message_data , &active_list );
					// Reply to message.
					//xQueueSend(  )
					break;
				case(DD_Message_Delete):
					DD_TaskList_Remove_1( received_message.message_sender , &active_list )

					// Reply to message.
					//xQueueSend(  )

					/*
					// Need to grab the data within the scheduler based off the sender.
					received_message.message_data = DD_TaskList_Get_DD_TaskHandle_t( received_message.message_sender , &active_list );

					// Check if the element still exists in active list.
					// Could manage this issue in DD_TaskList_Remove, would be more efficient.
					if( DD_TaskList_Task_Exists( received_message.message_sender , &active_list ) )
						DD_TaskList_Remove_2( received_message.message_sender , &active_list );
					*/

					break;
				case(DD_Message_ActiveList):
					break;
				case(DD_Message_OverdueList):
					break;
			}

			// Clear overdue tasks
			// NOTE: Have to somehow ensure that delete taks
			DD_TaskList_Transfer_Overdue( &active_list, &overdue_list );


		}


	}
}

/*
 * This primitive, creates a deadline driven scheduled task. It follows the steps outlined below
 * 1. Opens a queue
 * 2. Creates the task specified and assigns it the minimum priority possible
 * 3. Composes a create_task_message and sends it to the DD-scheduler
 * 4. Waits for a reply at the queue it created above
 * 5. Once the reply is received, it obtains it
 * 6. Destroys the queue
 * 7. Returns to the invoking task
 */

TaskHandle_t DD_Task_Create(DD_TaskHandle_t create_task)
{

	DD_Message_Queue = xQueueCreate( 1, sizeof(DD_Message_t));

	// Task create: xTaskCreate( TaskFunction_t Function ,  const char * const pcName, configMINIMAL_STACK_SIZE ,NULL ,UBaseType_t uxPriority , TaskHandle_t *pxCreatedTask);
	xTaskCreate( create_task->task_function , create_task->task_name , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_MINIMUM , &(create_task->task_handle));

	DD_Message_t create_message = { DD_Message_Create , create_task };

	// Send message to DD_Scheduler
	xQueueSend(DD_Message_Queue, &create_message, (TickType_t)10 );

	// Wait for read - Queue is empty means data is read.
	while (uxQueueMessagesWaiting(DD_Message_Queue) == 1);




	// Delete the message queue
	vQueueDelete(DD_Message_Queue);

	return create_task->task_handle;
}


uint32_t DD_Task_Delete(TaskHandle_t delete_task)
{
	// Create a queue of size with the parameters (could be smaller since includes pointers to next and prev) with size 1.
	DD_Message_Queue = xQueueCreate( 1, sizeof(DD_Task_t));

	// Maybe need to move this into the scheduler.
	DD_TaskHandle_t delete_task_struct =  DD_TaskList_Get_DD_TaskHandle_t( delete_task , &active_list );

	DD_Message_t delete_message = { DD_Message_Delete , delete_task_struct };

	xQueueSend(DD_Message_Queue, &delete_message, (TickType_t)10 );

	// Wait for read - Queue is empty means data is read.
	while (uxQueueMessagesWaiting(DD_Message_Queue) == 1);




	// Delete the message queue
	vQueueDelete(DD_Message_Queue);

	// Deletes the task passed into the function
	vTaskDelete( delete_task );
	// OR vTaskDelete( NULL );

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
