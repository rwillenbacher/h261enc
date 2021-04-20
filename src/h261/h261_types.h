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

#define H261_FRAME_TYPE_INTRA				0
#define H261_FRAME_TYPE_INTER				1

#define H261_PICTURE_START_CODE				16
#define H261_PICTURE_START_CODE_LENGTH		20

#define H261_SOURCE_FORMAT_QCIF 0
#define H261_SOURCE_FORMAT_CIF 1

#define H261_GOB_WIDTH		176
#define H261_GOB_HEIGHT		48

#define H261_MB_TYPE_INTRA					0
#define H261_MB_TYPE_INTRA_MQUANT			1
#define H261_MB_TYPE_INTER					2
#define H261_MB_TYPE_INTER_MQUANT			3
#define H261_MB_TYPE_INTER_MC				4
#define H261_MB_TYPE_INTER_MC_COEFF			5
#define H261_MB_TYPE_INTER_MQUANT_MC_COEFF	6
#define H261_MB_TYPE_INTER_MC_FILTER		7
#define H261_MB_TYPE_INTER_MC_FILTER_COEFF	8
#define H261_MB_TYPE_INTER_MQUANT_MC_FILTER_COEFF 9


#define H261_MB_MC	   1
#define H261_MB_FILTER 2
#define H261_MB_COEFF  4


#define H261_CODED_BLOCK_PATTERN_1	32
#define H261_CODED_BLOCK_PATTERN_2	16
#define H261_CODED_BLOCK_PATTERN_3	8
#define H261_CODED_BLOCK_PATTERN_4	4
#define H261_CODED_BLOCK_PATTERN_5	2
#define H261_CODED_BLOCK_PATTERN_6	1


#define H261_RATECTRL_MODE_QUANT	1
#define H261_RATECTRL_MODE_VBR		2

#define MAX_COST ( 1 << 30 )

#define H261_BLOCK_16x16 0
#define H261_BLOCK_8x8   1

#define H261_LEFT        0
#define H261_TOP         1
#define H261_RIGHT       2
#define H261_BOTTOM      3

/* functional defines */

#define H261_ABS( x ) ( ( x ) < 0 ? -( x ) : ( x ) )



/* h261 encoder structures */

typedef struct {
	Int32	i_width;
	Int32	i_height;
	Int32	i_stride_y;
	Int32	i_stride_c;

	UInt8	*pui8_Y;
	UInt8	*pui8_Cr;
	UInt8	*pui8_Cb;
} h261_frame_t;


typedef struct {
	UInt8 *pui8_bitstream;
	Int32 i_length;
	
	Int32 i_byte_count;
	Int32 i_next_bit;

	UInt8 *pui8_codeword_ptr;
	UInt32 ui_codeword;
	Int32  i_codeword_fill;

} h261_bitstream_t;

typedef struct {
	Int8 rgi16_mv[ 2 ];
} h261_mb_cache_t;

typedef struct h261_macroblock_s {
	/* syntax elements */
	Int32	i_macroblock_address;
	Int32	i_macroblock_type;		/* intra or inter */
	Int32	i_macroblock_type_flags;/* flags of present bitstream elements */

	Int32	i_macroblock_quant;
	Int32	rgi_mv[ 2 ];
	Int32	i_coded_block_pattern;

	DECLALIGNED( 16 ) Int16 rgi16_tcoeff[ 6 ][ 64 ];

	/* data elements */
	Int32	i_mb_x;		/* in pel coords */
	Int32	i_mb_y;
	Int32	rgi_me_mv[ 2 ];
	Int32   i_lambda;
	Int32   i_lambda_rdo;
	h261_mb_cache_t *ps_mb_cache;
	h261_mb_cache_t *rgps_neighbours[ 4 ];
} h261_macroblock_t;


typedef struct h261_mb_coding_cache_t {
	Int32 i_quantiser;
	Int32 i_last_mv_valid;
	Int32 i_last_mv_x;
	Int32 i_last_mv_y;
} h261_mb_coding_vars_t;


typedef struct {
	/* syntax elements */
	Int32	i_group_of_blocks_start_code; /* 16 bits, 0x0001 */
	Int32	i_group_number;
	Int32	i_quantiser;

	Int32	i_num_extra_insertion_information; /* always 0 */

	/* data elements */
	h261_macroblock_t s_macroblock;

	/* coding control */
	h261_mb_coding_vars_t s_coding_vars;
} h261_gob_t;


typedef struct {

	/* syntax elements */
	Int32	i_picture_start_code; /* 20 bits, 0x00010 */
	Int32	i_temporal_reference; /* continuity counter */

	/* ptype syntax elements */
	Int32	i_split_screen_indicator;
	Int32	i_document_camera_indicator;
	Int32	i_freeze_picture_release;
	Int32	i_source_format;
	Int32	i_highres_mode;
	Int32	i_spare_bit;	/* always 1 */

	Int32 i_num_extra_insertion_information; /* always 0 */
	UInt8 rgui8_extra_insertion_information[ 1 ];

	/* data elements */
	h261_gob_t		s_groups_of_blocks;

	/* coding control */
	Int32 i_quantiser;
	Int32 i_frame_type;

} h261_picture_t;


typedef struct {
	Int32 i_ratectrl_mode;
	Int32 i_current_quant;

	float f_framerate;
	float f_bitrate;
	float f_bits_quant;
	float f_wanted_bits;
	float f_actual_bits;

} h261_ratectrl_t;


typedef struct {
	Int32	i_source_format;

	Int32	i_frame_counter;
	h261_frame_t	s_current_frame;
	h261_frame_t	s_reconstructed_frame;
	h261_frame_t	s_reference_frame;

	h261_picture_t	s_picture;

	h261_bitstream_t s_bitstream;

	h261_ratectrl_t		s_ratectrl;

	h261_mb_cache_t *ps_mb_cache;

} h261_context_t;


typedef struct {
	Int32 i_frame_num;
	Int32 i_source_format;
	Int32 i_quantiser;
	Int32 i_frame_type;
} h261_picture_parameters_t;


