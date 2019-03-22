/*
 * DD_Task_List.c
 *
 *  Created on: Mar 20, 2019
 *      Author: amcote
 */

#include "DD_Task_List.h"

// Task Creation/Deletion (List Elements)
DD_TaskHandle_t DD_Task_Allocate()
{
	DD_TaskHandle_t newtask = (DD_TaskHandle_t)pvPortMalloc(sizeof(DD_Task_t));

	newtask->task_handle   = NULL;
	newtask->task_function = NULL;
	newtask->task_name     = "";
	newtask->deadline      = 0;
	newtask->creation_time = 0;
	newtask->next_cell     = NULL;
	newtask->previous_cell = NULL;

	return newtask;
}

bool DD_Task_Free(DD_TaskHandle_t task_to_remove)
{
	// return false if the task wasn't removed from active/overdue queue, or removed from active tasks.
	if(task_to_remove->task_handle != NULL || task_to_remove->next_cell != NULL || task_to_remove->previous_cell != NULL)
	{
		printf("ERROR(DD_Task_Free): Forgot to remove task from list, not deleting it. Fix the code.\n");
		return false;
	}

	// free the memory used by the task
	vPortFree((void*)task_to_remove);

	return true;
}


// Init the list structure with empty values.
void DD_TaskList_Init(DD_TaskListHandle_t init_list)
{
	init_list->list_length = 0;
	init_list->list_head   = NULL;
	init_list->list_tail   = NULL;
}


// Task List Access Functions

//DD_TaskList_Basic_Insert -> inserts element at the end

void DD_TaskList_Basic_Insert(DD_TaskHandle_t task_to_insert, DD_TaskListHandle_t insert_list)
{
	if(insert_list->list_length == 0)
		{
			insert_list->list_length = 1;
			insert_list->list_head = task_to_insert;
			insert_list->list_tail = task_to_insert;
		}
	else
	{
		DD_TaskHandle_t temp_swap = insert_list->list_tail; // get the current list tail
		insert_list->list_tail        = task_to_insert;
		temp_swap->next_cell          = insert_list;
		task_to_insert->previous_cell = temp_swap;
	}
}

// DD_TaskList_Deadline_Insert -> inserts a task into the active list based off priority.

// Priority Management:
// From head of the list, increment priority until the insertion location is reached.
void DD_TaskList_Deadline_Insert(DD_TaskHandle_t task_to_insert, DD_TaskListHandle_t insert_list)
{
	// Step 1: check if list is empty, else place element according to its deadline
	if(insert_list->list_length == 0)
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

	if((itr_priority + 1) == DD_TASK_PRIORITY_SCHEDULER)              // reached the highest level of priority
	{
		printf("ERROR: REACHED LIMIT OF NUMBER OF SCHEDULABLE TASKS! NOT INSERTING TASK");
		return;
	}

	itr_priority++; //increment the priority, set the new priority until the new task is reached.

	while( iterator != NULL )
	{
		if( task_to_insert->deadline < iterator->deadline ) //found location in list.
		{
			if( iterator == insert_list->list_head) // new element has earliest deadline, current head is getting replaced
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
			if(iterator->next_cell == insert_list->list_tail) // reached the end of the list, insert at end.
			{
				task_to_insert->next_cell     = NULL;
				task_to_insert->previous_cell = iterator;
				iterator->next_cell           = task_to_insert;
				insert_list->list_tail        = task_to_insert;

				(insert_list->list_length)++; // increment the list size
				vTaskPrioritySet(task_to_insert->task_handle, DD_TASK_PRIORITY_EXECUTION_BASE);
				return; // no need to continue the loop, even though its about to end.
			}

			//Current task has closer deadline than new task, and isn't the tail. Decrement priority.
			vTaskPrioritySet(iterator->task_handle, itr_priority );
			itr_priority--;
			iterator = iterator->next_cell; //keep going through the list
		} // end if/else deadline comparison
	} // end while
} // end DD_TaskList_Deadline_Insert


// DD_TaskList_Remove -> removes a task given its handle

// Priority Management:
// From head of the list, decrement priority until the insertion location is reached.
void DD_TaskList_Remove(DD_TaskHandle_t task_to_remove, DD_TaskListHandle_t remove_list)
{
	if(remove_list->list_length == 0)
	{
		printf("ERROR(DD_TaskList_Remove): trying to remove a task from an empty list...\n");
		return;
	}

	DD_TaskHandle_t iterator = remove_list->list_head; // start from head, closest deadline
	uint32_t itr_priority = uxTaskPriorityGet(iterator->task_handle); // grab the highest priority value
	itr_priority--;

	while( iterator != NULL )
	{
		if(iterator == task_to_remove) // found the task to remove
		{
			DD_TaskHandle_t prev_task = task_to_remove->previous_cell;
			DD_TaskHandle_t next_task = task_to_remove->next_cell;

			if(prev_task == NULL) // OR if(task_to_remove == remove_list->list_head)
				remove_list->list_head = next_task;

			if(next_task == NULL) // OR if(task_to_remove == remove_list->list_tail)
				remove_list->list_tail = prev_task;

			prev_task->next_cell     = next_task;
			next_task->previous_cell = prev_task;

			(remove_list->list_length)--; // decrement the list size
			return; // done with the removal
		}
		//Current isnt the one to remove. Decrement priority.
		vTaskPrioritySet(iterator->task_handle, itr_priority );
		itr_priority--;
	}
}

// goes through active list, removes overdue tasks, adds them to overdue list
void DD_TaskList_Transfer_Overdue(DD_TaskListHandle_t active_list, DD_TaskListHandle_t overdue_list)
{
	DD_TaskHandle_t iterator = active_list->list_head; // start from head, closest deadline
	TickType_t current_time = xTaskGetTickCount();     // fetch the current time to check deadline.
	while( iterator != NULL )
	{
		if(iterator->deadline < current_time) // passed the deadline.
		{
			DD_TaskList_Remove(iterator, active_list);
		}
	}
}
