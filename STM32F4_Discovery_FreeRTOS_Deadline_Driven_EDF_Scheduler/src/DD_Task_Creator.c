/*
 * DD_Task_Creator.c
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#include <DD_Task_Creator.h>

/*
 * LED defines in include.h
 * amber_led	LED3
 * green_led	LED4
 * red_led		LED5
 * blue_led	    LED6
 */


/*--------------------------- Periodic Tasks --------------------------------*/

// task function
void PeriodicTask ( void *pvParameters )
{
	while(1)
	{
		STM_EVAL_LEDToggle(amber_led);
		vTaskDelay(200);
		TaskHandle_t my_task = xTaskGetCurrentTaskHandle();
		DD_Task_Delete( my_task );
	}
}


void PeriodicTaskGenerator_1( void *pvParameters )
{
	while (1)
	{
		// Toggle the blue LED to know that the periodic task generator is running
		//STM_EVAL_LEDToggle(blue_led);

		printf("Generating tasks!\n");
		TickType_t deadline_gen_1 = 500;
		DD_TaskHandle_t generated_task = DD_Task_Allocate();

		generated_task->task_function = PeriodicTask;
		generated_task->task_name     = "PTGen_1_Task";
		generated_task->task_type     = DD_TT_Periodic;

		TickType_t current_time = xTaskGetTickCount();     // fetch the current time to calculate deadline.
		generated_task->creation_time = current_time;
		generated_task->deadline      = current_time + deadline_gen_1;

		DD_Task_Create( generated_task );

		vTaskDelay( 500 );
	}
}

/*--------------------------- Aperiodic Tasks --------------------------------*/

// Push Button Interrupt Handler
void EXTI0_IRQHandler(void) {
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
    	// Switch an LED
    	STM_EVAL_LEDToggle(green_led);

    	// The rest of this stuff is interrupt management.

    	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    	/* Notify the task that the transmission is complete. */
    	vTaskNotifyGiveFromISR( Aperiodic_task_gen_handle_1 , &xHigherPriorityTaskWoken );

        /* Clear interrupt flag (Want to do this as late as possible to avoid triggering the IRQ in the IRQ) */
        EXTI_ClearITPendingBit(EXTI_Line0);

        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
         should be performed to ensure the interrupt returns directly to the highest
         priority task.  The macro used for this purpose is dependent on the port in
         use and may be called portEND_SWITCHING_ISR(). */
    	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

void AperiodicTask ( void *pvParameters )
{
	while(1)
	{
		STM_EVAL_LEDToggle(red_led);
		TaskHandle_t my_task = xTaskGetCurrentTaskHandle();
		DD_Task_Delete( my_task );
	}
}

void AperiodicTaskGenerator( void *pvParameters )
{
	TickType_t deadline = 500;

	while (1)
	{
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

		printf("Aperiodic task being created!\n");

		DD_TaskHandle_t generated_task = DD_Task_Allocate();

		generated_task->task_function = AperiodicTask;
		generated_task->task_name     = "Aperiod_Task_1";
		generated_task->task_type     = DD_TT_Aperiodic;

		TickType_t current_time = xTaskGetTickCount();     // fetch the current time to calculate deadline.
		generated_task->creation_time = current_time;
		generated_task->deadline      = current_time + deadline;

		DD_Task_Create( generated_task );
	}
}

