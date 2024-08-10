#define BLOCK_SIZE 32768u
#define MAX_BLOCKS_COUNT 16

enum block_state_e
{
	DS_READING,	/* Block is currently reserved for reading */
	DS_PLAYING	/* Block is currently reserved for playing */
};

/* This contains a block for the I/O subsystem and playback of video and sound */
struct block_t
{
	unsigned int segment;
	enum block_state_e state;
};

extern int blocks_count;
void SetBlocksCount( int count );
#define BLOCKS_COUNT (blocks_count)
#define NEXT_BLOCK( var ) do { var++; if (var==BLOCKS_COUNT) var = 0; } while(0)
extern struct block_t blocks[MAX_BLOCKS_COUNT];
