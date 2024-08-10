#include <dos.h>
#include "block.h"

struct block_t blocks[BLOCK_COUNT];

/* 	Call DOS int21h to allocate a 64K (4096 paragraph) buffer and return the segment */
unsigned int BlockAllocate( unsigned byte_size )
{
	union REGS inregs;
	union REGS outregs;

	inregs.h.ah = 0x48;
	inregs.x.bx = byte_size / 16;	/* #### Shoud check multiple of 16 */
	intdos(&inregs, &outregs);

	DOSErr( "BlockAllocate()", &outregs);

	return outregs.x.ax;
}

void BlockFree( unsigned int segment )
{
	union REGS inregs;
	union REGS outregs;
	struct SREGS segregs;

	inregs.h.ah = 0x49;
	segregs.es = segment;
	intdosx(&inregs, &outregs, &segregs);

	DOSErr( "BlockFree()", &outregs);
}

void BlockInit()
{
	int i;
	for (i=0;i!=BLOCK_COUNT;i++)
	{
		blocks[i].segment = BlockAllocate(BLOCK_SIZE);
		blocks[i].state = DS_READING;

		printf( "BLOCK %d at %04x state=%d\n", i, blocks[i].segment, blocks[i].state );
	}
}

void BlockRelease()
{
	int i;
	for (i=0;i!=BLOCK_COUNT;i++)
	{
		BlockFree( blocks[i].segment );
		blocks[i].segment = 0;
	}
}
