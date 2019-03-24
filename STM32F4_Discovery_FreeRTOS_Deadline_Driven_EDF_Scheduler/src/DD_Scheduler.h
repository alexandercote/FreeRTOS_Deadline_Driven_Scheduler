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



/*--------------------------- DD Scheduler Message --------------------------------*/

// Message types
typedef enum DD_Message_Type_t
{
	DD_Message_Create,
	DD_Message_Delete,
	DD_Message_ActiveList,
	DD_Message_OverdueList,
}DD_Message_Type_t;

// Message structure
typedef struct DD_Message_t
{
	DD_Message_Type_t message_type;
	TaskHandle_t      message_sender; // Maybe send it a DD_TaskHandle_t? -> cant since wont have it for delete.
	void*             message_data;
}DD_Message_t;


/*--------------------------- DD Scheduler --------------------------------*/

void DD_Scheduler();

/*--------------------------- DD Scheduler Public Access Functions --------------------------------*/

TaskHandle_t        DD_Task_Create(DD_TaskHandle_t create_task);
uint32_t            DD_Task_Delete(TaskHandle_t delete_task);
DD_TaskListHandle_t DD_Return_Active_List();
DD_TaskListHandle_t DD_Return_Overdue_List();


#endif /* DD_SCHEDULER_H_ */
