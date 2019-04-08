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

/* DD_Scheduler Pseudo-Code
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
void DD_Scheduler( void *pvParameters )
{
    DD_Message_t received_message;
    DD_TaskHandle_t DD_task_handle = NULL;

    while(1)
    {
        if( xQueueReceive( DD_Scheduler_Message_Queue, (void*)&received_message, portMAX_DELAY ) == pdTRUE ) // wait until an item is present on the queue.
        {

            // Step 1: Clear overdue tasks
            DD_TaskList_Transfer_Overdue( &active_list, &overdue_list );

            // Step 2: Remove items from overdue list, only keep 10 most recent overdue.
            while( overdue_list.list_length > 5 )
            {
            	printf("\nDD_Scheduler: Removing item from overdue list.\n\n");
            	DD_TaskList_Remove_Head(  &overdue_list );
            }

            // Step 3: From queue message, get the request, and do stuff accordingly.
            switch(received_message.message_type)
            {
                case(DD_Message_Create):
                    // Need to pass in the DD_TaskHandle_t and the list to work on.
                    DD_task_handle = (DD_TaskHandle_t)received_message.message_data;
                    DD_TaskList_Deadline_Insert( DD_task_handle , &active_list );

                    // If its an aperiodic task, create a timer to callback on the deadline
                    if( DD_task_handle->task_type == DD_TT_Aperiodic)
                    {
                        // Timer name
                        char timer_name[50] = "T_";
                        strcat(timer_name, DD_task_handle->task_name );

                        TickType_t timer_period = DD_task_handle->deadline - xTaskGetTickCount();

                        // Check that timer period is positive or something...

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
                    DD_TaskList_Remove( received_message.message_sender , &active_list , true );
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

    // Delete the task that called this callback (was oneshot, we done with it)
    xTimerDelete( aperiodic_task->aperiodic_timer, 0 );
    // Clear the local timer variable for the aperiodic_task
    aperiodic_task->aperiodic_timer = NULL;

    // Stop the task and delete it.
    // Note: Can't call DD_task_delete or send a message to the scheduler to delete
    //       since we can't wait for a queue to send in a callback
    vTaskSuspend( aperiodic_task->task_handle );
    vTaskDelete( aperiodic_task->task_handle );

    // Need to validate that there wont be a memory leak here.
    // Should be aperiodic task -> into overdue_list -> remove from overdue list after 5 elements -> DD_Task_Free -> vPortFree
    // So should be okay.
} // end DD_Aperiodic_Timer_Callback



/*--------------------------- DD Scheduler Public Access Functions --------------------------------*/


// Initializes the lists and communication queues necessary for the DD_Scheduler
void DD_Scheduler_Init()
{
    debugprintf("Starting DD_Scheduler_Init - List Init, Queue Init, Task Creation for Scheduler and Monitor.\n\n");

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
    xTaskCreate( DD_Scheduler , "DD Scheduler Task" , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_SCHEDULER , NULL);
    xTaskCreate( MonitorTask  , "Monitoring Task"   , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_MONITOR   , NULL);

} // end DD_Scheduler_Init


/*
 * DD_Task_Create: creates a deadline driven scheduled task. It follows the steps outlined below
 * 1. Check that the DD_TaskHandle_t isn't null
 * 2. Create the task given the parameters
 * 3. Ensure the task was successfully created.
 * 4. Suspend the task until priority is assigned.
 * 5. Creates a message structure
 * 6. Send the message to the DD_Scheduler on its queue
 *   -> Let the DD_Scheduler assigned the priority
 * 7. Wait for the response from the DD_Scheduler that priority is assigned.
 * 8. Resume the newly created task.
 *
 * Params: DD_TaskHandle_t create_task
 *  -> Holds all the task parameters required for task creation.
 *  -> Everything but the task_handle must be set before calling this function.
 */
uint32_t DD_Task_Create(DD_TaskHandle_t create_task)
{

    // STEP 1: Verify that the input data isn't empty
    if( create_task == NULL )
    {
        printf("ERROR: Sent NULL DD_TaskHandle_t to DD_Task_Create! \n");
        return 0;
    }

    // STEP 2: Create the task in FreeRTOS with the parameters passed in, and minimum priority.
    xTaskCreate( create_task->task_function ,   // TaskFunction_t Function             -> the task that will run. Either a predefined periodic or aperiodic task.
                 create_task->task_name ,       // const char * const pcName           -> Name given to the task, limited in character length by configMAX_TASK_NAME_LEN in FreeRTOSConfig.h
                 configMINIMAL_STACK_SIZE ,     // configSTACK_DEPTH_TYPE usStackDepth -> Set to configMINIMAL_STACK_SIZE
                 (void*)create_task ,          // void *pvParameters                  -> Pass in the dd task handle so the function is aware of its parameters.
                 DD_TASK_PRIORITY_MINIMUM ,     // UBaseType_t uxPriority              -> Priority set to minimum, assigned by DD_scheduler
                 &(create_task->task_handle) ); // TaskHandle_t *pxCreatedTask         -> Used to pass a handle to the created task out of the xTaskCreate() function

    // STEP 3: Check that the task was actually created.
    if( create_task->task_handle == NULL )
    {
        printf("ERROR: DD_Task_Create's xTaskCreate didn't make a pointer! \n");
        return 0;
    }

    // STEP 4: Might want to suspend the task here, and resume it after the scheduler replies.
    vTaskSuspend( create_task->task_handle );

    // STEP 5: Create the message structure, with the DD_TaskHandle_t in the data field for the linked list element.
    DD_Message_t create_task_message = { DD_Message_Create ,            // Message Type / Request is to create a task
                                         xTaskGetCurrentTaskHandle() ,  // Message sender is the task generator, need to send this for task notification
                                         create_task };                 // DD_TaskHandle_t is a pointer to the task data, holds the task handle and deadline for the new task.

    // STEP 6: (SCHEDULER REQUEST SEND) Send the message to the DD_Scheduler queue
    if( DD_Scheduler_Message_Queue != NULL) // Check that the queue exists
    {
        if( xQueueSend(DD_Scheduler_Message_Queue, &create_task_message, portMAX_DELAY ) != pdPASS ) // ensure the message was sent
        {
            printf("ERROR: DD_Task_Create couldn't send request on DD_Scheduler_Message_Queue!\n");
            return 0;
        }
    }
    else // Queue doesn't exist, error out, entire system will fail.
    {
        printf("ERROR: DD_Scheduler_Message_Queue is NULL.\n");
        return 0;
    }

    // STEP 7: (SCHEDULER RECEIVE) Wait until the Scheduler replies.
    ulTaskNotifyTake( pdTRUE,          /* Clear the notification value before exiting. */
                      portMAX_DELAY ); /* Block indefinitely. */

    // STEP 8: Scheduler replied, priority assigned, start the task
    vTaskResume( create_task->task_handle );

    // Return with 1 for success.
    return 1;
} // end DD_Task_Create

/*
 * DD_Task_Delete: deletes a deadline driven scheduled task. It follows the steps outlined below
 * 1. Check that the TaskHandle_t isn't null
 * 2. Creates a message structure
 * 3. Send the message to the DD_Scheduler on its queue
 * 4. Wait for the response from the DD_Scheduler that priority is assigned.
 * 5. Delete the task.
 *
 * Params: TaskHandle_t delete_task
 *  -> Only need the task handle.
 *  -> DD_Scheduler will find the task handle in the active list if it exists
 *      -> Could be in overdue list, and so task wont be deleted until overdue list cleans up.
 *      -> However, will still perform vTaskDelete
 */
uint32_t DD_Task_Delete(TaskHandle_t delete_task)
{
    // STEP 1: Verify that the input data isn't empty
    if( delete_task == NULL )
    {
        printf("ERROR: DD_Task_Delete send a NULL TaskHandle_t! \n");
        return 0;
    }

    // STEP 2: Create the message structure, only know the sender but no info, but thats all thats required for DD_Scheduler.
    DD_Message_t delete_task_message = { DD_Message_Delete , // Message Type / Request is to delete a task
                                         delete_task ,       // Sender is the task to be deleted itself, so send it for task notification
                                         NULL };             // No need for additional data, DD_Scheduler will grab it

    // STEP 3: (SCHEDULER REQUEST SEND) Send the message to the DD_Scheduler queue
    if( DD_Scheduler_Message_Queue != NULL) // Check that the queue exists
    {
        if( xQueueSend(DD_Scheduler_Message_Queue, &delete_task_message, portMAX_DELAY ) != pdPASS ) // ensure the message was sent
        {
            printf("ERROR: DD_Task_Delete couldn't send request on DD_Scheduler_Message_Queue!\n");
            return 0;
        }
    }
    else // Queue doesn't exist, error out, entire system will fail.
    {
        printf("ERROR: DD_Scheduler_Message_Queue is NULL.\n");
        return 0;
    }

    // STEP 4: (SCHEDULER RECEIVE) Wait until the Scheduler replies.
    ulTaskNotifyTake( pdTRUE, portMAX_DELAY ); /* Block indefinitely. */

    // STEP 5: Deletes the task after DD_Scheduler has removed it from the active list and wiped its memory.
    vTaskDelete( delete_task );

    // Return with 1 for success.
    return 1;
} // end DD_Task_Delete




/*--------------------------- DD Scheduler Monitoring Functionality --------------------------------*/

/*
 * MonitorTask: Requests from the DD_Scheduler the active and overdue list data
 */
void MonitorTask ( void *pvParameters )
{

    while(1)
    {
    	printf("\nMonitoring Task: Current Time = %u, Priority = %u\n", (unsigned int)xTaskGetTickCount(), (unsigned int)uxTaskPriorityGet( NULL ));
        DD_Return_Active_List();
        DD_Return_Overdue_List();

        vTaskDelay(1000); // Wait for 1 seconds.
    }
}



/*
 * DD_Return_Active_List: Function will request DD_Scheduler to return a string containing info about the active list.
 * 1. Create the message structure
 * 2. Send request to DD_Scheduler
 * 3. Receive data from DD_Scheduler
 */
uint32_t DD_Return_Active_List( void )
{
    // STEP 1: Create the message structure, all that is required is the request type
    DD_Message_t active_list_message = { DD_Message_ActiveList, // Message Type / Request is to get active task data
    		                             NULL,                  // Don't need to send a sender as no task notification required.
										 NULL };                // Don't have any data to send

    // STEP 2: (SCHEDULER REQUEST SEND) Send the message to the DD_Scheduler queue
    if( DD_Scheduler_Message_Queue != NULL) // Check that the queue exists
    {
        if( xQueueSend(DD_Scheduler_Message_Queue, &active_list_message, portMAX_DELAY ) != pdPASS ) // ensure the message was sent
        {
            printf("ERROR: DD_Return_Active_List couldn't send request on DD_Scheduler_Message_Queue!\n");
            return 0;
        }
    }
    else // Queue doesn't exist, error out, entire system will fail.
    {
        printf("ERROR: DD_Scheduler_Message_Queue is NULL.\n");
        return 0;
    }

    // STEP 3: (SCHEDULER RECEIVE) Wait for the response with the data.
    if( DD_Monitor_Message_Queue != NULL) // Check that the queue exists
    {
        if( xQueueReceive( DD_Monitor_Message_Queue, &active_list_message, portMAX_DELAY  ) == pdTRUE ) // ensure the message was received
        {
            printf( "Active Tasks: \n%s\n", (char*)(active_list_message.message_data)); // Message was received, print out the data.
            vPortFree(active_list_message.message_data);                              // Free the memory used to store the string
            active_list_message.message_data = NULL;                                  // Set the pointer to that memory to NULL;
        }
    }
    else // Queue doesn't exist, error out, entire system will fail.
    {
        printf("ERROR: DD_Monitor_Message_Queue is NULL.\n");
        return 0;
    }

    return 1;
} // end DD_Return_Active_List

/*
 * DD_Return_Overdue_List: Function will request DD_Scheduler to return a string containing info about the overdue list.
 * 1. Create the message structure
 * 2. Send request to DD_Scheduler
 * 3. Receive data from DD_Scheduler
 */
uint32_t DD_Return_Overdue_List( void )
{
    // STEP 1: Create the message structure, all that is required is the request type
    DD_Message_t overdue_list_message = { DD_Message_OverdueList,    // Message Type / Request is to get active task data
    		                              NULL,                      // Don't need to send a sender as no task notification required.
										  NULL };                    // Don't have any data to send

    // STEP 2: (SCHEDULER REQUEST SEND) Send the message to the DD_Scheduler queue
    if( DD_Scheduler_Message_Queue != NULL) // Check that the queue exists
    {
        if( xQueueSend(DD_Scheduler_Message_Queue, &overdue_list_message, (TickType_t) portMAX_DELAY ) != pdPASS ) // ensure the message was sent
        {
            printf("ERROR: DD_Return_Overdue_List couldn't send request on DD_Scheduler_Message_Queue!\n");
            return 0;
        }
    }
    else // Queue doesn't exist, error out, entire system will fail.
    {
        printf("ERROR: DD_Scheduler_Message_Queue is NULL.\n");
        return 0;
    }

    // STEP 3: (SCHEDULER RECEIVE) Wait for the response with the data.
    if( DD_Monitor_Message_Queue != NULL) // Check that the queue exists
    {
        if( xQueueReceive( DD_Monitor_Message_Queue, &overdue_list_message, (TickType_t) portMAX_DELAY  ) == pdTRUE ) // ensure the message was received
        {
            printf( "Overdue Tasks: \n%s\n", (char*)(overdue_list_message.message_data));   // Message was received, print out the data.
            vPortFree(overdue_list_message.message_data);                                 // Free the memory used to store the string
            overdue_list_message.message_data = NULL;                                     // Set the pointer to that memory to NULL;
        }
    }
    else // Queue doesn't exist, error out, entire system will fail.
    {
        printf("ERROR: DD_Monitor_Message_Queue is NULL.\n");
        return 0;
    }

    return 1;
} // end DD_Return_Overdue_List
