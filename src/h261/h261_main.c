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

void usage( char *rgpui8_argv[] )
{
	printf( "%s: <cif file> [options]\n", rgpui8_argv[ 0 ] );
	printf( "options:\n" );
	printf( "-out <file>\n" );
	printf( "-rec <file>\n" );
	printf( "-bitrate <bitrate or quant>\n" );
	printf( "-snrin <file>\n" );
	printf( "-keyint <number>\n" );
}


Void h261_init_frame( h261_frame_t *ps_frame, Int32 i_width, Int32 i_height )
{
	Int32 i_frame_size_y, i_frame_size_c;

	ps_frame->i_width = i_width;
	ps_frame->i_height = i_height;
	ps_frame->i_stride_y = i_width;
	ps_frame->i_stride_c = i_width / 2;

	i_frame_size_y = ps_frame->i_stride_y * ps_frame->i_height;
	i_frame_size_c = ps_frame->i_stride_c * ( ps_frame->i_height / 2 );

	ps_frame->pui8_Y = malloc( i_frame_size_y );
	ps_frame->pui8_Cb = malloc( i_frame_size_c );
	ps_frame->pui8_Cr = malloc( i_frame_size_c );

	memset( ps_frame->pui8_Y, 0, i_frame_size_y );
	memset( ps_frame->pui8_Cb, 0, i_frame_size_c );
	memset( ps_frame->pui8_Cr, 0, i_frame_size_c );
}

Void h261_deinit_frame( h261_frame_t *ps_frame )
{
	free( ps_frame->pui8_Y );
	free( ps_frame->pui8_Cb );
	free( ps_frame->pui8_Cr );
}

Void get_frame_psnr( h261_frame_t *ps_original, h261_frame_t *ps_recon, double *pd_psnr )
{
	Int32 i_x, i_y, i_num_pel_x, i_num_pel_y, i_pel_stride, i_idx;
	UInt64 ui64_ssd;
	UInt8 *pui8_ref, *pui8_recon;

	for( i_idx = 0; i_idx < 3; i_idx++ )
	{
		if( i_idx == 0 )
		{
			pui8_ref = ps_original->pui8_Y;
			pui8_recon = ps_recon->pui8_Y;
			i_num_pel_x = ps_original->i_width;
			i_num_pel_y = ps_original->i_height;
			i_pel_stride = ps_original->i_stride_y;
		}
		else
		{
			if( i_idx == 1 )
			{
				pui8_ref = ps_original->pui8_Cb;
				pui8_recon = ps_recon->pui8_Cb;
			}
			else
			{
				pui8_ref = ps_original->pui8_Cr;
				pui8_recon = ps_recon->pui8_Cr;
			}
			i_num_pel_x = ps_original->i_width / 2;
			i_num_pel_y = ps_original->i_height / 2;
			i_pel_stride = ps_original->i_stride_c;
		}
		ui64_ssd = 0;
		for( i_y = 0; i_y < i_num_pel_y; i_y++ )
		{
			for( i_x = 0; i_x < i_num_pel_x; i_x++ )
			{
				UInt32 ui_delta;
				ui_delta = pui8_ref[ i_x ] - pui8_recon[ i_x ];
				ui64_ssd += ui_delta * ui_delta;
			}
			pui8_ref += i_pel_stride;
			pui8_recon += i_pel_stride;
		}

	    if( 0 == ui64_ssd )
	    {
			pd_psnr[ i_idx ] = 100.0;
		}
		else 
		{
	        pd_psnr[ i_idx ] = 10 * log10( 255.0 * 255.0 * ( i_num_pel_x * i_num_pel_y ) / ( double )( ui64_ssd ) );
	    }
	}
}



int main( Int32 i_argc, char *rgpui8_argv[] )
{
	UInt8 *pui8_bitstream;
	Int32 i_idx, i_width, i_height, i_frame_size, i_frame_counter, i_bitstream_length, i_quant, i_bitrate, i_y4m, i_keyint;
	Int32 i_sequence_size;
	double d_start_time, d_end_time, d_global_start_time, d_global_end_time, rgd_psnr[ 3 ], rgd_psnr_mean[ 3 ];
	FILE *f, *fout, *frecout, *fsnrin;
	char *pc_recon_fname, *pc_out_fname, *pc_snrin_fname;

	h261_frame_t *ps_source_frame;
	h261_frame_t *ps_recon_frame;
	h261_frame_t *ps_reference_frame;
	h261_context_t *ps_context;
	h261_bitstream_t *ps_bitstream;

	if( i_argc < 2 )
	{
		usage( rgpui8_argv );
		return 1;
	}

	f = fopen( rgpui8_argv[ 1 ], "rb" );
	if( !f )
	{
		usage( rgpui8_argv );
		return 1;
	}
	if( strstr( rgpui8_argv[ 1 ], ".y4m") )
	{
		while( fgetc( f ) != 0x0a );
		i_y4m = 1;
	}
	else
	{
		i_y4m = 0;
	}

	fout = NULL;
	frecout = NULL;
	fsnrin = NULL;
	i_quant = 10;
	i_keyint = 50;

	for( i_idx = 2; i_idx < i_argc; i_idx++ )
	{
		if( strcmp( rgpui8_argv[ i_idx ], "-out" ) == 0 )
		{
			fout = fopen( rgpui8_argv[ ++i_idx ], "wb" );
			if( !fout )
			{
				fprintf( stderr, "error: unable to open output file for writing\n" );
				return 2;
			}
		}
		else if( strcmp( rgpui8_argv[ i_idx ], "-rec" ) == 0 )
		{
			frecout = fopen( rgpui8_argv[ ++i_idx ], "wb" );
			if( !frecout )
			{
				fprintf( stderr, "error: unable to open recon file for writing\n" );
				return 3;
			}
		}
		else if( strcmp( rgpui8_argv[ i_idx ], "-bitrate" ) == 0 )
		{
			i_quant = atoi( rgpui8_argv[ ++i_idx ] );
		}
		else if( strcmp( rgpui8_argv[ i_idx ], "-snrin" ) == 0 )
		{
			fsnrin = fopen( rgpui8_argv[ ++i_idx ], "rb" );
			if( !fsnrin )
			{
				fprintf( stderr, "error: unable to open snrin file for reading\n" );
				return 4;
			}
		}
		else if( strcmp( rgpui8_argv[ i_idx ], "-keyint" ) == 0 )
		{
			i_keyint = atoi( rgpui8_argv[ ++i_idx ] );
		}
		else
		{
			fprintf( stderr, "error: unknown command line option '%s'\n", rgpui8_argv[ i_idx ] );
			usage( rgpui8_argv );
			return 5;
		}
	}

	ps_context = malloc( sizeof( h261_context_t ) );
	memset( ps_context, 0, sizeof( h261_context_t ) );

	ps_context->i_source_format = H261_SOURCE_FORMAT_CIF;
	i_width = 352;
	i_height = 288;
	i_frame_size = i_width * i_height;

	/* frame init */
	ps_source_frame = &ps_context->s_current_frame;
	ps_recon_frame = &ps_context->s_reconstructed_frame;
	ps_reference_frame = &ps_context->s_reference_frame;

	h261_init_frame( ps_source_frame, i_width, i_height );
	h261_init_frame( ps_recon_frame, i_width, i_height );
	h261_init_frame( ps_reference_frame, i_width, i_height );

	/* bitstream init */
	ps_bitstream = &ps_context->s_bitstream;
	h261_bitstream_init( ps_bitstream, 0x100000 );

	/* encoder core init */
	ps_context->ps_mb_cache = malloc( sizeof( h261_mb_cache_t ) * rgi16_mb_dim[ ps_context->i_source_format ][ 0 ] * rgi16_mb_dim[ ps_context->i_source_format ][ 1 ] );
	memset( ps_context->ps_mb_cache, 0, sizeof( h261_mb_cache_t ) * rgi16_mb_dim[ ps_context->i_source_format ][ 0 ] * rgi16_mb_dim[ ps_context->i_source_format ][ 1 ] );
	h261_initialize_pixelop_functions(); 
	h261_initialize_transform_functions();

	if( i_quant > 31 )
	{
		i_bitrate = i_quant * 1000;
		i_quant = 20;
		h261_init_ratectrl( ps_context, i_quant, i_bitrate, i_bitrate, H261_RATECTRL_MODE_VBR );
	}
	else
	{
		i_bitrate = 0;
		h261_init_ratectrl( ps_context, i_quant, i_bitrate, i_bitrate, H261_RATECTRL_MODE_QUANT );
	}

	i_frame_counter = 0;

	rgd_psnr_mean[ 0 ] = rgd_psnr_mean[ 1 ] = rgd_psnr_mean[ 2 ] = 0.0;
	i_sequence_size = 0;
	d_global_start_time = h261_get_time();

	while( 1 )
	{
		Int32 i_ret;
		h261_picture_parameters_t s_picture_parameters;

		if( i_y4m )
		{
			Int32 i_eof;
			while( 1 )
			{
				i_eof = fgetc( f );
				if( i_eof < 0 || i_eof == 0x0a )
				{
					break;
				}
			}
		}
		fread( ps_source_frame->pui8_Y, i_frame_size, 1, f );
		fread( ps_source_frame->pui8_Cb, i_frame_size / 4, 1, f );
		i_ret = fread( ps_source_frame->pui8_Cr, i_frame_size / 4, 1, f );
		if( i_ret <= 0 )
		{
			break;
		}

		d_start_time = h261_get_time();

		/* init picture */
		s_picture_parameters.i_frame_num = i_frame_counter;
		s_picture_parameters.i_source_format = ps_context->i_source_format;

		if( ( i_frame_counter % i_keyint ) == 0 )
		{
			s_picture_parameters.i_frame_type = H261_FRAME_TYPE_INTRA;
		}
		else
		{
			s_picture_parameters.i_frame_type = H261_FRAME_TYPE_INTER;
		}

		/* ratectrl */
		h261_ratectrl_init_picture( ps_context, &s_picture_parameters );

		/* encode macroblocks */
		h261_init_picture( ps_context, &s_picture_parameters );

		/* bitstream writer */
		h261_write_picture_header( ps_context );
		h261_encode_gobs( ps_context );

		/* bitstream out */
		h261_bitstream_get( ps_bitstream, &pui8_bitstream, &i_bitstream_length );
		if( fout )
		{
			fwrite( pui8_bitstream, i_bitstream_length, 1, fout );
		}
		h261_bitstream_advance( ps_bitstream );

		/* update functions */
		h261_ratectrl_update_picture( ps_context, i_bitstream_length );

		d_end_time = h261_get_time();

		if( fsnrin )
		{
			fread( ps_reference_frame->pui8_Y, i_frame_size, 1, fsnrin );
			fread( ps_reference_frame->pui8_Cb, i_frame_size / 4, 1, fsnrin );
			fread( ps_reference_frame->pui8_Cr, i_frame_size / 4, 1, fsnrin );
			get_frame_psnr( ps_source_frame, ps_reference_frame, &rgd_psnr[ 0 ] );
		}
		memcpy( ps_reference_frame->pui8_Y, ps_context->s_reconstructed_frame.pui8_Y, i_frame_size );
		memcpy( ps_reference_frame->pui8_Cb, ps_context->s_reconstructed_frame.pui8_Cb, i_frame_size / 4 );
		memcpy( ps_reference_frame->pui8_Cr, ps_context->s_reconstructed_frame.pui8_Cr, i_frame_size / 4 );

		if( !fsnrin )
		{
			get_frame_psnr( ps_source_frame, ps_reference_frame, &rgd_psnr[ 0 ] );
		}

		if( frecout )
		{
			fwrite( ps_context->s_reconstructed_frame.pui8_Y, i_frame_size, 1, frecout );
			fwrite( ps_context->s_reconstructed_frame.pui8_Cb, i_frame_size / 4, 1, frecout );
			fwrite( ps_context->s_reconstructed_frame.pui8_Cr, i_frame_size / 4, 1, frecout );
		}

		printf("frame %04d, quant=%d, size=%d, ypsnr=%f, cbpsnr=%f, crpsnr=%f, time=%.3fms\n",
			i_frame_counter,
			s_picture_parameters.i_quantiser,
			i_bitstream_length,
			rgd_psnr[ 0 ], rgd_psnr[ 1 ], rgd_psnr[ 2 ],
			d_end_time - d_start_time );

		rgd_psnr_mean[ 0 ] += rgd_psnr[ 0 ];
		rgd_psnr_mean[ 1 ] += rgd_psnr[ 1 ];
		rgd_psnr_mean[ 2 ] += rgd_psnr[ 2 ];
		i_sequence_size += i_bitstream_length;

		i_frame_counter++;

	}
	d_global_end_time = h261_get_time();

	printf("PSNR Mean: %f, %f, %f\n",
		rgd_psnr_mean[ 0 ] / i_frame_counter,
		rgd_psnr_mean[ 1 ] / i_frame_counter,
		rgd_psnr_mean[ 2 ] / i_frame_counter );
	printf("%f kBit/s, ", ( ( double ) i_sequence_size * 8 ) / ( ( double )i_frame_counter / 0.02997 ) );
	printf("%f fps\n", ( ( double ) i_frame_counter ) / ( ( d_global_end_time - d_global_start_time ) / 1000.0 ) );

	h261_bitstream_flush( ps_bitstream, &pui8_bitstream, &i_bitstream_length );
	if( i_bitstream_length && fout )
	{
		fwrite( pui8_bitstream, i_bitstream_length, 1, fout );
	}

	h261_ratectrl_finish( ps_context );

	h261_bitstream_deinit( ps_bitstream );
	h261_deinit_frame( ps_source_frame );
	h261_deinit_frame( ps_recon_frame );
	h261_deinit_frame( ps_reference_frame );

	free( ps_context->ps_mb_cache );
	free( ps_context );

	fclose( f );

	if( fout )
	{
		fclose( fout );
	}
	if( frecout )
	{
		fclose( frecout );
	}

	return 0;
}
