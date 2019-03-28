/*
 * DD_Task_List.c
 *
 *  Created on: Mar 20, 2019
 *      Author: amcote
 */

#include "DD_Task_List.h"

/*--------------------------- Task Creation/Deletion (List Elements) --------------------------------*/

/*
 * DD_Task_Allocate: Creates an instance of a DD_TaskHandle_t, and zeroes out the struct data.
 */
DD_TaskHandle_t DD_Task_Allocate()
{
    DD_TaskHandle_t newtask = (DD_TaskHandle_t)pvPortMalloc(sizeof(DD_Task_t));

    newtask->task_handle     = NULL;
    newtask->task_function   = NULL;
    newtask->task_name       = "";
    newtask->task_type       = DD_TT_Undefined;
    newtask->aperiodic_timer = NULL;
    newtask->creation_time   = 0;
    newtask->deadline        = 0;
    newtask->next_cell       = NULL;
    newtask->previous_cell   = NULL;

    return newtask;
}

/*
 * DD_Task_Free: Free the instance of a DD_TaskHandle_t, and zeroes out the struct data.
 */
bool DD_Task_Free(DD_TaskHandle_t task_to_remove)
{
	// Check input parameters are not NULL
	if( task_to_remove == NULL )
	{
		printf("ERROR(DD_Task_Free): one of the parameters passed was NULL.\n");
		return false;
	}

    // return false if the task wasn't removed from active/overdue queue, or removed from active tasks.
    if(task_to_remove->next_cell != NULL || task_to_remove->previous_cell != NULL)
    {
        printf("ERROR(DD_Task_Free): Forgot to remove task from list, not deleting it. Fix the code.\n");
        return false;
    }

    // Reset the data before freeing - some errors can occur if not done
    task_to_remove->task_handle      = NULL;
    task_to_remove->task_function    = NULL;
    task_to_remove->task_name        = "";
    task_to_remove->task_type        = DD_TT_Undefined;
    task_to_remove->aperiodic_timer  = NULL;
    task_to_remove->creation_time    = 0;
    task_to_remove->deadline         = 0;
    task_to_remove->next_cell        = NULL;
    task_to_remove->previous_cell    = NULL;

    // free the memory used by the task
    vPortFree((void*)task_to_remove);

    return true;
}


/*--------------------------- Task List Initialization --------------------------------*/

/*
 * DD_TaskList_Init: Initialize a list with zeroed out values
 */
void DD_TaskList_Init( DD_TaskListHandle_t init_list )
{
	// Check input parameters are not NULL
	if( init_list == NULL )
	{
		printf("ERROR(DD_TaskList_Init): one of the parameters passed was NULL.\n");
		return;
	}

    init_list->list_length = 0;
    init_list->list_head   = NULL;
    init_list->list_tail   = NULL;
}


/*--------------------------- Task List Access Functions --------------------------------*/


/*
 * DD_TaskList_Deadline_Insert: Insert a task into the active list based off the deadline, earliest deadline goes at the head
 *
 * Priority Management: From head of the list, increment priority until the insertion location is reached.
 */
void DD_TaskList_Deadline_Insert( DD_TaskHandle_t task_to_insert , DD_TaskListHandle_t insert_list )
{
	// Check input parameters are not NULL
	if( ( task_to_insert == NULL ) || ( insert_list == NULL ) )
	{
		printf("ERROR(DD_TaskList_Deadline_Insert): one of the parameters passed was NULL.\n");
		return;
	}

    // Step 1: check if list is empty, else place element according to its deadline
    if( insert_list->list_length == 0 )
    {
        insert_list->list_length = 1;
        insert_list->list_head = task_to_insert;
        insert_list->list_tail = task_to_insert;
        vTaskPrioritySet(task_to_insert->task_handle, DD_TASK_PRIORITY_EXECUTION_BASE);
        return; // quit early so the next part doesn't run
    }

    // Step 2: list isn't empty, need to iterate through list and place according to deadline

    DD_TaskHandle_t iterator = insert_list->list_head;                // start from head, closest deadline
    uint32_t itr_priority = uxTaskPriorityGet(iterator->task_handle); // grab the highest priority value

    if(( itr_priority + 1 ) == DD_TASK_PRIORITY_GENERATOR)              // reached the highest level of priority, at generator so thats it.
    {
        printf("ERROR: REACHED LIMIT OF NUMBER OF SCHEDULABLE TASKS! NOT INSERTING TASK");
        return;
    }

    itr_priority++; //increment the priority, set the new priority until the new task is reached.

    while( iterator != NULL )
    {
        if( task_to_insert->deadline < iterator->deadline ) //found location in list.
        {
            if( iterator == insert_list->list_head ) // new element has earliest deadline, current head is getting replaced
                insert_list->list_head = task_to_insert;

            task_to_insert->next_cell     = iterator;                     // place new task before iterator
            task_to_insert->previous_cell = iterator->previous_cell;      // Makes the previous task of iterator now the insert task's iterator
            iterator->previous_cell       = task_to_insert;               // make the iterator task previous to the newly inserted task

            (insert_list->list_length)++;                                 // increment the list size

            vTaskPrioritySet(task_to_insert->task_handle, itr_priority);
            return; // no need to continue the loop
        }
        else
        {
            if( iterator->next_cell == NULL ) // reached the end of the list (tail), insert at end.
            {
                task_to_insert->next_cell     = NULL;
                task_to_insert->previous_cell = iterator;
                iterator->next_cell           = task_to_insert;
                insert_list->list_tail        = task_to_insert;

                (insert_list->list_length)++; // increment the list size
                vTaskPrioritySet(iterator->task_handle, itr_priority );                         // update the current cell's priority
                vTaskPrioritySet(task_to_insert->task_handle, DD_TASK_PRIORITY_EXECUTION_BASE); // as the item is now the bottom, assign it bottom priority.
                return; // no need to continue the loop, even though its about to end.
            }

            //Current task has closer deadline than new task, and isn't the tail. Decrement priority.
            vTaskPrioritySet(iterator->task_handle, itr_priority );
            itr_priority--;
            iterator = iterator->next_cell; //keep going through the list
        } // end if/else deadline comparison
    } // end while
} // end DD_TaskList_Deadline_Insert


/*
 * DD_TaskList_Remove: Remove a task from the active list based off the task handle TaskHandle_t
 *
 * Priority Management: From head of the list, increment priority until the insertion location is reached.
 */
void DD_TaskList_Remove( TaskHandle_t task_to_remove , DD_TaskListHandle_t remove_list , bool clear_memory )
{
	// Check input parameters are not NULL
	if( ( task_to_remove == NULL ) || ( remove_list == NULL ) )
	{
		printf("ERROR(DD_TaskList_Remove): one of the parameters passed was NULL.\n");
		return;
	}

	// Check list isn't empty.
    if( remove_list->list_length == 0 )
    {
        printf("ERROR(DD_TaskList_Remove): trying to remove a task from an empty list...\n");
        return;
    }


    DD_TaskHandle_t iterator = remove_list->list_head; // start from head, closest deadline
    uint32_t itr_priority = uxTaskPriorityGet(iterator->task_handle); // grab the highest priority value

    while( iterator != NULL )
    {
        if( iterator->task_handle == task_to_remove ) // found the task to remove
        {
            // This means that delete was called before the timer callback executed.
            if( (iterator->task_type == DD_TT_Aperiodic) && (iterator->aperiodic_timer != NULL) )
            {
                xTimerStop( iterator->aperiodic_timer, 0 );      // Stop the timer
                xTimerDelete( iterator->aperiodic_timer, 0 );    // Delete the timer
                iterator->aperiodic_timer = NULL;                // Clear the timer in the DD_TaskHandle_t
            }

            // Removal management, could probably make this simpler, but this is explicit for each edge case.
            if( remove_list->list_length == 1 ) // Know we are removing the head and tail of the list.
            {
                remove_list->list_head = NULL;   // No more items in the list, head is null.
                remove_list->list_tail = NULL;   // No more items in the list, tail is null.
            }
            else if( task_to_remove == remove_list->list_head->task_handle ) //Removing the head of the list.
            {
                remove_list->list_head = iterator->next_cell;      // Make the new head of the list the next item in line
                iterator->next_cell->previous_cell = NULL;         // Ensure the next item in the list points to NULL for anything before it.
            }
            else if( task_to_remove == remove_list->list_tail->task_handle ) //Removing the tail of the list
            {
                remove_list->list_head = iterator->previous_cell;  // Make the new tail of the list the previous item in line
                iterator->previous_cell->next_cell = NULL;         // Ensure the previous item in the list points to NULL for anything after it.
            }
            else // Removing from middle of the list
            {
                iterator->previous_cell->next_cell = iterator->next_cell;
                iterator->next_cell->previous_cell = iterator->previous_cell;
                //DD_TaskHandle_t prev_task = iterator->previous_cell;
                //DD_TaskHandle_t next_task = iterator->next_cell;
                //prev_task->next_cell     = next_task;
                //next_task->previous_cell = prev_task;
            }

            (remove_list->list_length)--; // decrement the list size

            // Clear next/prev cell for removing task
            iterator->previous_cell = NULL;
            iterator->next_cell     = NULL;

            // Free DD_Task_t memory
            if( clear_memory )
            {
            	DD_Task_Free( iterator );
            }

            return; // done with the removal
        }
        //Current isn't the one to remove. Decrement priority.
        itr_priority--;
        vTaskPrioritySet(iterator->task_handle, itr_priority);

        iterator = iterator->next_cell;
    }

    // If we get to this point, that means the item wasn't in the list... Undo all the priority stuff we just did.
    // Go back through the list, and increment all reset the priorities.
    iterator = remove_list->list_tail;
    vTaskPrioritySet(iterator->task_handle, DD_TASK_PRIORITY_EXECUTION_BASE);

    while( iterator->previous_cell != NULL )
    {
        itr_priority++;
        iterator = iterator->previous_cell;
        vTaskPrioritySet(iterator->task_handle, itr_priority);
    }
} // end DD_TaskList_Remove

/*
 * DD_TaskList_Remove_Head: Remove the head of a list based off the task handle TaskHandle_t
 */
void DD_TaskList_Remove_Head( DD_TaskListHandle_t remove_list )
{
	// Check input parameters are not NULL
	if( ( remove_list == NULL ) )
	{
		printf("ERROR(DD_TaskList_Remove_Head): one of the parameters passed was NULL.\n");
		return;
	}

	// Check list isn't empty.
    if( remove_list->list_length == 0 )
    {
        printf("ERROR(DD_TaskList_Remove_Head): trying to remove a task from an empty list...\n");
        return;
    }


    DD_TaskHandle_t task_head = remove_list->list_head; // start from head, closest deadline

     if( remove_list->list_length == 1 ) // Know we are removing the head and tail of the list.
     {
         remove_list->list_head = NULL;   // No more items in the list, head is null.
         remove_list->list_tail = NULL;   // No more items in the list, tail is null.
     }
     else // if( task_to_remove == remove_list->list_head->task_handle ) //Removing the head of the list.
     {
         remove_list->list_head = task_head->next_cell;      // Make the new head of the list the next item in line
         task_head->next_cell->previous_cell = NULL;         // Ensure the next item in the list points to NULL for anything before it.
     }

     (remove_list->list_length)--; // decrement the list size

     // Clear next/prev cell for removing task
     task_head->previous_cell = NULL;
     task_head->next_cell     = NULL;

     // Free DD_Task_t memory
     DD_Task_Free( task_head );

} // end DD_TaskList_Remove_Head


/*
 * DD_TaskList_Transfer_Overdue: Cycles through active list, removes overdue tasks and transfers them into the overdue list
 */
void DD_TaskList_Transfer_Overdue( DD_TaskListHandle_t active_list , DD_TaskListHandle_t overdue_list )
{
	// Check input parameters are not NULL
	if( ( active_list == NULL ) || ( overdue_list == NULL ) )
	{
		printf("ERROR(DD_TaskList_Transfer_Overdue): one of the parameters passed was NULL.\n");
		return;
	}

    DD_TaskHandle_t iterator = active_list->list_head; // start from head, closest deadline
    TickType_t current_time = xTaskGetTickCount();     // fetch the current time to check deadline.
    while( iterator != NULL )
    {
        if( iterator->deadline < current_time ) // passed the deadline.
        {
        	// ACTIVE LIST MANAGEMENT
        	// Remove item from active list
        	DD_TaskList_Remove( iterator->task_handle , active_list, false );


            // OVERDUE LIST MANAGEMENT
            // Basic insert algorthm to append value to the end of the list


            if( overdue_list->list_length == 0 )
            {
            	overdue_list->list_length = 1;                  // first item in list, length is 1.
            	overdue_list->list_head = iterator;
            	overdue_list->list_tail = iterator;
            }
            else
            {
                DD_TaskHandle_t temp_swap     = overdue_list->list_tail; // get the current list tail
                overdue_list->list_tail       = iterator;       // make the new tail the task to insert
                temp_swap->next_cell          = iterator;       // set the old tail as the new task's previous cell
                iterator->previous_cell       = temp_swap;      // set the old tail's next cell as the new task

                (overdue_list->list_length)++;                   // increment the list size
            }

            // TASK MANAGEMENT
            // Suspend the task and delete it
            vTaskSuspend( iterator->task_handle );
            vTaskDelete( iterator->task_handle );
            // When the task is removed from the overdue list, it will clear the DD_Task_t data.

        }
        else // EDF active list format - no more deadlines past.
        {
            return;
        }

        iterator = iterator->next_cell;
    }
}



/*
 * DD_TaskList_Formatted_Data: Returns the task name and deadline of each task in a list, formatted in a string buffer.
 */
char * DD_TaskList_Formatted_Data( DD_TaskListHandle_t list )
{
    // Get the size of the list, and assign a buffer size based off the number of elements.
    uint32_t list_size = list->list_length;
    uint32_t size_of_buffer = ( (configMAX_TASK_NAME_LEN + 50) * (list_size + 1)) ;
    char* outputbuffer = (char*)pvPortMalloc(size_of_buffer); // vPortFree in DD_Return_Active_List/DD_Return_Overdue_List
    outputbuffer[0] = '\0'; // ensure that the new buffer doesn't hold an old string

    if(list_size == 0)
    {
    	char buffer[20] = ("List is empty.\n");
    	strcat( outputbuffer, buffer );
    	return outputbuffer;
    }

    DD_TaskHandle_t iterator = list->list_head; // start from head
    while( iterator != NULL )
    {
        char iteration_buffer[70];
        sprintf( iteration_buffer, "Task Name = %s, Deadline = %u \n", iterator->task_name, (unsigned int) iterator->deadline ); // need typecast to unsigned int to avoid warning.
        strcat( outputbuffer, iteration_buffer );

        iterator = iterator->next_cell;         // Go to the next element in the list
    }
    return outputbuffer;
}


