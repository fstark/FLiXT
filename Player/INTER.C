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


/* The block we are currently playing */
static int curblk = 0;   /* Buffer number */
static int blocknum = 0; /* Block number */

/* The code we are executing */
static unsigned code;

#define INTERRUPT 0x08

unsigned int_divider;

void InterruptInstall( double fps )
{
    double ticks_per_sec = 1192737.0; /* #### CHeck the real number */

    int_divider = (unsigned)(ticks_per_sec/fps);

    outportb( 0x43, 0x36 );
    outportb( 0x40, int_divider & 0xff );
    outportb( 0x40, int_divider >> 8 );

	oldfunc = getvect(INTERRUPT);
	setvect(INTERRUPT,func);

	printf( "Interrupt installed, old = %04x:%04x\n", FP_SEG(oldfunc), FP_OFF(oldfunc) );
	printf( "                     new = %04x:%04x\n", FP_SEG(func), FP_OFF(func) );

    atexit( InterruptRestore );
}

void InterruptRestore()
{
    outportb( 0x43, 0x36 );
    outportb( 0x40, 0xff );
    outportb( 0x40, 0xff );

	setvect(INTERRUPT,oldfunc);
}

void PlaybackInit()
{
    curblk = 0;
    code = block_size-2;
}

void PlaybackStep()
{
    unsigned far *code_ptr;

again:
    code_ptr = MK_FP( blocks[curblk].segment, code );

    /* If the block we want to play is "READING", we are stalling */
	if (blocks[curblk].state == DS_READING )
    {
	InterStall( blocknum );
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
        NEXT_BLOCK(curblk);
        blocknum++;
        code = block_size-2;
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
