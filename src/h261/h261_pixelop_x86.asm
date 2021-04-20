%if 0
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
%endif

%include "x86inc.asm"

SECTION_RODATA

ALIGN 16
M128_FILTER_HOR_MASK_INNER : dw 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x0000
ALIGN 16
M128_FILTER_HOR_MASK_OUTER : dw 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff
ALIGN 16
M128_FILTER_TWO			   : dw 2, 2, 2, 2, 2, 2, 2, 2
ALIGN 16
M128_FILTER_EIGHT		   : dw 8, 8, 8, 8, 8, 8, 8, 8

SECTION .text

INIT_XMM



;-----------------------------------------------------------------------------
;   Int32 __cdecl h261_get_sad_sse2( UInt8 *pui8_blk1, Int32 i_stride1, UInt8 *pui8_blk2, Int32 i_stride2 )
;-----------------------------------------------------------------------------

cglobal h261_get_sad_sse2, 4, 4, 8
    movdqu m6, [r0]
    movdqu m7, [r0+r1]
    lea    r0,  [r0+2*r1]
    movdqu m5, [r0]
    movdqu m4, [r0+r1]
    lea    r0,  [r0+2*r1]
    psadbw m6, [r2]
    psadbw m7, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m0, [r0]
    paddw  m6, m7
    psadbw m5, [r2]
    psadbw m4, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m2, [r0+r1]
    lea    r0,  [r0+2*r1]
    paddw  m5, m4
    movdqu m3, [r0]
    movdqu m1, [r0+r1]
    lea    r0,  [r0+2*r1]
    paddw  m6, m5
    psadbw m0, [r2]
    psadbw m2, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m7, [r0]
    paddw  m0, m2
    psadbw m3, [r2]
    psadbw m1, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m5, [r0+r1]
    lea    r0,  [r0+2*r1]
    paddw  m3, m1
    movdqu m4, [r0]
    paddw  m6, m0
    movdqu m0, [r0+r1]
    lea    r0,  [r0+2*r1]
    paddw  m6, m3
    psadbw m7, [r2]
    psadbw m5, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m2, [r0]
    paddw  m7, m5
    psadbw m4, [r2]
    psadbw m0, [r2+r3]
    lea    r2,  [r2+2*r3]
    movdqu m3, [r0+r1]
    lea    r0,  [r0+2*r1]
    paddw  m4, m0
    movdqu m1, [r0]
    paddw  m6, m7
    movdqu m7, [r0+r1]
    paddw  m6, m4
    psadbw m2, [r2]
    psadbw m3, [r2+r3]
    lea    r2,  [r2+2*r3]
    paddw  m2, m3
    psadbw m1, [r2]
    psadbw m7, [r2+r3]
    paddw  m1, m7
    paddw  m6, m2
    paddw  m6, m1
    
    movdqa  m7, m6
    psrldq  m6,  8
    paddw   m6, m7
    movd    eax,  m6

    RET


;-----------------------------------------------------------------------------
; Int32 h261_ssd_8x8_sse2( UInt8 *pui8_blk1, Int32 i_stride1, UInt8 *pui8_blk2, Int32 i_stride2 );
;-----------------------------------------------------------------------------

cglobal h261_ssd_8x8_sse2, 4, 4, 8
    pxor		m7, m7
    pxor		m6, m6
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	

    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

	pshufd		m7, m6, 11100101b
	pshufd		m5, m6, 11100110b
	pshufd		m4, m6, 11100111b
	paddd		m7, m6
	paddd		m7, m5
	paddd		m7, m4
	
	movd		eax, m7    
	RET


;-----------------------------------------------------------------------------
;   Int32 __cdecl h261_ssd_16x16_sse2( UInt8 *pui8_blk1, Int32 i_stride1, UInt8 *pui8_blk2, Int32 i_stride2 )
;-----------------------------------------------------------------------------

cglobal h261_ssd_16x16_sse2, 4, 4, 8
    pxor		m7, m7
    pxor		m6, m6

; 0-1
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	
    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	

; 2-3
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

; 4-5
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

; 6-7
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	
    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	
; 8-9
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]

    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	
    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

; 10-11
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

; 12-13
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]

    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

; 14-15
    lea			r0, [ r0 + r1 * 2 ]
    lea			r2, [ r2 + r3 * 2 ]
    
    movq		m0, [ r0 ]
    movq		m1, [ r0 + r1 ]
    movq		m2, [ r2 ]
    movq		m3, [ r2 + r3 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1
	
    movq		m0, [ r0 + 8 ]
    movq		m1, [ r0 + r1 + 8 ]
    movq		m2, [ r2 + 8 ]
    movq		m3, [ r2 + r3 + 8 ]
    punpcklbw	m0, m7
    punpcklbw	m1, m7
    punpcklbw	m2, m7
    punpcklbw	m3, m7

    psubsw		m0, m2
    psubsw		m1, m3
	pmaddwd		m0, m0
	pmaddwd		m1, m1

	paddd		m6, m0
	paddd		m6, m1

	pshufd		m7, m6, 11100101b
	pshufd		m5, m6, 11100110b
	pshufd		m4, m6, 11100111b
	paddd		m7, m6
	paddd		m7, m5
	paddd		m7, m4
	
	movd		eax, m7    
	RET



;-----------------------------------------------------------------------------------------------------------------
;Void __cdecl h261_sub_8x8_sse2( Int16 *pi16_diff, UInt8 *pui8_src1, Int32 i_stride_src1, UInt8 *pui8_src2, Int32 i_stride_src2 );
;-----------------------------------------------------------------------------------------------------------------
cglobal h261_sub_8x8_sse2, 5, 5, 5

%ifdef WIN64
	movsxd		r4, r4d
%endif

	pxor		m4, m4

	movq		m0, [ r1 ]
	movq		m1, [ r3 ]
	movq		m2, [ r1 + r2 ]
	movq		m3, [ r3 + r4 ]
	
	punpcklbw	m0, m4
	punpcklbw	m1, m4
	punpcklbw	m2, m4
	punpcklbw	m3, m4

	lea			r1, [ r1 + r2 * 2 ]
	lea			r3, [ r3 + r4 * 2 ]

	psubsw		m0, m1
	psubsw		m2, m3

	movdqu		[ r0 ], m0
	movdqu		[ r0 + 16 ], m2
	add			r0, 32
	
	movq		m0, [ r1 ]
	movq		m1, [ r3 ]
	movq		m2, [ r1 + r2 ]
	movq		m3, [ r3 + r4 ]
	
	punpcklbw	m0, m4
	punpcklbw	m1, m4
	punpcklbw	m2, m4
	punpcklbw	m3, m4

	lea			r1, [ r1 + r2 * 2 ]
	lea			r3, [ r3 + r4 * 2 ]

	psubsw		m0, m1
	psubsw		m2, m3

	movdqu		[ r0 ], m0
	movdqu		[ r0 + 16 ], m2
	add			r0, 32
	
	movq		m0, [ r1 ]
	movq		m1, [ r3 ]
	movq		m2, [ r1 + r2 ]
	movq		m3, [ r3 + r4 ]
	
	punpcklbw	m0, m4
	punpcklbw	m1, m4
	punpcklbw	m2, m4
	punpcklbw	m3, m4

	lea			r1, [ r1 + r2 * 2 ]
	lea			r3, [ r3 + r4 * 2 ]

	psubsw		m0, m1
	psubsw		m2, m3

	movdqu		[ r0 ], m0
	movdqu		[ r0 + 16 ], m2
	add			r0, 32
	
	movq		m0, [ r1 ]
	movq		m1, [ r3 ]
	movq		m2, [ r1 + r2 ]
	movq		m3, [ r3 + r4 ]
	
	punpcklbw	m0, m4
	punpcklbw	m1, m4
	punpcklbw	m2, m4
	punpcklbw	m3, m4

	lea			r1, [ r1 + r2 * 2 ]
	lea			r3, [ r3 + r4 * 2 ]

	psubsw		m0, m1
	psubsw		m2, m3

	movdqu		[ r0 ], m0
	movdqu		[ r0 + 16 ], m2

	RET

;-----------------------------------------------------------------------------------------------------------------
;Void __cdecl h261_add_8x8_sse2( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_base, Int32 i_base_stride, Int16 *pi_difference );
;-----------------------------------------------------------------------------------------------------------------
INIT_XMMS

cglobal h261_add_8x8_sse2, 5, 5, 5

	pxor		m4, m4

%rep 4
	movq		m0, [ r2 ]
	movq		m2, [ r2 + r3 ]
	movdqu		m1, [ r4 ]
	movdqu		m3, [ r4 + 16 ]
	
	punpcklbw	m0, m4
	punpcklbw	m2, m4

	paddsw		m0, m1
	paddsw		m2, m3
	
	packuswb	m0, m0
	packuswb	m2, m2
	
	movq		[ r0 ], m0
	movq		[ r0 + r1 ], m2

	lea			r0, [ r0 + r1 * 2 ]
	lea			r2, [ r2 + r3 * 2 ]
	add			r4, 32
%endrep

	RET

;
;Void h261_compensate_8x8_mmx( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_reference, Int32 i_reference_stride )
;
INIT_MMX
cglobal h261_compensate_8x8_mmx, 4, 4, 0
	movq       m0, [ r2 ]
	movq       m1, [ r2 + r3 ]
	lea		   r2, [ r2 + r3 * 2 ]
	movq       m2, [ r2 ]
	movq       m3, [ r2 + r3 ]
	lea		   r2, [ r2 + r3 * 2 ]
	movq       m4, [ r2 ]
	movq       m5, [ r2 + r3 ]
	lea		   r2, [ r2 + r3 * 2 ]
	movq       m6, [ r2 ]
	movq       m7, [ r2 + r3 ]
	
	movq	   [ r0 ], m0
	movq	   [ r0 + r1 ], m1
	lea		   r0, [ r0 + r1 * 2 ]
	movq	   [ r0 ], m2
	movq	   [ r0 + r1 ], m3
	lea		   r0, [ r0 + r1 * 2 ]
	movq	   [ r0 ], m4
	movq	   [ r0 + r1 ], m5
	lea		   r0, [ r0 + r1 * 2 ]
	movq	   [ r0 ], m6
	movq	   [ r0 + r1 ], m7
	
	emms
	
	RET
	

;
; Void h261_compensate_8x8_filter_sse2( UInt8 *pui8_destination, Int32 i_destination_stride, UInt8 *pui8_reference, Int32 i_reference_stride )
;	
INIT_XMM

%macro FILTER_HOR 3  ; src, dest, zero
	movq		xmm0, qword %1
	punpcklbw	xmm0, %3
	movdqa		xmm1, xmm0
	movdqa		xmm2, xmm0
	movdqa		xmm3, xmm0
	psllw		xmm0, 1
	pslldq		xmm1, 2
	psrldq		xmm2, 2
	psllw		xmm3, 2
	paddw		xmm0, xmm1		
	paddw		xmm0, xmm2
	pand		xmm0, [ M128_FILTER_HOR_MASK_INNER  ]
	pand		xmm3, [ M128_FILTER_HOR_MASK_OUTER  ]
	por			xmm0, xmm3
	movdqu		%2, xmm0
%endmacro

%macro FILTER_VERT 5 ; a b c dst temp
	movdqa		%5, %2
	psllw		%5, 1
	paddw		%5, [ M128_FILTER_EIGHT  ]
	paddw		%5, %1
	paddw		%5, %3
	psraw		%5, 4
	packuswb	%5, %5
	movq		%4, %5
%endmacro

; fixme: optimize
cglobal h261_compensate_8x8_filter_sse2, 4, 5
	sub			rsp, 128
	
	pxor		xmm4, xmm4

	FILTER_HOR  [ r2 ], [ rsp ], xmm4
	FILTER_HOR  [ r2 + r3 ], [ rsp + 16 ], xmm4
	lea			r2, [ r2 + r3 * 2 ]
	FILTER_HOR  [ r2 ], [ rsp + 32 ], xmm4
	FILTER_HOR  [ r2 + r3 ], [ rsp + 48 ], xmm4
	lea			r2, [ r2 + r3 * 2 ]
	FILTER_HOR  [ r2 ], [ rsp + 64 ], xmm4
	FILTER_HOR  [ r2 + r3 ], [ rsp + 80 ], xmm4
	lea			r2, [ r2 + r3 * 2 ]
	FILTER_HOR  [ r2 ], [ rsp + 96 ], xmm4
	FILTER_HOR  [ r2 + r3 ], [ rsp + 112 ], xmm4
	
	movdqu		xmm0, [ rsp ]
	movdqu		xmm1, [ rsp + 16 ]
	movdqu		xmm2, [ rsp + 32 ]
	
	movdqa		xmm4, xmm0
	paddw		xmm4, [ M128_FILTER_TWO  ]
	psraw		xmm4, 2
	packuswb	xmm4, xmm4
	movq		[ r0 ], xmm4

	FILTER_VERT xmm0, xmm1, xmm2, [ r0 + r1 ], xmm4
	movdqu		xmm0, [ rsp + 48 ]
	lea			r0, [ r0 + r1 * 2 ]
	FILTER_VERT xmm1, xmm2, xmm0, [ r0 ], xmm4
	movdqu		xmm1, [ rsp + 64 ]
	FILTER_VERT xmm2, xmm0, xmm1, [ r0 + r1 ], xmm4
	movdqu		xmm2, [ rsp + 80 ]
	lea			r0, [ r0 + r1 * 2 ]
	FILTER_VERT xmm0, xmm1, xmm2, [ r0 ], xmm4
	movdqu		xmm0, [ rsp + 96 ]
	FILTER_VERT xmm1, xmm2, xmm0, [ r0 + r1 ], xmm4
	movdqu		xmm1, [ rsp + 112 ]
	lea			r0, [ r0 + r1 * 2 ]
	FILTER_VERT xmm2, xmm0, xmm1, [ r0 ], xmm4

	paddw		xmm1, [ M128_FILTER_TWO  ]
	psraw		xmm1, 2
	packuswb	xmm1, xmm1
	movq		[ r0 + r1 ], xmm1

	add			rsp, 128
	RET
	