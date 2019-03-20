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
	newtask->deadline      = 0;
	newtask->creation_time = 0;
	newtask->next_cell     = NULL;
	newtask->previous_cell = NULL;

	return newtask;

}

bool DD_Task_Free(DD_TaskHandle_t task_to_remove)
{
	// return false if the task wasnt removed from active/overdue queue, or removed from active tasks.
	if(task_to_remove->task_handle != NULL || task_to_remove->next_cell != NULL || task_to_remove->previous_cell != NULL)
		return false;

	// remove the task from RTOS kernel management
	vTaskDelete(task_to_remove->task_handle);
	// free the memory used by the task
	vPortFree((void*)task_to_remove);

	return true;
}

// Task List Access Functions
void DD_TaskList_Deadline_Insert(DD_TaskHandle_t task_to_insert, DD_TaskListHandle_t insert_list)
{

}

void DD_TaskList_Remove(DD_TaskHandle_t task_to_remove, DD_TaskListHandle_t remove_list)
{

}
