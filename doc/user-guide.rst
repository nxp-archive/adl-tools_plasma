===================
Plasma User's Guide
===================

This document describes PLASMA (Parallel LAnguage for System Modeling and
Analysis).  It is a superset of C++ which adds concurrency constructs, a concept
of simulation time for discrete, and various safety features.

:Author: Brian Kahne 
:Contact: bkahne@freescale.com 
:revision: $Id$ 

.. contents::

-----------
Concurrency
-----------

This section discusses the new language features added in order to support
concurrency.  These fall into two main categories: Thread creation and thread
communication.

Thread Creation
===============

Par Block
---------

The **par** block executes each statement in parallel, waiting until all child
threads are finished before proceeding.  For example::

        int x,y,z;
        par {
          x = sub1();
          y = sub2();
        }
        z = sub3();

In the above example, the statement "x = sub1()" and the statement "y = sub2()"
will execute in parallel.  Both will complete before the statement "z = sub3()"
is executed.  To create a more complex in-line operation, simply use a
brace-delineated block, e.g.::

        par {
          {
            <sequence of statements>
          }
          {
            <sequence of statements>
          }
        }

PFor Loop
---------

The **pfor** loop acts like a for-loop, except that for each loop iteration, its
body is launched as a separate thread.  All threads must then complete before
execution continues past the loop.  For example::

        const int Max = 10;
        int results[Max];
        pfor (int i = 0; i != Max ++i)  {
          results[i] = sub(i);
        }

In this example, ten threads will be launched.  Each will return a result which
is stored into an element of the *results* array.

An important feature of the **pfor** loop is that variables declared in the loop
condition are passed by value to each thread, while all other variables are
passed by reference, and thus may be modified.  Thus, each thread contains a
copy of the index variable **i** but may directly modify **results**.

Thread Control
==============

The following functions provide control over threads.  These are declared in
``plasma-interface.h`` which is implicitly included in all plasma (.pa) files.

1. ``THandle pCurThread()``:  Return a handle to the current thread.

2. ``pWait(THandle)``:  Wait until the specified thread is finished.

3. ``pYield()``:  Have the current thread swap to the next ready thread.

4. ``pTerminate()``:  Kill the current thread.

5. ``pLock()``:  Turn off time-slicing.

6. ``pUnlock()``:  Turn on time-slicing.

5. ``pExit(int code)``:  Terminate the program with the specified exit code.

6. ``pAbort(char *)``:  Abort the program gracefully with error message and
   return exit code -1.

7.  ``pPanic(char *)``:  Abort program immediately with error message and return
    exit code -1.

Thread Communication
====================

Channels
--------

One method for threads to communicate among themselves is to use a channel.
This is simply a data structure which allows one thread to write a value to it
and another thread to read this value.  It is up to the channel to make sure
that these operations are safe and to ensure proper flow control.  Any class may
be a channel as long as it has a specific interface.  This interface is required
in order to use the **alt** and **afor** constructs.

The required interface for a channel of type T is:

1. ``T read()``: Returns a value read from the channel.  Blocks if no value is
   present.  Returns the last value read, until clear_ready() is called.

2. ``T get()``: Returns a value from the channel.  Blocks if no value is
   present.  Always fetches a new value.  After a call to this, read() will
   return this same value.

3. ``void write(T)``: Writes a value to the channel.  May block, depending
   upon the channel definition.

4. ``bool ready() const``: Returns true if the channel has a value.

5. ``void clear_ready()``:  Clears the ready status, forcing the fetch of a new
   value.

6. ``set_notify(Thread *t,int handle)``: Stores the thread and handle.  When the
   channel gets a value, it will wake this thread, giving it the handle.

7. ``clear_notify()``:  Clears the stored thread so that no notification will
   take place if a value is written to the channel.  It must be possible to
   call clear_notify() safely, e.g. this should not affect the behavior of
   a blocked writing thread.

Note that write(), read(), and clear_ready() are technically not required by **alt**
and **afor**.  Thus, it's possible to have a read-only channel.

Currently, Plasma contains the following channels.  These are declared in ``plasma.h``.

1. ``Channel<class Data>``:  This is a typed channel which reads and writes an
   object of type *Data*.  It contains only a single copy of this object; a
   second write will block if the first write's data has not been read.

Alt Blocks
----------

An **alt** block allows for unordered selection of data from channels.  Its
syntax is::

  alt {
    port (<value decl> ; <channel expr> ;) { <body> }
    ...
    [ { <default block> } ]
  }

Each **port** statement specifies a channel to be read (the channel expression) and
an optional declaration which will receive the channel value.  The **port** body
has access to this value.  If no value declaration is specified, the channel's
data is not accessible.  This is useful for channels whose data is simply a
boolean state, such as a time-out channel.

Upon entry to the **alt** block, all channels are checked for data.  If a
channel has data, the body of the corresponding **port** statement is executed.
If more than one channel is ready, a single port statement is selected
non-deterministically.  If no channels are ready, the thread will sleep until a
channel has data.

If a default block is specified, the **alt** block will never cause the thread
to sleep.  Instead, if no channels have data, the default block will be
executed.

Afor Blocks
-----------

An **afor** block is similar to an **alt** block, except that it allows the user
to loop over a data structure of channels.  Its syntax is::

  afor ( <s1> ; <s2> ; <s3> ) {
    port (<value decl> ; <channel expr> ;) { <body> }
    [ { <default block> } ]
  }

Only a single **port** statement is allowed.  The **afor** block is treated as a
for-loop, looping over all channels specified by the channel expression.  An
iterator variable must be declared in *s1*; its value is accessible to the
channel expression and the **port**'s body.

For example, the following code loops over an array of channels.  As in the
**alt** block, the thread will sleep if no channels are ready and there is not a
default block.::

  afor (int i = 0; i != (int)channels.size(); ++i) {
    port (int v; channels[i];) {
      printf ("Got a value from port %d:  %d\n",i,v);
      if (v < 0) ++donecount;
    }
  }

Plasma allows for non-integer index variables but this requires the creation of
an auxiliary data structure, so performance might be a little slower, e.g. using
an iterator rather than an integer as an index.

There are a few restrictions to follow for the **afor** block:

1.  You must declare the loop iterator in the first statement of the **afor**
    block.

2.  The loop will occur multiple times, so make sure that there are no
    side-effects.

3.  You only have access, within the **port** body, to the first loop iterator
    variable.  Therefore, avoid fancy **afor** loops which declare multiple
    variables in the first statement or update multiple variables in the third statement.

Serialized Data Structures
--------------------------

TBD

--------------
The Time Model
--------------

TBD

------------------------------
Additional Language Constructs
------------------------------

Let Blocks
==========

TBD

Lambda Functions
================

TBD

Garbage Collection
==================

TBD

-----------
Future Work
-----------

TBD
