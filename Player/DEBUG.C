#include <stdio.h>
#include "debug.h"

void DumpHex( const unsigned char far *data, unsigned size )
{
    unsigned i;
    for (i=0;i!=size;i++)
    {
        if (i%16 == 0)
            printf( "\n%04X: ", i );
        printf( "%02X ", data[i] );
    }
    printf( "\n" );
}
