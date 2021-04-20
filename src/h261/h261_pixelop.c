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

#include "h261_pixelop_x86.h"

Int32 ( *g_get_sad ) ( UInt8 *pui8_blk1, Int32 i_stride1, UInt8 *pui8_blk2, Int32 i_stride2 );
Int32 ( *g_get_ssd[ 2 ] ) ( UInt8 *pui8_blk1, Int32 i_stride1, UInt8 *pui8_blk2, Int32 i_stride2 );
Void ( *g_sub_8x8 ) ( Int16 *pi16_diff, UInt8 *pui8_src1, Int32 i_stride_src1, UInt8 *pui8_src2, Int32 i_stride_src2 );
Void ( *g_add_8x8 ) ( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_base, Int32 i_base_stride, Int16 *pi_difference );
Void ( *g_compensate_8x8 ) ( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_reference, Int32 i_reference_stride );
Void ( *g_compensate_8x8_filter ) ( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_reference, Int32 i_reference_stride );

Int32 h261_get_sad_REF( UInt8 *pui8_pel1, Int32 i_stride1, UInt8 *pui8_pel2, Int32 i_stride2 )
{
	Int32 i_x, i_y, i_sad;

	i_sad = 0;
	for( i_y = 0; i_y < 16; i_y++ )
	{
		for( i_x = 0; i_x < 16; i_x++ )
		{
			i_sad += H261_ABS( pui8_pel1[ i_x ] - pui8_pel2[ i_x ] );
		}
		pui8_pel1 += i_stride1;
		pui8_pel2 += i_stride2;
	}
	return i_sad;
}



#define SSD_X( lx, ly )										 \
	Int32 h261_ssd_##lx##x##ly##_REF( UInt8* pui8_blk1,		 \
									  Int32 i_stride1,		 \
									  UInt8* pui8_blk2,		 \
									  Int32 i_stride2 )		 \
{															 \
	Int32 i_ssd = 0;										 \
	Int32 x, y;												 \
	for( y = 0; y < ly; y++ )							     \
	{														 \
		for( x = 0; x < lx; x++ )							 \
		{													 \
			Int32 tmp = pui8_blk1[ x ] - pui8_blk2[ x ];     \
			i_ssd += tmp * tmp;							     \
		}													 \
		pui8_blk1 += i_stride1;								 \
		pui8_blk2 += i_stride2;								 \
	}														 \
	return i_ssd;											 \
}

SSD_X( 16, 16 )
SSD_X( 8,  8 )


Void h261_sub_8x8_REF( Int16 *pi_difference, UInt8 *pui8_src1, Int32 i_src1_stride, UInt8 *pui8_src2, Int32 i_src2_stride )
{
	Int32 i_x, i_y;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			pi_difference[ i_x ] = pui8_src1[ i_y * i_src1_stride + i_x ] - pui8_src2[ i_y * i_src2_stride + i_x ];
		}
		pi_difference += 8;
	}
}



Void compensate_8x8_REF( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_reference, Int32 i_reference_stride )
{
	Int32 i_x, i_y;
	UInt8 *pui8_dst, *pui8_src;

	pui8_dst = pui8_destination;
	pui8_src = pui8_reference;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			pui8_dst[ i_x ] = pui8_src[ i_x ];
		}
		pui8_dst += i_destination_stride;
		pui8_src += i_reference_stride;
	}
}

Void compensate_8x8_filter_REF( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_reference, Int32 i_reference_stride )
{
	Int32 rgi_temp[ 64 ], t1, t2, t3;
	Int32 i_y;
	UInt8 *pui8_dst, *pui8_src;
	Int32 *pi_tmp_src, *pi_tmp_dst;

	pui8_dst = pui8_destination;
	pui8_src = pui8_reference;
	pi_tmp_src = &rgi_temp[ 0 ];
	pi_tmp_dst = &rgi_temp[ 0 ];

	for( i_y = 0; i_y < 8; i_y++ )
	{
		t1 = pui8_src[ 0 ];
		t2 = pui8_src[ 1 ];
		t3 = pui8_src[ 2 ];
		pi_tmp_dst[ 0 ] = t1 * 4;
		pi_tmp_dst[ 1 ] = t1 + ( t2 * 2 ) + t3;
		t1 = pui8_src[ 3 ];
		pi_tmp_dst[ 2 ] = t2 + ( t3 * 2 ) + t1;
		t2 = pui8_src[ 4 ];
		pi_tmp_dst[ 3 ] = t3 + ( t1 * 2 ) + t2;
		t3 = pui8_src[ 5 ];
		pi_tmp_dst[ 4 ] = t1 + ( t2 * 2 ) + t3;
		t1 = pui8_src[ 6 ];
		pi_tmp_dst[ 5 ] = t2 + ( t3 * 2 ) + t1;
		t2 = pui8_src[ 7 ];
		pi_tmp_dst[ 6 ] = t3 + ( t1 * 2 ) + t2;
		pi_tmp_dst[ 7 ] = t2 * 4;

		pui8_src += i_reference_stride;
		pi_tmp_dst += 8;
	}

	for( i_y = 0; i_y < 8; i_y++ )
	{
		t1 = pi_tmp_src[ 0 ];
		t2 = pi_tmp_src[ 8 ];
		t3 = pi_tmp_src[ 16 ];
		pui8_dst[ 0 ] = ( t1 + 2 ) >> 2;
		pui8_dst[ i_destination_stride ] = ( t1 + ( t2 * 2 ) + t3 + 8 ) >> 4;

		t1 = pi_tmp_src[ 24 ];
		pui8_dst[ i_destination_stride * 2 ] = ( t2 + ( t3 * 2 ) + t1 + 8 ) >> 4;
		t2 = pi_tmp_src[ 32 ];
		pui8_dst[ i_destination_stride * 3 ] = ( t3 + ( t1 * 2 ) + t2 + 8 ) >> 4;
		t3 = pi_tmp_src[ 40 ];
		pui8_dst[ i_destination_stride * 4 ] = ( t1 + ( t2 * 2 ) + t3 + 8 ) >> 4;
		t1 = pi_tmp_src[ 48 ];
		pui8_dst[ i_destination_stride * 5 ] = ( t2 + ( t3 * 2 ) + t1 + 8 ) >> 4;
		t2 = pi_tmp_src[ 56 ];
		pui8_dst[ i_destination_stride * 6 ] = ( t3 + ( t1 * 2 ) + t2 + 8 ) >> 4;
		pui8_dst[ i_destination_stride * 7 ] = ( t2 + 2 ) >> 2;

		pui8_dst += 1;
		pi_tmp_src += 1;
	}
}


Void h261_add_8x8_REF( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_base, Int32 i_base_stride, Int16 *pi_difference )
{
	Int32 i_x, i_y, i_pel;

	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			i_pel = pui8_base[ i_y * i_base_stride + i_x ] + pi_difference[ i_x ];
			pui8_destination[ i_x ] = i_pel < 0 ? 0 : ( i_pel > 255 ? 255 : i_pel );
		}
		pui8_destination += i_destination_stride;
		pi_difference += 8;
	}
}





Void h261_initialize_pixelop_functions( )
{
	g_get_sad = h261_get_sad_REF;
	g_get_ssd[ H261_BLOCK_16x16 ] = h261_ssd_16x16_REF;
	g_get_ssd[ H261_BLOCK_8x8 ] = h261_ssd_8x8_REF;
	g_sub_8x8 = h261_sub_8x8_REF;
	g_add_8x8 = h261_add_8x8_REF;
	g_compensate_8x8 = compensate_8x8_REF;
	g_compensate_8x8_filter = compensate_8x8_filter_REF;

	if( 1 )
	{
		g_get_sad = h261_get_sad_sse2;
		g_get_ssd[ H261_BLOCK_16x16 ] = h261_ssd_16x16_sse2;
		g_get_ssd[ H261_BLOCK_8x8 ] = h261_ssd_8x8_sse2;
		g_sub_8x8 = h261_sub_8x8_sse2;
		g_add_8x8 = h261_add_8x8_sse2;
		g_compensate_8x8 = h261_compensate_8x8_mmx;
		g_compensate_8x8_filter = h261_compensate_8x8_filter_sse2;
	}
}
