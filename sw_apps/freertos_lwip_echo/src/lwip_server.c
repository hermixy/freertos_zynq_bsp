/*
 * basic protocol for MFEIT, for demonstration purpose only.
 * receive command, and send command with a header matching that of MFEIT
 * software
 *
 *  If you are connecting the Xplorer board directly to your host computer
 *  then a point to point (cross over) network cable is required.
 *  If you are connecting the Xplorer board to a network hub or switch
 *  then a standard network cable can be used.
 *
 *  and currently this driver can only run in point-to-point mode.
 *  going through a hub/switch is errornos
 *
 * liubenyuan@gmail.com
 * 2015-02-04
 */


/* Standard includes. */
#include "stdlib.h"
#include "string.h"

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/sockets.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* include debug functions */
#include "debug_server.h"
#include "freertos_eth.h"
#include "freertos_dma.h"

/* enable debug xil_printf */
#define DBG_SERVER 1

/*
 * basic MFEIT server code, a command & response DATA exchange system
 */
void echo_server( void *pvParameters )
{
    ( void ) pvParameters;

    /* static socket */
    static long lSocket, lClientFd;

    long lAddrLen = sizeof( struct sockaddr_in );
    struct sockaddr_in sLocalAddr;
    struct sockaddr_in client_addr;

    extern char *tcp_rx_ptr;

    int ret;
    int npacket;

    /* step 1. create */
    lSocket = lwip_socket(AF_INET, SOCK_STREAM, 0);

    if( lSocket >= 0 )
    {
        /* prepare bind on port */
        memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
        sLocalAddr.sin_family = AF_INET;
        sLocalAddr.sin_len = sizeof(sLocalAddr);
        sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sLocalAddr.sin_port = ntohs( ( ( unsigned short ) ECHO_PORT ) );

        /* step 2. bind */
        ret = lwip_bind( lSocket, ( struct sockaddr *) &sLocalAddr, sizeof( sLocalAddr ) );
        if( ret != 0 )
        {
            lwip_close( lSocket );
            vTaskDelete( NULL );
        }

        /* step 3. listen */
        ret = lwip_listen( lSocket, BACKLOG );
        if( ret != 0 )
        {
            lwip_close( lSocket );
            vTaskDelete( NULL );
        }

        for( ;; )
        {

            /* step 4. accept */
            lClientFd = lwip_accept(lSocket, ( struct sockaddr * ) &client_addr, ( u32_t * ) &lAddrLen );
            /* notify the user a new connection has been made */
#if(DBG_SERVER)
            xil_printf("(ZYNQ-ECH) accept connection\n");
#endif

            /* if client socket created */
            if( lClientFd > 0L )
            {

                npacket = 0;
                while(1)
                {

                    /* copy and wait for current BD */
                    u8 *RxBufferPtr = getValidRxBasePtr();
                    const unsigned int TX_SIZE = RX_REG_OFFSET + 16*MAX_PKT_LEN;

                    /* 1. read command */
                    ret = mfeit_read(lClientFd, tcp_rx_ptr);
                    if (ret == -1) {
                        /* notify the user COMMAND read errors */
#if(DBG_SERVER)
                        xil_printf("(ZYNQ-ECH) read ERROR\n");
#endif
                        break;
                    }

                    /* 2. write buffer */
                    ret = mfeit_write( lClientFd, (char *)RxBufferPtr, TX_SIZE );
                    if (ret == -1) {
                        /* notify the user DATA write errors */
#if(DBG_SERVER)
                        xil_printf("(ZYNQ-ECH) write ERROR\n");
#endif
                        break;
                    }

                    npacket ++;
                }

                /* gracefully exit */
                if (lwip_shutdown(lClientFd, SHUT_WR) == 0)
                    /* notify the user the remote has shutdown the connection */
#if(DBG_SERVER)
                    xil_printf("(ZYNQ-ECH) shutdown write\n");
#endif

                /* continue to receive until the last packet */
                do {
                    ret = mfeit_read(lClientFd, tcp_rx_ptr);
                } while (ret == 0);

                /* notify the user the server acknowledge the shutdown */
#if(DBG_SERVER)
                xil_printf("(ZYNQ-ECH) close connection, total = %d\n", npacket);
#endif
                lwip_close( lClientFd );
            }
        }
    }

    /* Will only get here if a listening socket could not be created. */
#if(DBG_SERVER)
    xil_printf("(ZYNQ-ECH) task killed !\n");
#endif

    vTaskDelete( NULL );
}


