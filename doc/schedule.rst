===========================
Plasma Development Schedule
===========================

This document describes PLASMA's development schedule.  It consists of a series
of entries, each representing a feature to be added.  A feature is considered
complete when its functionality, documentation, and corresponding regression
tests have been committed to CVS.  The list is in chronological order but this
order may change based upon external factors.

:Author: Brian Kahne 
:Contact: bkahne@freescale.com 
:revision: ``$Id$``

.. contents::

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

    Completed for 5/12/2004

Description:

    Define channel interface and implement basic alt block.  The alt block is
    like a case statement, except that each condition is a channel variable and
    a variable to map the channel's return value to.  The block blocks until one
    of the channels has data.  It then reads that data, maps it to the variable,
    and executes the code associated with that guard.

    Basic syntax is::

          alt {
            port(<value decl> ; <channel expr>): ....
            [ { default block } ]
          }

Implementation:

    A channel will be any type that has the required interface.  This is
    compile-time polymorphism, similar to how templates work.  The required
    interface is as follows.  For a channel of type T:

    1.  ``T read()``:  Returns a value read from the channel.  Blocks if no value is
        present.  Returns the last value read, until clear_ready() is called.

    2.  ``T get()``:  Returns a value from the channel.  Blocks if no value is
        present.  Always fetches a new value.  After a call to this, read() will
        return this same value.

    2.  ``void write(T)``:  Writes a value to the channel.  May block, depending
        upon the channel definition.

    3.  ``bool ready() const``:  Returns true if the channel has a value.

    4.  ``void clear_ready()``:  Clears the ready status, forcing the fetch of a new
        value.

    5.  ``set_notify(Thread *t,int handle)``:  Stores the thread and handle.  When
        the channel gets a value, it will wake this thread, giving it the handle.

    6.  ``clear_notify()``:  Clears the stored thread so that no notification will
        take place if a value is written to the channel.

    Some details about channel implementation:

    1.  Call pSleep() to block.  You must have stored a handle to the current
        thread somewhere else before this call, e.g. storing it in a channel
        member variable.

    2.  Call pWake() to awaken a thread.  The general protocol is that the waker
        clears the thread member variable of the channel and it does this
        *before* the call to pWake.

    3.  Call pAddReady() to add a thread to the ready queue, but not make it
        active.  No switching occurs (assuming processor is locked to avoid
        preemption).

    4.  A call to read() or get() should clear any notification.  Thus, with an
        alt block, only the channels that had set_notify() called need to have
        clear_notify() called if a ready channel is found.  The actual ready
        channel should not have clear_notify() called, since there could be a
        blocked writer waiting to go.

    Code conversion for the alt block will be:

    1.  Shutdown preemption.

    2.  Loop through all channels- if anything is ready, save handle and exit
        loop.  Else, call set_notify with current thread and handle (integer
        index of loop).

    3.  If nothing ready, sleep.

    4.  Case statement on return value of sleep, or index value from loop in
        (2).  Execute relevant code.

    5.  Call clear_notify on all threads.  Do this within a catch(...) block,
        too.

    6.  Alt blocks consume values, i.e. they call get().

Dependencies:

    1.  Need channel definition

    2.  Add ``int pSleep()``: Puts the thread to sleep.  Returns integer when thread
        wakes.

    3.  Add ``void pWake(Thread *t,int h)``:  Wakes thread, giving it h.

Regressions:

    1.  basic/chan1.pa

    2.  basic/chan2.pa

    3.  basic/chan3.pa

Looping Alt Block
-----------------

Status:

    Completed 5/12/2004

Description:

    Same as alt block, but allows the user to loop over a data structure.
    Syntax is::

      afor ( <s1> ; <s2> ; <s3> ) {
        port (<value decl> ; <channel expr> ; ) { <body> }
        [ { <default block> } ]
      }

    Only one port statement is allowed.  An iterator variable must be declared
    in <s1>.

Implementation:

    Same as for alt, except that we replicate the loop condition as a for-loop
    each time we deal with channnels.  If the iterator is not an integer, we
    create an auxiliary vector and store the values there.  We then store the
    corresponding index of the entry as the handle in each channel.

Dependencies:

    Completion of alt.

Regressions:

    1.  basic/chan4.pa

    2.  basic/chan5.pa

    3.  basic/chan6.pa

    4.  basic/chan7.pa

Spawn Operator
--------------

Status:

    Completed 5/18/2004

Description:

    Thread creation w/o synchronization, e.g.::

      spawn { foo(1,2,3); };

    Evaluates the argument (must resolve to a function or an object's member
    invocation).  The argument is launched as a thread.  The return value is an
    object which meets the specifications of a channel.  It will also have
    additional operators for thread control:

    1. wait():  Wait for thread to finish.

    2. kill():  Kill thread.

    The object will be a special type of channel, so you can use it in an alt
    block and attempts to fetch the value before the thread is finished will
    result in a block.  Unlike other channels, it will only ever have a single
    value, so calls to clear_ready() will be ignored.

    Spawn should handle all ways to invoke a function:

    1. Literal function call:       spawn(foo());

    2. Function pointer call:       p = foo; spawn(p());

    3. Method call w/reference:     spawn(a.b());

    4. Method call w/pointer:       spawn(a->b());

    5. Static method call:          spawn(A::b());

    6. Method pointer w/reference:  p = &A::b; spawn(a.*p());

    7. Method pointer w/pointer:    p = &A::b; spawn(a->*p());

Implementation:

    * Registered as a function call of a special dummy class.

    * Void functions not handled- everything returns a value.

Regressions:

    1. spawn1

    2. spawn2

    3. spawn3

    4. spawn4

Shared Data Structures
-----------------------

Status:

    TBD

Description:

    Shared data structures will allow serialized access to data, i.e. mutexes
    will wrap the actual data access, ensuring safe use between threads.  The
    most likely syntax will be a class attribute, e.g. pMutex class ... The
    public methods will then be wrapped with mutex access code.  A per-method
    modifier will allow this to be disabled (will implement only if easy to do
    with OpenC++).

Implementation:

    Should be a straight-forward use of OpenC++'s MOP.

Dependencies:

    TBD

Regressions:

    TBD

Thread Priorities
-----------------

Status:

    TBD

Decription:

    A thread will be able to change its priority using a function
    (pSetPriority(int)).  The lowest level of priority will be timesliced.
    Otherwise, all threads of the highest priority (0) will run to completion
    before any others.

    API:

    1. ``pSetPriority(int)``:  Set current thread's priority.  Spawned threads will
       run at their parents priority.

    2. ``pNumPriorities()``:  Return number of allowed priorities n, where
       priorities are (0..n-1).

    2. New config parameter in pSetup to set number of priorities.  Default is
       32.

Implementation:
    
    Array of thread queeues.  Scheduler will run high priority threads first.
    Timeslicing will only be turned on when running the lowest-priority threads.

Time Model
----------

Refer to twiki page for now.

Garbage Collection
------------------

Status:

    TBD

Description:

    Plasma is going to have a lot of producer/consumer type code, where the
    ownership of a particular piece of memory will be hard to track.  Garbage
    collection will make the code much easier to understand and less error-prone.

Implementation:

    Boehm garbage collector.

Dependencies:

    The main issue is getting it to handle user-threads.  It handles kernel
    threads and should be able to handle user-threads, but I don't know how to
    do it yet.

Regressions:

    TBD

Kernel Threads
--------------

Status:

    TBD

Description:

    Expand underlying RTOS to an M:N model, i.e. M kernel threads, each running
    N user threads.  Add a placement specifier to par so that threads may be
    dispatched to different kernel threads.  These kernel threads will be
    identified using a pCluster object.

Implementation:

    1.  Expand RTOS to handle kernel threads.  Probably use LinuxThreads.  The
        RTOS code will need mutexes around critical areas.

    2.  Create pCluster object.  Add code to spawn new kernel threads.

    3.  Expand par blocks to add placement specifier, e.g.::

        par {
          on (cluster1) { ... }
          on (cluster2) { ... }
        }

        The ``on (<cluster name>)`` block specifies a target cluster.  The
        brace-delimited code is launched as the thread.

    4.  Retrofit shared data structures with mutexes.

Dependencies:

    1.  Garbage collector needs to work with the kernel threadss.  This
        shouldn't be a problem, as the Boehm collector currently supports
        LinuxThreads.

Regressions:

    TBD

