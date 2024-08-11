#include <dos.h>
#include "block.h"

int blocks_count = 8;
unsigned block_size = 0xffff;
struct block_t blocks[MAX_BLOCKS_COUNT];

void SetBlocksCount( int count )
{
	blocks_count = count;
	if (blocks_count > MAX_BLOCKS_COUNT)
		blocks_count = MAX_BLOCKS_COUNT;
	printf( "[INFO] Using %d blocks\n", blocks_count );
}

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
	for (i=0;i!=BLOCKS_COUNT;i++)
	{
		blocks[i].segment = BlockAllocate( block_size );
		blocks[i].state = DS_READING;

		printf( "BLOCK %d at %04x state=%d\n", i, blocks[i].segment, blocks[i].state );
	}
}

void BlockRelease()
{
	int i;
	for (i=0;i!=BLOCKS_COUNT;i++)
	{
		BlockFree( blocks[i].segment );
		blocks[i].segment = 0;
	}
}
