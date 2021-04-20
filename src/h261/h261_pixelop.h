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


extern Int32 ( *g_get_sad ) ( UInt8 *pui8_blk1, Int32 i_stride1, UInt8 *pui8_blk2, Int32 i_stride2 );
extern Int32 ( *g_get_ssd[ 2 ] ) ( UInt8 *pui8_blk1, Int32 i_stride1, UInt8 *pui8_blk2, Int32 i_stride2 );
extern Void ( *g_sub_8x8 ) ( Int16 *pi16_diff, UInt8 *pui8_src1, Int32 i_stride_src1, UInt8 *pui8_src2, Int32 i_stride_src2 );
extern Void ( *g_add_8x8 ) ( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_base, Int32 i_base_stride, Int16 *pi_difference );
extern Void ( *g_compensate_8x8 ) ( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_reference, Int32 i_reference_stride );
extern Void ( *g_compensate_8x8_filter ) ( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_reference, Int32 i_reference_stride );

Void h261_initialize_pixelop_functions( );