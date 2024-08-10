int VideoOpen( char *video_file );
void VideoClose( int fd );
int ReadData( int fd, void far *buffer, int size );
void DOSErr( const char *function, const union REGS *outregs );
void GetMilli( long *ms );
