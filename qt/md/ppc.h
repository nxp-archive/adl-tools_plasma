/*

This code is derived from David Keppel's QuickThreads --
Threads-building toolkit. 

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

#ifndef QT_PPC_H
#define QT_PPC_H

#undef EABI
#undef POWEROPENABI

/* Hack:  For now, just support LINUX
   #if defined(LINUX) || defined(VXWORKS) */
#define EABI
/* #endif */

/* #if defined(DARWIN)
#define POWEROPENABI
#endif */

#ifdef EABI
  /* PPC EABI stacks must be doubleword aligned */
#define QT_STKALIGN 8
#endif

#ifdef POWEROPENABI
#define QT_STKALIGN 16
#endif

  /* and grow towards arithmetically lower addresses */
#define QT_GROW_DOWN

#define QT_R1 0
#ifdef EABI
#define QT_LR 1
#define QT_R14 2
#endif
#ifdef POWEROPENABI
#define QT_LR 2
#define QT_R13 17
#define QT_R14 (QT_R13+1)
#endif
#define QT_R15 (QT_R14+1)
#define QT_R16 (QT_R15+1)
#define QT_R17 (QT_R16+1)
#define QT_R31 (QT_R17+14)
#define QT_FPSCR_hi (QT_R31+1)  /* unused 32 bits */
#define QT_FPSCR (QT_FPSCR_hi+1)
#define QT_F14 (QT_FPSCR_hi+2)
#define QT_PAD (QT_F14+(18*2))
#define QT_V20 (QT_PAD+2)
#define QT_VTEMP (QT_V20+(12*4))
#define QT_NEXTFRAME QT_VTEMP+4
#define QT_NEXTLR (QT_NEXTFRAME+QT_LR)

#ifdef EABI
#define QT_STKBASE ((QT_NEXTLR+1)*4)
#define QT_VSTKBASE QT_STKBASE
#endif
#ifdef POWEROPENABI
#define QT_STKBASE ((QT_NEXTFRAME+18)*4)
#endif

#define QT_ONLY_INDEX QT_R17
#define QT_ARGU_INDEX QT_R14
#define QT_ARGT_INDEX QT_R15
#define QT_USER_INDEX QT_R16

typedef unsigned long qt_word_t;

/* Variable-length arguments are not yet implemented. */
extern void *qt_vargs (void *sp, int nbytes, void *vargs,
                       void *pt, void *startup,
                       void *vuserf, void *cleanup);

#define QT_VARGS(sp, nbytes, vargs, pt, startup, vuserf, cleanup) \
      (qt_vargs (sp, nbytes, vargs, pt, startup, vuserf, cleanup))

extern void qt_start(void);

/* Internal helper for putting stuff on stack. */
#define QT_SPUT(top, at, val)   \
    (((qt_word_t *)(top))[(at)] = (qt_word_t)(val))

#define QT_SET_NEXTLR(top, val) \
     QT_SPUT(top, QT_NEXTLR, ((qt_word_t)val))

#define QT_SET_NEXTSP(top) \
     QT_SPUT(top, QT_NEXTFRAME, ((char *)(top))+QT_STKBASE)

#define QT_SET_FPSCR(top, val) \
     QT_SPUT(top, QT_FPSCR, ((qt_word_t)val))

#define QT_SETSP(top) \
     QT_SPUT(top, QT_R1,((char *)(((qt_word_t *)top) + QT_NEXTFRAME)))

#define QT_ARGS_MD(top) \
     QT_SET_NEXTSP(top), \
     QT_SETSP(top),\
     QT_SET_NEXTLR(top, qt_start),\
     QT_SET_FPSCR(top, initial_fpscr)

#define initial_fpscr 0

#endif /* ifndef QT_PPC_H */
