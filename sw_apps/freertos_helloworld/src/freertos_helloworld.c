/*
 * main() do some house keeping job to make the whole program run.
 * It initialises two global control : GIC and WDT, for other program.
 *
 * A software timer is used to print the message "hello, world"
 * every 500ms.
 *
 * liu_benyuan <liubenyuan@gmail.com>
 */

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* XILINX includes. */
#include "xparameters.h"
#include "xscutimer.h"
#include "xscugic.h"

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

/*
 * The XILINX projects use a BSP that do not allow the start up code to be
 * altered easily. Therefore the vector table used by FreeRTOS is defined in
 * FreeRTOS_asm_vectors.S, which is part of this project.  Switch to use the
 * FreeRTOS vector table.
 */
extern void vPortInstallFreeRTOSVectorTable( void );

/*
 * The private watchdog is used as the 'timer' that generates run time
 * stats. This frequency means it will overflow quite quickly.
 */
XScuWdt xWatchDogInstance;

/*-----------------------------------------------------------*/

/*
 * The interrupt controller is initialized in this file, and made available to
 * other modules.
 */
XScuGic xInterruptController;

/*-----------------------------------------------------------*/

/*
 * The rate at which data is sent to the queue. (software timer)
 * The 500ms value is converted to ticks using the
 * portTICK_PERIOD_MS constant.
 */
#define mainTIMER_PERIOD_MS         ( 500 / portTICK_PERIOD_MS )

/* The LED toggled by the RX task. */
#define mainTIMER_LED               ( 0 )

/* A block time of zero just means "don't block". */
#define mainDONT_BLOCK              ( 0 )
/*-----------------------------------------------------------*/

/*
 * The callback for the timer that just toggles an LED to show the system is
 * running.
 */
static void prvLEDToggleTimer( TimerHandle_t pxTimer );
/*-----------------------------------------------------------*/

int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    /* A timer is used to toggle an LED just to show the application is
    executing. */
    TimerHandle_t xTimer;
    xTimer = xTimerCreate( "LED",                     /* Text name to make debugging easier. */
                           mainTIMER_PERIOD_MS,       /* The timer's period. */
                           pdTRUE,                    /* This is an auto reload timer. */
                           NULL,                      /* ID is not used. */
                           prvLEDToggleTimer );       /* The callback function. */

    /*
     * Start the timer. (NOTE : this timer is a software timer)
     */
    configASSERT( xTimer );
    xTimerStart( xTimer, mainDONT_BLOCK );

    /*
     * Fire & Run. Start the tasks and timer running.
     */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
    line will never be reached.  If the following line does execute, then
    there was either insufficient FreeRTOS heap memory available for the idle
    and/or timer tasks to be created, or vTaskStartScheduler() was called from
    User mode.  See the memory management section on the FreeRTOS web site for
    more details on the FreeRTOS heap http://www.freertos.org/a00111.html.  The
    mode from which main() is called is set in the C start up code and must be
    a privileged mode (not user mode). */
    for( ;; );
}
/*-----------------------------------------------------------*/

static void prvLEDToggleTimer( TimerHandle_t pxTimer )
{
    /* Prevent compiler warnings. */
    ( void ) pxTimer;

    /*
     * Just do something (i.e., toggle an LED)
     * to show the application is running.
     */
    xil_printf("hello, world\n");

}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
BaseType_t xStatus;
XScuGic_Config *pxGICConfig;

    /*
     * Ensure no interrupts execute while the scheduler is in an inconsistent
     * state.  Interrupts are automatically enabled when the scheduler is
     * started.
     */
    portDISABLE_INTERRUPTS();

    /* Obtain the configuration of the GIC. */
    pxGICConfig = XScuGic_LookupConfig( XPAR_SCUGIC_SINGLE_DEVICE_ID );

    /*
     * Sanity check the FreeRTOSConfig.h settings are correct for the hardware.
     */
    configASSERT( pxGICConfig );
    configASSERT( pxGICConfig->CpuBaseAddress == ( configINTERRUPT_CONTROLLER_BASE_ADDRESS + configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET ) );
    configASSERT( pxGICConfig->DistBaseAddress == configINTERRUPT_CONTROLLER_BASE_ADDRESS );

    /* Install a default handler for each GIC interrupt. */
    xStatus = XScuGic_CfgInitialize( &xInterruptController, pxGICConfig, pxGICConfig->CpuBaseAddress );
    configASSERT( xStatus == XST_SUCCESS );

    /* Remove compiler warning if configASSERT() is not defined. */
    ( void ) xStatus;

    /*
     * The interrupt table will be set up automatically
     * once the task starts.
     *
     * DO NOT use 'Xil_ExceptionEnable()', as that enables interrupts
     * which will do bad things if interrupt triggered before
     * vTaskStartScheduler()
     */

    /*
     * The XILINX projects use a BSP that do not allow the start up code to be
     * altered easily.  Therefore the vector table used by FreeRTOS is defined in
     * FreeRTOS_asm_vectors.S, which is part of this project.  Switch to use the
     * FreeRTOS vector table.
     */
    vPortInstallFreeRTOSVectorTable();

}
/*-----------------------------------------------------------*/