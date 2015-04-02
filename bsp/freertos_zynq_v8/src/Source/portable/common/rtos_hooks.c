#include "rtos_hooks.h"

/* please remove DEBUG in release version */
#define DBG_HOOKS ( 1 )

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
volatile unsigned long ul = 0;

    ( void ) pcFile;
    ( void ) ulLine;

#if(DBG_HOOKS)
    xil_printf("(CRITICAL) vAssertCalled triggered\n");
#endif

    taskENTER_CRITICAL();
    {
        /* Set ul to a non-zero value using the debugger to step out of this
        function. */
        while( ul == 0 )
        {
            portNOP();
        }
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

/* enable debug for RTOS hooks
 * can be removed when the code is stable */

#if(configUSE_HOOKS && configUSE_MALLOC_FAILED_HOOK)
void vApplicationMallocFailedHook( void )
{

#if(DBG_HOOKS)
    xil_printf("(CRITICAL) vApplicationMallocFailedHook triggered\n");
#endif

    /* Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
#endif
/*-----------------------------------------------------------*/

#if(configUSE_HOOKS && configUSE_STACK_OVERFLOW_HOOK)
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

#if(DBG_HOOKS)
    xil_printf("(CRITICAL) vApplicationStackOverflowHook triggered\n");
#endif

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();

    for( ;; );
}
#endif
/*-----------------------------------------------------------*/

#if(configUSE_HOOKS && configUSE_IDLE_HOOK)
void vApplicationIdleHook( void )
{
    /* This is just a trivial example of an idle hook.  It is called on each
    cycle of the idle task.  It must *NOT* attempt to block.  In this case the
    idle task just queries the amount of FreeRTOS heap that remains.  See the
    memory management section on the http://www.FreeRTOS.org web site for memory
    management options.  If there is a lot of heap memory free then the
    configTOTAL_HEAP_SIZE value in FreeRTOSConfig.h can be reduced to free up
    RAM. */

    /* We disable this feature now as I do not understand what it's doing.
     * Will it affect the memory overflow of LwIP applications ? */

    //volatile size_t xFreeHeapSpace;
    //xFreeHeapSpace = xPortGetFreeHeapSize();

    /* Remove compiler warning about xFreeHeapSpace being set but never used. */
    //( void ) xFreeHeapSpace;
}
#endif
/*-----------------------------------------------------------*/

#if(configUSE_HOOKS && configUSE_TICK_HOOK)
void vApplicationTickHook( void ){
    ;
}
#endif
/*-----------------------------------------------------------*/

#if(configUSE_HOOKS && configUSE_TICK_HOOK)
void vInitialiseTimerForRunTimeStats( void )
{
extern XScuWdt xWatchDogInstance;
XScuWdt_Config *pxWatchDogInstance;
uint32_t ulValue;
const uint32_t ulMaxDivisor = 0xff, ulDivisorShift = 0x08;

    /*
     * Initialize the SCU Private Wdt driver so that it is ready to use.
     */
    pxWatchDogInstance = XScuWdt_LookupConfig( XPAR_SCUWDT_0_DEVICE_ID );

    /*
     * This is where the virtual address would be used, this example
     * uses physical address.
     */
    XScuWdt_CfgInitialize( &xWatchDogInstance, pxWatchDogInstance, pxWatchDogInstance->BaseAddr );

    /*
     * set the watchdog timers
     */
    ulValue = XScuWdt_GetControlReg( &xWatchDogInstance );
    ulValue |= ulMaxDivisor << ulDivisorShift;
    XScuWdt_SetControlReg( &xWatchDogInstance, ulValue );

    /*
     * Load the watchdog counter register, and
     * Put the watchdof in Timer mode.
     */
    XScuWdt_LoadWdt( &xWatchDogInstance, UINT_MAX );
    XScuWdt_SetTimerMode( &xWatchDogInstance );

    /*
     * Start the ScuWdt device.
     */
    XScuWdt_Start( &xWatchDogInstance );
}
#endif
/*-----------------------------------------------------------*/

#if(configUSE_HOOKS && configUSE_TICK_HOOK)
void vRestartWatchDog( void )
{
    extern XScuWdt xWatchDogInstance;
    XScuWdt_RestartWdt(&xWatchDogInstance);
}
#endif
/*-----------------------------------------------------------*/

