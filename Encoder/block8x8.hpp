//  Derived from petflix code
//  Should be turned into common utility code

//  Removed the horrible read from samples with gamma, etc

#ifndef BLOCK_INCLUDED
#define BLOCK_INCLUDED

#include <cstdint>
#include <cstddef>
#include <math.h>
#include <bitset>
#include <iostream>
#include <numeric>
#include <array>

/**
 * An 8x8 bit block, with 3 level of details
 */
class block8x8
{
    uint64_t data_; //  bitmap

    int z0_[4][4];  //  zooms
    int z1_[2][2];
    int z2_;

    bool get( int x, int y ) const { return  data_ & ( (uint64_t)1 << (y*8+(7-x)) ); }

    public:
        block8x8( const uint8_t *p, size_t stride=1 )
        {
            set_data( p, stride );

            //  Clear zoom levels
            for (int i=0;i!=4;i++)
                for (int j=0;j!=4;j++)
                    z0_[i][j] = 0;
            z1_[0][0] = z1_[0][1] = z1_[1][0] = z1_[1][1] = 0;
            z2_ = 0;

            //  Build zoom levels
            for (int y=0;y!=8;y++)
                for (int x=0;x!=8;x++)
                {
                    bool b = get(x,y);
                    if (b)
                    {
                        // std::cout << data_ << " " << x << "," << y << std::endl;
                        z0_[x/2][y/2]++;
                        z1_[x/4][y/4]++;
                        z2_++;
                    }
                }
            // std::cout << "--" << z2_ << "\n";

/* This code was added in the alternate cosntructor from the petflix code
            for (int i=0;i!=4;i++)
                for (int j=0;j!=4;j++)
                    z0_[i][j] /= 255-128/4;
            z1_[0][0] /= 255-128/16;
            z1_[0][1] /= 255-128/16;
            z1_[1][0] /= 255-128/16;
            z1_[1][1] /= 255-128/16;
            z2_ /= 255-128/64;
*/
        }

        void set_data( const uint8_t *p, size_t stride=1 )
        {
            data_ = 0;
            for (int i=0;i!=8;i++)
            {
                data_ |= ((uint64_t)*p)<<(i*8);
                p += stride;
            }
        }

        uint8_t line_at( int l ) const
        {
            return (data_>>l*8)&0xff;
        }

        std::array<int,4> distance_( const block8x8 &o ) const
        {
            std::array<int,4> res;

            std::bitset<64> bs( data_ ^ o.data_ );
            res[0] = bs.count();

            int d = 0;
            for (int x=0;x!=4;x++)
                for (int y=0;y!=4;y++)
                    d += std::abs( z0_[x][y]-o.z0_[x][y] );
            res[1] = d;

            d = 0;
            d += std::abs( z1_[0][0]-o.z1_[0][0] );
            d += std::abs( z1_[0][1]-o.z1_[0][1] );
            d += std::abs( z1_[1][0]-o.z1_[1][0] );
            d += std::abs( z1_[1][1]-o.z1_[1][1] );
            res[2] = d;

            res[3] = std::abs( z2_-o.z2_ );

            return res;
        }

        int distance( const block8x8 &o ) const
        {
            auto res = distance_( o );
            // return std::accumulate( std::begin(res), std::end(res), 0, []( auto a, auto b ){ return a+b*1.1; } );
            return res[0]+res[1]+res[2]+res[3];
        }

        void dump() const
        {
            for (int y=0;y!=8;y++)
            {
                for (int x=0;x!=8;x++)
                    std::clog << (get(x,y)?'X':'.');
                std::clog << "\n";
            }
            for (int y=0;y!=4;y++) { for (int x=0;x!=4;x++) std::clog << z0_[x][y] << " ";  std::clog << " "; }
            std::clog << "/ ";
            std::clog << z1_[0][0] << " " << z1_[0][1] << "  " << z1_[1][0] << " " << z1_[1][1] << " / " << z2_ << "\n";
        }
};

#endif
