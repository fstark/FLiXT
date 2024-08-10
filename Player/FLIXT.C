/* Define HGC for Hercules code */
/* If not defined, generic dosbos-x test code */
#define noHGC

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <assert.h>
#include "block.h"
#include "inter.h"
#include "format.h"
#include "debug.h"
#include "dosutil.h"

/*	-----------------------------------------------------------------------
	HGC+ info
	RAM = B0000-BFFFF
	----------------------------------------------------------------------- */

int video_fd;


typedef int (far *videoexec_t)();

void VideoPlay( int block_index )
{
	videoexec_t code = (videoexec_t)MK_FP( blocks[block_index].segment, 0 );

	assert( block_index < BLOCK_COUNT );
#ifdef VERBOSE
	printf( "PLAY %d at %04x state=%d\n", block_index, blocks[block_index].segment, blocks[block_index].state );
#endif
	assert( blocks[block_index].state == DS_PLAYING );


	blocks[block_index].state = DS_READING;
}

int BlockWait( int block_index, enum block_state_e state )
{
	do
	{
		if (kbhit())
		{
			switch (getch())
			{
				case 27:
					return 0;
				case ' ':
					while (!kbhit())
						;
					if (getch()==27)
						return 0;
			}
		}
	} while (blocks[block_index].state != state);

	return 1;
}

#define FONT 0xb400
#ifdef HGC
	#define VIDEO 0xb000
#else
	#define VIDEO 0xb800
	#define SKIP_VIDEO_CHANGE
#endif

int font_size;
int width;
int height;
int page_size;

int VideoReadInt()
{
	int i;
	ReadData( video_fd, &i, 2 );
	return i;
}

void VideoReadFont()
{
	font_size = VideoReadInt();
	ReadData( video_fd, MK_FP( FONT, 0x0000), 16*font_size );
	width = VideoReadInt();
	height = VideoReadInt();
	page_size = width * height * 2;
	printf( "Font size=%d, width=%d, height=%d, page_size=%d\n", font_size, width, height, page_size );
}


int BlockRead( int block_index )
{
	int res;

	assert( block_index < BLOCK_COUNT );
#ifdef VERBOSE
	printf( "READ %d at %04x state=%d\n", block_index, blocks[block_index].segment, blocks[block_index].state );
#endif
	assert( blocks[block_index].state == DS_READING );

	res = ReadData( video_fd, MK_FP( blocks[block_index].segment, 0 ), BLOCK_SIZE );

	blocks[block_index].state = DS_PLAYING;

	return res;
}

void VideoReadLoop( int sync )
{
	int i=0;
	while (1)
	{
		if (BlockWait( i, DS_READING )==0)
			return ;	/* Abort */
		if (!BlockRead( i ))
			return ;	/* Finished */
		if (sync)
			while (blocks[i].state == DS_PLAYING)
			{
				PlaybackStep();
			}
		i = (i+1)%BLOCK_COUNT;
	}
}

void VideoWaitFinish()
{
	int i;
	for (i=0;i!=BLOCK_COUNT;i++)
	{
		BlockWait( i, DS_READING );
	}
}

void Usage( const char *name )
{
	const char *p = name;
	const char *q = name;
	while (*q)
	{
		if (*q=='\\' || *q=='/')
			p = q+1;
		q++;
	}

	printf( "Usage:\n\n" );
	printf( "  %s /PLAY FILENAME\n   -- Plays the FliXT file\n\n", p );
	printf( "  %s /INFO FILENAME\n   -- Informations about the FliXT\n\n", p );
	printf( "  %s /SYNC FILENAME\n   -- Plays in a synchronous way for debugging pruposes\n", p );
}

/* -----------------------------------------------------------------------
	----------------------------------------------------------------------- */

int main( int argc, char **argv )
{
	struct video_format_t format;

	/* Parse the argements:
		/INFO FILENAME : displays the format information
		/PLAY FILENAME : plays the file
	*/
	if (argc!=3)
	{
		Usage( argv[0] );
		return 1;
	}

	if (!strcmp(argv[1],"/INFO"))
	{
		video_fd = VideoOpen( argv[2] );
		FormatLoad( video_fd, &format );
		FormatInfo( &format );
		VideoClose( video_fd );
		return 0;
	}

	if (!strcmp(argv[1],"/SYNC"))
	{
		long ts1;
		long ts2;
		BlockInit();
		video_fd = VideoOpen( argv[2] );
		FormatLoad( video_fd, &format );
		FormatExecuteTweaks( &format, video_fd );
		GetMilli( &ts1 );
		VideoReadLoop( 1 );
		GetMilli( &ts2 );
		FormatUnexecuteTweaks( &format, video_fd );
		VideoClose( video_fd );
		BlockRelease();

		printf( "FliXT played in : %ldms\n", ts2-ts1 );
		return 0;
	}

	if (!strcmp(argv[1],"/PLAY"))
	{
		BlockInit();
		video_fd = VideoOpen( argv[2] );
		FormatLoad( video_fd, &format );
		FormatExecuteTweaks( &format, video_fd );
		InterruptInstall();
		VideoReadLoop( 0 );
		VideoWaitFinish();
		InterruptRestore();
		FormatUnexecuteTweaks( &format, video_fd );
		VideoClose( video_fd );
		BlockRelease();

		return 0;
	}

	Usage( argv[0] );
	return 1;
}


