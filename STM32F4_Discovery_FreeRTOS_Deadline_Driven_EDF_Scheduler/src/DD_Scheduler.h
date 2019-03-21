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



static DD_TaskList_t active_list;
static DD_TaskList_t overdue_list;

// Queues for Task_Create and Task_Delete
QueueHandle_t DD_Message = 0;
//QueueHandle_t DD_Create_Message = 0;
//QueueHandle_t DD_Delete_Message = 0;


// Scheduler

void DD_Scheduler();

// Public Helper Functions

TaskHandle_t  DD_Task_Create(DD_TaskHandle_t create_task);
uint32_t      DD_Task_Delete(DD_TaskHandle_t delete_task);
DD_TaskListHandle_t      DD_Return_Active_List();
DD_TaskListHandle_t      DD_Return_Overdue_List();



#endif /* DD_SCHEDULER_H_ */
