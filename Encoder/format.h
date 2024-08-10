/* The definition of the video format */

#include <stdint.h>

enum video_hardware_e : uint16_t
{
    VH_MDA,                 /* Monochrome display adatper */
    VH_HERCULES,            /* Hercules graphics card */
    VH_HERCULES_PLUS        /* Hercules plus graphics card */
};

enum video_tweaks_e : uint16_t
{
    VT_DONE,                /* No more tweak */
    VT_RAMFONT48k,          /* Hercules plus 48k ramfont mode */
    VT_LOAD                 /* Loads some data in the graphics card */
};

struct video_tweak_t
{
    enum video_tweaks_e tweak;
    union
    {
        struct
        {
        /* Pre-loading of some data (next data block) */
        /* Used to store the RAMFONT, or a poster */
            uint16_t segment;
            uint16_t offset;
            uint16_t size;
        } load;
        char filler[30];    /* Make sure structure is 32 bytes */
    } args;
};

#define HEADER_SIZE 4096

struct video_format_t
{
        /*  This is used as a comment field
            so head -2 can show the command line that created the file */
    char filler[2048];

        /* What hardware is needed */
    enum video_hardware_e hardware;

        /* Video tweaks to get hardware ready */
    struct video_tweak_t tweaks[4];

        /* The block size */
        /* Video should be read in chunks of block_size */
    uint16_t block_size;

    char shame[HEADER_SIZE];
};

void FormatLoad( int fd, struct video_format_t *format );
void FormatInfo( struct video_format_t *format );
