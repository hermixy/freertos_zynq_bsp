/*
 * main() do some house keeping job to make the whole program run.
 * It initialises two global control : GIC and WDT, for other program.
 *
 * A AXI timer is used to print the message "hello, world"
 * every 500ms.
 *
 * liu_benyuan <liubenyuan@gmail.com>
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "freertos_axitimer.h"
#include "freertos_gpio.h"

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

/* Handles of the test tasks that must be accessed from other test tasks. */
static TaskHandle_t xSlaveHandle;

/* TIMER 1 */
XTmrCtr xTimer;
static SemaphoreHandle_t xTimerSemaphore = NULL;
void TimerISR(void *CallBackRef, u8 TmrCtrNumber);
static void TimerTask(void *pvTask);

const u16 TimerID = XPAR_AXI_TIMER_0_DEVICE_ID;
const u16 TimerINTR = XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR;
const BaseType_t TimerPRIORITY = ( configMAX_API_CALL_INTERRUPT_PRIORITY + 1 );
const BaseType_t intsemTIMER_PRIORITY = ( tskIDLE_PRIORITY + 1 );
const u32 TimerCnt = 50000000UL;

/*-----------------------------------------------------------*/

int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    /* Create the semaphores that are given from an interrupt. */
    xTimerSemaphore = xSemaphoreCreateBinary();
    configASSERT( xTimerSemaphore );

    /* create the task */
    xTaskCreate(TimerTask,
                "A-Timer",
                configMINIMAL_STACK_SIZE,
                NULL,
                intsemTIMER_PRIORITY,
                &xSlaveHandle );

    /* RTOS way */
    AXITimer_Init(&xTimer, TimerID);
    AXITimer_OptSet(&xTimer, 0, XTC_DOWN_COUNT_OPTION  |
                                XTC_AUTO_RELOAD_OPTION |
                                XTC_INT_MODE_OPTION);
    AXITimer_LoadSet(&xTimer, 0, TimerCnt);
    AXITimer_CallbackSet(&xTimer, 0, TimerINTR, TimerISR);
    AXITimer_Start(&xTimer, 0);

    /* initialise GPIO */
    GPIO_Init();

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

/* pending for Timer0 semaphore */
static void TimerTask(void *pvTask)
{
const TickType_t TimerBlock = 200 * portTICK_PERIOD_MS;

    while(1) {
        if( xSemaphoreTake( xTimerSemaphore, ( TickType_t ) TimerBlock ) == pdPASS ) {
            GPIO_Toggle( partstLED_OUTPUT );
        }
    }

}
/*-----------------------------------------------------------*/

/* post semaphore when INIT */
void TimerISR(void *CallBackRef, u8 TmrCtrNumber)
{
u32 ControlStatusReg;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* mask interrupt */
    ControlStatusReg = XTmrCtr_ReadReg(xTimer.BaseAddress,
                                       0,
                                       XTC_TCSR_OFFSET);

    XTmrCtr_WriteReg( xTimer.BaseAddress,
                      0,
                      XTC_TCSR_OFFSET,
                      ControlStatusReg | XTC_CSR_INT_OCCURED_MASK);

    /* post semaphore */
    xSemaphoreGiveFromISR( xTimerSemaphore, &xHigherPriorityTaskWoken );

    /* reset timer */
    XTmrCtr_Reset(CallBackRef,TmrCtrNumber);

    /* we yield to previous tasks */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

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

