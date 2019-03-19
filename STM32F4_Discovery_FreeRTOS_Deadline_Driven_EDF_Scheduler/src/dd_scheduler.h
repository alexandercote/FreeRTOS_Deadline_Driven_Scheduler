/*
 * dd_scheduler.h
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#ifndef DD_SCHEDULER_H_
#define DD_SCHEDULER_H_


/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stm32f4_discovery.h"

/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"


// Priority Definition

# define DD_TASK_PRIORITY_IDLE      (0)
# define DD_TASK_PRIORITY_MINIMUM   (1)
# define DD_TASK_PRIORITY_MONITOR   (2)
# define DD_TASK_PRIORITY_EXECUTION (configMAX_PRIORITIES - 1)
# define DD_TASK_PRIORITY_SCHEDULER (configMAX_PRIORITIES) // set to the highest priority, defined in FreeRTOSConfig.h


// Struct definitions (defined by requirements)

typedef struct {
	TaskHandle_t    task_handle;
	TaskFunction_t  task_function;
	TickType_t      deadline;
	TickType_t      creation_time;
	struct DD_Task* next_cell;
	struct DD_Task* previous_cell;
} DD_Task;


typedef struct {
	uint_32  list_length;
	DD_Task* list_head;
	DD_Task* list_tail;
} DD_Task_List ;


// Queues for Task_Create and Task_Delete
QueueHandle_t DD_Create_Message = 0;
QueueHandle_t DD_Delete_Message = 0;


// Scheduler

void DD_Scheduler();

// Public Helper Functions

TaskHandle_t DD_Task_Create(DD_Task create_task);
uint_32      DD_Task_Delete(DD_Task delete_task);
uint_32      DD_Return_Active_List(DD_Task_List* list);
uint_32      DD_Return_Overdue_List(DD_Task_List* list);



#endif /* DD_SCHEDULER_H_ */
