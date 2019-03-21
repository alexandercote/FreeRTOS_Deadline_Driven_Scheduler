/*
 * dd_scheduler.h
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#ifndef DD_SCHEDULER_H_
#define DD_SCHEDULER_H_


#include "includes.h"
#include "DD_Task_List.h"


// Priority Definition

# define DD_TASK_PRIORITY_IDLE      (0)
# define DD_TASK_PRIORITY_MONITOR   (1)
# define DD_TASK_PRIORITY_MINIMUM   (2)
# define DD_TASK_PRIORITY_EXECUTION (configMAX_PRIORITIES - 1)
# define DD_TASK_PRIORITY_SCHEDULER (configMAX_PRIORITIES) // set to the highest priority, defined in FreeRTOSConfig.h


static DD_TaskList_t active_list;
static DD_TaskList_t overdue_list;

// Queues for Task_Create and Task_Delete
QueueHandle_t DD_Message = 0;
//QueueHandle_t DD_Create_Message = 0;
//QueueHandle_t DD_Delete_Message = 0;


// Scheduler

void DD_Scheduler();

// Public Helper Functions

TaskHandle_t DD_Task_Create(DD_Task_t create_task);
uint_32      DD_Task_Delete(DD_Task_t delete_task);
uint_32      DD_Return_Active_List(DD_Task_List* list);
uint_32      DD_Return_Overdue_List(DD_Task_List* list);



#endif /* DD_SCHEDULER_H_ */
