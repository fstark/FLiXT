#include "format.h"
#include <dos.h>
#include <stdio.h>
#include "dosutil.h"
#include "herc.h"
#include "debug.h"
#include "block.h"

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

    block_size = format->block_size;    /* #### UGLY AS HELL SIDE EFFECT */
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

void FormatExecuteTweaks( struct video_format_t *format, int video_fd, int fake )
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
                    unsigned segment = tweak->args.load.segment;
                    unsigned offset = tweak->args.load.offset;
                    char far *buffer;
                    
                    printf( "[INFO] Loading %04X bytes at %04X:%04X...", tweak->args.load.size, segment, offset );
                    fflush( stdout );

                    if (fake)
                    {
                        segment = BlockAllocate( tweak->args.load.size );
                        offset = 0;
                    }

                    buffer  = MK_FP( segment, offset );
                    ReadData( video_fd, buffer, tweak->args.load.size );

                    if (fake)
                    {
                        BlockFree( segment );
                    }

                    printf( "done\n" );
                }
                break;
            case VT_RAMFONT48k:
                if (!fake)
                    HerculesRamfont48K();
                break;
            default:
                printf( "? Unknown tweak %d\n", tweak->tweak );
                break;
        }
    }
}

void FormatUnexecuteTweaks( struct video_format_t *format, int video_fd, int fake )
{
    int i;

    video_fd = video_fd; /* Remove warning from TC */

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
                if (!fake)
                    HerculesTextMode();
                break;
            default:
                printf( "? Unknown tweak %d\n", tweak->tweak );
                break;
        }
    }
}
