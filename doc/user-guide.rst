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

Spawn Operator
--------------

The **spawn** operator creates a thread and returns a handle object which allows
the result of the thread to be retrieved.  **spawn** takes a single argument
which must be some sort of function invocation.  This might be an actual
function, a method call, etc.  The list of supported types is:

1. Literal function call: ``spawn(foo());``

2. Function pointer call: ``p = foo; spawn(p());``

3. Method call w/reference: ``spawn(a.b());``

4. Method call w/pointer: ``spawn(a->b());``

5. Static method call: ``spawn(A::b());``

6. Method pointer w/reference: ``p = &A::b; spawn(a.*p());``

7. Method pointer w/pointer:  ``p = &A::b; spawn(a->*p());``

The function's arguments are evaluated immediately; a thread is then launched
of the function with its arguments.  The function itself must return some type
of value; void functions are not allowed.  In addition, the result type must
have a default constructor.

The **spawn** operator returns an object of type **Result<T>**, where **T** is
the return type of the invoked function.  Calling the *value()* method returns
the result of the thread; if the thread is not yet finished, it will block.
Calling *wait()* will wait until the thread is finished and calling *kill()*
will termminate the thread.  In the latter case, the result of the thread will
be the default constructor value of the return type.

A simple example is::

  double foo(double a,double b)
  {
    int xx = 0;
    for (int i = 0; i != 100000000; ++i) {
      xx += 1;
    }
    return a*a + b*b;
  }

  int pMain(int argc,const char *argv[])
  { 
    Result<double> r1 = spawn(foo(1.1,2.2));
    Result<double> r2 = spawn(foo(2.7,9.8));
    cout << "Result is:  " << r1.value() << ", " << r2.value() << endl;
    return 0;
  }

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

2. ``QueueChan<class Data>``:  This is a typed queued channel:  It allows for
   multiple producers but still requires a single consumer.  By default, the
   queue size is not fixed, but the user may set a maximum size by specifying it
   as the constructor argument.

3. ``ResChan<class Data>``: The **spawn** operator may be interfaced to an
   **alt** construct by using this class.  This is a read-only channel which
   will return the result value of the spawned thread.  For example::

     double foo(double a,double b)
     {
       int xx = 0;
       for (int i = 0; i != 100000000; ++i) {
         xx += 1;
       }
       return a*a + b*b;
     }

     int bar(int a)
     {
       return a * a * a;
     }

     void check(ResChan<double> &a,ResChan<int> &b)
     {
       for (int i = 0; i != 2; ++i) {
         alt {
           a.port(double x) {
             cout << "x:  " << x << endl;
           }
           b.port (int y) {
             cout << "y:  " << y << endl;        
           }
         }
       }
     }

     int pMain(int argc,const char *argv[])
     { 
       ResChan<double> r1 = spawn(foo(1.1,2.2));
       ResChan<int> r2 = spawn(bar(123));
       check(r1,r2);
       return 0;
     }

Alt Blocks
----------

An **alt** block allows for unordered selection of data from channels.  Its
syntax is::

  alt {
    <channel expr> [ . | -> ] port (<value decl>) { <body> }
    [ alt { ... } ]
    [ afor { ... } ]
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

**alt** and **afor** (explained below) blocks may be nested within **alt**
blocks.  This allows the user to block on multiple collections of channels, or a
collection of channels plus one or more single channels, etc.

Afor Blocks
-----------

An **afor** block is similar to an **alt** block, except that it allows the user
to loop over a data structure of channels.  Its syntax is::

  afor ( <s1> ; <s2> ; <s3> ) {
    <channel expr> [. | -> ] port (<value decl>) { <body> }
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
    channels[i].port (int v) {
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
    variables in the first statement or update multiple variables in the third
    statement.

As noted above, an **afor** block may be nested within an **alt** block.  This
allows you to block on one or more collections and/or to block on a collection
plus one or more single channels.  For example, thhe following code will block
on a collection and an override channel:::

  alt {
    afor (int i = 0; i != (int)channels.size(); ++i) {
      channels[i].port (int v) {
        printf ("Got a value from port %d:  %d\n",i,v);
        if (v < 0) ++donecount;
      }
    }
    stopchan.port (bool b) {
      if (b) {
        printf ("Got a stop command!\n");
      }
    }
  }

Shared Data Structures
----------------------

Threads may also commmunicate using shared data structures whose access methods
are protected by special synchronization primitives.  There are two means to do
this.  The easiest is to declare a class as being a mutex class::

  pMutex class X { };

This will wrap all public methods of class **X**, except for constructors and
its destructor, with serialization code.  To prevent this on a per-method basis,
use the modifier *pNoMutex*::

  pMutex class Foo {
  public:
    // Not protected.
    Foo();
    ~Foo();
    // Protected.
    int a();
    // Not protected.
    pNoMutex int b();
  private:
    // Not protected.
    int c();
  };    

Be careful with using *pNoMutex*:  Since it disables serialization, it is
inherently dangerous.  It is useful, though, when you have a constant method
whose return value would not be affected by a thread preemption.  For example, a
method which returns a constant which is only initialized at construction time.

The other method for creating a shared data structure is to directly use the
``pLock()`` and ``pUnlock()`` primitives.  This is more error prone than using
*pMutex* but might be necessary in some cases, such as for protecting a plain function::

  void msg(const char *fmt, ...) {
    pLock();
    va_list ap;
    va_start(ap,fmt);
    vprintf(fmt,ap);
    va_end(ap);
    pUnlock();
  }

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
