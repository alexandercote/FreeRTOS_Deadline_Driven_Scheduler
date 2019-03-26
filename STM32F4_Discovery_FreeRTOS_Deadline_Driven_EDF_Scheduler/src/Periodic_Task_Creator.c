/*
 * Periodic_Task_Creator.c
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#include "Periodic_Task_Creator.h"

/*
 * LED defines in include.h
 * amber_led	LED3
 * green_led	LED4
 * red_led		LED5
 * blue_led	    LED6
 */

// task function
void PeriodicTask ( void *pvParameters )
{
	while(1)
	{
		STM_EVAL_LEDToggle(amber_led);
		vTaskDelay(2000);
		STM_EVAL_LEDToggle(amber_led);
		TaskHandle_t my_task = xTaskGetCurrentTaskHandle();
		DD_Task_Delete( my_task );
	}
}

// Prob should  change this to pass in parameters to generate or something, but this works for one task.
void PeriodicTaskGenerator_1( void *pvParameters )
{
	while (1)
	{
		// Toggle the blue LED to know that the periodic task generator is running
		STM_EVAL_LEDToggle(blue_led);

		printf("Generating tasks!\n");
		TickType_t deadline_gen_1 = 5000;
		DD_TaskHandle_t generated_task = DD_Task_Allocate();

		generated_task->task_function = PeriodicTask;
		generated_task->task_name     = "PTGen_1_Task";

		TickType_t current_time = xTaskGetTickCount();     // fetch the current time to calculate deadline.

		generated_task->creation_time = current_time;
		generated_task->deadline      = current_time + deadline_gen_1;

		DD_Task_Create( generated_task );

		vTaskDelay( 5000 );
	}
}

