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


typedef struct {
	UInt8 i_code;
	UInt8 i_length;
} h261_vlc_t;


#define H261_COEFF_RUN_LEVEL_ESCAPE -1

typedef struct {
	UInt8 i_run;
	UInt8 i_level;
	h261_vlc_t s_vlc;
} h261_coeff_run_level_t;


/* macroblock address vlc table */
#define H261_MBA_START_CODE 0
#define H261_MBA_STUFFING   34
extern h261_vlc_t h261_mba_table[];

/* macroblock type vlc table */
extern h261_vlc_t h261_mtype_table[ 10 ];

/* coded block pattern vlc table */
extern h261_vlc_t h261_cbp_table[ 64 ];

/* motion vector difference vlc table */
extern h261_vlc_t h261_mvd_table[ 32 ];

/* run / level vlc table */
extern h261_coeff_run_level_t h261_coeff_run_level_table[ ];

/* [ run ][ level ] idx into h261_coeff_run_level_table */
extern UInt8 rgi_h261_run_level_lut[ 27 ][ 16 ];

/* motion vector difference lookup table */
#define H261_MVD_TRANSLATION_TABLE_OFFSET 32
extern UInt8 rgi_mvd_translation_table[ 64 ];

/* zig zag lookup table */
extern UInt8 rgi_transmission_order_table[ 64 ];

extern Int32 rgi_lambda2[ 32 ];
extern Int32 rgi_lambda[ 32 ];

extern Int16 rgi16_mb_dim[ 2 ][ 2 ];
