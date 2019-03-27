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

void PeriodicTask ( void *pvParameters );
void PeriodicTaskGenerator_1( void *pvParameters );
void AperiodicTask( void *pvParameters );
void AperiodicTaskGenerator( void *pvParameters );

#endif /* DD_TASK_CREATOR_H_ */
