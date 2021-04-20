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

#define H261_RATECTRL_BITS 6
#define H261_RATECTRL_ONE ( 1 << H261_RATECTRL_BITS )
#define H261_RATECTRL_HALF ( 1 << ( H261_RATECTRL_BITS - 1 ) )
/* 29.97 */
#define H261_RATECTRL_TIME_SCALE 1918

/* pow( 2.0, f/5 ), f = -31 to 31 */
static Int32 h261_ratectrl_qscale[ 63 ] = {
	0, 1, 1, 1, 1, 1, 2, 2, 2, 3, /* -31 - -22 */
	3, 4, 4, 5, 6, 6, 8, 9, 10, 12, /* -21 - -12 */
	13, 16, 18, 21, 24, 27, 32, 36, 42, 48, 55,  /* -11 - -1 */
	64, /* 0 */
	73, 84, 97, 111, 128, 147, 168, 194, 222, 256, 294, /* 1 - 11 */
	337, 388, 445, 512, 588, 675, 776, 891, 1024, 1176, /* 12 - 21 */
	1351, 1552, 1782, 2048, 2352, 2702, 3104, 3565, 4096, 4705 /* 22 - 31 */
};


Int32 h261_ratectrl_mul( Int32 m1, Int32 m2 )
{
	Int64 m;

	m = ( ( Int64 ) m1 ) * ( ( Int64 ) m2 );

	return ( Int32 ) ( m >> H261_RATECTRL_BITS );
}


Int32 h261_ratectrl_div( Int32 i_div1, Int32 i_div2 )
{
	Int64 d;

	d = ( ( ( Int64 ) i_div1 ) << H261_RATECTRL_BITS ) / i_div2;

	return ( Int32 ) d;
}


Int32 h261_ratectrl_int_to_fp( Int32 i_int )
{
	return i_int << H261_RATECTRL_BITS;
}


Int32 h261_ratectrl_fp_to_int( Int32 i_fp )
{
	return i_fp >> H261_RATECTRL_BITS;
}


Int32 h261_ratectrl_qscale_for_quant( Int32 i_quant )
{
	i_quant = MIN( MAX( i_quant, 0 ), 31 );
	i_quant += 31;

	return h261_ratectrl_qscale[ i_quant ];
}


Int32 h261_ratectrl_quant_for_qscale( Int32 i_qscale )
{
	Int32 i_pos, i_step, i_lower_idx, i_upper_idx;

	i_pos = 31;
	i_step = 16;

	while( i_step )
	{
		if( h261_ratectrl_qscale[ i_pos ] < i_qscale )
		{
			i_pos += i_step;
		}
		else
		{
			i_pos -= i_step;
		}
		i_step = i_step >> 1;
	}

	if( h261_ratectrl_qscale[ i_pos ] < i_qscale )
	{
		i_lower_idx = i_pos;
		i_upper_idx = MIN( i_pos + 1, 62 );
	}
	else
	{
		i_lower_idx = MAX( 0, i_pos - 1 );
		i_upper_idx = i_pos;
	}

	if( i_qscale - h261_ratectrl_qscale[ i_lower_idx ] < h261_ratectrl_qscale[ i_upper_idx ] - i_qscale )
	{
		i_pos = i_lower_idx;
	}
	else
	{
		i_pos = i_upper_idx;
	}

	return i_pos - 31;
}


Void h261_init_ratectrl( h261_context_t *ps_ctx, Int32 i_quant, Int32 i_bitrate, Int32 i_vbv_size, Int32 i_mode )
{
	Int32 i_idx;

	h261_ratectrl_t *ps_ratectrl;

	ps_ratectrl = &ps_ctx->s_ratectrl;

	ps_ratectrl->i_ratectrl_mode = i_mode;
	ps_ratectrl->i_current_quant = i_quant;

	ps_ratectrl->f_bitrate = i_bitrate;
	ps_ratectrl->f_framerate = 1.0 / 29.97;

	ps_ratectrl->f_actual_bits = i_bitrate * ps_ratectrl->f_framerate;
	ps_ratectrl->f_wanted_bits = i_bitrate * ps_ratectrl->f_framerate;
	ps_ratectrl->f_bits_quant = ps_ratectrl->f_actual_bits * ps_ratectrl->i_current_quant;

}


Void h261_ratectrl_finish( h261_context_t *ps_ctx )
{
	h261_ratectrl_t *ps_ratectrl;

	ps_ratectrl = &ps_ctx->s_ratectrl;

	fprintf( stderr, "ratectrl final quantizer: %f\n", ps_ratectrl->f_bits_quant / ps_ratectrl->f_wanted_bits );

}


Void h261_ratectrl_init_picture( h261_context_t *ps_ctx, h261_picture_parameters_t *ps_picture )
{
	h261_ratectrl_t *ps_ratectrl;

	ps_ratectrl = &ps_ctx->s_ratectrl;

	if( ps_ratectrl->i_ratectrl_mode == H261_RATECTRL_MODE_QUANT )
	{
		ps_picture->i_quantiser = ps_ratectrl->i_current_quant;
	}
	else if( ps_ratectrl->i_ratectrl_mode == H261_RATECTRL_MODE_VBR )
	{
		float f_quantizer, f_quantizer_adjust;

		f_quantizer = ps_ratectrl->f_bits_quant / ps_ratectrl->f_actual_bits;
		f_quantizer_adjust = MIN( 2.0, MAX( 0.5, 1.0 + ( ( ps_ratectrl->f_actual_bits - ps_ratectrl->f_wanted_bits ) / ps_ratectrl->f_bitrate ) ) );
		f_quantizer = f_quantizer * f_quantizer_adjust;

		ps_ratectrl->i_current_quant = MIN( 31, MAX( 0, ( Int32 ) floorf( f_quantizer + 0.5f ) ) );
		ps_picture->i_quantiser = ps_ratectrl->i_current_quant;
	}
}


Void h261_ratectrl_update_picture( h261_context_t *ps_ctx, Int32 i_bitstream_bytes )
{
	h261_ratectrl_t *ps_ratectrl;

	ps_ratectrl = &ps_ctx->s_ratectrl;

	if( ps_ratectrl->i_ratectrl_mode == H261_RATECTRL_MODE_VBR )
	{
		ps_ratectrl->f_actual_bits += i_bitstream_bytes * 8;
		ps_ratectrl->f_wanted_bits += ps_ratectrl->f_bitrate * ps_ratectrl->f_framerate;
		ps_ratectrl->f_bits_quant += i_bitstream_bytes * 8 * ps_ratectrl->i_current_quant;
	}
}


