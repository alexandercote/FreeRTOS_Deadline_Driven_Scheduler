/*
 * Monitor_Tasks.h
 *
 *  Created on: Mar 7, 2019
 *      Author: amcote
 */

#ifndef DD_MONITOR_TASK_H_
#define DD_MONITOR_TASK_H_

#include "includes.h"



void MonitorTask ( void *pvParameters );

// Timer7 used for Monitor Task
void SetupRunTimeStatsTimer(void);
void TIM7_IRQHandler(void);

#endif /* DD_MONITOR_TASK_H_ */
