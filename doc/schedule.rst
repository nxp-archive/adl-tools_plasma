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
            <channel expr> [ . | -> ] port(<value decl>): ....
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

    chan1 - chan9.

Looping Alt Block
-----------------

Status:

    Completed 5/12/2004

Description:

    Same as alt block, but allows the user to loop over a data structure.
    Syntax is::

      afor ( <s1> ; <s2> ; <s3> ) {
        <channel expr> [ . | -> ] port (<value decl>) { <body> }
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

    Completed 5/20/2004

Description:

    Shared data structures will allow serialized access to data, i.e. mutexes
    will wrap the actual data access, ensuring safe use between threads.  The
    most likely syntax will be a class attribute, e.g. pMutex class ... The
    public methods will then be wrapped with mutex access code.  A per-method
    modifier will allow this to be disabled (will implement only if easy to do
    with OpenC++).

Implementation:

    Straightforward use of OpenC++'s example "WrapperClass".

    Variadic function support is not perfect but can be made to work.  You can't
    write a true variadic function, e.g. ``foo(const char *fmt,..)``, because
    you can't pass the variable argument list.  Instead, you must write a
    va_list function directly, e.g. ``foo(const char *fmt,va_list ap)``.  Plasma
    will then create a variadic version and a v_list version for you that are
    wrapped with locking code.
    
Regressions:

    1. mutex1

Thread Priorities
-----------------

Status:

    Completed 6/4/2004.

Decription:

    A thread will be able to change its priority using a function
    (pSetPriority(int)).  The lowest level of priority will be timesliced.
    Otherwise, all threads of the highest priority (0) will run to completion
    before any others.

    API:

    1. ``pSetPriority(int)``:  Set current thread's priority.  Spawned threads will
       run at their parents priority.

    2. ``pGetPriorities()``:  Return current thread's priority.

    3. ``pLowestPriority()``:  Lowest priority (timeslice queue).

    4. New config parameter, ``_priority_count`` in pSetup to set number of priorities.  Default is
       32.

    5. Optional second argument to spawn of a priority, e.g. ``spawn(foo(),0);``

    6. Optional second argument to on block of a priority, e.g. ``on(p1,0) { ... }``

Implementation:
    
    Array of thread queues.  Scheduler will run high priority threads first.
    Timeslicing will only be turned on when running the lowest-priority threads.

    To the user, 0 is the highest priority, but internally 0 represents the
    lowest value and thus what we timeslice on.  

    The scheduler calls ``get_ready()``, which returns the next thread to run,
    respecting priorities.  

    The ``preempt()`` function calls ``Processor::ts_okay()``, which returns
    false if we're in the kernel or we're in a non-timesliceable thread.

Regressions:

    * pri1 - pri4.

Support For Multiple Processors
-------------------------------

Status:  

    Completed 6/4/2004.

Description:

    Users will be able to instantiate a **Processor** object.  A spawn
    pseudo-method will allow them to launch a thread on that processor.  Using
    an on-block, e.g.::

      par {
        on(<processor> [,<priority>]) { ... }
      }

    will allow for a similar feature using **par** blocks.  Support for **pfor**
    will also be included.

Implementation:

    * Rename **Processor** to **Cluster**.

    * A **Processor** object will be a handle around **Cluster**.

    * A global variable will contain a pointer to the current **Cluster**.  Most of
      the interface functions will use that value, except for some that take a
      cluster.  A new interface function will return a **Cluster** object
      pointing to the current cluster.

    * The **System** object will have a queue of clusters.  Each cluster will
      make one pass through its threads, then pass to the next cluster.

    * Add spawn pseudo method and add support for optional second parameter
      setting priority.

Regressions:

    * proc1 - 3.

Time Model
----------

Status:

    Completed 6/15/2004.

Description:

    For more information, refer to the twiki page.  In short, users may call
    **pDelay(<n>)** to delay for **n** time units or call **pBusy(<n>)** to
    consume **n** time cycles.  When a processor is busy, it does no other work,
    whereas a delay means that a process is just waiting.

Implementation:

    Refer to twiki page for the basic flow.  In short, time is maintained within
    System.  Two priority queues (stl priority queues) exist:  One for delayed
    objects and one for busy objects.  If an object called pDelay, it's added to
    the delay queue and if an object called pBusy, it's added to the busy
    queue.  Note that to use pBusy, you must set ConfigParms::_busyOkay or else
    pBusy will not be allowed.  This disables preemption- the only task
    switching will be done when calls to pDelay or pBusy are made.

    Time model functions:

    * pBusy():  Consumes time.

    * pDelay():  Delays a thread.

    * pTime():  Returns current time.

    The delay queue stores Thread objects, ordered by decreasing time (smallest
    time is at the front).  The time is the sum of the starting time and the
    delay size (both stored in the Thread).  

    The busy queue stores processors, also ordered by decreasing time.  The time
    is the busy thread's start time + busy time.  The busy thread is identified
    by finding the highest priority non-empty queue, then looking at the back.
    This is the case b/c the busy thread is added back to its respective
    priority queue by the pBusy routine.

    At a given point in time, we cycle through all processors.  For each
    processor, we execute all available jobs.  When no more processors exist
    with jobs to run, we call System::update_time().  It looks at both queues
    and chooses a new time that is the smallest of the next items on the two
    queues.  This becomes the new time.  We then transfer all delayed threads
    which have the same time as current back to their owning processors and add
    those processors back to the ready queue.  Duplication is handled by having
    Cluster::add_proc() only add a processor if its state is not "Running".  We
    then add back all busy processors whose time has expired.  Then we continue.

    If a delayed thread is ready to run, but its processor is busy, we interrupt
    the busy if the thread has higher priority than the busying thread.  We
    record how much busy time has been consumed and re-enqueue the processor.
    For the lowest priority threads, they are considered to be timesliced.  A
    configuration parameter, ConfigParms::_simtimeslice, determines the
    timeslice amount.  A thread of the lowest priority that is busy will
    actually add itself to the busy queue using the timeslice amount.  The busy
    routine itself tracks the total amount of busy time required and loops,
    re-busying the thread until all time has expired.  Thus, for timesliced
    threads and for interrupted threads, the routine sees that more time is
    required and loops as necessary.

Regressions:

    * time1 - 4.

Garbage Collection
------------------

Status:

    Completed 6/17/2004.

Description:

    Plasma is going to have a lot of producer/consumer type code, where the
    ownership of a particular piece of memory will be hard to track.  Garbage
    collection will make the code much easier to understand and less error-prone.

Implementation:

    Boehm garbage collector.  The main issue is that the collector needs to know
    about all roots in the system, i.e. thread stacks.  This is accomplished as
    follows:

    1.  A list exists (System::_active_list) that records all active threads.
        When a thread is realized, it is added to this list.  Each Thread object
        has a **nt** and **pt** pointer for storing this information.  When a
        thread is destroyed, it is removed from the list.

    2.  In addition to the bottom of the stack, each thread records the top of
        the stack.  This is set whenever a thread is swapped out by calling
        Thread::setStackEnd().

    3.  Whhen the collector is called, it called the function pointer
        GC_push_other_roots.  This is set to the function
        System::push_other_roots(), which iterates over the active list, pushing
        information about the top and bottom of the stack.

    Other routines used are GC_lock(), which does nothing since we do not use
    kernel threads at this point, and GC_stop_world() and GC_start_world(),
    which turn preemtion off and on.

Dependencies:

    The main issue is getting it to handle user-threads.  It handles kernel
    threads and should be able to handle user-threads, but I don't know how to
    do it yet.

Regressions:

    No explicit tests- the rest of the regressions should test its usage.

Timeouts
--------

Status:

    Completed 6/18/2004.

Description:

    Create a channel that has a backing thread which wakes up and writes to the
    channel after a specified amount of time.  Use in alt blocks.

Implementation:

    Created the Timeout class- it's in Interface.h.  It's written entirely in
    C++ so that I could hide the implementation and not have to worry about
    linking Plasma code in with the rest of the thread package.  

Regressions:

    * chan12

Clocked Channels
----------------

Investigate further.  Most likely this will be a channel whose writes are
guarded by delay statements.  The delay will come from a clock object.


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

