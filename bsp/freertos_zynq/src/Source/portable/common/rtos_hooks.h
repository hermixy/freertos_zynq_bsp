
#ifndef RTOS_HOOKS_H_
#define RTOS_HOOKS_H_

/* standard includes */
#include <limits.h>

/* freeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* xilinx */
#include "xscutimer.h"
#include "xscugic.h"
#include "xil_exception.h"

void vAssertCalled( const char * pcFile, unsigned long ulLine );

/*
 * Prototypes for the standard FreeRTOS callback/hook functions implemented
 * within this file.
 */
#if(configUSE_HOOKS && configUSE_MALLOC_FAILED_HOOK)
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
#endif

#if(configUSE_HOOKS && configUSE_APPLICATION_TASK_TAG)
#endif

#if(configUSE_HOOKS && configUSE_IDLE_HOOK)
void vApplicationIdleHook( void );
#endif

#if(configUSE_HOOKS && configUSE_TICK_HOOK)
void vApplicationTickHook( void );
void vInitialiseTimerForRunTimeStats( void );
void vRestartWatchDog( void );
#endif

#endif /* RTOS_HOOKS_H_ */

