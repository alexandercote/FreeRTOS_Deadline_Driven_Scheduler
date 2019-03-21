
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


#include "DD_Scheduler.h"
#include "DD_Monitor_Task.h"

volatile unsigned long ulHighFrequencyTimerTicks = 0;
// Initialization declaration
void HardwareInit(void);


/*-----------------------------------------------------------*/

int main(void)
{
	HardwareInit(); // Initialize the GPIO and ADC


	xTaskCreate( MonitorTask , "Monitoring Task",configMINIMAL_STACK_SIZE ,NULL ,DD_TASK_PRIORITY_MONITOR,   NULL);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return 0;
} // end main
/*-----------------------------------------------------------*/


void HardwareInit()
{ // Initializes GPIO and ADC

	/* Ensure all priority bits are assigned as preemption priority bits.
	http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	NVIC_SetPriorityGrouping( 0 );

	/*
	// 1. Init GPIO
	GPIO_InitTypeDef      SHIFT_1_GPIO_InitStructure;
	GPIO_InitTypeDef      SHIFT_2_GPIO_InitStructure;
	GPIO_InitTypeDef      TRAFFIC_GPIO_InitStructure;

	// Enable all GPIO clocks for GPIO, reduce potential of missing one in future updates.
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    SHIFT_1_GPIO_InitStructure.GPIO_Pin   = SHIFT_REG_1_PIN | SHIFT_REG_CLK_1_PIN;   // Shift register 1 output and clock set on same unique GPIO port.
    SHIFT_1_GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;                           // Set output mode
    SHIFT_1_GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                           // Set push-pull mode
    SHIFT_1_GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;                        // Disable pull-ups / pull-downs
    SHIFT_1_GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;                        // Set higher speed to allow quick shifting refresh for shift register (Max for shift register itself is 25Mhz)
    GPIO_Init(SHIFT_REG_1_PORT, &SHIFT_1_GPIO_InitStructure);

    SHIFT_2_GPIO_InitStructure.GPIO_Pin   = SHIFT_REG_2_PIN | SHIFT_REG_CLK_2_PIN;   // Shift register 2 output and clock set on same unique GPIO port.
    SHIFT_2_GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;                           // Set output mode
    SHIFT_2_GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                           // Set push-pull mode
    SHIFT_2_GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;                        // Disable pull-ups / pull-downs
    GPIO_Init(SHIFT_REG_2_PORT, &SHIFT_2_GPIO_InitStructure);

    TRAFFIC_GPIO_InitStructure.GPIO_Pin   = TRAFFIC_LIGHT_RED_PIN | TRAFFIC_LIGHT_YELLOW_PIN | TRAFFIC_LIGHT_GREEN_PIN;    // Traffic light GPIO same unique GPIO port.
    TRAFFIC_GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;                                                                 // Set output mode
    TRAFFIC_GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                                                                 // Set push-pull mode
    TRAFFIC_GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;                                                              // Disable pull-ups / pull-downs
    GPIO_Init(TRAFFIC_LIGHT_PORT, &TRAFFIC_GPIO_InitStructure);


	// 2. Init ADC
	ADC_InitTypeDef       ADC_InitStructure;
	GPIO_InitTypeDef      ADC_GPIO_InitStructure;

	// Enable GPIO and ADC clocks for ADC
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // Configure ADC1 Channel11 pin as analog input
    ADC_GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;                              // Using PC1, channel 11 of ADC
    ADC_GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;                            // Set analog mode
    ADC_GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;                        // Set push-pull mode
    GPIO_Init(GPIOC, &ADC_GPIO_InitStructure);

    // ADC1 Init
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      // Set ADC for 12 bit resolution (highest)
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                          // Enable continuous scanning for ADC
    ADC_InitStructure.ADC_ExternalTrigConv = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;                                  // Perform a single conversion when start conversion is called
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE );                                                     // Enable ADC1
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11 , 1, ADC_SampleTime_84Cycles);
    */
} // end HardwareInit




void vApplicationMallocFailedHook( void )
{
	/* The malloc failed hook is enabled by setting
	configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

	Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software 
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.  pxCurrentTCB can be
	inspected in the debugger if the task name passed into this function is
	corrupt. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
	FreeRTOSConfig.h.

	This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amount of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}
/*-----------------------------------------------------------*/

