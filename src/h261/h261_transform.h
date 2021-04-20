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


extern Void ( *g_fdct_8x8 ) ( Int16 *pi16_source, Int16 *pi16_destination );
extern Void ( *g_idct_8x8 ) ( Int16 *pi16_source, Int16 *pi16_destination );

Void h261_quant8x8_intra_fw( Int32 *pi_coeffs, Int32 i_stride, Int32 i_quant );
Void h261_quant8x8_intra_bw( Int16 *pi_coeffs, Int32 i_stride, Int32 i_quant );

Int32 h261_quant8x8_inter_fw( Int16 *pi_coeffs, Int32 i_stride, Int32 i_quant );
Void h261_quant8x8_inter_bw( Int16 *pi_coeffs, Int32 i_stride, Int32 i_quant );

Int32 h261_quant8x8_trellis_fw( h261_context_t *ps_ctx, Int16 *pi_coeffs, Int32 i_quant, Int32 i_intra );

Void h261_initialize_transform_functions( );