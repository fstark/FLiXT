#include "stats.h"
#include <dos.h>
#include "dosutil.h"
#include "block.h"

#include <conio.h>

#define MAX_STALL 256

int stall_blocks[MAX_STALL];
int stall_frames[MAX_STALL];
int stall_count = 0;

void InterStall( unsigned block, unsigned frame )
{
    if (stall_count < MAX_STALL)
    {
        stall_blocks[stall_count] = block;
        stall_frames[stall_count] = frame;
        stall_count++;
    }
}

static long bytes_read = 0;

void BytesRead( unsigned bytes )
{
    bytes_read += bytes;
}

static unsigned long ts_begin = 0;
static unsigned long ts_end = 0;

void StatsBegin()
{
    GetMilli( &ts_begin );
}
void StatsEnd()
{
    GetMilli( &ts_end );
    if (ts_end<ts_begin)
    {
        ts_end -= ts_begin;
        ts_begin = 0;
    }
}

void DumpStats()
{
    int i;

    printf( "\n\nElapsed time: %ld ms\n", ts_end-ts_begin );

    if (stall_count != 0)
    {
        if (stall_count == MAX_STALL)
            printf( "WARNING: more than %d stalls\n", MAX_STALL );

        printf( "Stalls: \n" );
        for (i=0;i!=stall_count;i++)
        {
            printf( "%03d:%03d ", stall_blocks[i], stall_frames[i] );
        }
        printf( "\n" );
        printf( "%d stalls\n", stall_count );
    }

    printf( "Read        : %ld KB\n", bytes_read/1024 );
	printf( "Read speed  : %.2f KB/s\n", (1.0f*bytes_read)/(ts_end-ts_begin) );

}
