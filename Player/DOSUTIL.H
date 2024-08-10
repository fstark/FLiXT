int VideoOpen( const char *video_file );
void VideoClose( int fd );
int ReadData( int fd, void far *buffer, unsigned size );
void DOSErr( const char *function, const union REGS *outregs );
void GetMilli( unsigned long *ms );
