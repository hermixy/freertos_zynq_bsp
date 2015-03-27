/*
 * debug_server.h
 */

#ifndef DEBUG_SERVER_H_
#define DEBUG_SERVER_H_

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE	100

/* Dimensions the buffer into which string outputs can be placed. */
#define cmdMAX_OUTPUT_SIZE	1024

/* maximum connections */
#define BACKLOG             6

/* service port */
#define DEBUG_PORT          27000
#define ECHO_PORT           27015

/* registered functions */
void debug_server( void *pvParameters );
void echo_server( void *pvParameters );

#endif /* DEBUG_SERVER_H_ */
