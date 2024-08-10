#define noVERBOSE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include "block.h"

static const char* errors[] =
{
	"",
	"Invalid function number",
	"File not found",
	"Path not found",
	"Too many open files",
	"Access denied",
	"Invalid handle",
	"Memory control blocks destroyed",
	"Insufficient memory",
	"Invalid memory block address",
	"Invalid environment",
	"Invalid format",
	"Invalid access code",
	"Invalid data",
	"Reserved",
	"Invalid drive specified",
	"Attempt to remove current directory",
	"Not same device",
	"No more files",
	"Disk is write-protected",
	"Unknown unit",
	"Drive not ready",
	"Unknown command",
	"CRC error",
	"Bad request structure length",
	"Seek error",
	"Unknown media type",
	"Sector not found",
	"Printer out of paper",
	"Write fault",
	"Read fault",
	"General failure"
};

void DOSErr( const char *function, const union REGS *outregs )
{
	if (outregs->x.cflag)
	{
		const char *err = "Unknown error";
		int errcode = outregs->x.ax;
		if (errcode<sizeof(errors)/sizeof(errors[0]))
			err = errors[errcode];
		fprintf( stderr, "%s : DOS ERR=%d (%s)\n", function, errcode, err );
		exit( 1 );
	}
}

/* Read from video_file into a far buffer */

int VideoOpen( char *video_file )
{
	union REGS inregs;
	union REGS outregs;
	struct SREGS segregs;

	inregs.h.ah = 0x3d;
	inregs.h.al = 0;

	segregs.ds = FP_SEG( video_file );
	inregs.x.dx = (unsigned int)video_file;

	intdosx( &inregs, &outregs, &segregs );
	DOSErr( "VideoOpen()", &outregs );

#ifdef VERBOSE
	printf( "File handle=%d\n", outregs.x.ax );
#endif
	return outregs.x.ax;
}

void VideoClose( int fd )
{
	union REGS inregs;
	union REGS outregs;
	struct SREGS segregs;

	inregs.h.ah = 0x3e;
	inregs.x.bx = fd;

	intdosx( &inregs, &outregs, &segregs );

	DOSErr( "VideoClose()", &outregs );
}

int ReadData( int fd, void far *buffer, int size )
{
	union REGS inregs;
	union REGS outregs;
	struct SREGS segregs;

	inregs.h.ah = 0x3f;
	inregs.x.bx = fd;
	inregs.x.cx = size;
	segregs.ds = FP_SEG( buffer );
	inregs.x.dx = FP_OFF( buffer );

	intdosx( &inregs, &outregs, &segregs );
	DOSErr( "ReadData()", &outregs );

	return outregs.x.ax!=0;
}

/*
	Return timestamp in milliseconds
*/
void GetMilli( long *ms )
{
	union REGS inregs;
	union REGS outregs;

	long hours;
	long mins;
	long secs;
	long huns;

	inregs.h.ah = 0x2c;
	intdos( &inregs, &outregs );

	DOSErr( "GetTimeStamp()", &outregs );

	hours = outregs.h.ch;
	mins = outregs.h.cl;
	secs = outregs.h.dh;
	huns = outregs.h.dl;

	*ms = huns * 10 + 1000L * (secs + 60L * (mins + 60L * hours));
}
