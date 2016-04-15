/*
 * !!! IMPORTANT NOTE !!!
 * The GCC libraries that ship with the Xilinx SDK make use of the floating
 * point registers.  To avoid this causing corruption it is necessary to avoid
 * their use.  For this reason main.c contains very basic C implementations of
 * the standard C library functions memset(), memcpy() and memcmp(), which are
 * are used by FreeRTOS itself.  Defining these functions in the project
 * prevents the linker pulling them in from the library.  Any other standard C
 * library functions that are used by the application must likewise be defined
 * in C.
 *
 * copied from freertos-zynq demo
 *
 */

#include "mem_stdc.h"

/*-----------------------------------------------------------*/

void *memcpy( void *pvDest, const void *pvSource, size_t ulBytes )
{
unsigned char *pcDest = ( unsigned char * ) pvDest, *pcSource = ( unsigned char * ) pvSource;
size_t x;

    for( x = 0; x < ulBytes; x++ )
    {
        *pcDest = *pcSource;
        pcDest++;
        pcSource++;
    }

    return pvDest;
}
/*-----------------------------------------------------------*/

void *memset( void *pvDest, int iValue, size_t ulBytes )
{
unsigned char *pcDest = ( unsigned char * ) pvDest;
size_t x;

    for( x = 0; x < ulBytes; x++ )
    {
        *pcDest = ( unsigned char ) iValue;
        pcDest++;
    }

    return pvDest;
}
/*-----------------------------------------------------------*/

int memcmp( const void *pvMem1, const void *pvMem2, size_t ulBytes )
{
const unsigned char *pucMem1 = pvMem1, *pucMem2 = pvMem2;
size_t x;

    for( x = 0; x < ulBytes; x++ )
    {
        if( pucMem1[ x ] != pucMem2[ x ] )
        {
            break;
        }
    }

    return ulBytes - x;
}
