===================
Plasma User's Guide
===================

This document describes PLASMA's development schedule.  It consists of a series
of entries, each representing a feature to be added.  A feature is considered
complete when its functionality, documentation, and corresponding regression
tests have been committed to CVS.  The list is in chronological order but this
order may change based upon external factors.

:Author: Brian Kahne 
:Contact: bkahne@freescale.com 
:revision: $Id$ 

.. contents::

Feature Template
----------------

Status:

    TBD

Description:

    TBD

Implementation:

    TBD

Dependencies:

    TBD

Regressions:

    TBD

Par Block
---------

Status:

    Completed 4/23/2004

Description:

    A par block launches each of its constituent expressions as separate
    threads.  It proceeds only when all threads are finished.

Implementation:

    1. The block will be converted such that each expression becomes its own
       subroutine.  The parameter passed in will be a structure whose members
       are pointers to any variables used by the expression.

    2. Each thread will be launched via a call to pCreate.  Then a call to
       pWait will exist for each thread.

Dependencies:

    1. Thread library.

    2. Parser is able to parse "plain" block structures.

Regressions:

    1. basic/par1.pa

    2. basic/qsort.pa

Pfor block
----------

Status:

    Completed 4/29/2004

Description:

    For-loop syntax, where body of the loop is launched as a thread.  Construct
    blocks until all threads are finished.

Implementation:

    Same as for par block, except that the argument structure is allocated on
    the thread's stack structure.  Each variable declared in loop's guard is
    passed by value, while everything else is passed by reference.

Dependencies:

    1. Thread library.

    2. Parser is able to parse "plain" block structures.
    
Regressions:

    1. basic/par2.pa

    2. basic/par3.pa

Channels (Basic Alt block)
--------------------------

Status:

    Scheduled for 5/14/2004

Description:

    Define channel interface and implement basic alt block.  The alt block is
    like a case statement, except that each condition is a channel variable and
    a variable to map the channel's return value to.  The block blocks until one
    of the channels has data.  It then reads that data, maps it to the variable,
    and executes the code associated with that guard.

    Basic syntax is::

          alt {
            with(chan1,x): ....
            with(chan2,x): ....
          }

Implementation:

    A channel will be any type that has the required interface.  This is
    compile-time polymorphism, similar to how templates work.  The required
    interfacee is:

    1.  read()

    2.  write()

    3.  ready()

    4.  clear_ready()

    5.  set_notify()

    6.  clear_notify()

Dependencies:

    1.  Need channel definition

    2.  Probably need more thread interfaces- pSleep, which will sleep and
        return an integer handle when awakened.

Regressions:

    TBD

Looping Alt Block
-----------------

TBD

Garbage Collection
------------------

TBD

Spawn Operator
--------------

TBD

Time Model
----------

TBD
