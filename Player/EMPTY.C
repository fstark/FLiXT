extern void f(int);

void sample( int data_seg, int offset )
{
    f( data_seg );
    f( offset);
}
