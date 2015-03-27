/* LwIP tasks */

/* Standard includes. */
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"

/* include the port-dependent configuration */
#define LWIP_PORT_INIT_IPADDR(addr)   IP4_ADDR((addr), configIP_ADDR0,configIP_ADDR1,configIP_ADDR2,configIP_ADDR3)
#define LWIP_PORT_INIT_GW(addr)       IP4_ADDR((addr), 192,168,1,1)
#define LWIP_PORT_INIT_NETMASK(addr)  IP4_ADDR((addr), 255,255,255,0)

/* The maximum time to block waiting to obtain the xTxBufferMutex to become
available. */
#define lwipappsMAX_TIME_TO_WAIT_FOR_TX_BUFFER_MS   ( 29 / portTICK_RATE_MS )

/*-----------------------------------------------------------*/

/* print the IP autonegotiation result of MAC */
void vStatusCallback( struct netif *pxNetIf )
{
char pcMessage[20];
struct in_addr* pxIPAddress;

    if( netif_is_up( pxNetIf ) != 0 )
    {
        strcpy( pcMessage, "IP=" );
        pxIPAddress = ( struct in_addr* ) &( pxNetIf->ip_addr );
        strcat( pcMessage, inet_ntoa( ( *pxIPAddress ) ) );
        xil_printf( pcMessage );
        xil_printf("\n");
    }
    else
    {
        xil_printf( "Network is down" );
    }
}

/* Called from the TCP/IP thread. */
void lwIPAppsInit( void *pvArgument )
{
ip_addr_t xIPAddr, xNetMask, xGateway;
extern err_t xemacpsif_init( struct netif *netif );
extern void xemacif_input_thread( void *netif );
static struct netif xNetIf;
/*
 * The function that implements the lwIP based sockets command interpreter
 * server.
 */
extern void echo_server( void *pvParameters );
extern void debug_server( void *pvParameters );

    ( void ) pvArgument;

    /* Set up the network interface. */
    ip_addr_set_zero( &xGateway );
    ip_addr_set_zero( &xIPAddr );
    ip_addr_set_zero( &xNetMask );

    LWIP_PORT_INIT_GW(&xGateway);
    LWIP_PORT_INIT_IPADDR( &xIPAddr );
    LWIP_PORT_INIT_NETMASK(&xNetMask);

    /* Set MAC address */
    xNetIf.hwaddr_len = 6;
    xNetIf.hwaddr[ 0 ] = configMAC_ADDR0;
    xNetIf.hwaddr[ 1 ] = configMAC_ADDR1;
    xNetIf.hwaddr[ 2 ] = configMAC_ADDR2;
    xNetIf.hwaddr[ 3 ] = configMAC_ADDR3;
    xNetIf.hwaddr[ 4 ] = configMAC_ADDR4;
    xNetIf.hwaddr[ 5 ] = configMAC_ADDR5;

    /* setup the network interface */
    netif_set_default( netif_add( &xNetIf, &xIPAddr, &xNetMask, &xGateway, ( void * ) XPAR_XEMACPS_0_BASEADDR, xemacpsif_init, tcpip_input ) );
    netif_set_status_callback( &xNetIf, vStatusCallback );
    netif_set_up( &xNetIf );

    /* parsing the network interface :
     * The priority for the task that unblocked by the MAC interrupt to process received packets,
     * which should be REALTIME */
    sys_thread_new( "lwIP_In", xemacif_input_thread, &xNetIf, configMINIMAL_STACK_SIZE, REALTIME_PRIORITY );

    /* Create the FreeRTOS defined basic command server.  This demonstrates use
    of the lwIP sockets API. */
    xTaskCreate( echo_server, "MFEIT-ECH", configMINIMAL_STACK_SIZE * 5, NULL, HIGHEST_PRIORITY, NULL );

}


