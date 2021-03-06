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

h261_vlc_t h261_mba_table[] = {
	{ 1, 16 },
	{ 1, 1 },
	{ 3, 3 },
	{ 2, 3 },
	{ 3, 4 },
	{ 2, 4 }, /* 5 */
	{ 3, 5 },
	{ 2, 5 },
	{ 7, 7 },
	{ 6, 7 },
	{ 11, 8 }, /* 10 */
	{ 10, 8 },
	{ 9, 8 },
	{ 8, 8 },
	{ 7, 8 },
	{ 6, 8 }, /* 15 */
	{ 23, 10 },
	{ 22, 10 },
	{ 21, 10 },
	{ 20, 10 },
	{ 19, 10 }, /* 20 */
	{ 18, 10 },
	{ 35, 11 },
	{ 34, 11 },
	{ 33, 11 },
	{ 32, 11 }, /* 25 */
	{ 31, 11 },
	{ 30, 11 },
	{ 29, 11 },
	{ 28, 11 },
	{ 27, 11 }, /* 30 */
	{ 26, 11 },
	{ 25, 11 },
	{ 24, 11 },
	{ 15, 11 } /* mba stuffing */
};



h261_vlc_t h261_mtype_table[ 10 ] = {
	{ 1, 4 },
	{ 1, 7 },
	{ 1, 1 },
	{ 1, 5 },
	{ 1, 9 },
	{ 1, 8 },
	{ 1, 10 },
	{ 1, 3 },
	{ 1, 2 },
	{ 1, 6 }
};


h261_vlc_t h261_cbp_table[ 64 ] = {
	{ -1, -1 },/* 0 */
	{ 11, 5 }, /* 1 */
	{  9, 5 },  /* 2 */
	{ 13, 6 }, /* 3 */
	{ 13, 4 }, /* 4 */
	{ 23, 7 }, /* 5 */
	{ 19, 7 }, /* 6 */
	{ 31, 8 }, /* 7 */
	{ 12, 4 }, /* 8 */
	{ 22, 7 }, /* 9 */
	{ 18, 7 }, /* 10 */
	{ 30, 8 }, /* 11 */
	{ 19, 5 }, /* 12 */
	{ 27, 8 }, /* 13 */
	{ 23, 8 }, /* 14 */
	{ 19, 8 }, /* 15 */
	{ 11, 4 }, /* 16 */
	{ 21, 7 }, /* 17 */
	{ 17, 7 }, /* 18 */
	{ 29, 8 }, /* 19 */
	{ 17, 5 }, /* 20 */
	{ 25, 8 }, /* 21 */
	{ 21, 8 }, /* 22 */
	{ 17, 8 }, /* 23 */
	{ 15, 6 }, /* 24 */
	{ 15, 8 }, /* 25 */
	{ 13, 8 }, /* 26 */
	{  3, 9 }, /* 27 */
	{ 15, 5 }, /* 28 */
	{ 11, 8 }, /* 29 */
	{  7, 8 }, /* 30 */
	{  7, 9 }, /* 31 */
	{ 10, 4 }, /* 32 */
	{ 20, 7 }, /* 33 */
	{ 16, 7 }, /* 34 */
	{ 28, 8 }, /* 35 */
	{ 14, 6 }, /* 36 */
	{ 14, 8 }, /* 37 */
	{ 12, 8 }, /* 38 */
	{ 2,  9 }, /* 39 */
	{ 16, 5 }, /* 40 */
	{ 24, 8 }, /* 41 */
	{ 20, 8 }, /* 42 */
	{ 16, 8 }, /* 43 */
	{ 14, 5 }, /* 44 */
	{ 10, 8 }, /* 45 */
	{  6, 8 }, /* 46 */
	{  6, 9 }, /* 47 */
	{ 18, 5 }, /* 48 */
	{ 26, 8 }, /* 49 */
	{ 22, 8 }, /* 50 */
	{ 18, 8 }, /* 51 */
	{ 13, 5 }, /* 52 */
	{  9, 8 }, /* 53 */
	{  5, 8 }, /* 54 */
	{  5, 9 }, /* 55 */
	{ 12, 5 }, /* 56 */
	{  8, 8 }, /* 57 */
	{  4, 8 }, /* 58 */
	{  4, 9 }, /* 59 */
	{  7, 3 }, /* 60 */
	{ 10, 5 }, /* 61 */
	{  8, 5 }, /* 62 */
	{ 12, 6 }  /* 63 */
};

h261_vlc_t h261_mvd_table[ 32 ] =
{
	{ 25, 11 },
	{ 27, 11 },
	{ 29, 11 },
	{ 31, 11 },
	{ 33, 11 },
	{ 35, 11 },
	{ 19, 10 },
	{ 21, 10 },
	{ 23, 10 },
	{ 7, 8 },
	{ 9, 8 },
	{ 11, 8 },
	{ 7, 7 },
	{ 3, 5 },
	{ 3, 4 },
	{ 3, 3 },
	{ 1, 1 }, /* 0 */
	{ 2, 3 },
	{ 2, 4 },
	{ 2, 5 },
	{ 6, 7 },
	{ 10, 8 },
	{ 8, 8 },
	{ 6, 8 },
	{ 22, 10 },
	{ 20, 10 },
	{ 18, 10 },
	{ 34, 11 },
	{ 32, 11 },
	{ 30, 11 },
	{ 28, 11 },
	{ 26, 11 }
};


UInt8 rgi_mvd_translation_table[ 64 ] = {
	-1, /* -32 */
	-1, /* -31 */
	18, /* -30 */
	19, /* -29 */
	20, /* -28 */
	21, /* -27 */
	22, /* -26 */
	23, /* -25 */
	24, /* -24 */
	25, /* -23 */
	26, /* -22 */
	27, /* -21 */
	28, /* -20 */
	29, /* -19 */
	30, /* -18 */
	31, /* -17 */
	0, /* -16 */
	1, /* -15 */
	2, /* -14 */
	3, /* -13 */
	4, /* -12 */
	5, /* -11 */
	6, /* -10 */
	7, /* -9 */
	8, /* -8 */
	9, /* -7 */
	10, /* -6 */
	11, /* -5 */
	12, /* -4 */
	13, /* -3 */
	14, /* -2 */
	15, /* -1 */
	16, /* 0 */
	17, /* 1 */
	18, /* 2 */
	19, /* 3 */
	20, /* 4 */
	21, /* 5 */
	22, /* 6 */
	23, /* 7 */
	24, /* 8 */
	25, /* 9 */
	26, /* 10 */
	27, /* 11 */
	28, /* 12 */
	29, /* 13 */
	30, /* 14 */
	31, /* 15 */
	0, /* 16 */
	1, /* 17 */
	2, /* 18 */
	3, /* 19 */
	4, /* 20 */
	5, /* 21 */
	6, /* 22 */
	7, /* 23 */
	8, /* 24 */
	9, /* 25 */
	10, /* 26 */
	11, /* 27 */
	12, /* 28 */
	13, /* 29 */
	14, /* 30 */
	-1, /* 31 */
};


h261_coeff_run_level_t h261_coeff_run_level_table[] = {
	{ 0, 1, { 3, 2 } },
	{ 0, 2, { 4, 4 } },
	{ 0, 3, { 5, 5 } },
	{ 0, 4, { 6, 7 } },
	{ 0, 5, { 38, 8 } },
	{ 0, 6, { 33, 8 } },
	{ 0, 7, { 10, 10 } },

	{ 0, 8, { 29, 12 } },
	{ 0, 9, { 24, 12 } },
	{ 0, 10, { 19, 12 } },
	{ 0, 11, { 16, 12 } },

	{ 0, 12, { 26, 13 } },
	{ 0, 13, { 25, 13 } },
	{ 0, 14, { 24, 13 } },
	{ 0, 15, { 23, 13 } },
	
	{ 1, 1, { 3, 3 } },
	{ 1, 2, { 6, 6 } },
	{ 1, 3, { 37, 8 } },
	{ 1, 4, { 12, 10 } },
	{ 1, 5, { 27, 12 } },
	{ 1, 6, { 22, 13 } },
	{ 1, 7, { 21, 13 } },

	{ 2, 1, { 5, 4 } },
	{ 2, 2, { 4, 7 } },
	{ 2, 3, { 11, 10 } },
	{ 2, 4, { 20, 12 } },
	{ 2, 5, { 20, 13 } },

	{ 3, 1, { 7, 5 } },
	{ 3, 2, { 36, 8 } },
	{ 3, 3, { 28, 12 } },
	{ 3, 4, { 19, 13 } },

	{ 4, 1, { 6, 5 } },
	{ 4, 2, { 15, 10 } },
	{ 4, 3, { 18, 12 } },

	{ 5, 1, { 7, 6 } },
	{ 5, 2, { 9, 10 } },
	{ 5, 3, { 18, 13 } },

	{ 6, 1, { 5, 6 } },
	{ 6, 2, { 30, 12 } },

	{ 7, 1, { 4, 6 } },
	{ 7, 2, { 21, 12 } },

	{ 8, 1, { 7, 7 } },
	{ 8, 2, { 17, 12 } },


	{ 9, 1, { 5, 7 } },
	{ 9, 2, { 17, 13 } },

	{ 10, 1, { 39, 8 } },
	{ 10, 2, { 16, 13 } },

	{ 11, 1, { 35, 8 } },
	{ 12, 1, { 34, 8 } },
	{ 13, 1, { 32, 8 } },
	{ 14, 1, { 14, 10 } },
	{ 15, 1, { 13, 10 } },
	{ 16, 1, { 8, 10 } },


	{ 17, 1, { 31, 12 } },
	{ 18, 1, { 26, 12 } },
	{ 19, 1, { 25, 12 } },
	{ 20, 1, { 23, 12 } },
	{ 21, 1, { 22, 12 } },

	{ 22, 1, { 31, 13 } },
	{ 23, 1, { 30, 13 } },
	{ 24, 1, { 29, 13 } },
	{ 25, 1, { 28, 13 } },
	{ 26, 1, { 27, 13 } },

	{ H261_COEFF_RUN_LEVEL_ESCAPE, H261_COEFF_RUN_LEVEL_ESCAPE, { 0, 0 } }
};

UInt8 rgi_h261_run_level_lut[ 27 ][ 16 ] = {
 { 63,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14 },
 { 63, 15, 16, 17, 18, 19, 20, 21, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 22, 23, 24, 25, 26, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 27, 28, 29, 30, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 31, 32, 33, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 34, 35, 36, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 37, 38, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 39, 40, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 41, 42, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 43, 44, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 45, 46, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 47, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 48, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 49, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 50, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 51, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 52, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 53, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 54, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 55, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 56, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 57, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 58, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 59, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 60, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 61, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 },
 { 63, 62, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63 }
};


UInt8 rgi_transmission_order_table[ 64 ] = {
	 0,  1,  8, 16,  9,  2,  3, 10, /* 8 */
	17, 24, 32, 25, 18, 11,  4,  5, /* 16 */
	12, 19, 26, 33, 40, 48, 41, 34, /* 24 */
	27, 20, 13,  6,  7, 14, 21, 28, /* 32 */
	35, 42, 49, 56, 57, 50, 43, 36, /* 40 */
	29, 22, 15, 23, 30, 37, 44, 51, /* 48 */
	58, 59, 52, 45, 38, 31, 39, 46, /* 56 */
	53, 60, 61, 54, 47, 55, 62, 63
};

/* ( quantizer^2 ) * 0.75 */
Int32 rgi_lambda2[ 32 ] = {
	0, 192, 768, 1728, 3072, 4800, 6912, 9408, 12288,
	15552, 19200, 23232, 27648, 32448, 37632, 43200,
	49152, 55488, 62208, 69312, 76800, 84672, 92928,
	101568, 110592, 120000, 129792, 139968, 150528,
	161472, 172800, 184512
};

/* ? */
Int32 rgi_lambda[ 32 ] = {
	0, 114, 228, 343, 457, 572, 686, 800, 915, 1029,
	1144, 1258, 1373, 1487, 1601, 1716, 1830, 1945,
	2059, 2174, 2288, 2402, 2517, 2631, 2746, 2860,
	2975, 3089, 3203, 3318, 3432, 3547
};


Int16 rgi16_mb_dim[ 2 ][ 2 ] = {
	{ 174 / 16, 144 / 16 },
	{ 352 / 16, 288 / 16 }
};
