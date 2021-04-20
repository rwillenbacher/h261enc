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

#define RDOPT
#define h261_write_macroblock h261_size_macroblock
#define h261_write_macroblock_block h261_size_macroblock_block
#include "h261_mbcoder.c"
#undef RDOPT


Int32 h261_macroblock_get_ssd( h261_context_t *ps_ctx, h261_macroblock_t *ps_mb )
{
	Int32 i_mb_offset;
	Int32 i_ssd;

	h261_frame_t *ps_frame;
	h261_frame_t *ps_recon;

	ps_frame = &ps_ctx->s_current_frame;
	ps_recon = &ps_ctx->s_reconstructed_frame;

	i_mb_offset = ps_mb->i_mb_x + ps_mb->i_mb_y * ps_frame->i_stride_y;
	i_ssd = g_get_ssd[ H261_BLOCK_16x16 ]( ps_frame->pui8_Y + i_mb_offset, ps_frame->i_stride_y, ps_recon->pui8_Y + i_mb_offset, ps_recon->i_stride_y );
	i_mb_offset = ( ps_mb->i_mb_x >> 1 ) + ( ps_mb->i_mb_y >> 1 ) * ps_frame->i_stride_c;
	i_ssd += g_get_ssd[ H261_BLOCK_8x8 ]( ps_frame->pui8_Cb + i_mb_offset, ps_frame->i_stride_c, ps_recon->pui8_Cb + i_mb_offset, ps_recon->i_stride_c );
	i_ssd += g_get_ssd[ H261_BLOCK_8x8 ]( ps_frame->pui8_Cr + i_mb_offset, ps_frame->i_stride_c, ps_recon->pui8_Cr + i_mb_offset, ps_recon->i_stride_c );

	return i_ssd;
}

Int32 var_16x16_c( UInt8 *pui8_pel, Int32 i_stride )
{                                             
    UInt32 i_sum = 0, i_sqr = 0;
	Int32 i_x, i_y;
    for( i_y = 0; i_y < 16; i_y++ )              
    {                                         
        for( i_x = 0; i_x < 16; i_x++ )          
        {                                     
            i_sum += pui8_pel[i_x];                    
            i_sqr += pui8_pel[i_x] * pui8_pel[i_x];           
        }                                     
        pui8_pel += i_stride;                      
    }                                         
	return i_sqr - ( ( i_sum * i_sum ) / 256 );
}


Int32 h261_macroblock_get_var( h261_context_t *ps_ctx, h261_macroblock_t *ps_mb )
{
	Int32 i_mb_offset;
	Int32 i_var;

	h261_frame_t *ps_frame;

	ps_frame = &ps_ctx->s_current_frame;

	i_mb_offset = ps_mb->i_mb_x + ps_mb->i_mb_y * ps_frame->i_stride_y;
	i_var = var_16x16_c( ps_frame->pui8_Y + i_mb_offset, ps_frame->i_stride_y );

	return i_var;
}


Int32 h261_macroblock_cost_rdopt( h261_context_t *ps_ctx, h261_macroblock_t *ps_mb, Int32 i_mb_type )
{
	Int32 i_ssd, i_bits, i_coded, i_mb_flags;

	i_mb_flags = ps_mb->i_macroblock_type_flags;

	if( i_mb_type == H261_MB_TYPE_INTRA )
	{
		h261_encode_macroblock_intra( ps_ctx );
		i_coded = 1;
	}
	else if( i_mb_type == H261_MB_TYPE_INTER )
	{
		h261_encode_macroblock_inter( ps_ctx );
		i_coded = ps_mb->i_macroblock_type_flags;
	}
	if( i_coded )
	{
		i_bits = h261_size_macroblock( ps_ctx, 0 );
	}
	else
	{
		i_bits = 0;
	}
	h261_reconstruct_macroblock( ps_ctx );

	i_ssd = h261_macroblock_get_ssd( ps_ctx, ps_mb );

	ps_mb->i_macroblock_type_flags = i_mb_flags;

	return ( Int32 ) ( i_ssd + ( ( ( ( UInt64 )i_bits ) * ps_mb->i_lambda_rdo ) >> 8 ) );
}


Void h261_macroblock_optimize_quantizer( h261_context_t *ps_ctx, h261_macroblock_t *ps_mb, Int32 i_mb_type )
{
	Int32 i_best_cost, i_best_quantizer, i_quantizer, i_end_quantizer, i_cost, i_original_quantizer, i_original_cost, i_num_fail, i_max_fail, i_mfail;
	Int32 i_dir;

	i_best_cost = MAX_COST;

	i_best_cost = i_original_cost = h261_macroblock_cost_rdopt( ps_ctx, ps_mb, i_mb_type );
	i_best_quantizer = i_original_quantizer = ps_mb->i_macroblock_quant;
	i_end_quantizer = ps_mb->i_macroblock_quant + 10;

	ps_mb->i_macroblock_quant -= 1;
	if( ps_mb->i_macroblock_quant > 0 )
	{
		i_cost = h261_macroblock_cost_rdopt( ps_ctx, ps_mb, i_mb_type );
	}
	else
	{
		i_cost = MAX_COST;
	}
	ps_mb->i_macroblock_quant += 1;
	if( i_cost < i_best_cost )
	{
		i_best_cost = i_cost;
		i_best_quantizer = ps_mb->i_macroblock_quant - 1;
		i_dir = -1;
		i_max_fail = 2;
	}
	else
	{
		i_max_fail = 2 ;
		i_dir = 1;
	}

	if( i_dir < 0 )
	{
		ps_mb->i_macroblock_quant += 1;
		if( ps_mb->i_macroblock_quant < 32 )
		{
			i_cost = h261_macroblock_cost_rdopt( ps_ctx, ps_mb, i_mb_type );
		}
		else
		{
			i_cost = MAX_COST;
		}
		ps_mb->i_macroblock_quant -= 1;
		if( i_cost < i_best_cost )
		{
			i_dir = 1;
			i_best_quantizer = ps_mb->i_macroblock_quant + 1;
			i_best_cost = i_cost;
			i_max_fail = 2;
		}
	}

	i_mfail = 0;
	i_num_fail = 0;

	for( i_quantizer = i_best_quantizer + i_dir; i_num_fail <= i_max_fail; i_quantizer += i_dir )
	{
		if( i_quantizer <= 0 || i_quantizer > 31 )
		{
			break;
		}
		ps_mb->i_macroblock_quant = i_quantizer;
		i_cost = h261_macroblock_cost_rdopt( ps_ctx, ps_mb, i_mb_type );
		if( i_cost < i_best_cost )
		{
			i_best_cost = i_cost;
			i_mfail = MAX( i_mfail, i_num_fail );
			i_num_fail = 0;
			i_best_quantizer = i_quantizer;
		}
		else
		{
			i_num_fail++;
		}
	}

	ps_mb->i_macroblock_quant = i_best_quantizer;
}

Int32 h261_macroblock_optimize_cbp( h261_context_t *ps_ctx )
{
	Int32 i_idx;
	Int32 i_accum_has_coeff;
	Int32 i_x, i_y, i_mv_x, i_mv_y;
	Int32 i_bits, i_cost_coeff, i_cost_non_coeff;
	Int16 *pi16_coeffs;
	UInt8 rgui8_reference[ 64 ];
	UInt8 *rgpui8_c_sources[ 2 ][ 2 ];

	DECLALIGNED( 16 ) Int16 rgi16_temp[ 64 ];

	const Int32 rgi_y_offs[ 4 ][ 2 ] = {
		{ 0, 0 },
		{ 8, 0 },
		{ 0, 8 },
		{ 8, 8 },
	};

	const Int32 rgi_cbp_table[ 4 ] = {
		H261_CODED_BLOCK_PATTERN_1,
		H261_CODED_BLOCK_PATTERN_2,
		H261_CODED_BLOCK_PATTERN_3,
		H261_CODED_BLOCK_PATTERN_4
	};

	const Int32 rgi_cbp_table_c[ 2 ] = {
		H261_CODED_BLOCK_PATTERN_5,
		H261_CODED_BLOCK_PATTERN_6
	};

	h261_picture_t *ps_picture;
	h261_gob_t *ps_gob;
	h261_macroblock_t *ps_mb;
	h261_frame_t *ps_frame;
	h261_frame_t *ps_reference;

	ps_picture = &ps_ctx->s_picture;
	ps_gob = &ps_picture->s_groups_of_blocks;
	ps_mb = &ps_gob->s_macroblock;
	ps_frame = &ps_ctx->s_current_frame;
	ps_reference = &ps_ctx->s_reference_frame;

	if( ps_mb->i_macroblock_type_flags & H261_MB_MC )
	{
		i_mv_x = ps_mb->rgi_mv[ 0 ];
		i_mv_y = ps_mb->rgi_mv[ 1 ];
	}
	else
	{
		i_mv_x = ps_mb->rgi_mv[ 0 ] = 0;
		i_mv_y = ps_mb->rgi_mv[ 1 ] = 0;
	}

	i_accum_has_coeff = 0;

	for( i_idx = 0; i_idx < 4; i_idx++ )
	{
		if( ps_mb->i_coded_block_pattern & rgi_cbp_table[ i_idx ] )
		{
			Int16 rgi_coeff_copy[ 64 ];
			UInt8 *pui8_current, *pui8_reference;

			i_x = ps_mb->i_mb_x + rgi_y_offs[ i_idx ][ 0 ];
			i_y = ps_mb->i_mb_y + rgi_y_offs[ i_idx ][ 1 ];

			pui8_current = ps_frame->pui8_Y + ( i_y * ps_frame->i_stride_y + i_x );
			pui8_reference = ps_reference->pui8_Y + ( ( i_y + i_mv_y ) * ps_frame->i_stride_y + i_x + i_mv_x );
			memcpy( rgi_coeff_copy, &ps_mb->rgi16_tcoeff[ i_idx ][ 0 ], sizeof( Int16 ) * 64 );
			pi16_coeffs = &rgi_coeff_copy[ 0 ];

			if( ps_mb->i_macroblock_type_flags & H261_MB_FILTER )
			{
				g_compensate_8x8_filter( rgui8_reference, 8, pui8_reference, ps_reference->i_stride_y );
			}
			else
			{
				g_compensate_8x8( rgui8_reference, 8, pui8_reference, ps_reference->i_stride_y );
			}

			i_cost_non_coeff = g_get_ssd[ H261_BLOCK_8x8 ]( pui8_current, ps_frame->i_stride_y, rgui8_reference, 8 );

			h261_quant8x8_inter_bw( pi16_coeffs, 8, ps_mb->i_macroblock_quant );
			g_idct_8x8( pi16_coeffs, rgi16_temp );
			g_add_8x8( rgui8_reference, 8, rgui8_reference, 8, rgi16_temp );

			i_bits = h261_size_macroblock_block( ps_ctx, i_idx );
			i_cost_coeff = g_get_ssd[ H261_BLOCK_8x8 ]( pui8_current, ps_frame->i_stride_y, rgui8_reference, 8 );
			i_cost_coeff += ( i_bits * ps_mb->i_lambda_rdo ) >> 8;

			if( i_cost_non_coeff < i_cost_coeff )
			{
				ps_mb->i_coded_block_pattern &= ~rgi_cbp_table[ i_idx ];
			}
		}
	}

	i_x = ps_mb->i_mb_x / 2;
	i_y = ps_mb->i_mb_y / 2;
	i_mv_x /= 2;
	i_mv_y /= 2;

	rgpui8_c_sources[ 0 ][ 0 ] = ps_frame->pui8_Cb;
	rgpui8_c_sources[ 0 ][ 1 ] = ps_reference->pui8_Cb;
	rgpui8_c_sources[ 1 ][ 0 ] = ps_frame->pui8_Cr;
	rgpui8_c_sources[ 1 ][ 1 ] = ps_reference->pui8_Cr;

	for( i_idx = 0; i_idx < 2; i_idx++ )
	{
		if( ps_mb->i_coded_block_pattern & rgi_cbp_table_c[ i_idx ] )
		{
			Int16 rgi_coeff_copy[ 64 ];
			UInt8 *pui8_current, *pui8_reference;

			pui8_current = rgpui8_c_sources[ i_idx ][ 0 ] + ( i_y * ps_frame->i_stride_c + i_x );
			pui8_reference = rgpui8_c_sources[ i_idx ][ 1 ] + ( ( i_y + i_mv_y ) * ps_frame->i_stride_c + i_x + i_mv_x );
			memcpy( rgi_coeff_copy, &ps_mb->rgi16_tcoeff[ i_idx + 4 ][ 0 ], sizeof( Int16 ) * 64 );
			pi16_coeffs = &rgi_coeff_copy[ 0 ];

			if( ps_mb->i_macroblock_type_flags & H261_MB_FILTER )
			{
				g_compensate_8x8_filter( rgui8_reference, 8, pui8_reference, ps_reference->i_stride_c );
			}
			else
			{
				g_compensate_8x8( rgui8_reference, 8, pui8_reference, ps_reference->i_stride_c );
			}

			i_cost_non_coeff = g_get_ssd[ H261_BLOCK_8x8 ]( pui8_current, ps_frame->i_stride_c, rgui8_reference, 8 );

			h261_quant8x8_inter_bw( pi16_coeffs, 8, ps_mb->i_macroblock_quant );
			g_idct_8x8( pi16_coeffs, rgi16_temp );
			g_add_8x8( rgui8_reference, 8, rgui8_reference, 8, rgi16_temp );

			i_bits = h261_size_macroblock_block( ps_ctx, i_idx + 4 );
			i_cost_coeff = g_get_ssd[ H261_BLOCK_8x8 ]( pui8_current, ps_frame->i_stride_c, rgui8_reference, 8 );
			i_cost_coeff += ( i_bits * ps_mb->i_lambda_rdo ) >> 8;

			if( i_cost_non_coeff < i_cost_coeff )
			{
				ps_mb->i_coded_block_pattern &= ~rgi_cbp_table_c[ i_idx ];
			}
		}
	}

	if( ps_mb->i_coded_block_pattern == 0 )
	{
		ps_mb->i_macroblock_type_flags &= ~H261_MB_COEFF;
	}

	return ps_mb->i_macroblock_type_flags;
}


Int32 h261_macroblock_refine_mv_rdo( h261_context_t *ps_ctx )
{
	Int32 i_iter, i_pos, i_bpos, i_mv_x, i_mv_y, i_found;
	Int32 i_min_mv_x, i_min_mv_y, i_max_mv_x, i_max_mv_y;
	Int32 i_sad_threshold, i_sad, i_best_cost, i_cost;
	Int32 i_prev_pos;
	Int32 rgi_num_search_pos[ 5 ] = { 3, 3, 3, 3, 4 };
	Int32 rgi_search_pos_idx[ 5 ][ 4 ] = { { 0, 2, 3, 1 }, { 1, 2, 3, 0 }, { 0, 1, 2, 3 }, { 0, 1, 3, 2 }, { 0, 1, 2, 3 } };
	Int32 i_search_pos[ 4 ][ 2 ] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
	UInt8 *pui8_curr, *pui8_ref;

	h261_picture_t *ps_picture;
	h261_frame_t *ps_reference_frame;
	h261_gob_t *ps_gob;
	h261_macroblock_t *ps_mb;

	ps_picture = &ps_ctx->s_picture;
	ps_gob = &ps_picture->s_groups_of_blocks;
	ps_mb = &ps_gob->s_macroblock;

	ps_reference_frame = &ps_ctx->s_reference_frame;
	i_min_mv_x = MAX( 0, ps_mb->i_mb_x - 15 ) - ps_mb->i_mb_x;
	i_min_mv_y = MAX( 0, ps_mb->i_mb_y - 15 ) - ps_mb->i_mb_y;
	i_max_mv_x = MIN( ps_reference_frame->i_width - 16, ps_mb->i_mb_x + 15 ) - ps_mb->i_mb_x;
	i_max_mv_y = MIN( ps_reference_frame->i_height - 16, ps_mb->i_mb_y + 15 ) - ps_mb->i_mb_y;

	pui8_curr = ps_ctx->s_current_frame.pui8_Y + ps_mb->i_mb_x + ps_mb->i_mb_y * ps_ctx->s_current_frame.i_stride_y;
	pui8_ref = ps_reference_frame->pui8_Y + ps_mb->i_mb_x + ps_mb->i_mb_y * ps_reference_frame->i_stride_y;

	i_mv_x = ps_mb->rgi_me_mv[ 0 ];
	i_mv_y = ps_mb->rgi_me_mv[ 1 ];

	ps_mb->i_macroblock_type_flags = H261_MB_MC;
	ps_mb->rgi_mv[ 0 ] = i_mv_x;
	ps_mb->rgi_mv[ 1 ] = i_mv_y;
	i_best_cost = h261_macroblock_cost_rdopt( ps_ctx, ps_mb, H261_MB_TYPE_INTER );
	ps_mb->i_macroblock_type_flags = H261_MB_MC | H261_MB_FILTER;
	ps_mb->rgi_mv[ 0 ] = i_mv_x;
	ps_mb->rgi_mv[ 1 ] = i_mv_y;
	i_best_cost = MIN( h261_macroblock_cost_rdopt( ps_ctx, ps_mb, H261_MB_TYPE_INTER ), i_best_cost );
	i_sad_threshold = g_get_sad( pui8_ref + i_mv_x + i_mv_y * ps_reference_frame->i_stride_y, ps_reference_frame->i_stride_y, pui8_curr, ps_ctx->s_current_frame.i_stride_y );

	i_prev_pos = 4;
	for( i_iter = 0; i_iter < 5; i_iter++ )
	{
		i_found = 0;
		for( i_pos = 0; i_pos < rgi_num_search_pos[ i_prev_pos ]; i_pos++ )
		{
			ps_mb->rgi_mv[ 0 ] = i_mv_x + i_search_pos[ rgi_search_pos_idx[ i_prev_pos ][ i_pos ] ][ 0 ];
			ps_mb->rgi_mv[ 1 ] = i_mv_y + i_search_pos[ rgi_search_pos_idx[ i_prev_pos ][ i_pos ] ][ 1 ];
			if( ps_mb->rgi_mv[ 0 ] >= i_min_mv_x && ps_mb->rgi_mv[ 0 ] <= i_max_mv_x && ps_mb->rgi_mv[ 1 ] >= i_min_mv_y && ps_mb->rgi_mv[ 1 ] <= i_max_mv_y )
			{
				UInt8 *pui8_mv_ref;

				/* sad early termination */
				pui8_mv_ref = pui8_ref + ( ps_mb->rgi_mv[ 0 ]  ) + ( ps_mb->rgi_mv[ 1 ] ) * ps_reference_frame->i_stride_y;
				i_sad = g_get_sad( pui8_mv_ref, ps_reference_frame->i_stride_y, pui8_curr, ps_ctx->s_current_frame.i_stride_y );

				if( i_sad < ( ( i_sad_threshold * 120 ) / 100 ) + 50 )
				{
					ps_mb->i_macroblock_type_flags = H261_MB_MC;
					i_cost = h261_macroblock_cost_rdopt( ps_ctx, ps_mb, H261_MB_TYPE_INTER );
					ps_mb->i_macroblock_type_flags = H261_MB_MC | H261_MB_FILTER;
					i_cost = MIN( h261_macroblock_cost_rdopt( ps_ctx, ps_mb, H261_MB_TYPE_INTER ), i_cost );
				}
				else
				{
					i_cost = MAX_COST;
				}
			}
			else
			{
				i_cost = MAX_COST;
			}
			if( i_cost < i_best_cost )
			{
				i_sad_threshold = i_sad;
				i_best_cost = i_cost;
				i_bpos = rgi_search_pos_idx[ i_prev_pos ][ i_pos ];
				i_found = 1;
			}
		}
		if( i_found )
		{
			i_prev_pos = i_bpos;
			i_mv_x += i_search_pos[ i_bpos ][ 0 ];
			i_mv_y += i_search_pos[ i_bpos ][ 1 ];
		}
		else
		{
			break;
		}
	}
	ps_mb->rgi_me_mv[ 0 ] = i_mv_x;
	ps_mb->rgi_me_mv[ 1 ] = i_mv_y;

	return i_best_cost;
}


Int32 h261_encode_macroblock( h261_context_t *ps_ctx )
{
	Int32 i_ret;
	Int32 i_mb_type, i_var;
	Int32 i_best_rdo_flags, i_best_rdo_cost, i_mb_flags, i_rdo_cost;

	h261_picture_t *ps_picture;
	h261_gob_t *ps_gob;
	h261_macroblock_t *ps_mb;

	ps_picture = &ps_ctx->s_picture;
	ps_gob = &ps_picture->s_groups_of_blocks;
	ps_mb = &ps_gob->s_macroblock;

	ps_picture = &ps_ctx->s_picture;

	/* data elements */
	ps_mb->i_macroblock_quant = ps_gob->i_quantiser;

	i_var = h261_macroblock_get_var( ps_ctx, ps_mb );
	i_var = ( Int32 ) ( ( ( log( i_var + 1 ) / log( 2.0 ) ) - 13 ) * 1.0 );
	ps_mb->i_macroblock_quant = MIN( 31, MAX( ps_mb->i_macroblock_quant + i_var, 1 ) );

	ps_mb->i_lambda = rgi_lambda[ ps_mb->i_macroblock_quant ];
	ps_mb->i_lambda_rdo = rgi_lambda2[ ps_mb->i_macroblock_quant ];

	i_mb_type = H261_MB_TYPE_INTRA;
	ps_mb->i_macroblock_type_flags = 0;

	if( ps_picture->i_frame_type == H261_FRAME_TYPE_INTER )
	{
		h261_get_macroblock_mv( ps_ctx );
		i_mb_type = H261_MB_TYPE_INTER;

		i_mb_flags = 0;
		ps_mb->i_macroblock_type_flags = i_best_rdo_flags = i_mb_flags;
		i_best_rdo_cost = h261_macroblock_cost_rdopt( ps_ctx, ps_mb, H261_MB_TYPE_INTER );

		h261_macroblock_refine_mv_rdo( ps_ctx );

		i_mb_flags |= H261_MB_MC;
		ps_mb->rgi_mv[ 0 ] = ps_mb->rgi_me_mv[ 0 ];
		ps_mb->rgi_mv[ 1 ] = ps_mb->rgi_me_mv[ 1 ];
		ps_mb->i_macroblock_type_flags = i_mb_flags;
		i_rdo_cost = h261_macroblock_cost_rdopt( ps_ctx, ps_mb, H261_MB_TYPE_INTER );
		if( i_rdo_cost < i_best_rdo_cost )
		{
			i_best_rdo_flags = i_mb_flags;
			i_best_rdo_cost = i_rdo_cost;
		}

		i_mb_flags |= H261_MB_FILTER;
		ps_mb->i_macroblock_type_flags = i_mb_flags;
		i_rdo_cost = h261_macroblock_cost_rdopt( ps_ctx, ps_mb, H261_MB_TYPE_INTER );
		if( i_rdo_cost < i_best_rdo_cost )
		{
			i_best_rdo_flags = i_mb_flags;
			i_best_rdo_cost = i_rdo_cost;
		}

		ps_mb->i_macroblock_type_flags = 0;
		i_rdo_cost = h261_macroblock_cost_rdopt( ps_ctx, ps_mb, H261_MB_TYPE_INTRA );
		if( i_rdo_cost < i_best_rdo_cost )
		{
			i_mb_type = H261_MB_TYPE_INTRA;
			i_best_rdo_flags = 0;
		}

		ps_mb->i_macroblock_type_flags = i_best_rdo_flags;

		/* saving the MV even when we encode intra helps future prediction */
		ps_mb->ps_mb_cache->rgi16_mv[ 0 ] = ps_mb->rgi_me_mv[ 0 ];
		ps_mb->ps_mb_cache->rgi16_mv[ 1 ] = ps_mb->rgi_me_mv[ 1 ];
	}
	else
	{
		ps_mb->ps_mb_cache->rgi16_mv[ 0 ] = 0;
		ps_mb->ps_mb_cache->rgi16_mv[ 1 ] = 0;
	}

	/* optimize best mode... no gain */
	/* h261_macroblock_optimize_quantizer( ps_ctx, ps_mb, i_mb_type ); no gain */

	if( i_mb_type == H261_MB_TYPE_INTER )
	{
		h261_encode_macroblock_inter( ps_ctx );
		h261_macroblock_optimize_cbp( ps_ctx );
		i_ret = ps_mb->i_macroblock_type_flags;
	}
	else /*if( i_mb_type == H261_MB_TYPE_INTRA )*/
	{
		h261_encode_macroblock_intra( ps_ctx );
		i_ret = 1;
	}

	return i_ret;
}
