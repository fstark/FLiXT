#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include "inter.h"
#include "block.h"
#include "debug.h"
#include "stats.h"

extern demo( unsigned int offset, unsigned int segment );

void far interrupt (*oldfunc) ();
void interrupt func();

void InterruptInstall()
{
	oldfunc = getvect(0x1c);
	setvect(0x1c,func);

	printf( "Interrupt installed, old = %04x:%04x\n", FP_SEG(oldfunc), FP_OFF(oldfunc) );
	printf( "                     new = %04x:%04x\n", FP_SEG(func), FP_OFF(func) );

	(*(char far *)MK_FP( 0xb800, 0x0000 )) = 'A';
}

void InterruptRestore()
{
	setvect(0x1c,oldfunc);
	printf( "Interrupt restored\n" );
}

/* The block we are currently playing */
static int curblk = 0;

/* The code we are executing */
static unsigned code = BLOCK_SIZE-2;

void PlaybackStep()
{
    unsigned far *code_ptr;
    
again:
    code_ptr = MK_FP( blocks[curblk].segment, code );

    /* If the block we want to play is "READING", we are stalling */
	if (blocks[curblk].state == DS_READING )
    {
        InterStall( curblk, (BLOCK_SIZE-code-2)/2 );
    }
    else if (*code_ptr != 0xffffu)
    {
#ifndef SKIP_EXEC
        demo( *code_ptr, blocks[curblk].segment );
#endif
       code -= 2;
    }
    else
    /* If the current code is 0xffff, we finished the frame */
    {
        /* We put the block back available to read */
        blocks[curblk].state = DS_READING;

        /* And go the the next one */
        curblk = (curblk+1)%BLOCK_COUNT;
        code = BLOCK_SIZE-2;
        goto again;
    }

    /* Call previous handler */
	if (oldfunc)
		(*oldfunc)();
}

void interrupt func()
{
    PlaybackStep();
}
