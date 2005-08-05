===============
Plasma Abstract
===============

The ability to enhance single-thread performance, such as by increasing clock
frequency, is reaching a point of diminishing returns: power is becoming a
dominate factor and limiting scalability. Adding additional cores is a
scalable way to increase performance, but it requires that customers have a
method for developing multi-threaded applications.

Plasma, (Parallel LAnguage for System Modeling and Analysis) is a parallel
language for system modeling and multi-threaded application development
implemented as a superset of C++. The language extensions are based upon those
found in Occam, which is based upon CSP (Communicating Sequential Processes) by
C. A. R. Hoare.

The goal of the Plasma project is to investigate whether a language with the
appropriate constructs might be used to ease of the task of developing highly
multi-threaded software.  In addition, through the inclusion of a discrete-event
simulation API, we seek to simplify the task of system modeling and increase
productivity through clearer representation and increased compile-time checking
of the more difficult-to-get-right aspects of systems models (the concurrency).

The result is a single language which allows users to develop a parallel
application and then to model it within the context of a system, allowing for
hardware-software partitioning and various other early tradeoff analyses.  We
believe that this language offers a simpler and more concise syntax than other
offerings and can be targeted at a large range of potential architectures,
including heterogeneous systems and those without shared memory.
