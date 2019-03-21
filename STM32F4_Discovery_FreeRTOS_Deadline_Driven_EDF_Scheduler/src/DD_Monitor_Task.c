/*
 * Monitor_Tasks.c
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#include "DD_Monitor_Task.h"

extern volatile unsigned long ulHighFrequencyTimerTicks;

// https://www.freertos.org/rtos-run-time-stats.html
// http://blog.atollic.com/visualizing-run-time-statistics-using-freertos

void MonitorTask ( void *pvParameters )
{
	///* Get tasks */
	//uint32_t size = 50*uxTaskGetNumberOfTasks();
	//// if not valid, then just malloc it
	//char buffer[size];
	//vTaskGetRunTimeStats(buffer);
	///* print out tasks */
	//printf("\n%s\n", buffer);
	//vTaskDelay(3000);

	DD_TaskListHandle_t temp_active =  DD_Return_Active_List();
	DD_TaskListHandle_t temp_overdue = DD_Return_Overdue_List();

	TickType_t current_time = xTaskGetTickCount();


}


//void SetupRunTimeStatsTimer(void)
//{
//    /* Set interrupt priority and enable TIMER7 interrupt in NVIC */
//
//    NVIC_SetPriority(TIM7_IRQn, NVIC_PriorityGroup_0);
//    NVIC_EnableIRQ(TIM7_IRQn);
//
//    /* Clock the TIMER7 peripheral */
//
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
//    TIM_TimeBaseInitTypeDef TimBaseStruct;
//    /* Setup a pre-scaler (PSC) and a timer period (ARR) to generate an interrupt at ~20KHz */
//    TimBaseStruct.TIM_Prescaler = 0x1194;
//    TimBaseStruct.TIM_Period = 0x1;
//    TIM_TimeBaseInit(TIM7, &TimBaseStruct);
//    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
//
//    /* Start TIMER7 */
//
//    TIM_Cmd(TIM7, ENABLE);
//}
//
//
//void TIM7_IRQHandler(void)
//{
//    /* Clear the pending bit in NVIC and TIMER7 */
//    NVIC_ClearPendingIRQ(TIM7_IRQn);
//    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
//
//    /* Increment the counter used to measure execution time */
//    ulHighFrequencyTimerTicks++;
//}
