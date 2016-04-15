/*
 * very basic implementation of std c calls
 * which are used by freeRTOS itself.
 */

#ifndef MEM_STDC_H_
#define MEM_STDC_H_

#include <stdio.h>
#include <limits.h>

void *memcpy( void *pvDest, const void *pvSource, size_t ulBytes );
void *memset( void *pvDest, int iValue, size_t ulBytes );
int memcmp( const void *pvMem1, const void *pvMem2, size_t ulBytes );

#endif /* MEM_STDC_H_ */
