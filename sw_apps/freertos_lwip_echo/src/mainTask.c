/*
 * copied from FreeRTOS demo folder,
 * starts the main task.
 *
 * liu_benyuan <liubenyuan@gmail.com>
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* lwIP includes. */
#include "lwip/tcpip.h"

/* all other tasks */
extern void lwIPAppsInit( void *pvArguments );
extern void app_timer( void );

/*-----------------------------------------------------------*/
void mainTask( void )
{

    /* INIT lwIP and start lwIP tasks. */
    tcpip_init( lwIPAppsInit, NULL );

    /* starts timer APP */
    app_timer();

    /* Fire & Run. Start the tasks and timer running. */
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

