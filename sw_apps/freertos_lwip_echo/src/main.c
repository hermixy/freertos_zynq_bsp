/*
 * main() do some house keeping job to make the whole program run.
 * It initialises two global control : GIC and WDT, for other program.
 *
 * It triggers mainTask() to call all the tasks.
 *
 * liu_benyuan <liubenyuan@gmail.com>
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

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

int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    /* start tasks */
    extern void mainTask( void );
    mainTask();

    /* should not reach here */
    return 0;

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


