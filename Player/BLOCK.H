#define BLOCK_SIZE 32768u
#define BLOCK_COUNT 12

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

extern struct block_t blocks[BLOCK_COUNT];
