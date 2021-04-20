/*
Copyright (c) 2013, Ralf Willenbacher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/


#include "h261_decl.h"

#include "h261_transform_x86.h"

Void ( *g_fdct_8x8 ) ( Int16 *pi16_source, Int16 *pi16_destination );
Void ( *g_idct_8x8 ) ( Int16 *pi16_source, Int16 *pi16_destination );
Int32 ( *g_quant_inter_8x8 )( Int16 *pi16_coeff, Int32 i_inv_scale );

#define RND1BITS ( 11 )
#define RND2BITS ( 31 - RND1BITS )

static const Int16 rgi16_h261_fdct_cs1[ 8 ][ 8 ] = {
		{ 16383,  16383,  16383,  16383,  16383,  16383,  16383,  16383 },
		{ 22724,  19265,  12872,   4520,  -4520, -12872, -19265, -22724 },
		{ 21406,   8867,  -8867, -21406, -21406,  -8867,   8867,  21406 },
		{ 19265,  -4520, -22724, -12872,  12872,  22724,   4520, -19265 },
		{ 16383, -16383, -16383,  16383,  16383, -16383, -16383,  16383 },
		{ 12872, -22724,   4520,  19265, -19265,  -4520,  22724, -12872 },
		{  8867, -21406,  21406,  -8867,  -8867,  21406, -21406,   8867 },
		{  4520, -12872,  19265, -22724,  22724, -19265,  12872,  -4520 },
};
static const Int16 rgi16_h261_fdct_cs2[ 8 ][ 8 ] = {
		{ 16385,  16385,  16385,  16385,  16385,  16385,  16385,  16385 },
		{ 22726,  19266,  12873,   4521,  -4521, -12873, -19266, -22726 },
		{ 21408,   8867,  -8867, -21408, -21408,  -8867,   8867,  21408 },
		{ 19266,  -4521, -22726, -12873,  12873,  22726,   4521, -19266 },
		{ 16385, -16385, -16385,  16385,  16385, -16385, -16385,  16385 },
		{ 12873, -22726,   4521,  19266, -19266,  -4521,  22726, -12873 },
		{  8867, -21408,  21408,  -8867,  -8867,  21408, -21408,   8867 },
		{  4521, -12873,  19266, -22726,  22726, -19266,  12873,  -4521 },
};



Void h261_fdct_8x8_REF( Int16 *pi16_block, Int16 *pi16_dst )
{
	int i_i, i_j, i_k;
	Int32 i_s;
	Int16 rgi16_tmp[ 64 ];
	Int32 rgi16_e[ 4 ][ 8 ], rgi16_ee[ 2 ][ 8 ];

#define RND( x, y ) ( ( ( x ) + ( ( y ) ? ( 1 << ( y - 1 ) ) : 0 ) ) >> ( y ) )
#define MUL( x, m ) ( ( x ) * ( m ) )

	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 1; i_j < 8; i_j += 2 )
		{
			i_s = 0;
			for( i_k = 0; i_k < 4; i_k++ )
			{
				i_s += rgi16_h261_fdct_cs1[ i_j ][ i_k ] * ( pi16_block[ 8 * i_k + i_i ] - pi16_block[ 8 * ( 7 - i_k ) + i_i ] );
			}
			rgi16_tmp[ 8 * i_i + i_j ] = RND( i_s, RND1BITS );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_k = 0; i_k < 4; i_k++ )
		{
			rgi16_e[ i_k ][ i_i ] = ( pi16_block[ 8 * i_k + i_i ] + pi16_block[ 8 * ( 7 - i_k ) + i_i ] );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 2; i_j < 8; i_j += 4 )
		{
			i_s = 0;
			for( i_k = 0; i_k < 2; i_k++ )
			{
				i_s += rgi16_h261_fdct_cs1[ i_j ][ i_k ] * ( rgi16_e[ i_k ][ i_i ] - rgi16_e[ 3 - i_k ][ i_i ] );
			}
			rgi16_tmp[ 8 * i_i + i_j ] = RND( i_s, RND1BITS );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_k = 0; i_k < 2; i_k++ )
		{
			rgi16_ee[ i_k ][ i_i ] = ( rgi16_e[ i_k ][ i_i ] + rgi16_e[ 3 - i_k ][ i_i ] );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 0; i_j < 8; i_j += 4 )
		{
			i_s = 0;
			for( i_k = 0; i_k < 2; i_k++ )
			{
				i_s += rgi16_h261_fdct_cs1[ i_j ][ i_k ] * rgi16_ee[ i_k ][ i_i ];
			}
			rgi16_tmp[ 8 * i_i + i_j ] = RND( i_s, RND1BITS );
		}
	}

	/* ... */

	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 1; i_j < 8; i_j += 2 )
		{
			i_s = 0;
			for( i_k = 0; i_k < 4; i_k++ )
			{
				i_s += rgi16_h261_fdct_cs2[ i_j ][ i_k ] * ( rgi16_tmp[ 8 * i_k + i_i ] - rgi16_tmp[ 8 * ( 7 - i_k ) + i_i ] );
			}
			pi16_dst[ 8 * i_i + i_j ] = RND( i_s, RND2BITS );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_k = 0; i_k < 4; i_k++ )
		{
			rgi16_e[ i_k ][ i_i ] = ( rgi16_tmp[ 8 * i_k + i_i ] + rgi16_tmp[ 8 * ( 7 - i_k ) + i_i ] );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 2; i_j < 8; i_j += 4 )
		{
			i_s = 0;
			for( i_k = 0; i_k < 2; i_k++ )
			{
				i_s += rgi16_h261_fdct_cs2[ i_j ][ i_k ] * ( rgi16_e[ i_k ][ i_i ] - rgi16_e[ 3 - i_k ][ i_i ] );
			}
			pi16_dst[ 8 * i_i + i_j ] = RND( i_s, RND2BITS );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_k = 0; i_k < 2; i_k++ )
		{
			rgi16_ee[ i_k ][ i_i ] = ( rgi16_e[ i_k ][ i_i ] + rgi16_e[ 3 - i_k ][ i_i ] );
		}
	}
	for( i_i = 0; i_i < 8; i_i++ )
	{
		for( i_j = 0; i_j < 8; i_j += 4 )
		{
			i_s = 0;
			for( i_k = 0; i_k < 2; i_k++ )
			{
				i_s += rgi16_h261_fdct_cs2[ i_j ][ i_k ] * rgi16_ee[ i_k ][ i_i ];
			}
			pi16_dst[ 8 * i_i + i_j ] = RND( i_s, RND2BITS );
		}
	}
}


#define RND1BITS ( 11 )
#define RND2BITS ( 31 - RND1BITS )

static const Int16 rgi16_h261_idct_cs1[ 8 ][ 8 ] = {
		{ 16383,  16383,  16383,  16383,  16383,  16383,  16383,  16383 },
		{ 22724,  19265,  12872,   4520,  -4520, -12872, -19265, -22724 },
		{ 21406,   8867,  -8867, -21406, -21406,  -8867,   8867,  21406 },
		{ 19265,  -4520, -22724, -12872,  12872,  22724,   4520, -19265 },
		{ 16383, -16383, -16383,  16383,  16383, -16383, -16383,  16383 },
		{ 12872, -22724,   4520,  19265, -19265,  -4520,  22724, -12872 },
		{  8867, -21406,  21406,  -8867,  -8867,  21406, -21406,   8867 },
		{  4520, -12872,  19265, -22724,  22724, -19265,  12872,  -4520 },
};
static const Int16 rgi16_h261_idct_cs2[ 8 ][ 8 ] = {
		{ 16385,  16385,  16385,  16385,  16385,  16385,  16385,  16385 },
		{ 22726,  19266,  12873,   4521,  -4521, -12873, -19266, -22726 },
		{ 21408,   8867,  -8867, -21408, -21408,  -8867,   8867,  21408 },
		{ 19266,  -4521, -22726, -12873,  12873,  22726,   4521, -19266 },
		{ 16385, -16385, -16385,  16385,  16385, -16385, -16385,  16385 },
		{ 12873, -22726,   4521,  19266, -19266,  -4521,  22726, -12873 },
		{  8867, -21408,  21408,  -8867,  -8867,  21408, -21408,   8867 },
		{  4521, -12873,  19266, -22726,  22726, -19266,  12873,  -4521 },
};


void h261_idct_8x8_REF( Int16 *pi16_block, Int16 *pi16_dst )
{
	int i_j, i_k;
	Int16 rgi16_tmp[ 64 ];
	Int32 rgi_e[ 4 ], rgi_o[ 4 ];
	Int32 rgi_ee[ 2 ], rgi_eo[ 2 ];


#define RND( x, y ) ( ( ( x ) + ( ( y ) ? ( 1 << ( y - 1 ) ) : 0 ) ) >> ( y ) )
#define MUL( x, m ) ( ( x ) * ( m ) )

	for( i_j = 0; i_j < 8; i_j++ )
	{
		rgi_o[ 0 ] = rgi16_h261_idct_cs1[ 1 ][ 0 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_h261_idct_cs1[ 3 ][ 0 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_h261_idct_cs1[ 5 ][ 0 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_h261_idct_cs1[ 7 ][ 0 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 1 ] = rgi16_h261_idct_cs1[ 1 ][ 1 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_h261_idct_cs1[ 3 ][ 1 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_h261_idct_cs1[ 5 ][ 1 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_h261_idct_cs1[ 7 ][ 1 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 2 ] = rgi16_h261_idct_cs1[ 1 ][ 2 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_h261_idct_cs1[ 3 ][ 2 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_h261_idct_cs1[ 5 ][ 2 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_h261_idct_cs1[ 7 ][ 2 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 3 ] = rgi16_h261_idct_cs1[ 1 ][ 3 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_h261_idct_cs1[ 3 ][ 3 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_h261_idct_cs1[ 5 ][ 3 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_h261_idct_cs1[ 7 ][ 3 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_eo[ 0 ] = rgi16_h261_idct_cs1[ 2 ][ 0 ] * pi16_block[ i_j + 8 * 2 ] + rgi16_h261_idct_cs1[ 6 ][ 0 ] * pi16_block[ i_j + 8 * 6 ];
		rgi_eo[ 1 ] = rgi16_h261_idct_cs1[ 2 ][ 1 ] * pi16_block[ i_j + 8 * 2 ] + rgi16_h261_idct_cs1[ 6 ][ 1 ] * pi16_block[ i_j + 8 * 6 ];
		rgi_ee[ 0 ] = rgi16_h261_idct_cs1[ 0 ][ 0 ] * pi16_block[ i_j + 8 * 0 ] + rgi16_h261_idct_cs1[ 4 ][ 0 ] * pi16_block[ i_j + 8 * 4 ];
		rgi_ee[ 1 ] = rgi16_h261_idct_cs1[ 0 ][ 1 ] * pi16_block[ i_j + 8 * 0 ] + rgi16_h261_idct_cs1[ 4 ][ 1 ] * pi16_block[ i_j + 8 * 4 ];

		rgi_e[ 0 ] = rgi_ee[ 0 ] + rgi_eo[ 0 ];
		rgi_e[ 1 ] = rgi_ee[ 1 ] + rgi_eo[ 1 ];
		rgi_e[ 2 ] = rgi_ee[ 1 ] - rgi_eo[ 1 ];
		rgi_e[ 3 ] = rgi_ee[ 0 ] - rgi_eo[ 0 ];

		for( i_k = 0; i_k < 4; i_k++ )
		{
			rgi16_tmp[ i_j + 8 * i_k ] = RND( rgi_e[ i_k ] + rgi_o[ i_k ], RND1BITS );
			rgi16_tmp[ i_j + 8 * ( i_k + 4 ) ] = RND( rgi_e[ 3 - i_k ] - rgi_o[ 3 - i_k ], RND1BITS );
		}
	}

	for( i_j = 0; i_j < 8; i_j++ )
	{
		rgi_e[ 0 ] = rgi16_h261_idct_cs2[ 0 ][ 0 ] * rgi16_tmp[ i_j * 8 + 0 ] + rgi16_h261_idct_cs2[ 2 ][ 0 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_h261_idct_cs2[ 4 ][ 0 ] * rgi16_tmp[ i_j * 8 + 4 ] + rgi16_h261_idct_cs2[ 6 ][ 0 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 1 ] = rgi16_h261_idct_cs2[ 0 ][ 1 ] * rgi16_tmp[ i_j * 8 + 0 ] + rgi16_h261_idct_cs2[ 2 ][ 1 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_h261_idct_cs2[ 4 ][ 1 ] * rgi16_tmp[ i_j * 8 + 4 ] + rgi16_h261_idct_cs2[ 6 ][ 1 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 2 ] = rgi16_h261_idct_cs2[ 0 ][ 1 ] * rgi16_tmp[ i_j * 8 + 0 ] + -rgi16_h261_idct_cs2[ 2 ][ 1 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_h261_idct_cs2[ 4 ][ 1 ] * rgi16_tmp[ i_j * 8 + 4 ] + -rgi16_h261_idct_cs2[ 6 ][ 1 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 3 ] = rgi16_h261_idct_cs2[ 0 ][ 0 ] * rgi16_tmp[ i_j * 8 + 0 ] + -rgi16_h261_idct_cs2[ 2 ][ 0 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_h261_idct_cs2[ 4 ][ 0 ] * rgi16_tmp[ i_j * 8 + 4 ] + -rgi16_h261_idct_cs2[ 6 ][ 0 ] * rgi16_tmp[ i_j * 8 + 6 ];

		rgi_o[ 0 ] = rgi16_h261_idct_cs2[ 1 ][ 0 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_h261_idct_cs2[ 3 ][ 0 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_h261_idct_cs2[ 5 ][ 0 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_h261_idct_cs2[ 7 ][ 0 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 1 ] = rgi16_h261_idct_cs2[ 1 ][ 1 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_h261_idct_cs2[ 3 ][ 1 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_h261_idct_cs2[ 5 ][ 1 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_h261_idct_cs2[ 7 ][ 1 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 2 ] = rgi16_h261_idct_cs2[ 1 ][ 2 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_h261_idct_cs2[ 3 ][ 2 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_h261_idct_cs2[ 5 ][ 2 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_h261_idct_cs2[ 7 ][ 2 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 3 ] = rgi16_h261_idct_cs2[ 1 ][ 3 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_h261_idct_cs2[ 3 ][ 3 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_h261_idct_cs2[ 5 ][ 3 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_h261_idct_cs2[ 7 ][ 3 ] * rgi16_tmp[ i_j * 8 + 7 ];

		for( i_k = 0; i_k < 4; i_k++ )
		{
			pi16_dst[ i_j * 8 + i_k ] = RND( rgi_e[ i_k ] + rgi_o[ i_k ], RND2BITS );
			pi16_dst[ i_j * 8 + ( i_k + 4 ) ] = RND( rgi_e[ 3 - i_k ] - rgi_o[ 3 - i_k ], RND2BITS );
		}
	}
}





#define CLAMP_127( x ) ( ( x ) < -127 ? -127 : ( ( x ) > 127 ? 127 : ( x ) ) )

Void h261_quant8x8_intra_fw( Int32 *pi_coeffs, Int32 i_stride, Int32 i_quant )
{
	Int32 i_y, i_x, i_dc;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			if( i_y == 0 && i_x == 0 )
			{
				i_dc = pi_coeffs[ 0 ] / 8;
				if( i_dc < 1 )
				{
					i_dc = 1;
				}
				else if( i_dc > 254 )
				{
					i_dc = 254;
				}
				if( i_dc == 128 )
				{
					i_dc = 255;
				}
				pi_coeffs[ 0 ] = i_dc;
			}
			else
			{
				pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_127( pi_coeffs[ i_y * i_stride + i_x ] / ( i_quant * 2 ) );
			}
		}
	}
}


#define CLAMP_2047( x ) ( ( x ) < -2048 ? -2048 : ( ( x ) > 2047 ? 2047 : ( x ) ) )

Void h261_quant8x8_intra_bw( Int16 *pi_coeffs, Int32 i_stride, Int32 i_quant )
{
	Int32 i_y, i_x, i_dc;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			if( i_y == 0 && i_x == 0 )
			{
				i_dc = pi_coeffs[ 0 ];
				if( i_dc == 255 )
				{
					i_dc = 1024;
				}
				else
				{
					i_dc = i_dc * 8;
				}
				pi_coeffs[ 0 ] = i_dc;
			}
			else
			{
				Int32 i_level;
				i_level = pi_coeffs[ i_y * i_stride + i_x ];
				if( i_quant & 1 )
				{
					if( i_level > 0 )
					{
						pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( ( i_level * 2 + 1 ) * i_quant );
					}
					else if( i_level < 0 )
					{
						pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( ( i_level * 2 - 1 ) * i_quant );
					}
					else
					{
						pi_coeffs[ i_y * i_stride + i_x ] = 0;
					}
				}
				else
				{
					if( i_level > 0 )
					{
						pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( ( ( i_level * 2 + 1 ) * i_quant ) - 1 );
					}
					else if( i_level < 0 )
					{
						pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( ( ( i_level * 2 - 1 ) * i_quant ) + 1 );
					}
					else
					{
						pi_coeffs[ i_y * i_stride + i_x ] = 0;
					}
				}
			}
		}
	}
}



Int32 h261_quant8x8_inter_fw( Int16 *pi_coeffs, Int32 i_stride, Int32 i_quant )
{
	Int32 i_y, i_x, i_level, i_inv_scale;
	Int32 i_coeff_accum;

	i_coeff_accum = 0;

	i_inv_scale = ( 1 << 16 ) / ( i_quant * 2 );

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			if( pi_coeffs[ i_y * i_stride + i_x ] > 0 )
			{
				i_level = CLAMP_127( ( pi_coeffs[ i_y * i_stride + i_x ] * i_inv_scale ) >> 16 );
			}
			else
			{
				i_level = -CLAMP_127( ( -pi_coeffs[ i_y * i_stride + i_x ] * i_inv_scale ) >> 16 );
			}
			
			pi_coeffs[ i_y * i_stride + i_x ] = i_level;
			i_coeff_accum |= i_level;
		}
	}
	return i_coeff_accum;
}

Int32 h261_quant8x8_inter_REF( Int16 *pi_coeffs, Int32 i_inv_scale )
{
	Int32 i_y, i_x, i_level;
	Int32 i_coeff_accum;

	i_coeff_accum = 0;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			if( pi_coeffs[ i_y * 8 + i_x ] > 0 )
			{
				i_level = CLAMP_127( ( pi_coeffs[ i_y * 8 + i_x ] * i_inv_scale ) >> 16 );
			}
			else
			{
				i_level = -CLAMP_127( ( -pi_coeffs[ i_y * 8 + i_x ] * i_inv_scale ) >> 16 );
			}
			
			pi_coeffs[ i_y * 8 + i_x ] = i_level;
			i_coeff_accum |= i_level;
		}
	}
	return i_coeff_accum;
}


Int32 h261_size_run_level( Int32 i_run, Int32 i_level )
{
	Int32 i_ulevel, i_vlc_table_idx;
	i_ulevel = i_level < 0 ? -i_level : i_level;

	if( i_run < 27 && i_ulevel < 16 )
	{
		i_vlc_table_idx = rgi_h261_run_level_lut[ i_run ][ i_ulevel ];
	}
	else
	{
		i_vlc_table_idx = 63;
	}
	
	if( i_vlc_table_idx != 63 )
	{
		return h261_coeff_run_level_table[ i_vlc_table_idx ].s_vlc.i_length + 1;
	}
	else
	{
		return 20;
	}
}


Int32 h261_quant8x8_trellis_fw( h261_context_t *ps_ctx, Int16 *pi_coeffs, Int32 i_quant, Int32 i_intra )
{
	Int32 i_dc, i_nz;
	Int32 i_coeff, i_start, i_num_coeff, i_last_coeff, i_idx, i_idx2, i_x, i_y, i_inv_scale, i_run, i_level;
	Int16 rgi16_levels[ 64 ];
	Int32 rgi_coeffs[ 64 ];
	Int32 rgi_idx[ 64 ];
	Int32 rgi_level[ 64 ];

	Int32 i_active_toggle;
	Int32 rgi_path_active[ 2 ][ 65 ];
	Int32 rgi_path_idx[ 65 ][ 65 ];
	Int32 rgi_path_level[ 65 ][ 65 ];
	Int32 rgi_path_cost[ 65 ];
	Int32 i_candidate_level, i_dir, i_end, i_ssd, i_bits, i_cost, i_num_path_coeffs, i_lambda;

	i_lambda = ps_ctx->s_picture.s_groups_of_blocks.s_macroblock.i_lambda_rdo;

	i_inv_scale = ( 1 << 16 ) / ( i_quant * 2 );

	i_run = 0;
	i_last_coeff = -1;
	i_num_coeff = 0;

	if( i_intra )
	{
		i_start = 1;
		i_dc = pi_coeffs[ 0 ] / 8;
		if( i_dc < 1 )
		{
			i_dc = 1;
		}
		else if( i_dc > 254 )
		{
			i_dc = 254;
		}
		if( i_dc == 128 )
		{
			i_dc = 255;
		}
		pi_coeffs[ 0 ] = i_dc;
	}
	else
	{
		i_dc = 0;
		i_start = 0;
	}

	memcpy( rgi16_levels, pi_coeffs, sizeof( Int16 ) * 64 );

	if( i_intra )
	{
		rgi16_levels[ 0 ] = 0;
	}
	i_nz = g_quant_inter_8x8( rgi16_levels, i_inv_scale );

	if( i_nz )
	{
		for( i_idx = i_start; i_idx < 64; i_idx++ )
		{
			i_level = rgi16_levels[ rgi_transmission_order_table[ i_idx ] ];

			if( i_level != 0 )
			{
				rgi_coeffs[ i_num_coeff ] = pi_coeffs[ rgi_transmission_order_table[ i_idx ] ];
				rgi_idx[ i_num_coeff ] = i_idx;
				rgi_level[ i_num_coeff ] = i_level;
				i_num_coeff++;
				i_last_coeff = i_idx;
			}
		}
	}

	memset( pi_coeffs + i_start, 0, sizeof( Int16 ) * ( 64 - i_start ) );
	if( !i_nz )
	{
		return i_dc;
	}
	if( i_last_coeff < 0 )
	{
		return 0;
	}
	
	memset( &rgi_path_active, 0, sizeof( rgi_path_active ) );
	memset( &rgi_path_cost, 0, sizeof( rgi_path_cost ) );

	i_active_toggle = 0;
	rgi_path_active[ i_active_toggle ][ 0 ] = 1;

	for( i_idx = 0; i_idx < i_num_coeff; i_idx++ )
	{
		i_level = rgi_level[ i_idx ];

		if( i_level > 0 )
		{
			i_dir = -1;
		}
		else
		{
			i_dir = 1;
		}
		i_end = i_level + i_dir * 2;
		for( i_candidate_level = i_level; i_candidate_level != i_end; i_candidate_level += i_dir )
		{
			if( i_quant & 1 )
			{
				if( i_candidate_level > 0 )
				{
					i_coeff = CLAMP_2047( ( i_candidate_level * 2 + 1 ) * i_quant );
				}
				else if( i_level < 0 )
				{
					i_coeff = CLAMP_2047( ( i_candidate_level * 2 - 1 ) * i_quant );
				}
				else
				{
					i_coeff = 0;
				}
			}
			else
			{
				if( i_candidate_level > 0 )
				{
					i_coeff = CLAMP_2047( ( ( i_candidate_level * 2 + 1 ) * i_quant ) - 1 );
				}
				else if( i_level < 0 )
				{
					i_coeff = CLAMP_2047( ( ( i_candidate_level * 2 - 1 ) * i_quant ) + 1 );
				}
				else
				{
					i_coeff = 0;
				}
			}
			i_ssd = ( i_coeff - rgi_coeffs[ i_idx ] ) * ( i_coeff - rgi_coeffs[ i_idx ] );

			for( i_idx2 = i_idx; i_idx2 >= 0; i_idx2-- )
			{
				if( rgi_path_active[ i_active_toggle ][ i_idx2 ] )
				{
					if( i_candidate_level != 0 )
					{
						Int32 i_run;
						if( i_idx2 > 0 )
						{
							i_run = rgi_idx[ i_idx ] - rgi_path_idx[ i_idx2 ][ i_idx2 ] - 1;
						}
						else
						{
							i_run = rgi_idx[ i_idx ];
						}
						i_bits = h261_size_run_level( i_run, i_candidate_level );
						i_cost = rgi_path_cost[ i_idx2 ] + ( ( i_bits * i_lambda ) >> 8 ) + i_ssd;

						if( !rgi_path_active[ !i_active_toggle ][ i_idx2 + 1 ] || i_cost < rgi_path_cost[ i_idx2 + 1 ] )
						{
							memcpy( &rgi_path_idx[ i_idx2 + 1 ][ 0 ], &rgi_path_idx[ i_idx2 ][ 0 ], sizeof( Int32 ) * ( i_idx2 + 1 ) );
							memcpy( &rgi_path_level[ i_idx2 + 1 ][ 0 ], &rgi_path_level[ i_idx2 ][ 0 ], sizeof( Int32 ) * ( i_idx2 + 1 ) );
							rgi_path_idx[ i_idx2 + 1 ][ i_idx2 + 1 ] = rgi_idx[ i_idx ];
							rgi_path_level[ i_idx2 + 1 ][ i_idx2 + 1 ] = i_candidate_level;
							rgi_path_active[ !i_active_toggle ][ i_idx2 + 1 ] = 1;
							rgi_path_cost[ i_idx2 + 1 ] = i_cost;
						}
					}
					else
					{
						/* last coeff candidate iter, we can overwrite/activate current path */
						i_cost = rgi_path_cost[ i_idx2 ] + i_ssd;
						if( !rgi_path_active[ !i_active_toggle ][ i_idx2 ] || i_cost < rgi_path_cost[ i_idx2 ] )
						{
							rgi_path_active[ !i_active_toggle ][ i_idx2 ] = 1;
							rgi_path_cost[ i_idx2 ] = i_cost;
						}
					}
				}
			}
		}
		memset( &rgi_path_active[ i_active_toggle ][ 0 ], 0, sizeof( Int32 ) * 65 );
		i_active_toggle = i_active_toggle^1;
	}

	i_cost = MAX_COST;
	i_idx2 = 0;
	for( i_idx = 0; i_idx <= i_num_coeff; i_idx++ )
	{
		if( rgi_path_active[ i_active_toggle ][ i_idx ] && rgi_path_cost[ i_idx ] < i_cost )
		{
			i_idx2 = i_idx;
			i_cost = rgi_path_cost[ i_idx ];
		}
	}
	for( i_idx = 1; i_idx <= i_idx2; i_idx++ )
	{
		if( rgi_path_level[ i_idx2 ][ i_idx ] != rgi_level[ i_idx - 1 ] )
		{
			i_idx = i_idx;
		}
		pi_coeffs[ rgi_transmission_order_table[ rgi_path_idx[ i_idx2 ][ i_idx ] ] ] = rgi_path_level[ i_idx2 ][ i_idx ];
	}
	return i_idx2 != 0;
}

Void h261_quant8x8_inter_bw( Int16 *pi_coeffs, Int32 i_stride, Int32 i_quant )
{
	Int32 i_y, i_x;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			Int32 i_level;
			i_level = pi_coeffs[ i_y * i_stride + i_x ];
			if( i_quant & 1 )
			{
				if( i_level > 0 )
				{
					pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( ( i_level * 2 + 1 ) * i_quant );
				}
				else if( i_level < 0 )
				{
					pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( ( i_level * 2 - 1 ) * i_quant );
				}
				else
				{
					pi_coeffs[ i_y * i_stride + i_x ] = 0;
				}
			}
			else
			{
				if( i_level > 0 )
				{
					pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( ( ( i_level * 2 + 1 ) * i_quant ) - 1 );
				}
				else if( i_level < 0 )
				{
					pi_coeffs[ i_y * i_stride + i_x ] = CLAMP_2047( ( ( i_level * 2 - 1 ) * i_quant ) + 1 );
				}
				else
				{
					pi_coeffs[ i_y * i_stride + i_x ] = 0;
				}
			}
		}
	}
}



Void h261_initialize_transform_functions( )
{
	g_fdct_8x8 = h261_fdct_8x8_REF;
	g_idct_8x8 = h261_idct_8x8_REF;

	g_quant_inter_8x8 = h261_quant8x8_inter_REF;

	if( 1 )
	{
		g_fdct_8x8 = h261_fdct_8x8_sse2;
		g_idct_8x8 = h261_idct_8x8_sse2;
		g_quant_inter_8x8 = h261_quant_inter_8x8_sse2;
	}
}
