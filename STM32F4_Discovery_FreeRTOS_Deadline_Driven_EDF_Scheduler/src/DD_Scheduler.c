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

static QueueHandle_t DD_Scheduler_Message_Queue;
static QueueHandle_t DD_Monitor_Message_Queue;


/*--------------------------- DD Scheduler Timer Callback --------------------------------*/

static void DD_Aperiodic_Timer_Callback( xTimerHandle xTimer );


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
	DD_TaskHandle_t DD_task_handle = NULL;

	while(1)
	{
		if( xQueueReceive( DD_Scheduler_Message_Queue, (void*)&received_message, (TickType_t) portMAX_DELAY ) == pdTRUE ) // wait until an item is present on the queue.
		{

			// Step 1: Clear overdue tasks
			DD_TaskList_Transfer_Overdue( &active_list, &overdue_list );

			// Step 2: Remove items from overdue list, only keep 10 most recent overdue.
			while( overdue_list.list_length > 10 )
			{
				DD_TaskList_Remove( overdue_list.list_head , &overdue_list );
			}

			// Step 3: From queue message, get the request, and do stuff accordingly.
			switch(received_message.message_type)
			{
				case(DD_Message_Create):
					// Need to pass in the DD_TaskHandle_t and the list to work on.
				    DD_task_handle = (DD_TaskHandle_t)received_message.message_data;
				    DD_TaskList_Deadline_Insert( DD_task_handle , &active_list );

					// If its a periodic task, create a timer to callback on the deadline
					if( DD_task_handle->task_type == DD_TT_Aperiodic)
					{
						// Timer name
						char timer_name[50] = "Timer_";
						strcat(timer_name, DD_task_handle->task_name );

						TickType_t timer_period = DD_task_handle->deadline - xTaskGetTickCount();

						// TimerHandle_t xTimerCreate ( const char * const pcTimerName, const TickType_t xTimerPeriod, const UBaseType_t uxAutoReload, void * const pvTimerID, TimerCallbackFunction_t pxCallbackFunction );
						DD_task_handle->aperiodic_timer = xTimerCreate( timer_name ,                   // Append "Timer_" to the task name
								                                        timer_period ,                 // Timer period set to the remaining timer before deadline
																		pdFALSE ,                      // Auto-reload disabled
																		(void*)DD_task_handle ,        // Pass the DD_TaskHandle_t so that it can be used in the timercallback.
																		DD_Aperiodic_Timer_Callback );

						// Start the timer
						xTimerStart( DD_task_handle->aperiodic_timer, 0 );
					}

					// Reply to message.
					xTaskNotifyGive( received_message.message_sender );
					break;

				case(DD_Message_Delete):

					// if the task is aperiodic, and DD_Task_Delete was called first, need to delete timer.
						// Deal with this in DD_TaskList_Remove

					// Remove element from list, and free its memory, and if its aperiodic, stops the timer
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

					if(uxQueueSpacesAvailable(DD_Monitor_Message_Queue) == 0) // monitor queue is full, wipe it
					{
						xQueueReset( DD_Monitor_Message_Queue );
					}
					// Reply to request with data.

					// Reply to request with data.
					if( DD_Monitor_Message_Queue != NULL)
					{
						if( xQueueSend( DD_Monitor_Message_Queue, &received_message, (TickType_t) portMAX_DELAY ) != pdPASS )
						{
							printf("ERROR: DD_Scheduler couldn't send request on DD_Monitor_Message_Queue!\n");
							break;
						}
					}
					else
					{
						printf("ERROR: DD_Monitor_Message_Queue is NULL.\n");
						break;
					}
					break;

				case(DD_Message_OverdueList):
					// Store the string in the message data, which will be sent back to reply
					received_message.message_data = (void*)DD_TaskList_Formatted_Data( &overdue_list );

				    // Check if monitor queue is full, if so clear old data
					if(uxQueueSpacesAvailable(DD_Monitor_Message_Queue) == 0)
					{
						xQueueReset( DD_Monitor_Message_Queue );
					}

					// Reply to request with data.
					if( DD_Monitor_Message_Queue != NULL)
					{
						if( xQueueSend( DD_Monitor_Message_Queue, &received_message, (TickType_t) portMAX_DELAY ) != pdPASS )
						{
							printf("ERROR: DD_Scheduler couldn't send request on DD_Monitor_Message_Queue!\n");
							break;
						}
					}
					else
					{
						printf("ERROR: DD_Monitor_Message_Queue is NULL.\n");
						break;
					}
					break;

			} // end switch

		} // end queue receive
	} // end while
} // end DD_Scheduler


/* This callback occurs when the deadline for the aperiodic task completes.
 * Need to remove the aperiodic task, since DD_Task_Delete
 *
 * Steps:
 * - Get DD_TaskHandle_t from Timer ID
 * - Delete timer, clear DD_TaskHandle_t->aperiodic_task for future use
 * - Suspend Task, Delete Task
 */
static void DD_Aperiodic_Timer_Callback( xTimerHandle xTimer )
{
	/* Need to get access to the DD_TaskHandle_t given the xTimer.
	 * Could search through the lists for the aperiodic_timer field in the DD_TaskHandle_t.
	 * However, we stored the DD_TaskHandle_t as the timer ID. Pointers are amazing.
	 */
	DD_TaskHandle_t aperiodic_task = (DD_TaskHandle_t)pvTimerGetTimerID( xTimer );

	// Clear the local timer variable for the aperiodic_task
	aperiodic_task->aperiodic_timer = NULL;

	// Delete the task that called this callback (was oneshot, we done with it)
	xTimerDelete( xTimer, 0 );

	// Stop the task and delete it.
	// Note: Can't call DD_task_delete or send a message to the scheduler to delete
	//       since we can't wait for a queue to send in a callback
	vTaskSuspend( aperiodic_task->task_handle );
	vTaskDelete( aperiodic_task->task_handle );

	// Need to validate that there wont be a memory leak here.
	// Should be aperiodic task -> into overdue_list -> remove from overdue list after 10 elements -> DD_Task_Free -> vPortFree
	// So should be okay.
} // end DD_Aperiodic_Timer_Callback



/*--------------------------- DD Scheduler Public Access Functions --------------------------------*/


// Initializes the lists and communication queues necessary for the DD_Scheduler
void DD_Scheduler_Init()
{
	// Step 1: Initialize the active and overdue list
	DD_TaskList_Init( &active_list );
	DD_TaskList_Init( &overdue_list );

	// Step 2: Initialize DD_Scheduler_Message_Queue
	DD_Scheduler_Message_Queue = xQueueCreate(DD_TASK_RANGE, sizeof(DD_Message_t));
	vQueueAddToRegistry(DD_Scheduler_Message_Queue,"Scheduler Queue");

	// Step 3: Initialize DD_Monitor_Message_Queue
	DD_Monitor_Message_Queue = xQueueCreate(2, sizeof(DD_Message_t)); // Should only ever have 2 requests on the queue.
	vQueueAddToRegistry(DD_Monitor_Message_Queue,"Monitor Queue");

	// Step 4: Create the DD_Scheduler and Monitor Tasks
	xTaskCreate( DD_Scheduler            , "DD Scheduler Task" , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_SCHEDULER , NULL);
	xTaskCreate( MonitorTask             , "Monitoring Task"                , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_MONITOR   , NULL);

} // end DD_Scheduler_Init


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
	// Verify that the input data isn't empty
	if( create_task == NULL )
	{
		printf("ERROR: Sent NULL DD_TaskHandle_t to DD_Task_Create! \n");
		return 0;
	}


	// Task create: xTaskCreate( TaskFunction_t Function ,  const char * const pcName, configMINIMAL_STACK_SIZE ,NULL ,UBaseType_t uxPriority , TaskHandle_t *pxCreatedTask);
	xTaskCreate( create_task->task_function , create_task->task_name , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_MINIMUM , &(create_task->task_handle));

	if( create_task->task_handle == NULL )
	{
		printf("ERROR: DD_Task_Create's xTaskCreate didn't make a pointer! \n");
		return 0;
	}

	// Create the message structure, with the DD_TaskHandle_t in the data field for the linked list element.
	DD_Message_t create_task_message = { DD_Message_Create , xTaskGetCurrentTaskHandle() , create_task };

	// Send the message to the DD_Scheduler queue
	if( DD_Scheduler_Message_Queue != NULL)
	{
		if( xQueueSend(DD_Scheduler_Message_Queue, &create_task_message, (TickType_t) portMAX_DELAY ) != pdPASS )
		{
			printf("ERROR: DD_Task_Create couldn't send request on DD_Scheduler_Message_Queue!\n");
			return 0;
		}
	}
	else
	{
		printf("ERROR: DD_Scheduler_Message_Queue is NULL.\n");
		return 0;
	}

	// Wait until the Scheduler replies.
    ulTaskNotifyTake( pdTRUE, (TickType_t) portMAX_DELAY ); /* Block indefinitely. */

	return 0;
} // end DD_Task_Create


uint32_t DD_Task_Delete(TaskHandle_t delete_task)
{
	// Verify that the input data isn't empty
	if( delete_task == NULL )
	{
		printf("ERROR: DD_Task_Delete send a NULL TaskHandle_t! \n");
		return 0;
	}

	// Create the message structure, only know the sender but no info, but thats all thats required for DD_Scheduler.
	DD_Message_t delete_task_message = { DD_Message_Delete , delete_task , NULL };

	// SCHEDULER REQUEST SEND: Send the message to the DD_Scheduler queue
	if( DD_Scheduler_Message_Queue != NULL)
	{
		if( xQueueSend(DD_Scheduler_Message_Queue, &delete_task_message, (TickType_t) portMAX_DELAY ) != pdPASS )
		{
			printf("ERROR: DD_Task_Delete couldn't send request on DD_Scheduler_Message_Queue!\n");
			return 0;
		}
	}
	else
	{
		printf("ERROR: DD_Scheduler_Message_Queue is NULL.\n");
		return 0;
	}

	// SCHEDULER RECEIVE: Wait until the Scheduler replies.
    ulTaskNotifyTake( pdTRUE, (TickType_t) portMAX_DELAY ); /* Block indefinitely. */

	// Deletes the task after DD_Scheduler has removed it from the active list and wiped its memory.
	vTaskDelete( delete_task );

	return 1;
} // end DD_Task_Delete




/*--------------------------- DD Scheduler Monitoring Functionality --------------------------------*/

void MonitorTask ( void *pvParameters )
{
	while(1)
	{
		uint32_t tasks_count = uxTaskGetNumberOfTasks();
		printf("Number of tasks = %u \n", (unsigned int)tasks_count);
		DD_Return_Active_List();
		DD_Return_Overdue_List();

		vTaskDelay(1000);
	}
}


// Function will request DD_Scheduler to return a string containing info about the active list.
uint32_t DD_Return_Active_List( void )
{
	// Create the message structure, all that is required is the request type
	DD_Message_t active_list_message = { DD_Message_ActiveList, NULL, NULL };

	// SCHEDULER REQUEST SEND: Send the message to the DD_Scheduler queue
	if( DD_Scheduler_Message_Queue != NULL)
	{
		if( xQueueSend(DD_Scheduler_Message_Queue, &active_list_message, (TickType_t) portMAX_DELAY ) != pdPASS )
		{
			printf("ERROR: DD_Return_Active_List couldn't send request on DD_Scheduler_Message_Queue!\n");
			return 0;
		}
	}
	else
	{
		printf("ERROR: DD_Scheduler_Message_Queue is NULL.\n");
		return 0;
	}

	// SCHEDULER RECEIVE: Wait for the response with the data.
	if( DD_Monitor_Message_Queue != NULL)
	{
		if( xQueueReceive( DD_Monitor_Message_Queue, &active_list_message, (TickType_t) portMAX_DELAY  ) == pdTRUE )
		{
			printf( "Active Tasks: \n%s", (char*)(active_list_message.message_data));
			vPortFree(active_list_message.message_data);
			active_list_message.message_data = NULL;
		}
	}
	else
	{
		printf("ERROR: DD_Monitor_Message_Queue is NULL.\n");
		return 0;
	}

	return 1;
} // end DD_Return_Active_List

// Function will request DD_Scheduler to return a string containing info about the overdue list.
uint32_t DD_Return_Overdue_List( void )
{
	// Create the message structure, all that is required is the request type
	DD_Message_t overdue_list_message = { DD_Message_OverdueList, NULL, NULL };

	// SCHEDULER REQUEST SEND: Send the message to the DD_Scheduler queue
	if( DD_Scheduler_Message_Queue != NULL)
	{
		if( xQueueSend(DD_Scheduler_Message_Queue, &overdue_list_message, (TickType_t) portMAX_DELAY ) != pdPASS )
		{
			printf("ERROR: DD_Return_Overdue_List couldn't send request on DD_Scheduler_Message_Queue!\n");
			return 0;
		}
	}
	else
	{
		printf("ERROR: DD_Scheduler_Message_Queue is NULL.\n");
		return 0;
	}

	// SCHEDULER RECEIVE: Wait for the response with the data.
	if( DD_Monitor_Message_Queue != NULL)
	{
		if( xQueueReceive( DD_Monitor_Message_Queue, &overdue_list_message, (TickType_t) portMAX_DELAY  ) == pdTRUE )
		{
			printf( "Overdue Tasks: \n%s", (char*)(overdue_list_message.message_data));
			vPortFree(overdue_list_message.message_data);
			overdue_list_message.message_data = NULL;
		}
	}
	else
	{
		printf("ERROR: DD_Monitor_Message_Queue is NULL.\n");
		return 0;
	}

	return 1;
} // end DD_Return_Overdue_List
