/*
 * dd_scheduler.c
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#include "DD_Scheduler.h"

/*--------------------------- DD Scheduler Variables --------------------------------*/

static DD_TaskList_t active_list;
static DD_TaskList_t overdue_list;

// Queues for Task_Create and Task_Delete
QueueHandle_t DD_Scheduler_Message_Queue = NULL;
QueueHandle_t DD_Monitor_Message_Queue = NULL;

/*--------------------------- DD Scheduler --------------------------------*/

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

void DD_Scheduler( void *pvParameters )
{
	DD_Message_t received_message;

	while(1)
	{
		if( xQueueReceive( DD_Scheduler_Message_Queue, &received_message, portMAX_DELAY ) ) // wait until an item is present on the queue.
		{
			// going through DD_Message_Type_t enum
			switch(received_message.message_type)
			{
				case(DD_Message_Create):
					// Need to pass in the DD_TaskHandle_t and the list to work on.
				    DD_TaskList_Deadline_Insert( received_message.message_data , &active_list );

					// Reply to message.
					xTaskNotifyGive( received_message.message_sender );

					break;
				case(DD_Message_Delete):
					// Remove element from list, and free its memory.
					DD_TaskList_Remove( received_message.message_sender , &active_list );
					// NOTE: If the task was moved to the overdue list, it wont have been deleted.
				    //       The memory stored for DD_Task_t will still exist for that task.
				    //       Will need a method to wipe the overdue list at one point.

					// Reply to message.
					xTaskNotifyGive( received_message.message_sender );

					break;
				case(DD_Message_ActiveList):
					// Store the string in the message data, which will be sent back to reply
					received_message.message_data = (void*)DD_TaskList_Formatted_Data( &active_list );


					break;
				case(DD_Message_OverdueList):
					// Store the string in the message data, which will be sent back to reply
					received_message.message_data = (void*)DD_TaskList_Formatted_Data( &overdue_list );


					break;
			}

			// Clear overdue tasks
			DD_TaskList_Transfer_Overdue( &active_list, &overdue_list );


		}


	}
}





/*--------------------------- DD Scheduler Public Access Functions --------------------------------*/


// Initializes the lists and communication queues necessary for the DD_Scheduler
void DD_Scheduler_Init()
{
	// Step 1: Initialize the active and overdue list
	DD_TaskList_Init( &active_list );
	DD_TaskList_Init( &overdue_list );

	// Step 2: Initialize DD_Scheduler_Message_Queue
	DD_Scheduler_Message_Queue = xQueueCreate(DD_TASK_RANGE, sizeof(DD_Message_t));

	// Step 3: Initialize DD_Monitor_Message_Queue
	DD_Monitor_Message_Queue = xQueueCreate(2, sizeof(DD_Message_t)); // Should only ever have 2 requests on the queue.

	// Step 4: Create the DD_Scheduler and Monitor Tasks
	xTaskCreate( DD_Scheduler            , "Deadline Driven Scheduler Task" , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_SCHEDULER , NULL);
	xTaskCreate( MonitorTask             , "Monitoring Task"                , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_MONITOR   , NULL);

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

uint32_t DD_Task_Create(DD_TaskHandle_t create_task)
{
	// The task handle is automatically updated after xTaskCreate is called, due to the pointer passed into the function

	// Task create: xTaskCreate( TaskFunction_t Function ,  const char * const pcName, configMINIMAL_STACK_SIZE ,NULL ,UBaseType_t uxPriority , TaskHandle_t *pxCreatedTask);
	xTaskCreate( create_task->task_function , create_task->task_name , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_MINIMUM , &(create_task->task_handle));


	// Create the message structure, with the DD_TaskHandle_t in the data field for the linked list element.
	DD_Message_t create_message = { DD_Message_Create , create_task->task_handle, create_task };

	// Send the message to the DD_Scheduler queue
	xQueueSend(DD_Scheduler_Message_Queue, &create_message, (TickType_t)10 );

	// Wait until the Scheduler replies.
    ulTaskNotifyTake( pdTRUE,          /* Clear the notification value before exiting. */
                      portMAX_DELAY ); /* Block indefinitely. */

	return 0;
}


uint32_t DD_Task_Delete(TaskHandle_t delete_task)
{

	// Create the message structure, only know the sender but no info, but thats all thats required for DD_Scheduler.
	DD_Message_t delete_message = { DD_Message_Delete , delete_task , NULL };

	// Send the message to the DD_Scheduler queue
	xQueueSend(DD_Scheduler_Message_Queue, &delete_message, (TickType_t)10 );

	// Wait until the Scheduler replies.
    ulTaskNotifyTake( pdTRUE,          /* Clear the notification value before exiting. */
                      portMAX_DELAY ); /* Block indefinitely. */


	// Deletes the task after DD_Scheduler has removed it from the active list and wiped its memory.
	vTaskDelete( delete_task );

	return 0;
}

// Function will request DD_Scheduler to return a string containing info about the active list.
DD_TaskListHandle_t DD_Return_Active_List()
{



	return &active_list;
}

// Function will request DD_Scheduler to return a string containing info about the overdue list.
DD_TaskListHandle_t DD_Return_Overdue_List()
{
	return &overdue_list;
}
