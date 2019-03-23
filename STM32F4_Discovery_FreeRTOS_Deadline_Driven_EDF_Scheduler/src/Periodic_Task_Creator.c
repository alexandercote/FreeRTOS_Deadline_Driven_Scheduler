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
		DD_Task_Delete(  );
	}

}

void PeriodicTaskGenerator(DD_TaskHandle_t periodic_task, TickType_t deadline )
{

}
