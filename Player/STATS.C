#include "stats.h"
#include <dos.h>
#include "dosutil.h"
#include "block.h"

#include <conio.h>

#define MAX_STALL 256

int stall_blocks[MAX_STALL];
int stall_frames[MAX_STALL];
int last_stall = 0;
int stall_count = 0;

void InterStall( unsigned block )
{
        /* First time is special case */
    if (stall_count == 0)
        stall_blocks[last_stall] = block;

    stall_count++;

        /* Look if stall on new block */
    if (block!=stall_blocks[last_stall])
    {
        if (last_stall == MAX_STALL-1)
            return; /* No more room */
        last_stall++;
        stall_blocks[last_stall] = block;
        stall_frames[last_stall] = 0;
    }

        /* One additional stall on this block */
    stall_frames[last_stall]++;
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

    /* If we recorded some stall, we go to the next */
    if (stall_frames[last_stall])
        last_stall++;
}

void DumpStats()
{
    int i;

    Cleanup();

    printf( "\n\nElapsed time: %ld ms\n", ts_end-ts_begin );

    if (last_stall != 0)
    {
        if (last_stall == MAX_STALL)
            printf( "WARNING: more than %d stalls\n", MAX_STALL );

        printf( "Stalls: \n" );
        for (i=0;i!=last_stall;i++)
        {
            printf( "%03d:%03d ", stall_blocks[i], stall_frames[i] );
        }
        printf( "\n" );
        printf( "%d stalls\n", stall_count );
    }

    printf( "Read        : %ld KB\n", bytes_read/1024 );
	printf( "Read speed  : %.2f KB/s\n", (1.0f*bytes_read)/(ts_end-ts_begin) );

}
