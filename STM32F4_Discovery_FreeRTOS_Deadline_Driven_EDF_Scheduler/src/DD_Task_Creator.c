/*
 * DD_Task_Creator.c
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#include <DD_Task_Creator.h>

/*
 * LED defines in include.h
 * amber_led    LED3
 * green_led    LED4
 * red_led      LED5
 * blue_led     LED6
 */


/*--------------------------- Periodic Tasks --------------------------------*/

/*
 * PeriodicTask_1
 * Deadline:       5000 ms
 * Execution Time: 2000 ms
 *
 */
void PeriodicTask_1 ( void *pvParameters )
{
	// DD_TaskHandle_t of created task passed in pvParameters
	DD_TaskHandle_t myself = (DD_TaskHandle_t)pvParameters;

	TickType_t current_time;
	TickType_t previous_tick; //Need this to "debounce" the xTaskGetTickCount(), so that you only execute one task per tickcount.
	TickType_t execution_time = 2000 / portTICK_PERIOD_MS;

    while(1)
    {
    	// Get the current time
    	current_time = xTaskGetTickCount();
    	previous_tick = current_time;
    	debugprintf("PeriodicTask_1: Current time = %u. \n", (unsigned int)current_time);

    	// Action perfomed by periodic task
        STM_EVAL_LEDToggle(amber_led);

        // Simulating execution time.
        while( current_time < (myself->creation_time + execution_time) )
        {
        	current_time = xTaskGetTickCount();
        	if( current_time != previous_tick )
        	{
				if( current_time % 100 == 0 )
				{
					STM_EVAL_LEDToggle(amber_led);
				}
        	}
        	previous_tick = current_time;
        }

        STM_EVAL_LEDOff(amber_led);
        TickType_t relative_deadline = myself->deadline - current_time;
        vTaskDelayUntil( &current_time, relative_deadline );

        TaskHandle_t my_task = xTaskGetCurrentTaskHandle();
        DD_Task_Delete( my_task );
    }
} // end PeriodicTask_1


void PeriodicTaskGenerator_1( void *pvParameters )
{
    while (1)
    {
        TickType_t deadline_gen_1 = 5000;
        DD_TaskHandle_t generated_task = DD_Task_Allocate();

        generated_task->task_function = PeriodicTask_1;
        generated_task->task_name     = "PTGen_1_Task";
        generated_task->task_type     = DD_TT_Periodic;

        TickType_t current_time = xTaskGetTickCount();     // fetch the current time to calculate deadline.
        generated_task->creation_time = current_time;
        generated_task->deadline      = current_time + deadline_gen_1;

        DD_Task_Create( generated_task );

        vTaskDelay( 5000 );
    }
}


/*
 * PeriodicTask_1
 * Deadline:       8000 ms
 * Execution Time: 2000 ms
 *
 */
void PeriodicTask_2 ( void *pvParameters )
{
	// DD_TaskHandle_t of created task passed in pvParameters
	DD_TaskHandle_t myself = (DD_TaskHandle_t)pvParameters;

	TickType_t current_time;
	TickType_t previous_tick; //Need this to "debounce" the xTaskGetTickCount(), so that you only execute one task per tickcount.
	TickType_t execution_time = 2000 / portTICK_PERIOD_MS;

    while(1)
    {
    	// Get the current time
    	current_time = xTaskGetTickCount();
    	previous_tick = current_time;
    	debugprintf("PeriodicTask_2: Current time = %u. \n", (unsigned int)current_time);

    	// Action perfomed by periodic task
        STM_EVAL_LEDToggle(green_led);

        // Simulating execution time.
        while( current_time < (myself->creation_time + execution_time) )
        {
        	current_time = xTaskGetTickCount();
        	if( current_time != previous_tick )
        	{
				if( current_time % 250 == 0 )
				{
					STM_EVAL_LEDToggle(green_led);
				}
        	}
        	previous_tick = current_time;
        }

        STM_EVAL_LEDOff(green_led);
        TickType_t relative_deadline = myself->deadline - current_time;
        vTaskDelayUntil( &current_time, relative_deadline );

        TaskHandle_t my_task = xTaskGetCurrentTaskHandle();
        DD_Task_Delete( my_task );
    }
} // end PeriodicTask_2


void PeriodicTaskGenerator_2( void *pvParameters )
{
    while (1)
    {
        TickType_t deadline_gen_2 = 8000;
        DD_TaskHandle_t generated_task = DD_Task_Allocate();

        generated_task->task_function = PeriodicTask_2;
        generated_task->task_name     = "PTGen_2_Task";
        generated_task->task_type     = DD_TT_Periodic;

        TickType_t current_time = xTaskGetTickCount();     // fetch the current time to calculate deadline.
        generated_task->creation_time = current_time;
        generated_task->deadline      = current_time + deadline_gen_2;

        DD_Task_Create( generated_task );

        vTaskDelay( 8000 );
    }
}



/*
 * PeriodicTask_3
 * Deadline:       9000 ms // Wont kill itself at deadline -> force overdue
 * Execution Time: 1000 ms
 */
void PeriodicTask_3 ( void *pvParameters )
{
	// DD_TaskHandle_t of created task passed in pvParameters
	DD_TaskHandle_t myself = (DD_TaskHandle_t)pvParameters;

	TickType_t current_time;
	TickType_t previous_tick; //Need this to "debounce" the xTaskGetTickCount(), so that you only execute one task per tickcount.
	TickType_t execution_time = 1000 / portTICK_PERIOD_MS;

    while(1)
    {
    	// Get the current time
    	current_time = xTaskGetTickCount();
    	previous_tick = current_time;
    	debugprintf("PeriodicTask_3: Current time = %u. \n", (unsigned int)current_time);

    	// Action perfomed by periodic task
        STM_EVAL_LEDToggle(blue_led);

        // Simulating execution time.
        while( current_time < (myself->creation_time + execution_time) )
        {
        	current_time = xTaskGetTickCount();
        	if( current_time != previous_tick )
        	{
				if( current_time % 100 == 0 )
				{
					STM_EVAL_LEDToggle(blue_led);
				}
        	}
        	previous_tick = current_time;
        }
        STM_EVAL_LEDOff(blue_led);
        vTaskDelay( 8500 ); // 9s deadline, 1s execution, 8.5s delay -> force overdue

    }
} // end PeriodicTask_3


void PeriodicTaskGenerator_3( void *pvParameters )
{
    while (1)
    {
        TickType_t deadline_gen_3 = 9000;
        DD_TaskHandle_t generated_task = DD_Task_Allocate();

        generated_task->task_function = PeriodicTask_3;
        generated_task->task_name     = "PTGen_3_Task";
        generated_task->task_type     = DD_TT_Periodic;

        TickType_t current_time = xTaskGetTickCount();     // fetch the current time to calculate deadline.
        generated_task->creation_time = current_time;
        generated_task->deadline      = current_time + deadline_gen_3;

        DD_Task_Create( generated_task );

        vTaskDelay( 9000 );
    }
}






/*--------------------------- Aperiodic Tasks --------------------------------*/

// Push Button Interrupt Handler
void EXTI0_IRQHandler(void) {
    /* Make sure that interrupt flag is set */
    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        // Switch an LED
        //STM_EVAL_LEDToggle(green_led);

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

/*
 * AperiodicTask_1
 * Deadline:       2000 ms
 * Execution Time: 500 ms
 */
void AperiodicTask_1 ( void *pvParameters )
{
	// DD_TaskHandle_t of created task passed in pvParameters
	DD_TaskHandle_t myself = (DD_TaskHandle_t)pvParameters;

	TickType_t current_time;
	TickType_t previous_tick; //Need this to "debounce" the xTaskGetTickCount(), so that you only execute one task per tickcount.
	TickType_t execution_time = 500 / portTICK_PERIOD_MS;

    while(1)
    {
    	// Get the current time
    	current_time = xTaskGetTickCount();
    	previous_tick = current_time;
    	debugprintf("AperiodicTask_1: Current time = %u. \n", (unsigned int)current_time);

    	// Action perfomed by periodic task
        STM_EVAL_LEDToggle(red_led);

        // Simulating execution time.
        while( current_time < (myself->creation_time + execution_time) )
        {
        	current_time = xTaskGetTickCount();
        	if( current_time != previous_tick )
        	{
				if( current_time % 75 == 0 )
				{
					STM_EVAL_LEDToggle(red_led);
				}
        	}
        	previous_tick = current_time;
        }

        STM_EVAL_LEDOff(red_led);
        TickType_t relative_deadline = myself->deadline - current_time;
        vTaskDelayUntil( &current_time, relative_deadline );

        TaskHandle_t my_task = xTaskGetCurrentTaskHandle();
        DD_Task_Delete( my_task );
    }
} // end AperiodicTask_1

void AperiodicTaskGenerator( void *pvParameters )
{
    TickType_t deadline = 500;

    while (1)
    {
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

        printf("Aperiodic task being created!\n");

        DD_TaskHandle_t generated_task = DD_Task_Allocate();

        generated_task->task_function = AperiodicTask_1;
        generated_task->task_name     = "Aperiod_Task_1";
        generated_task->task_type     = DD_TT_Aperiodic;

        TickType_t current_time = xTaskGetTickCount();     // fetch the current time to calculate deadline.
        generated_task->creation_time = current_time;
        generated_task->deadline      = current_time + deadline;

        DD_Task_Create( generated_task );
    }
}

