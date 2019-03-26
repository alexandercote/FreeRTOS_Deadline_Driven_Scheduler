/*
 * DD_Task_List.h
 *
 *  Created on: Mar 20, 2019
 *      Author: amcote
 */

#ifndef DD_TASK_LIST_H_
#define DD_TASK_LIST_H_

#include "includes.h"

/*--------------------------- Task Priority Definitions --------------------------------*/

# define DD_TASK_PRIORITY_IDLE           (0)
# define DD_TASK_PRIORITY_MINIMUM        (1)
# define DD_TASK_PRIORITY_MONITOR        (2)
# define DD_TASK_PRIORITY_GENERATOR      (3)
# define DD_TASK_PRIORITY_EXECUTION_BASE (4)
# define DD_TASK_PRIORITY_SCHEDULER      (configMAX_PRIORITIES) // set to the highest priority, defined in FreeRTOSConfig.h

# define DD_TASK_RANGE (DD_TASK_PRIORITY_SCHEDULER - 1 - DD_TASK_PRIORITY_EXECUTION_BASE) // Number of tasks allowed

/*--------------------------- Task Element Definition --------------------------------*/

typedef enum DD_Task_Type_t
{
	DD_TT_Undefined,
	DD_TT_Periodic,
	DD_TT_Aperiodic
}DD_Task_Type_t;


typedef struct DD_Task_t
{
	TaskHandle_t      task_handle;
	TaskFunction_t    task_function;
	const char *      task_name;
	DD_Task_Type_t    task_type;
	TickType_t        creation_time;
	TickType_t        deadline;
	struct DD_Task_t* next_cell;
	struct DD_Task_t* previous_cell;
} DD_Task_t;

typedef DD_Task_t* DD_TaskHandle_t;


/*--------------------------- Task List Definition --------------------------------*/

typedef struct DD_TaskList_t
{
	uint32_t        list_length;
	DD_TaskHandle_t list_head;
	DD_TaskHandle_t list_tail;
} DD_TaskList_t;

typedef DD_TaskList_t* DD_TaskListHandle_t;


/*--------------------------- Task Creation/Deletion (List Elements) --------------------------------*/

// Creates a DD_TaskHandle_t
DD_TaskHandle_t DD_Task_Allocate();
// Frees the memory allocated from DD_Task_Allocate()
bool DD_Task_Free( DD_TaskHandle_t task_to_remove );


/*--------------------------- Task Initialization --------------------------------*/

// Initialize list with 0 values.
void DD_TaskList_Init( DD_TaskListHandle_t init_list );


/*--------------------------- Task List Access Functions --------------------------------*/

// Insert element at end of the list.
void DD_TaskList_Basic_Insert( DD_TaskHandle_t task_to_insert , DD_TaskListHandle_t insert_list );

// Insert element in ordered list based off deadline.
void DD_TaskList_Deadline_Insert( DD_TaskHandle_t task_to_insert , DD_TaskListHandle_t insert_list );

// Remove item from list given its DD_TaskHandle_t
void DD_TaskList_Remove( TaskHandle_t task_to_remove , DD_TaskListHandle_t remove_list );

// Check if task exists in list
bool DD_TaskList_Task_Exists( DD_TaskHandle_t task , DD_TaskListHandle_t list );

// Removes overdue tasks from active list, adds them to the overdue list.
void DD_TaskList_Transfer_Overdue( DD_TaskListHandle_t active_list , DD_TaskListHandle_t overdue_list );

// Returns a formatted string of task names and deadline in a list.
char * DD_TaskList_Formatted_Data( DD_TaskListHandle_t list );

// Return the DD_TaskHandle_t given a TaskHandle_t
DD_TaskHandle_t DD_TaskList_Get_DD_TaskHandle_t( TaskHandle_t task , DD_TaskListHandle_t list );


#endif /* DD_TASK_LIST_H_ */
