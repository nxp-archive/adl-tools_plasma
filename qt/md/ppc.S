/*
    
QuickThreads -- Threads-building toolkit.
Copyright (c) 1993 by David Keppel

Permission to use, copy, modify and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice and this notice
appear in all copies.  This software is provided as a
proof-of-concept and for demonstration purposes; there is no
representation about the suitability of this software for any
purpose.

This code is derived from OpenMCL:
        
   Copyright (C) 1994-2001 Digitool, Inc
   This file is part of OpenMCL.  

   OpenMCL is licensed under the terms of the Lisp Lesser GNU Public
   License , known as the LLGPL and distributed with OpenMCL as the
   file "LICENSE".  The LLGPL consists of a preamble and the LGPL,
   which is distributed with OpenMCL as the file "LGPL".  Where these
   conflict, the preamble takes precedence.  

   OpenMCL is referenced in the preamble as the "LIBRARY."

   The LLGPL is also available online at
   http://opensource.franz.com/preamble.html    

*/

/* Frame needs room for backlink, lr, 18 GPRs, FPSCR, 18 FPRs, 12 VRs, the
	VSCR, and VRSAVE, and must be aligned on a 16-byte boundary */
	 	
/*
	In general, a frame looks like:
+---------+
| backptr |	back link to previous frame
+---------+
| save lr |	slot for CALLEE to save lr in
+---------+
| save r14|	user function param 'pu'
+---------+
| save r15|	user function param 'pt'
+---------+
| save r16|	user function 'userf'
+---------+
| save r17|	'only'
+---------+
    ...
+---------+
| save r31|
+---------+
| fpscr   |	64 bits
|	  |
+---------+
| save f14|
|	  |
+---------+
   ...
+---------+
| save f31|
|	  |
+---------+    
|  pad    |      64 bits
|         |
+---------+
| save v20|	128 bits
|         |
+---------+
   ...
+---------+
| save v31|
|         |
+---------+
| vrtemp  |  xxxx, xxxx, vrsave, vscr
|         |
+---------+
		When 'qt_start' runs, there should be a backlink
		and save_lr slot underneath the top frame.
*/

#define sp r1
#define QT_BACKLINK 0
#define QT_SAVElr 4
#define QT_first_nvr r14
#define QT_SAVEr14 8
#define QT_SAVEnvrs QT_SAVEr14
#define QT_SAVEfpscr (QT_SAVEr14+((32-14)*4))
#define QT_SAVEf14 (QT_SAVEfpscr+8)
#define QT_SAVEpad (QT_SAVEf14+((32-14)*8))
#define QT_SAVEv20 (QT_SAVEpad+8)
#define QT_SAVEvtemp (QT_SAVEv20+((32-20)*16))
#define QT_SAVEvrsave (QT_SAVEvtemp+8) 
#define QT_FRAMESIZE (QT_SAVEvtemp+16)
    
    .text

    .globl qt_abort
    .globl qt_block
    .globl qt_blocki

qt_abort:   
qt_blocki:      
qt_block:   
    andi. r0,sp,8
    mflr r0
    stw r0,(QT_SAVElr)(sp)
    bne 1f
    stwu sp,-(QT_FRAMESIZE)(sp)
    b 2f
1:  
    stwu sp,-(QT_FRAMESIZE+8)(sp)
2:
    /* Save GPRs */
    stmw QT_first_nvr,(QT_SAVEnvrs)(sp)
    /* Save FPRs */
    mffs f0
    stfd f0,(QT_SAVEfpscr)(sp)
    stfd f14,QT_SAVEf14+(8*0)(sp)
    stfd f15,QT_SAVEf14+(8*1)(sp)
    stfd f16,QT_SAVEf14+(8*2)(sp)
    stfd f17,QT_SAVEf14+(8*3)(sp)
    stfd f18,QT_SAVEf14+(8*4)(sp)
    stfd f19,QT_SAVEf14+(8*5)(sp)
    stfd f20,QT_SAVEf14+(8*6)(sp)
    stfd f21,QT_SAVEf14+(8*7)(sp)
    stfd f22,QT_SAVEf14+(8*8)(sp)
    stfd f23,QT_SAVEf14+(8*9)(sp)
    stfd f24,QT_SAVEf14+(8*10)(sp)
    stfd f25,QT_SAVEf14+(8*11)(sp)
    stfd f26,QT_SAVEf14+(8*12)(sp)
    stfd f27,QT_SAVEf14+(8*13)(sp)
    stfd f28,QT_SAVEf14+(8*14)(sp)
    stfd f29,QT_SAVEf14+(8*15)(sp)
    stfd f30,QT_SAVEf14+(8*16)(sp)
    stfd f31,QT_SAVEf14+(8*17)(sp)

    /* Save VRs */
    li r14,QT_SAVEv20
    stvx v20,r14,sp
    li r14,QT_SAVEv20+(16*1)
    stvx v21,r14,sp
    li r14,QT_SAVEv20+(16*2)
    stvx v22,r14,sp
    li r14,QT_SAVEv20+(16*3)
    stvx v23,r14,sp
    li r14,QT_SAVEv20+(16*4)
    stvx v24,r14,sp
    li r14,QT_SAVEv20+(16*5)
    stvx v25,r14,sp
    li r14,QT_SAVEv20+(16*6)
    stvx v26,r14,sp
    li r14,QT_SAVEv20+(16*7)
    stvx v27,r14,sp            
    li r14,QT_SAVEv20+(16*8)
    stvx v28,r14,sp
    li r14,QT_SAVEv20+(16*9)
    stvx v29,r14,sp
    li r14,QT_SAVEv20+(16*10)
    stvx v30,r14,sp
    li r14,QT_SAVEv20+(16*11)
    stvx v31,r14,sp
    mfvscr v31
    li r14,QT_SAVEvtemp
    stvx v31,r14,sp
    mfspr r14,256  /* VRSAVE */
    stw r14,(QT_SAVEvrsave)(sp)
    
    /* Call helper. */
    mtctr r3
    mr r3,sp
    mr sp,r6
    bctrl

    /* Load FPRs */
    lfd f0,(QT_SAVEfpscr)(sp)
    mtfsf 0xff,f0
    lfd f14,QT_SAVEf14+(8*0)(sp)
    lfd f15,QT_SAVEf14+(8*1)(sp)
    lfd f16,QT_SAVEf14+(8*2)(sp)
    lfd f17,QT_SAVEf14+(8*3)(sp)
    lfd f18,QT_SAVEf14+(8*4)(sp)
    lfd f19,QT_SAVEf14+(8*5)(sp)
    lfd f20,QT_SAVEf14+(8*6)(sp)
    lfd f21,QT_SAVEf14+(8*7)(sp)
    lfd f22,QT_SAVEf14+(8*8)(sp)
    lfd f23,QT_SAVEf14+(8*9)(sp)
    lfd f24,QT_SAVEf14+(8*10)(sp)
    lfd f25,QT_SAVEf14+(8*11)(sp)
    lfd f26,QT_SAVEf14+(8*12)(sp)
    lfd f27,QT_SAVEf14+(8*13)(sp)
    lfd f28,QT_SAVEf14+(8*14)(sp)
    lfd f29,QT_SAVEf14+(8*15)(sp)
    lfd f30,QT_SAVEf14+(8*16)(sp)
    lfd f31,QT_SAVEf14+(8*17)(sp)

    /* Load VRs */
    li r14,QT_SAVEv20+(16*0)
    lvx v20,r14,sp
    li r14,QT_SAVEv20+(16*1)
    lvx v21,r14,sp
    li r14,QT_SAVEv20+(16*2)
    lvx v22,r14,sp
    li r14,QT_SAVEv20+(16*3)
    lvx v23,r14,sp
    li r14,QT_SAVEv20+(16*4)
    lvx v24,r14,sp
    li r14,QT_SAVEv20+(16*5)
    lvx v25,r14,sp
    li r14,QT_SAVEv20+(16*6)
    lvx v26,r14,sp
    li r14,QT_SAVEv20+(16*7)
    lvx v27,r14,sp
    li r14,QT_SAVEv20+(16*8)
    lvx v28,r14,sp
    li r14,QT_SAVEv20+(16*9)
    lvx v29,r14,sp
    li r14,QT_SAVEv20+(16*10)
    lvx v30,r14,sp
    li r14,QT_SAVEv20+(16*11)
    lvx v31,r14,sp
    li r14,QT_SAVEvtemp
    lvx v0,r14,sp
    mtvscr v0
    lwz r14,(QT_SAVEvrsave)(sp)
    mtspr 256,r14
    
    /* Load GPRs */
    lmw QT_first_nvr,(QT_SAVEnvrs)(sp)

    /* Restore caller and return. */
    lwz sp,(QT_BACKLINK)(sp)
    lwz r0,(QT_SAVElr)(sp)
    mtlr r0
    blr

    .globl qt_start
    .type qt_start,@function
    
qt_start:
    .globl qt_error
    mtctr r17		/* Ready to call 'only' */
    mr r3,r14		/* Load up user function pu. */
    mr r4,r15		/* ... user function pt */
    mr r5,r16		/* ... user function userf */
    bctrl		/* Call only(pu,pt,userf) */
    b qt_error		/* should never get here */

qt_vargs:
    .globl qt_vargs
    b qt_error          /* variable arguments not yet implemented. */