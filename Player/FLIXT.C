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
#include "stats.h"

/*	-----------------------------------------------------------------------
	HGC+ info
	RAM = B0000-BFFFF
	----------------------------------------------------------------------- */

int video_fd;


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

	assert( block_index < BLOCKS_COUNT );
#ifdef VERBOSE
	printf( "READ %d at %04x state=%d\n", block_index, blocks[block_index].segment, blocks[block_index].state );
#endif
	assert( blocks[block_index].state == DS_READING );

	res = ReadData( video_fd, MK_FP( blocks[block_index].segment, 0 ), block_size );
	BytesRead( block_size );

	blocks[block_index].state = DS_PLAYING;

	return res;
}

void VideoPreload()
{
	int i;

	for (i=0;i!=BLOCKS_COUNT;i++)
	{
		BlockRead( i );
	}
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
		NEXT_BLOCK(i);
	}
}




void VideoStatRead()
{
	int i;
	for (i=0;BlockRead( i%BLOCKS_COUNT );i++)
	{
		blocks[i%BLOCKS_COUNT].state = DS_READING;	/* Immediate re-usable */
		printf( "%d ", i );
	}
}

void VideoWaitFinish()
{
	int i;
	for (i=0;i!=BLOCKS_COUNT;i++)
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
	printf( "  %s /SYNC FILENAME\n   -- Plays in a synchronous way for debugging pruposes\n\n", p );
	printf( "  %s /BENC FILENAME\n   -- Bench the machine against a sepecific FliXT file\n", p );
}

/* -----------------------------------------------------------------------
	----------------------------------------------------------------------- */

int arg_info = 0;
int arg_play = 0;
int arg_sync = 0;
int arg_benc = 0;

const char *filename = "OUT.VID";

double fps = 18.2;

int RegisterArgument( char *name )
{
printf( "[%s]\n", name );

	if (!strncmp(name,"INFO",3))
	{
		arg_info = 1;
		return 0;
	}
	if (!strncmp(name,"PLAY",4))
	{
		arg_play = 1;
		return 0;
	}
	if (!strncmp(name,"SYNC",3))
	{
		arg_sync = 1;
		return 0;
	}
	if (!strncmp(name,"BENC",4))
	{
		arg_benc = 1;
		return 0;
	}
	return 1;
}

int RegisterArgumentValue( char *name, char *value )
{
	if (!strcmp(name,"BUF"))
	{
		SetBlocksCount( atoi(value) );
		return 0;
	}
	if (!strcmp(name,"FPS"))
	{
		fps = atoi(value);
		return 0;
	}
	return 1;
}

/*
	Parses an argument in the form of:
	/NAME
	or
	/NAME=VALUE
	Calls RegisterArgument with the name in the first case
	and RegisterArgumentValue with the name and value in the second case
	return value of RegisterArgument/RegisterArgumentValue or 1 if error.
*/
int ParseArgument( const char *p )
{
	char name[255];
	char value[255];

	if (strlen(p)>255)
		return 1;

	if (*p!='/')
	{
		filename = p;
		return 0;
	}

	p++;
	
	/* Copy the name part */
	{
		char *q = name;
		while (*p && *p!='=')
			*q++ = toupper(*p++);
		*q = 0;
	}

	value[0] = 0;
	/* Copy the value part, if any */
	if (*p=='=')
	{
		p++;
		{
			char *q = value;
			while (*p)
				*q++ = *p++;
			*q = 0;
		}
	}

	/* Call RegisterArgument or RegisterArgumentValue */
	if (value[0])
		return RegisterArgumentValue( name, value );
	return RegisterArgument( name );
}

int main( int argc, char **argv )
{
	struct video_format_t format;
	int i;

	for (i=1;i!=argc;i++)
	{
		if (ParseArgument( argv[i] ))
		{
			printf( "Unknown argument %s\n", argv[i] );
			Usage( argv[0] );
			return 1;
		}
	}

	if (arg_info + arg_play + arg_sync + arg_benc != 1)
	{
		printf( "You must specify one of /INFO, /PLAY, /SYNC or /STAT\n" );
		Usage( argv[0] );
		return 1;
	}

	if (arg_info==1)
	{
		video_fd = VideoOpen( filename );
		FormatLoad( video_fd, &format );
		FormatInfo( &format );
		VideoClose( video_fd );
		return 0;
	}

	if (arg_sync==1)
	{
		video_fd = VideoOpen( filename );
		FormatLoad( video_fd, &format );
		FormatExecuteTweaks( &format, video_fd, 0 );
		BlockInit();
		StatsBegin();
		PlaybackInit();
		VideoReadLoop( 1 );
		StatsEnd();
		VideoClose( video_fd );
		BlockRelease();

		DumpStats();

		return 0;
	}

	if (arg_play==1)
	{
		video_fd = VideoOpen( filename );
		FormatLoad( video_fd, &format );
		FormatExecuteTweaks( &format, video_fd, 0 );
		BlockInit();
		VideoPreload();     /* Avoid early stalls */
		PlaybackInit();
		InterruptInstall( fps );
		VideoReadLoop( 0 );
		VideoWaitFinish();
		InterruptRestore();
		VideoClose( video_fd );
		BlockRelease();
		DumpStats();

		return 0;
	}

	if (arg_benc==1)
	{
		video_fd = VideoOpen( filename );
		FormatLoad( video_fd, &format );
		BlockInit();
		PlaybackInit();
		FormatExecuteTweaks( &format, video_fd, 1 );
		StatsBegin();
		VideoStatRead();
		StatsEnd();
		VideoClose( video_fd );
		BlockRelease();

		DumpStats();

		return 0;
	}

	Usage( argv[0] );
	return 1;
}


