#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include "inter.h"
#include "block.h"
#include "debug.h"

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
    
#ifdef VERBOSE
    printf( "PlaybackStep() block %d state=%s\n", curblk, blocks[curblk].state==DS_READING?"DS_READING":"DS_PLAYING" );
#endif

    /* Debug display */
	(*(char far *)MK_FP( 0xb800, 0x0000 )) = '0'+curblk;
	(*(char far *)MK_FP( 0xb800, 0x0004 )) = '0'+blocks[curblk].state;

again:
    code_ptr = MK_FP( blocks[curblk].segment, code );

    /* If the block we want to play is "READING", we are stalling */
	if (blocks[curblk].state == DS_READING )
    {
    	(*(char far *)MK_FP( 0xb800, 0x0000 )) = '*';
        return;
    }
#ifdef VERBOSE
 printf( "\n EXEC %04x:%04x %04x\n", FP_SEG(code_ptr), FP_OFF(code_ptr), *code_ptr );
#endif
    if (*code_ptr != 0xffffu)
    {
        void far *p = demo;
#ifdef VERBOSE
        printf( "Will execute demo() at %04X:%04X\n", FP_SEG(p), FP_OFF(p) );
#endif
/*
        while (!kbhit())
            ;
*/

/*
        printf( "  Exec request: %04x:%04x=%04x\n", FP_SEG(code_ptr), FP_OFF(code_ptr), *code_ptr );
*/
#ifndef SKIP_EXEC
        demo( *code_ptr, blocks[curblk].segment );
#endif

       code-=2;
    }
    else
    /* If the current code is 0x00, we finished the frame */
    {
#ifdef VERBOSE
        printf( "BLOCK %d => DS_READING\n", curblk );
#endif
        /* We put the block back available to read */
        blocks[curblk].state = DS_READING;

        /* And go the the next one */
        curblk = (curblk+1)%BLOCK_COUNT;
        code = BLOCK_SIZE-2;
        goto again;
    }

/*	if (oldfunc)
		(*oldfunc)();
	(*(char far *)MK_FP( 0xb800, 0x0000 ))++;
*/
}

void interrupt func()
{
    PlaybackStep();
}
