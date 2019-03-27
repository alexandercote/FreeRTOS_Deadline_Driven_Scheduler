/*
 * DD_Task_Creator.h
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#ifndef DD_TASK_CREATOR_H_
#define DD_TASK_CREATOR_H_

#include "includes.h"
#include "DD_Scheduler.h"



/*--------------------------- Generator Task Handles --------------------------------*/

TaskHandle_t Periodic_task_gen_handle_1;
TaskHandle_t Aperiodic_task_gen_handle_1;

/*--------------------------- Periodic Tasks --------------------------------*/

void PeriodicTask ( void *pvParameters );
void PeriodicTaskGenerator_1( void *pvParameters );

/*--------------------------- Aperiodic Tasks --------------------------------*/

void EXTI0_IRQHandler(void);
void AperiodicTask( void *pvParameters );
void AperiodicTaskGenerator( void *pvParameters );


#endif /* DD_TASK_CREATOR_H_ */
