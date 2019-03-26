/*
 * Periodic_Task_Creator.c
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#include "Periodic_Task_Creator.h"

// task function
void PeriodicTask ( void *pvParameters )
{

	vTaskDelay(200);
	while(1)
	{
		TaskHandle_t my_task = xTaskGetCurrentTaskHandle();
		DD_Task_Delete( my_task );
	}

}

// Prob should  change this to pass in parameters to generate or something, but this works for one task.
void PeriodicTaskGenerator_1( void *pvParameters )
{
	TickType_t deadline_gen_1 = 500;
	while (1)
	{
		DD_TaskHandle_t generated_task = DD_Task_Allocate();

		generated_task->task_function = PeriodicTask;
		generated_task->task_name     = "PTGen_1_Task";

		TickType_t current_time = xTaskGetTickCount();     // fetch the current time to calculate deadline.

		generated_task->creation_time = current_time;
		generated_task->deadline      = current_time + deadline_gen_1;

		DD_Task_Create( generated_task );

		vTaskDelay( deadline_gen_1 );
	}
}

