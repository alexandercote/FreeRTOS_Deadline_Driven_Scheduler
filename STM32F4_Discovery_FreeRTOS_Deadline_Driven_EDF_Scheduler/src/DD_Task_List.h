/*
 * DD_Task_List.h
 *
 *  Created on: Mar 20, 2019
 *      Author: amcote
 */

#ifndef DD_TASK_LIST_H_
#define DD_TASK_LIST_H_

#include "includes.h"

// Priority Definition

# define DD_TASK_PRIORITY_IDLE           (0)
# define DD_TASK_PRIORITY_MINIMUM        (1)
# define DD_TASK_PRIORITY_MONITOR        (2)
# define DD_TASK_PRIORITY_EXECUTION_BASE (3)
# define DD_TASK_PRIORITY_SCHEDULER      (configMAX_PRIORITIES) // set to the highest priority, defined in FreeRTOSConfig.h


typedef struct DD_Task_t
{
	TaskHandle_t      task_handle;
	TaskFunction_t    task_function;
	const char *      task_name;
	TickType_t        deadline;
	TickType_t        creation_time;
	struct DD_Task_t* next_cell;
	struct DD_Task_t* previous_cell;
} DD_Task_t;

typedef DD_Task_t* DD_TaskHandle_t;

typedef struct DD_TaskList_t
{
	uint32_t        list_length;
	DD_TaskHandle_t list_head;
	DD_TaskHandle_t list_tail;
} DD_TaskList_t;

typedef DD_TaskList_t* DD_TaskListHandle_t;


// Task Creation/Deletion (List Elements)
DD_TaskHandle_t DD_Task_Allocate();
bool DD_Task_Free(DD_TaskHandle_t task_to_remove);

// Task List Access Functions
void DD_TaskList_Init(DD_TaskListHandle_t init_list);
void DD_TaskList_Basic_Insert(DD_TaskHandle_t task_to_insert, DD_TaskListHandle_t insert_list);
void DD_TaskList_Deadline_Insert(DD_TaskHandle_t task_to_insert, DD_TaskListHandle_t insert_list);
void DD_TaskList_Remove(DD_TaskHandle_t task_to_remove, DD_TaskListHandle_t remove_list);
void DD_TaskList_Transfer_Overdue(DD_TaskListHandle_t active_list, DD_TaskListHandle_t overdue_list);

#endif /* DD_TASK_LIST_H_ */
