# FreeRTOS_Deadline_Driven_Scheduler
Deadline driven scheduler using Earliest Deadline First (EDF) implemented ontop of the FreeRTOS scheduler.

System validation completed using a toy application utilizing on-board LEDs and and the user button to create aperiodic tasks on the STM32F407G Discovery board.

![image](test1.gif)

| Task     | Execution time         | Relative Deadline  | LED Colour   |
| ------------- |:-------------:| -----:| -----:|
| Periodic Task 1 | 95 | 500 | Amber   |
| Periodic Task 2 | 150 |   500 |  Green |
| Periodic Task 3 | 250     |    750 |  Blue  |
| Aperiodic Task 1 | 50     | 200   | Red |

*all times are in ticks set at 1kHz, therefore 1000 ticks = 1 second*

