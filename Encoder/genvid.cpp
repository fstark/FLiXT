// Generates a test movie file for performance evaluation

#include <iostream>
#include <fstream>
#include <cassert>

#include "format.h"

void WriteHGPHeader(std::ofstream& fp, const uint8_t* font48)
{
    struct video_format_t format;

    memset(&format, 0xff, sizeof(format));

    format.hardware = VH_HERCULES_PLUS;
    format.tweaks[0].tweak = VT_RAMFONT48k;

    format.tweaks[1].tweak = VT_LOAD;
    format.tweaks[1].args.load.segment = 0xB400;
    format.tweaks[1].args.load.offset = 0;
    format.tweaks[1].args.load.size = 0xC000;

    /*    format.hardware = VH_MDA;

    format.tweaks[0].tweak = VT_LOAD;
    format.tweaks[0].args.load.segment = 0xB000;
    format.tweaks[0].args.load.offset = 0;
    format.tweaks[0].args.load.size = 0x1000;
*/

    format.tweaks[2].tweak = VT_DONE;
    format.block_size = 32768;

    printf( "Writing header %d bytes\n", sizeof(format) );

    //  Write format to fp
    fp.write(reinterpret_cast<const char*>(&format), HEADER_SIZE);

    //  Write font to fp
    fp.write( (char *)font48, 0xC000);
}

#include "block8x8.hpp"

class video_char
{
    uint8_t data[16];
    block8x8 block_;    //  For high perf distance computations

public:
    video_char():block_(data, 8) {};

    const uint8_t &operator[](int line) const { return data[line]; }
    uint8_t &operator[](int line) { return data[line]; }

    void make_block()
    {
        block_.set_data( data );
    }

    friend int distance( const video_char &a, const video_char &b );

    std::vector<u_int8_t> as_uint8()
    {
        std::vector<u_int8_t> v;
        for (int i=0;i!=16;i++)
            v.push_back(data[i]);
        return v;
    }

    //  Dump the character to stderr as hex
    void dump() const
    {
        for (int i=0;i!=16;i++)
            fprintf( stderr, "%02x ", data[i] );
        fprintf( stderr, "\n" );
    }
};

int distance( const video_char &a, const video_char &b )
{
    return a.block_.distance( b.block_ ); // would be better to have a static distance function
}

#define MAXCHARS 3072
// #define COLS 90
// #define ROWS 40

video_char video_chars[MAXCHARS];

class screen_t
{
    int w_;
    int h_;
    uint16_t *data_;

public:
    screen_t( int w, int h ) : w_(w), h_(h)
    {   
        data_ = new uint16_t[w*h];
        // fill data with 0
        for (int i=0;i!=w_*h_;i++)
            data_[i] = 0;
    }
    ~screen_t() { delete[] data_; }

    // Copy constructor
    screen_t( const screen_t &s ) : w_(s.w_), h_(s.h_)
    {
        data_ = new uint16_t[w_*h_];
        for (int i=0;i!=w_*h_;i++)
            data_[i] = s.data_[i];
    }

    // Copy assignment operator
    screen_t &operator=( const screen_t &s )
    {
        if (this!=&s)
        {
            delete[] data_;
            w_ = s.w_;
            h_ = s.h_;
            data_ = new uint16_t[w_*h_];
            for (int i=0;i!=w_*h_;i++)
                data_[i] = s.data_[i];
        }
        return *this;
    }

    int width() const { return w_; }
    int height() const { return h_; }

    // return the ith character
    uint16_t &operator[](int i) { return data_[i]; }
    uint16_t operator[](int i) const { return data_[i]; }

    // return the the data as uint16_t pointer
    uint16_t *get_uint16() { return data_; }

    //  Count number of different characters between two screens
    int count_diff( const screen_t &s ) const
    {
        int count = 0;
        for (int i=0;i!=w_*h_;i++)
            if (data_[i]!=s.data_[i])
                count++;
        return count;
    }

    // Count the number of different characters there are in this screen
    // by creating an vector up to MAXCHARS and incrementing the index
    int count_diff() const
    {
        std::vector<int> count(MAXCHARS, 0);
        for (int i=0;i!=w_*h_;i++)
            count[data_[i]]++;
        int total = 0;
        for (int i=0;i!=MAXCHARS;i++)
            if (count[i])
                total++;
        return total;
    }

    // dumps the screen to stderr as a wxh matrix of 2 bytes hex
    void dump() const
    {
        for (int i=0;i!=h_;i++)
        {
            for (int j=0;j!=w_;j++)
                fprintf( stderr, "%02x ", data_[i*w_+j] );
            fprintf( stderr, "\n" );
        }
    }

    //  Generates a w*h int vector with 0 or 255 depending of the value from the characters in the screen
    //  index into video_chars by the data[i] value
    std::vector<int> make_image() const
    {
        // result
        std::vector<int> result(w_*h_*8*8);
        // fill with black
        for (int i=0;i!=w_*h_;i++)
            result[i] = 0;
        // for each 8x8 character in the screen
        for (int i=0;i!=w_*h_;i++)
        {
            // for each line in the character
            for (int j=0;j!=8;j++)
            {
                // for each pixel in the line
                for (int k=0;k!=8;k++)
                {
                    // if the pixel is set, set the corresponding pixel in the result
                    if (video_chars[data_[i]][j] & (1<<(7-k)))
                        result[(i%w_)*8+k + (i/w_)*8*8*w_+j*8*w_] = 255;
                }
            }
        }

        return result;
    }

};

// include libpng
#include <png.h>

void write_grayscale_png( int w, int h, const std::vector<int> &data, const std::string &filename )
{
    // Create a grayscale PNG image using libpng with content of 'data' and the given width and height
    // and write it to the given filename

    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        std::cerr << "Error creating PNG write structure" << std::endl;
        fclose(fp);
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cerr << "Error creating PNG info structure" << std::endl;
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr << "Error during PNG creation" << std::endl;
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    std::vector<png_byte> row(w);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            row[x] = static_cast<png_byte>(data[y * w + x]);
        }
        png_write_row(png_ptr, row.data());
    }

    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

struct change_t
{
    int index;
    int char_before;
    int char_current;
    int char_after;

    int delta_before;
    int delta_after;
    int detal_skip;

    float impact() const { return delta_before; }
};

struct change_t make_change( const screen_t &s0, const screen_t &s1, const screen_t &s2, int index )
{
    struct change_t c;
    c.index = index;
    c.char_before = s0[index];
    c.char_current = s1[index];
    c.char_after = s2[index];

    c.delta_before = distance( video_chars[c.char_before], video_chars[c.char_current] );
    c.delta_after = distance( video_chars[c.char_current], video_chars[c.char_after] );
    c.detal_skip = distance( video_chars[c.char_before], video_chars[c.char_after] );

    return c;
}

//  Applies the changes to the screen
void apply_changes( screen_t &s, const std::vector<struct change_t> &changes )
{
    for (auto &c: changes)
    {
        s[c.index] = c.char_current;
    }
}

std::vector<struct change_t> list_changes( const screen_t &s0, const screen_t &s1, const screen_t &s2, bool display=false )
{
    /* We stats the middle frame */
    printf( "Uniques:%4d Prev Changes:%4d Next Changes:%4d Skip Changes:%4d\n", s1.count_diff(), s1.count_diff(s0), s1.count_diff(s2), s2.count_diff(s0) );

    /* Build the list of characters that changed */
    std::vector<struct change_t> changes;
    for (int i=0;i!=s1.width()*s1.height();i++)
    {
        if (s0[i]!=s1[i])
        {
            changes.push_back( make_change( s0, s1, s2, i ) );
        }
    }

    //  Sort the changes by impact descenging
    std::sort( changes.begin(), changes.end(), []( const struct change_t &a, const struct change_t &b ) { return a.impact()>b.impact(); } );

    if (display)
    {
        for (auto &c: changes)
        {
            printf( "Index:%4d Char:%4d->%4d->%4d Delta:%4d->%4d->%4d\n", c.index, c.char_before, c.char_current, c.char_after, c.delta_before, c.delta_after, c.detal_skip );
        }
    }

    //  Truncate the list to the 300 most impactful changes
    if (changes.size()>10000)
        changes.resize(10000);
    
    return changes;
}

class span_t
{
public:
    u_int16_t start_;
    u_int16_t end_;
    std::vector<u_int16_t> data_;

    span_t( const change_t &c ) : start_(c.index), end_(c.index), data_{(u_int16_t)c.char_current} {}

    // Return true if the change is just at the end of the span
    bool is_adjacent( const change_t &c ) const
    {
        return c.index==end_+1;
    }

    // Add a change to the span
    void add( const change_t &c )
    {
        assert( is_adjacent( c ) );
        end_ = c.index;
        data_.push_back( c.char_current );
    }
};

//  Constructs a list of spans from an ordered list of changes
std::vector<span_t> make_spans(const std::vector<change_t>& changes)
{
    std::vector<span_t> spans;
    if (changes.empty()) {
        return spans;
    }
    spans.push_back(span_t(changes[0]));
    for (size_t i = 1; i < changes.size(); i++) {
        if (spans.back().is_adjacent(changes[i])) {
            spans.back().add(changes[i]);
        } else {
            spans.push_back(span_t(changes[i]));
        }
    }
    return spans;
}

class change_assembler
{
    std::vector<uint16_t> data_;        //  Data to transfer
    std::vector<uint8_t> instructions;  //  Instructions to execute to transfer the data
    std::vector<uint16_t> offsets_;     //  Offsets of the instruction streams

    size_t recover_data;
    size_t recover_instructions;

    std::vector<uint16_t> current_data_;

    bool verbose_ = false;

    void mov_si( uint16_t value )
    {
        instructions.push_back( 0xbe );             
        instructions.push_back( value&0xff );
        instructions.push_back( value>>8 );

        if (verbose_)
            printf( "    mov si,0x%04x\n", value );
    }

    void mov_di( uint16_t value, uint16_t data )
    {
        instructions.push_back( 0xbf );             
        instructions.push_back( value&0xff );
        instructions.push_back( value>>8 );

        if (verbose_)
            printf( "    mov di,0x%04x ; 0x%04x\n", value, data );
    }

    void movsw()
    {
        instructions.push_back( 0xa5 );             

        if (verbose_)
            printf( "    movsw\n" );
    }

    void retf()
    {
        instructions.push_back( 0xcb );             
        if (verbose_)
            printf( "    retf\n" );
    }

    void mov_cx( u_int16_t value )
    {
        instructions.push_back( 0xb9 );             
        instructions.push_back( value&0xff );
        instructions.push_back( value>>8 );

        if (verbose_)
            printf( "    mov cx,0x%04x\n", value );
    }

    void rep_movsw()
    {
        instructions.push_back( 0xf3 );             
        instructions.push_back( 0xa5 );             

        if (verbose_)
            printf( "    rep movsw\n" );
    }

    void start()
    {
        current_data_.clear();

        recover_data = data_.size();
        recover_instructions = instructions.size();

            //  Offset of this code stream
        offsets_.push_back( instructions.size() );

            //  mov si, start of data
        mov_si( data_.size()*2 );
    }

    void add_change( const struct change_t &c )
    {
            // Add the character
        current_data_.push_back( c.char_current );
        data_.push_back( c.char_current );
            //  mov di, c.index
        mov_di( c.index*2, c.char_current );
            //  movsw
        movsw();
    }

    void add_change( u_int16_t index, u_int16_t data )
    {
            // Add the character
        current_data_.push_back( data );
        data_.push_back( data );
            //  mov di, c.index
        mov_di( index*2, data );
            //  movsw
        movsw();
    }

    void add_change2( u_int16_t index, u_int16_t data0, u_int16_t data1 )
    {
            // Add the character
        current_data_.push_back( data0 );
        current_data_.push_back( data1 );
        data_.push_back( data0 );
        data_.push_back( data1 );
            //  mov di, c.index
        mov_di( index*2, data0 );
            //  movsw
        movsw();
        movsw();
    }

    void add_change3( u_int16_t index, u_int16_t data0, u_int16_t data1, u_int16_t data2 )
    {
            // Add the character
        current_data_.push_back( data0 );
        current_data_.push_back( data1 );
        current_data_.push_back( data2 );
        data_.push_back( data0 );
        data_.push_back( data1 );
        data_.push_back( data2 );
            //  mov di, c.index
        mov_di( index*2, data0 );
            //  movsw
        movsw();
        movsw();
        movsw();
    }

    void add_changes( u_int16_t index, std::vector<u_int16_t> data )
    {
            // Add the character
        for (auto &d: data)
        {
            current_data_.push_back( d );
            data_.push_back( d );
        }
            //  mov di, c.index
        mov_di( index*2, data[0] );
            //  mov cx, data.size()
        mov_cx( data.size() );
            //  rep_movsw
        rep_movsw();
    }

    void add_span( const span_t &s )
    {
        if (s.end_==s.start_)
        {
            add_change( s.start_, s.data_[0] );
        }
        else if (s.end_-s.start_==1)
        {
            add_change2( s.start_, s.data_[0], s.data_[1] );
        }
        else if (s.end_-s.start_==2)
        {
            add_change3( s.start_, s.data_[0], s.data_[1], s.data_[2] );
        }
        else
        {
            add_changes( s.start_, s.data_ );
        }
    }

    void stop()
    {
            //  retf
        retf();

            //  Add the data
        // data_.insert( std::end(data_), std::begin(current_data_), std::end(current_data_) );

#if DUMP
        printf( "\n\n\n\n" );
#endif
        // Print content
    }

    size_t calc_size() const
    {
        return data_.size()*2 + instructions.size() + offsets_.size()*2 + 2;
    }

    void revert()
    {
        data_.resize( recover_data );
        instructions.resize( recover_instructions );
        offsets_.pop_back();
    }

public:
    bool add_spans( const std::vector<span_t> &spans, bool verbose=false )
    {
        verbose_ = verbose;
        start();
        for (auto &c: spans)
            add_span( c );
        stop();
        if (calc_size()>32768)
        {
            revert();
            return false;
        }
        return true;
    }

    std::vector<uint8_t> as_vector( uint16_t size ) const
    {
        std::vector<uint8_t> result;
        //  Add the data
        for (auto &d: data_)
        {
            result.push_back( d&0xff );
            result.push_back( d>>8 );
        }
        auto code_start = result.size();
        //  Add the instructions
        for (auto &i: instructions)
            result.push_back( i );
        //  Fill the rest 
        while (result.size()<size)
            result.push_back( 0xaa );
        //  Replace the end with offsets
        auto p = std::end(result);
        for (auto &o: offsets_)
        {
            *--p = (o+code_start)>>8;
            *--p = (o+code_start)&0xff;
        }
        *--p = 0xff;
        *--p = 0xff;

printf( "Generated block of %d bytes\n", result.size() );

        return result;
    }
};


// Main takes two optional arguments
// -in filename : input filename (default in.txt)
// -out filename : output filename (default OUT.VID)
int main( int argc, char **argv )
{

    // FILE *fp = fopen("OUT.VID", "wb");
    // test( fp );
    // fclose(fp);
    // exit(0);


    //  Parse arguments
    std::string in_filename = "in.txt";
    std::string out_filename = "OUT.VID";
    for (int i=1;i!=argc;i++)
    {
        if (!strcmp(argv[i], "-in"))
        {
            if (i+1<argc)
                in_filename = argv[i+1];
        }
        if (!strcmp(argv[i], "-out"))
        {
            if (i+1<argc)
                out_filename = argv[i+1];
        }
    }

    // Open IN.VID file for writing
    std::ofstream video_file(out_filename, std::ios::binary | std::ios::out);

printf( "Reading from %s, writing to %s\n", in_filename.c_str(), out_filename.c_str() );

    //  Open in.tx for reading as a std::istream
    std::ifstream in_file(in_filename);   

    //  Read one string from input, check it is "FONT"
    char font[4];
    in_file.read(font, 4);
    if (strncmp(font, "FONT",4))
    {
        fprintf( stderr, "Invalid file format (FONT not found), read [%c%c%c%c]\n", font[0], font[1], font[2], font[3] );
        return 1;
    }

    //  Read the number of chars from stdin
    uint16_t num_chars;
    in_file >> num_chars;
    if (num_chars>MAXCHARS)
    {
        std::cerr << "Too many chars" << std::endl;
        return 1;
    }

    printf( "Using palette of %d chars\n", num_chars );

    //  Write the size of the font to the file
    // video_file.write((char*)&num_chars, sizeof(num_chars));

    printf( "READING FONT\n" );

    std::vector<uint8_t> font_data;

    for (size_t i=0;i!=num_chars;i++)
    {
        for (int j=0;j!=8;j++)
        {
            int d = 0;
            // Each line contains 8 '0' or '1' that represent the 8 pixels of the character
            for (int k=0;k!=8;k++)
            {
                char c;
                do
                {
                    in_file >> c;
                } while (c!='0' && c!='1');
                d = (d << 1) | (c - '0');
            }
            video_chars[i][j] = d;
        }
        video_chars[i].make_block();
        //  Write all the characters to the file
        // video_file.write((char*)video_chars[i].as_uint8().data(), 16);
        auto data = video_chars[i].as_uint8();
        font_data.insert(font_data.end(), data.begin(), data.end());
    }

    while (font_data.size()<0xC000)
        font_data.push_back(0);

    WriteHGPHeader( video_file, font_data.data() );

    std::vector<int> replacement(num_chars, 0);
    // Fill replacements with 0 to num_chars
    std::iota(replacement.begin(), replacement.end(), 0);

    //  Find all chars that have a distance of 0
    // std::vector<int> zero_distance;
    int count_identical = 0;
    for (int i=0;i!=num_chars;i++)
    {
        for (int j=0;j<i-1;j++)
        {
            if (distance(video_chars[i], video_chars[j])==0)
            {
                printf( "Chars %d and %d have a distance of 0\n", i, j );
                // zero_distance.push_back(i);
                // video_chars[i].dump();
                // video_chars[j].dump();
                count_identical++;
                replacement[i] = j;
                break;
            }
        }
    }

    printf( "%d characters are redundant\n", count_identical );

    printf( "READING FRAMES\n" );

    //  Read one string from stdin, check it is "FRAMES"
    std::string str;
    in_file >> str;
    printf( "[%s]\n", str.c_str() );

    if (str!="FRAMES")
    {
        std::cerr << "Invalid file format (FRAMES not found)" << std::endl;
        return 1;
    }

    u_int16_t w;
    u_int16_t h;
    in_file >> w;
    in_file >> h;
    printf( "[%dx%d]\n", w,h );

    //  Write the width and height to the file
//    video_file.write((char*)&w, sizeof(w));
//    video_file.write((char*)&h, sizeof(h));

    screen_t s0(w,h);   //  two frames before
    screen_t s1(w,h);   //  one frame before

    long total_changes = 0;
    int frame = 0;
    //  Reads a screen of data from stdin
    change_assembler ca;

    long total_bytes = 0;

    while (in_file)
    {
            //  Read screen
        screen_t s(w,h);
        for (int i=0;i!=w*h;i++)
        {
            in_file >> s[i];
            s[i] = replacement[s[i]];
        }

        // s.dump();

        total_changes += s.count_diff(s1);

        printf( "Frame %4d - ", frame );
        auto changes = list_changes( s0, s1, s, frame==271 );
        // Sorts the change by index
        std::sort( changes.begin(), changes.end(), []( const struct change_t &a, const struct change_t &b ) { return a.index<b.index; } );
        // Generates the spans
        auto spans = make_spans(changes);
        s1 = s0;
        apply_changes( s1, changes );
            //  Write screen (oops for the endianess...)
        // video_file.write( (char *)s1.get_uint16(), w*h*2 );

        if (!ca.add_spans( spans, frame==271 ))
        {
            auto data = ca.as_vector( 0x8000 );
            video_file.write( (char *)data.data(), 0x8000 );
            total_bytes += 0x8000;
            ca = change_assembler();
            ca.add_spans( spans );
        }

        write_grayscale_png(w*8, h*8, s1.make_image(), "/tmp/out/frame" + std::to_string(frame) + ".png");

        s0 = s1;
        s1 = s;

        frame++;

        // if (frame==1000)
        //     break;
    }

    //  Write the last changes
    auto data = ca.as_vector( 0x8000 );
    video_file.write( (char *)data.data(), 0x8000 );
    total_bytes += 0x8000;

    printf( "\nTotal changes: %ld, average change: %f, average screen change: %02.2f%%\n", total_changes, (double)total_changes/frame, ((double)total_changes/frame)/(w*h)*100 );
    printf( "  Average bytes per frame: %d\n", total_bytes/frame );

    // Close the file
    video_file.close();

    return 0;
}
