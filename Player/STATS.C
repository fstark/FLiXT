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

void DumpStats()
{
    int i;

    if (stall_count == 0)
    {   return ;
    }

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
