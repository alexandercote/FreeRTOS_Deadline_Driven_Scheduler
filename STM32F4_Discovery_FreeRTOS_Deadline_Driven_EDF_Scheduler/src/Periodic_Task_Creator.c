/*
 * Periodic_Task_Creator.c
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#include "Periodic_Task_Creator.h"

// task function
void PeriodicTask (void *pvParameters)
{

	vTaskDelay(200);
	while(1)
	{
		TaskHandle_t my_task = xTaskGetCurrentTaskHandle();
		DD_Task_Delete( my_task );
	}

}

void PeriodicTaskGenerator(DD_TaskHandle_t periodic_task, TickType_t deadline )
{

}
