#include "format.h"
#include <dos.h>
#include "dosutil.h"
#include "herc.h"
#include "debug.h"

static char *hardware[] = {
    "MDA (Monochrome Display Adapter)",
    "HGC (Hercules Graphics Card)",
    "HGC+ (Hercules Plus Graphics Card)"
};

void FormatLoad( int fd, struct video_format_t *format )
{
    int i;

    ReadData( fd, format, HEADER_SIZE );

    /* Display 4096 hex caracters pointed to by format in 16 columns with 4 chars hex offset */
    /* DumpHex( format, HEADER_SIZE ); */
}

void FormatInfo( struct video_format_t *format )
{
    int i;

    printf( "Video card required: %s\n\n", hardware[format->hardware] );

    printf( "Load instructions:\n" );
    for (i=0;i!=4;i++)
    {
        struct video_tweak_t *tweak = &format->tweaks[i];
        if (tweak->tweak == VT_DONE)
            break;
        switch (tweak->tweak)
        {
            case VT_LOAD:
                printf( "- Load %04XH bytes at %04X:%04X\n",
                    tweak->args.load.size,
                    tweak->args.load.segment,
                    tweak->args.load.offset);
                break;
            case VT_RAMFONT48k:
                printf( "- Switch card to RAMFONT 48k mode\n" );
                break;
            default:
                printf( "? Unknown tweak %d\n", tweak->tweak );
                break;
        }
    }

    printf( "\nBlock size: %04XH (%u) bytes\n", format->block_size, format->block_size );
}

void FormatExecuteTweaks( struct video_format_t *format, int video_fd )
{
    int i;

    for (i=0;i!=4;i++)
    {
        struct video_tweak_t *tweak = &format->tweaks[i];
        if (tweak->tweak == VT_DONE)
            break;
        switch (tweak->tweak)
        {
            case VT_LOAD:
                {
#ifndef SKIP_VIDEO
                    char far *buffer = MK_FP( tweak->args.load.segment, tweak->args.load.offset );
#else
                    unsigned seg = BlockAllocate( tweak->args.load.size );
                    char far *buffer = MK_FP( seg, 0 );
#endif
                    ReadData( video_fd, buffer, tweak->args.load.size );
                }
                break;
            case VT_RAMFONT48k:
#ifndef SKIP_VIDEO
                HerculesRamfont48K();
#endif
                break;
            default:
                printf( "? Unknown tweak %d\n", tweak->tweak );
                break;
        }
    }
}

void FormatUnexecuteTweaks( struct video_format_t *format, int video_fd )
{
    int i;

    for (i=0;i!=4;i++)
    {
        struct video_tweak_t *tweak = &format->tweaks[i];
        if (tweak->tweak == VT_DONE)
            break;
        switch (tweak->tweak)
        {
            case VT_LOAD:
                break;
            case VT_RAMFONT48k:
                HerculesTextMode();
                break;
            default:
                printf( "? Unknown tweak %d\n", tweak->tweak );
                break;
        }
    }
}
