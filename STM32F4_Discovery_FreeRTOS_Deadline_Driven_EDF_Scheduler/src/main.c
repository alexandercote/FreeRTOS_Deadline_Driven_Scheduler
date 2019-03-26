
/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#include "FreeRTOSHooks.h"
#include "DD_Scheduler.h"
#include "Periodic_Task_Creator.h"


// Initialization declaration
void HardwareInit( void );
void List_Tests( void );

/*-----------------------------------------------------------*/

int main(void)
{
	HardwareInit(); // Initialize the GPIO and ADC

	DD_Scheduler_Init();

	xTaskCreate( PeriodicTaskGenerator_1 , "Generator Task"                 , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_GENERATOR , NULL);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return 0;
} // end main
/*-----------------------------------------------------------*/



// Tests the insert and delete functions.
// Attempts to moc the DD_Scheduler functionality.
void List_Tests()
{
	DD_TaskList_t active_list;
	DD_TaskList_Init( &active_list );

    DD_Scheduler_Init();

	DD_TaskHandle_t task_1 = DD_Task_Allocate();
	task_1->task_function = PeriodicTask;
	task_1->task_name     = "task uno";
	task_1->deadline      = (TickType_t) 300;
	xTaskCreate( task_1->task_function , task_1->task_name , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_MINIMUM , &(task_1->task_handle));
	DD_TaskHandle_t task_2 = DD_Task_Allocate();
	task_2->task_function = PeriodicTask;
	task_2->task_name     = "task dos";
	task_2->deadline      = (TickType_t) 200;
	xTaskCreate( task_2->task_function , task_2->task_name , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_MINIMUM , &(task_2->task_handle));
	DD_TaskHandle_t task_3 = DD_Task_Allocate();
	task_3->task_function = PeriodicTask;
	task_3->task_name     = "task tres";
	task_3->deadline      = (TickType_t) 100;
	xTaskCreate( task_3->task_function , task_3->task_name , configMINIMAL_STACK_SIZE , NULL , DD_TASK_PRIORITY_MINIMUM , &(task_3->task_handle));

	/* Start the tasks and timer running. */
	//vTaskStartScheduler();

    DD_TaskList_Deadline_Insert( task_1 , &active_list );
    DD_TaskList_Deadline_Insert( task_2 , &active_list );
    DD_TaskList_Deadline_Insert( task_3 , &active_list );

    vTaskStartScheduler();



    DD_TaskList_Remove( task_1->task_handle , &active_list );
    vTaskDelete( task_1->task_handle );
    DD_Task_Free(task_1);
    // Attempt to use task_1 again, should fail.


    DD_TaskList_Remove( task_2->task_handle , &active_list );
    vTaskDelete( task_2->task_handle );
    DD_Task_Free(task_2);
    DD_Task_Free(task_3); // check removal safety verification
    DD_TaskList_Remove( task_3->task_handle , &active_list );
    vTaskDelete( task_3->task_handle );
    DD_Task_Free(task_3);

	return;
}



void HardwareInit()
{ // Initializes GPIO and ADC

	/* Ensure all priority bits are assigned as preemption priority bits.
	http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	NVIC_SetPriorityGrouping( 0 );

	/* Initialize LEDs */
	STM_EVAL_LEDInit(amber_led);
	STM_EVAL_LEDInit(green_led);
	STM_EVAL_LEDInit(red_led);
	STM_EVAL_LEDInit(blue_led);

	// Initialize the pushbutton (either GPIO: BUTTON_MODE_GPIO or external interrupt: BUTTON_MODE_EXTI)
	STM_EVAL_PBInit( BUTTON_USER, BUTTON_MODE_GPIO );
	NVIC_SetPriority( USER_BUTTON_EXTI_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 ); // Must be above configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY


} // end HardwareInit






