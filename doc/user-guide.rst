===================
Plasma User's Guide
===================

This document describes PLASMA (Parallel LAnguage for System Modelling and
Analysis).  It is a superset of C++ which adds concurrency constructs, a concept
of simulation time for discret, and various safety features.

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

Thread Communication
====================

Channels
--------

TBD

Alt Blocks
----------

TBD

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
