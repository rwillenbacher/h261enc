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


Void h261_encode_macroblock_intra( h261_context_t *ps_ctx )
{
	Int32 i_idx;
	Int32 i_x, i_y, i_mb_x, i_mb_y;
	Int16 rgi16_temp[ 64 ];
	Int16 *pi16_coeffs;

	UInt8 *pui8_source;

	const Int32 i_y_offs[ 4 ][ 2 ] = {
		{ 0, 0 },
		{ 8, 0 },
		{ 0, 8 },
		{ 8, 8 },
	};

	h261_picture_t *ps_picture;
	h261_gob_t *ps_gob;
	h261_macroblock_t *ps_mb;
	h261_frame_t *ps_frame;

	ps_picture = &ps_ctx->s_picture;
	ps_gob = &ps_picture->s_groups_of_blocks;
	ps_mb = &ps_gob->s_macroblock;
	ps_frame = &ps_ctx->s_current_frame;

	/* data elements */
	ps_mb->i_macroblock_type = H261_MB_TYPE_INTRA;
	ps_mb->i_macroblock_type_flags = H261_MB_COEFF;
	ps_mb->i_coded_block_pattern = 0; /* cbp isnt used for intra blocks but we set it anyway */

	i_mb_x = ps_mb->i_mb_x;
	i_mb_y = ps_mb->i_mb_y;

	for( i_idx = 0; i_idx < 4; i_idx++ )
	{
		Int32 i_mb_ox, i_mb_oy;

		i_mb_ox = i_mb_x + i_y_offs[ i_idx ][ 0 ];
		i_mb_oy = i_mb_y + i_y_offs[ i_idx ][ 1 ];

		pi16_coeffs = &ps_mb->rgi16_tcoeff[ i_idx ][ 0 ];

		pui8_source = ps_frame->pui8_Y + ( i_mb_oy * ps_frame->i_stride_y ) + i_mb_ox;
		for( i_y = 0; i_y < 8; i_y++ )
		{
			for( i_x = 0; i_x < 8; i_x++ )
			{
				rgi16_temp[ i_y * 8 + i_x ] = pui8_source[ i_y * ps_frame->i_stride_y + i_x ];
			}
		}
		g_fdct_8x8( rgi16_temp, pi16_coeffs );
		h261_quant8x8_trellis_fw( ps_ctx, pi16_coeffs, ps_mb->i_macroblock_quant, 1 );
	}
	ps_mb->i_coded_block_pattern |= H261_CODED_BLOCK_PATTERN_1 | H261_CODED_BLOCK_PATTERN_2 |
		H261_CODED_BLOCK_PATTERN_3 | H261_CODED_BLOCK_PATTERN_4;

	pi16_coeffs = &ps_mb->rgi16_tcoeff[ 4 ][ 0 ];
	pui8_source = ps_frame->pui8_Cb + ( ( i_mb_y / 2 ) * ps_frame->i_stride_c ) + ( i_mb_x / 2 );
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			rgi16_temp[ i_y * 8 + i_x ] = pui8_source[ i_y * ps_frame->i_stride_c + i_x ];
		}
	}
	g_fdct_8x8( rgi16_temp, pi16_coeffs );
	h261_quant8x8_trellis_fw( ps_ctx, pi16_coeffs, ps_mb->i_macroblock_quant, 1 );
	ps_mb->i_coded_block_pattern |= H261_CODED_BLOCK_PATTERN_5;

	pi16_coeffs = &ps_mb->rgi16_tcoeff[ 5 ][ 0 ];
	pui8_source = ps_frame->pui8_Cr + ( ( i_mb_y / 2 ) * ps_frame->i_stride_c ) + ( i_mb_x / 2 );
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			rgi16_temp[ i_y * 8 + i_x ] = pui8_source[ i_y * ps_frame->i_stride_c + i_x ];
		}
	}
	g_fdct_8x8( rgi16_temp, pi16_coeffs );
	h261_quant8x8_trellis_fw( ps_ctx, pi16_coeffs, ps_mb->i_macroblock_quant, 1 );
	ps_mb->i_coded_block_pattern |= H261_CODED_BLOCK_PATTERN_6;
	
}


Void h261_encode_macroblock_inter( h261_context_t *ps_ctx )
{
	Int32 i_idx, i_has_coeff, i_accum_has_coeff;
	Int32 i_x, i_y, i_mv_x, i_mv_y, i_mb_x, i_mb_y;
	Int16 rgi16_temp[ 64 ];
	Int16 *pi16_coeffs;
	UInt8 rgui8_reference[ 64 ];
	UInt8 *rgpui8_c_sources[ 2 ][ 2 ];

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

	/* data elements */
	ps_mb->i_macroblock_type = H261_MB_TYPE_INTER;
	ps_mb->i_coded_block_pattern = 0;

	i_mb_x = ps_mb->i_mb_x;
	i_mb_y = ps_mb->i_mb_y;

	/* unmask not needed MC flag if possible */
	if( ( ps_mb->rgi_mv[ 0 ] | ps_mb->rgi_mv[ 1 ] ) == 0 && !( ps_mb->i_macroblock_type_flags & H261_MB_FILTER ) )
	{
		ps_mb->i_macroblock_type_flags &= ~H261_MB_MC;
	}

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
		UInt8 *pui8_current, *pui8_reference;

		i_x = i_mb_x + rgi_y_offs[ i_idx ][ 0 ];
		i_y = i_mb_y + rgi_y_offs[ i_idx ][ 1 ];

		pui8_current = ps_frame->pui8_Y + ( i_y * ps_frame->i_stride_y + i_x );
		pui8_reference = ps_reference->pui8_Y + ( ( i_y + i_mv_y ) * ps_frame->i_stride_y + i_x + i_mv_x );
		pi16_coeffs = &ps_mb->rgi16_tcoeff[ i_idx ][ 0 ];

		if( ps_mb->i_macroblock_type_flags & H261_MB_FILTER )
		{
			g_compensate_8x8_filter( rgui8_reference, 8, pui8_reference, ps_reference->i_stride_y );
		}
		else
		{
			g_compensate_8x8( rgui8_reference, 8, pui8_reference, ps_reference->i_stride_y );
		}

		g_sub_8x8( rgi16_temp, pui8_current, ps_frame->i_stride_y, rgui8_reference, 8 );
		g_fdct_8x8( rgi16_temp, pi16_coeffs );

		i_has_coeff = h261_quant8x8_trellis_fw( ps_ctx, pi16_coeffs, ps_mb->i_macroblock_quant, 0 );

		i_accum_has_coeff |= i_has_coeff;

		if( i_has_coeff )
		{
			ps_mb->i_coded_block_pattern |= rgi_cbp_table[ i_idx ];
		}
	}

	i_x = i_mb_x / 2;
	i_y = i_mb_y / 2;
	i_mv_x /= 2;
	i_mv_y /= 2;

	rgpui8_c_sources[ 0 ][ 0 ] = ps_frame->pui8_Cb;
	rgpui8_c_sources[ 0 ][ 1 ] = ps_reference->pui8_Cb;
	rgpui8_c_sources[ 1 ][ 0 ] = ps_frame->pui8_Cr;
	rgpui8_c_sources[ 1 ][ 1 ] = ps_reference->pui8_Cr;

	for( i_idx = 0; i_idx < 2; i_idx++ )
	{
		UInt8 *pui8_current, *pui8_reference;

		pui8_current = rgpui8_c_sources[ i_idx ][ 0 ] + ( i_y * ps_frame->i_stride_c + i_x );
		pui8_reference = rgpui8_c_sources[ i_idx ][ 1 ] + ( ( i_y + i_mv_y ) * ps_frame->i_stride_c + i_x + i_mv_x );
		pi16_coeffs = &ps_mb->rgi16_tcoeff[ 4 + i_idx ][ 0 ];

		if( ps_mb->i_macroblock_type_flags & H261_MB_FILTER )
		{
			g_compensate_8x8_filter( rgui8_reference, 8, pui8_reference, ps_reference->i_stride_c );
		}
		else
		{
			g_compensate_8x8( rgui8_reference, 8, pui8_reference, ps_reference->i_stride_c );
		}
		g_sub_8x8( rgi16_temp, pui8_current, ps_frame->i_stride_c, rgui8_reference, 8 );
		g_fdct_8x8( rgi16_temp, pi16_coeffs );

		i_has_coeff = h261_quant8x8_trellis_fw( ps_ctx, pi16_coeffs, ps_mb->i_macroblock_quant, 0 );

		i_accum_has_coeff |= i_has_coeff;

		if( i_has_coeff )
		{
			ps_mb->i_coded_block_pattern |= rgi_cbp_table_c[ i_idx ];
		}
	}


	if( i_accum_has_coeff )
	{
		ps_mb->i_macroblock_type_flags |= H261_MB_COEFF;
	}
}


#define CLAMP_256( x ) ( ( x ) < -255 ? -255 : ( ( x ) > 255 ? 255 : ( x ) ) )
#define CLAMP_256U( x ) ( ( x ) < 0 ? 0 : ( ( x ) > 255 ? 255 : ( x ) ) )

Void h261_reconstruct_macroblock_intra( h261_context_t *ps_ctx )
{
	Int32 i_idx, i_x, i_y, i_mb_x, i_mb_y;
	Int16 *pi16_coeffs;
	UInt8 *pui8_destination;
	DECLALIGNED( 16 ) Int16 rgi16_temp[ 64 ];

	const Int32 i_y_offs[ 4 ][ 2 ] = {
		{ 0, 0 },
		{ 8, 0 },
		{ 0, 8 },
		{ 8, 8 },
	};

	h261_picture_t *ps_picture;
	h261_gob_t *ps_gob;
	h261_macroblock_t *ps_mb;
	h261_frame_t *ps_current;
	h261_frame_t *ps_recon;

	ps_picture = &ps_ctx->s_picture;
	ps_gob = &ps_picture->s_groups_of_blocks;
	ps_mb = &ps_gob->s_macroblock;
	ps_current = &ps_ctx->s_current_frame;
	ps_recon = &ps_ctx->s_reconstructed_frame;

	i_mb_x = ps_mb->i_mb_x;
	i_mb_y = ps_mb->i_mb_y;

	for( i_idx = 0; i_idx < 4; i_idx++ )
	{
		Int32 i_mb_ox, i_mb_oy;

		pi16_coeffs = &ps_mb->rgi16_tcoeff[ i_idx ][ 0 ];

		h261_quant8x8_intra_bw( pi16_coeffs, 8, ps_mb->i_macroblock_quant );
		g_idct_8x8( pi16_coeffs, &rgi16_temp[ 0 ] );

		i_mb_ox = i_y_offs[ i_idx ][ 0 ] + i_mb_x;
		i_mb_oy = i_y_offs[ i_idx ][ 1 ] + i_mb_y;

		pui8_destination = &ps_recon->pui8_Y[ i_mb_oy * ps_current->i_stride_y + i_mb_ox ];
		for( i_y = 0; i_y < 8; i_y++ )
		{
			for( i_x = 0; i_x < 8; i_x++ )
			{
				pui8_destination[ i_y * ps_current->i_stride_y + i_x ] = CLAMP_256U( rgi16_temp[ i_y * 8 + i_x ] );
			}
		}
	}


	pi16_coeffs = &ps_mb->rgi16_tcoeff[ 4 ][ 0 ];
	h261_quant8x8_intra_bw( pi16_coeffs, 8, ps_mb->i_macroblock_quant );
	g_idct_8x8( pi16_coeffs, &rgi16_temp[ 0 ] );

	pui8_destination = &ps_recon->pui8_Cb[ ( i_mb_y / 2 ) * ps_current->i_stride_c + ( i_mb_x / 2 ) ];
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			pui8_destination[ i_y * ps_current->i_stride_c + i_x ] = CLAMP_256U( rgi16_temp[ i_y * 8 + i_x ] );
		}
	}

	pi16_coeffs = &ps_mb->rgi16_tcoeff[ 5 ][ 0 ];
	h261_quant8x8_intra_bw( pi16_coeffs, 8, ps_mb->i_macroblock_quant );
	g_idct_8x8( pi16_coeffs, &rgi16_temp[ 0 ] );

	pui8_destination = &ps_recon->pui8_Cr[ ( i_mb_y / 2 ) * ps_current->i_stride_c + ( i_mb_x / 2 ) ];
	for( i_y = 0; i_y < 8; i_y++ )
	{
		for( i_x = 0; i_x < 8; i_x++ )
		{
			pui8_destination[ i_y * ps_current->i_stride_c + i_x ] = CLAMP_256U( rgi16_temp[ i_y * 8 + i_x ] );
		}
	}

}



Void h261_reconstruct_macroblock_inter( h261_context_t *ps_ctx )
{
	Int32 i_idx;
	Int32 i_x, i_y, i_mv_x, i_mv_y, i_mb_x, i_mb_y;
	DECLALIGNED( 16 ) Int16 rgi16_temp[ 64 ];
	Int16 *pi16_coeffs;
	UInt8 *rgpui8_c_sources[ 2 ][ 3 ];

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
	h261_frame_t *ps_recon;
	h261_frame_t *ps_reference;

	ps_picture = &ps_ctx->s_picture;
	ps_gob = &ps_picture->s_groups_of_blocks;
	ps_mb = &ps_gob->s_macroblock;
	ps_frame = &ps_ctx->s_current_frame;
	ps_recon = &ps_ctx->s_reconstructed_frame;
	ps_reference = &ps_ctx->s_reference_frame;

	i_mb_x = ps_mb->i_mb_x;
	i_mb_y = ps_mb->i_mb_y;

	i_mv_x = ps_mb->rgi_mv[ 0 ];
	i_mv_y = ps_mb->rgi_mv[ 1 ];

	for( i_idx = 0; i_idx < 4; i_idx++ )
	{
		UInt8 *pui8_current, *pui8_reference, *pui8_recon;

		i_x = i_mb_x + rgi_y_offs[ i_idx ][ 0 ];
		i_y = i_mb_y + rgi_y_offs[ i_idx ][ 1 ];

		pui8_current = ps_frame->pui8_Y + ( i_y * ps_frame->i_stride_y + i_x );
		pui8_recon = ps_recon->pui8_Y + ( i_y * ps_frame->i_stride_y + i_x );
		pui8_reference = ps_reference->pui8_Y + ( ( i_y + i_mv_y ) * ps_frame->i_stride_y + i_x + i_mv_x );
		pi16_coeffs = &ps_mb->rgi16_tcoeff[ i_idx ][ 0 ];

		if( ps_mb->i_macroblock_type_flags & H261_MB_FILTER )
		{
			g_compensate_8x8_filter( pui8_recon, ps_recon->i_stride_y, pui8_reference, ps_reference->i_stride_y );
		}
		else
		{
			g_compensate_8x8( pui8_recon, ps_recon->i_stride_y, pui8_reference, ps_reference->i_stride_y );
		}

		if( rgi_cbp_table[ i_idx ] & ps_mb->i_coded_block_pattern )
		{
			h261_quant8x8_inter_bw( pi16_coeffs, 8, ps_mb->i_macroblock_quant );
			g_idct_8x8( pi16_coeffs, rgi16_temp );
			g_add_8x8( pui8_recon, ps_frame->i_stride_y, pui8_recon, ps_recon->i_stride_y, rgi16_temp );
		}
	}

	i_mb_x /= 2;
	i_mb_y /= 2;
	i_mv_x /= 2;
	i_mv_y /= 2;

	rgpui8_c_sources[ 0 ][ 0 ] = ps_frame->pui8_Cb;
	rgpui8_c_sources[ 0 ][ 1 ] = ps_reference->pui8_Cb;
	rgpui8_c_sources[ 0 ][ 2 ] = ps_recon->pui8_Cb;
	rgpui8_c_sources[ 1 ][ 0 ] = ps_frame->pui8_Cr;
	rgpui8_c_sources[ 1 ][ 1 ] = ps_reference->pui8_Cr;
	rgpui8_c_sources[ 1 ][ 2 ] = ps_recon->pui8_Cr;

	for( i_idx = 0; i_idx < 2; i_idx++ )
	{
		UInt8 *pui8_current, *pui8_reference, *pui8_recon;

		i_x = i_mb_x;
		i_y = i_mb_y;

		pui8_current = rgpui8_c_sources[ i_idx ][ 0 ] + ( i_y * ps_frame->i_stride_c + i_x );
		pui8_reference = rgpui8_c_sources[ i_idx ][ 1 ] + ( ( i_y + i_mv_y ) * ps_frame->i_stride_c + i_x + i_mv_x );
		pui8_recon = rgpui8_c_sources[ i_idx ][ 2 ] + ( i_y * ps_frame->i_stride_c + i_x );
		pi16_coeffs = &ps_mb->rgi16_tcoeff[ 4 + i_idx ][ 0 ];

		if( ps_mb->i_macroblock_type_flags & H261_MB_FILTER )
		{
			g_compensate_8x8_filter( pui8_recon, ps_recon->i_stride_c, pui8_reference, ps_reference->i_stride_c );
		}
		else
		{
			g_compensate_8x8( pui8_recon, ps_recon->i_stride_c, pui8_reference, ps_reference->i_stride_c );
		}

		if( rgi_cbp_table_c[ i_idx ] & ps_mb->i_coded_block_pattern )
		{
			h261_quant8x8_inter_bw( pi16_coeffs, 8, ps_mb->i_macroblock_quant );
			g_idct_8x8( pi16_coeffs, rgi16_temp );
			g_add_8x8( pui8_recon, ps_frame->i_stride_c, pui8_recon, ps_recon->i_stride_c, rgi16_temp );
		}
	}
}


Void h261_reconstruct_macroblock( h261_context_t *ps_ctx )
{
	h261_picture_t *ps_picture;
	h261_gob_t *ps_gob;
	h261_macroblock_t *ps_mb;

	ps_picture = &ps_ctx->s_picture;
	ps_gob = &ps_picture->s_groups_of_blocks;
	ps_mb = &ps_gob->s_macroblock;

	switch( ps_mb->i_macroblock_type )
	{
	case H261_MB_TYPE_INTRA:
		h261_reconstruct_macroblock_intra( ps_ctx );
		break;

	case H261_MB_TYPE_INTER:
		h261_reconstruct_macroblock_inter( ps_ctx );
		break;
	}
}
