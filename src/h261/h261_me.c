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


Int32 h261_get_macroblock_mv_cost( h261_context_t *ps_ctx, h261_macroblock_t *ps_mb, Int32 i_mv_x, Int32 i_mv_y )
{
	Int32 i_mvd_x, i_mvd_y;
	if( ps_ctx->s_picture.s_groups_of_blocks.s_coding_vars.i_last_mv_valid )
	{
		i_mvd_x = i_mv_x - ps_ctx->s_picture.s_groups_of_blocks.s_coding_vars.i_last_mv_x;
		i_mvd_y = i_mv_y - ps_ctx->s_picture.s_groups_of_blocks.s_coding_vars.i_last_mv_y;
	}
	else
	{
		i_mvd_x = ps_mb->rgi_mv[ 0 ];
		i_mvd_y = ps_mb->rgi_mv[ 1 ];
	}
	i_mvd_x += H261_MVD_TRANSLATION_TABLE_OFFSET;
	i_mvd_y += H261_MVD_TRANSLATION_TABLE_OFFSET;
	i_mvd_x = rgi_mvd_translation_table[ i_mvd_x ];
	i_mvd_y = rgi_mvd_translation_table[ i_mvd_y ];

	if( i_mvd_x < 0 || i_mvd_y < 0 )
	{
		Int32 *pi_null = 0;
		*pi_null = 0;
	}

	return ( ( h261_mvd_table[ i_mvd_x ].i_length + h261_mvd_table[ i_mvd_y ].i_length ) * ps_mb->i_lambda ) >> 8;
}


#define VALID_MV( i_mv_x, i_mv_y ) ( i_mv_x >= i_min_mv_x && i_mv_x <= i_max_mv_x && i_mv_y >= i_min_mv_y && i_mv_y <= i_max_mv_y )

#define TRY_MV( i_mv_x, i_mv_y )																			\
do {																										\
	pui8_mv_ref = pui8_ref + ( i_mv_x ) + ( i_mv_y ) * ps_reference_frame->i_stride_y;						\
	i_sad = g_get_sad( pui8_mv_ref, ps_reference_frame->i_stride_y, pui8_curr, ps_frame->i_stride_y );		\
	if( i_sad < i_best_sad )																				\
	{																										\
		i_sad += h261_get_macroblock_mv_cost( ps_ctx, ps_mb, ( i_mv_x ), ( i_mv_y ) );						\
		if( i_sad < i_best_sad )																			\
		{																									\
			i_best_mv_x = i_mv_x;																			\
			i_best_mv_y = i_mv_y;																			\
			i_best_sad = i_sad;																				\
		}																									\
	}																										\
} while( 0 )


Void h261_get_macroblock_mv( h261_context_t *ps_ctx )
{
	Int32 i_min_mv_x, i_min_mv_y, i_max_mv_x, i_max_mv_y;
	Int32 i_mv_x, i_mv_y, i_omv_x, i_omv_y, i_best_mv_x, i_best_mv_y, i_idx, i_scale, i_range;
	Int32 i_sad, i_best_sad;
	UInt8 *pui8_curr, *pui8_ref, *pui8_mv_ref;

	Int32 rgi_diamond[ 4 ][ 2 ] = {
		{ -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }
	};

	Int32 rgi_diamond_s2[ 6 ][ 2 ] = {
		{ -1, -1 }, { 1, -1 },
		{ -1, 1 }, { 1, 1 },
		{  0, -2 }, { 0, 2}
	};

	Int32 rgi_hexagon[ 6 ][ 2 ] = {
		{ -2, 0 }, { 2, 0 }, { -1, 2 }, { 1, 2 }, { -1, -2 }, { 1, -2 }
	};

	Int32 rgi_big_hexagon[ 16 ][ 2 ] = {
		{ -4, -2 },{ -4, -1 },{ -4,  0 },{ -4,  1 },{ -4,  2 },{  4, -2 },
		{  4, -1 },{  4,  0 },{  4,  1 },{  4,  2 },{ -2, -3 },{  2, -3 },
		{ -2,  3 },{  2,  3 },{  0, -4 },{  0,  4 }};



	h261_macroblock_t *ps_mb;
	h261_frame_t *ps_frame;
	h261_frame_t *ps_reference_frame;

	ps_mb = &ps_ctx->s_picture.s_groups_of_blocks.s_macroblock;
	ps_frame = &ps_ctx->s_current_frame;
	ps_reference_frame = &ps_ctx->s_reference_frame;

	i_min_mv_x = MAX( 0, ps_mb->i_mb_x - 15 ) - ps_mb->i_mb_x;
	i_min_mv_y = MAX( 0, ps_mb->i_mb_y - 15 ) - ps_mb->i_mb_y;
	i_max_mv_x = MIN( ps_reference_frame->i_width - 16, ps_mb->i_mb_x + 15 ) - ps_mb->i_mb_x;
	i_max_mv_y = MIN( ps_reference_frame->i_height - 16, ps_mb->i_mb_y + 15 ) - ps_mb->i_mb_y;


	pui8_curr = ps_frame->pui8_Y + ps_mb->i_mb_x + ps_mb->i_mb_y * ps_frame->i_stride_y;
	pui8_ref = ps_reference_frame->pui8_Y + ps_mb->i_mb_x + ps_mb->i_mb_y * ps_frame->i_stride_y;

	/* modified simplified umhs */

	i_best_mv_x = i_best_mv_y = 0;
	i_best_sad = MAX_COST;

	TRY_MV( 0, 0 );

	/* try prediction vector */
	if( ps_ctx->s_picture.s_groups_of_blocks.s_coding_vars.i_last_mv_valid &&
		VALID_MV( ps_ctx->s_picture.s_groups_of_blocks.s_coding_vars.i_last_mv_x, ps_ctx->s_picture.s_groups_of_blocks.s_coding_vars.i_last_mv_y ) )
	{
		TRY_MV( ps_ctx->s_picture.s_groups_of_blocks.s_coding_vars.i_last_mv_x, ps_ctx->s_picture.s_groups_of_blocks.s_coding_vars.i_last_mv_y );
	}
	else
	{
		/* try left */
		if( ps_mb->rgps_neighbours[ H261_LEFT ] &&
			VALID_MV( ps_mb->rgps_neighbours[ H261_LEFT ]->rgi16_mv[ 0 ], ps_mb->rgps_neighbours[ H261_LEFT ]->rgi16_mv[ 1 ] ) )
		{
			TRY_MV( ps_mb->rgps_neighbours[ H261_LEFT ]->rgi16_mv[ 0 ], ps_mb->rgps_neighbours[ H261_LEFT ]->rgi16_mv[ 1 ] );
		}
	}

	/* try temporal */
	TRY_MV( ps_mb->ps_mb_cache->rgi16_mv[ 0 ], ps_mb->ps_mb_cache->rgi16_mv[ 1 ] );
	
	/* try temporal right */
	if( ps_mb->rgps_neighbours[ H261_RIGHT ] &&
		VALID_MV( ps_mb->rgps_neighbours[ H261_RIGHT ]->rgi16_mv[ 0 ], ps_mb->rgps_neighbours[ H261_RIGHT ]->rgi16_mv[ 1 ] ) )
	{
		TRY_MV( ps_mb->rgps_neighbours[ H261_RIGHT ]->rgi16_mv[ 0 ], ps_mb->rgps_neighbours[ H261_RIGHT ]->rgi16_mv[ 1 ] );
	}

	/* try top */
	if( ps_mb->rgps_neighbours[ H261_TOP ] &&
		VALID_MV( ps_mb->rgps_neighbours[ H261_TOP ]->rgi16_mv[ 0 ], ps_mb->rgps_neighbours[ H261_TOP ]->rgi16_mv[ 1 ] ) )
	{
		TRY_MV( ps_mb->rgps_neighbours[ H261_TOP ]->rgi16_mv[ 0 ], ps_mb->rgps_neighbours[ H261_TOP ]->rgi16_mv[ 1 ] );
	}

	/* try temporal bottom */
	if( ps_mb->rgps_neighbours[ H261_BOTTOM ] &&
		VALID_MV( ps_mb->rgps_neighbours[ H261_BOTTOM ]->rgi16_mv[ 0 ], ps_mb->rgps_neighbours[ H261_BOTTOM ]->rgi16_mv[ 1 ] ) )
	{
		TRY_MV( ps_mb->rgps_neighbours[ H261_BOTTOM ]->rgi16_mv[ 0 ], ps_mb->rgps_neighbours[ H261_BOTTOM ]->rgi16_mv[ 1 ] );
	}

	i_omv_x = i_mv_x = i_best_mv_x;
	i_omv_y = i_mv_y = i_best_mv_y;

	/* hexagon refine */
	for( i_idx = 0; i_idx < 6; i_idx++ )
	{
		if( VALID_MV( i_mv_x + rgi_hexagon[ i_idx ][ 0 ], i_mv_y + rgi_hexagon[ i_idx ][ 1 ] ) )
		{
			TRY_MV( i_mv_x + rgi_hexagon[ i_idx ][ 0 ], i_mv_y + rgi_hexagon[ i_idx ][ 1 ] );
		}
	}

	i_omv_x = i_mv_x = i_best_mv_x;
	i_omv_y = i_mv_y = i_best_mv_y;

	/* diamond refine */
	for( i_range = 0; i_range < 3; i_range++ )
	{
		for( i_idx = 0; i_idx < 4; i_idx++ )
		{
			if( VALID_MV( i_mv_x + rgi_diamond[ i_idx ][ 0 ], i_mv_y + rgi_diamond[ i_idx ][ 1 ] ) )
			{
				TRY_MV( i_mv_x + rgi_diamond[ i_idx ][ 0 ], i_mv_y + rgi_diamond[ i_idx ][ 1 ] );
			}
			i_omv_x = i_mv_x = i_best_mv_x;
			i_omv_y = i_mv_y = i_best_mv_y;
		}
	}


	/* cross */
	for( i_mv_x = -15; i_mv_x < 16; i_mv_x++ )
	{
		if( VALID_MV( i_mv_x, i_mv_y ) )
		{
			TRY_MV( i_mv_x, i_mv_y );
		}
	}
	for( i_mv_y = -15; i_mv_y < 16; i_mv_y++ )
	{
		if( VALID_MV( i_mv_x, i_mv_y ) )
		{
			TRY_MV( i_mv_x, i_mv_y );
		}
	}

	i_omv_x = i_mv_x = i_best_mv_x;
	i_omv_y = i_mv_y = i_best_mv_y;

	/* uneven multi hexagon search */

	for( i_scale = 1; i_scale < 5; i_scale++ )
	{
		for( i_idx = 0; i_idx < 16; i_idx++ )
		{
			if( VALID_MV( i_mv_x + rgi_big_hexagon[ i_idx ][ 0 ] * i_scale, i_mv_y + rgi_big_hexagon[ i_idx ][ 1 ] * i_scale ) )
			{
				TRY_MV( i_mv_x + rgi_big_hexagon[ i_idx ][ 0 ] * i_scale, i_mv_y + rgi_big_hexagon[ i_idx ][ 1 ] * i_scale );
			}
		}
	}

	i_omv_x = i_mv_x = i_best_mv_x;
	i_omv_y = i_mv_y = i_best_mv_y;


	/* hexagon based search */
	for( i_range = 8; i_range >= 0; i_range-- )
	{
		i_omv_x = i_mv_x = i_best_mv_x;
		i_omv_y = i_mv_y = i_best_mv_y;
		for( i_idx = 0; i_idx < 6; i_idx++ )
		{
			if( VALID_MV( i_mv_x + rgi_hexagon[ i_idx ][ 0 ], i_mv_y + rgi_hexagon[ i_idx ][ 1 ] ) )
			{
				TRY_MV( i_mv_x + rgi_hexagon[ i_idx ][ 0 ], i_mv_y + rgi_hexagon[ i_idx ][ 1 ] );
			}
		}
		if( i_best_mv_x == i_mv_x && i_best_mv_y == i_mv_y )
		{
			break;
		}
	}

	i_omv_x = i_mv_x = i_best_mv_x;
	i_omv_y = i_mv_y = i_best_mv_y;

	/* diamond based search */
	for( i_idx = 0; i_idx < 6; i_idx++ )
	{
		if( VALID_MV( i_mv_x + rgi_diamond_s2[ i_idx ][ 0 ], i_mv_y + rgi_diamond_s2[ i_idx ][ 1 ] ) )
		{
			TRY_MV( i_mv_x + rgi_diamond_s2[ i_idx ][ 0 ], i_mv_y + rgi_diamond_s2[ i_idx ][ 1 ] );
		}
	}
	for( i_range = 8; i_range >= 0; i_range-- )
	{
		i_omv_x = i_mv_x = i_best_mv_x;
		i_omv_y = i_mv_y = i_best_mv_y;

		for( i_idx = 0; i_idx < 4; i_idx++ )
		{
			if( VALID_MV( i_mv_x + rgi_diamond[ i_idx ][ 0 ], i_mv_y + rgi_diamond[ i_idx ][ 1 ] ) )
			{
				TRY_MV( i_mv_x + rgi_diamond[ i_idx ][ 0 ], i_mv_y + rgi_diamond[ i_idx ][ 1 ] );
			}
		}
		if( i_best_mv_x == i_mv_x && i_best_mv_y == i_mv_y )
		{
			break;
		}
	}

	ps_mb->rgi_me_mv[ 0 ] = i_best_mv_x;
	ps_mb->rgi_me_mv[ 1 ] = i_best_mv_y;
}












