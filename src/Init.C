//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

//
// Main program- initializes resources and starts threads.
//

#include <iostream>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>

#include "Machine.h"
#include "System.h"
#include "Cluster.h"
#include "Proc.h"

using namespace std;

namespace plasma {

  static jmp_buf caller; // state for non-local exit
  static int code;       // return code

  static void *sig_int;  // save buffers for original signal handlers
  static void *sig_term;
  static void *sig_hup;
  static void *sig_chld;
  static void *sig_usr1;

  // ----------------------------------------------------------------------------
  //
  // trigger shutdown of program
  //
  // ----------------------------------------------------------------------------
  static void shutdown(int sig) 
  {
    if (sig == SIGCHLD)             // child has terminated
      {
        cout << "\nBKRT aborted: child has terminated.\n";
      }
    else if (sig == SIGINT)         // ctrl-c 
      {
        cout << "\nBKRT aborted on user request.\n"; 
      }
    else if (sig != SIGHUP)         // non-proper termination
      {
        cout << "\nBKRT process: signal " << sig << " received (core dumped).\n"; 
      }
    code = 255;
    if (sig == SIGCHLD)
      {
        exit(code);                 // error exit if child has died
      }
    if (sig == SIGINT)
      {
        exit(code);                 // error exit on ctrl-c
      }
    if (sig != SIGHUP) 
      {
        abort();                    // produce core and exit
      }
    signal(SIGINT, SA_HANDLER(sig_int));   // ctrl c
    signal(SIGTERM, SA_HANDLER(sig_term)); // kill
    signal(SIGCHLD, SA_HANDLER(sig_chld)); // death of child
    signal(SIGHUP, SA_HANDLER(sig_hup));   // termination triggered
    signal(SIGUSR1, SA_HANDLER(sig_usr1)); // garbage collection
    longjmp(caller, 1);                    // non-local jump to saved state
  }

  // Terminate program with return code.
  void pExit(int code)
  {
    pLock();
    thesystem.shutdown(code); // shutdown system and deliver exit code
    thecluster.runscheduler();
  }

  // Abort program with error message.
  void pAbort(const char *msg)
  {
    pLock();
    cerr << "\nPlasma aborted: " << msg << ".\n";
    thesystem.shutdown(-1);              // shutdown system and deliver exit code -1
    thecluster.runscheduler();
  }

  // Abort program with error message and immediate exit.
  void pPanic(const char *msg)
  {
    cerr << "\nPlasma panic: " << msg << ".\n";
    abort();                        // produce core
  }

}

using namespace plasma;

// Globals for the system and the cluster (handler of processors).
System  thesystem;
Cluster thecluster;

// Main entry routine that must be supplied by the user.
extern int pMain(int,const char *[]);

// This class holds the main starting point and runs the pMain
// routine with argc and argv.
class StartThread : public Thread {
public:
  StartThread(int argc,const char **argv,Proc *p) :
    _argc(argc), _argv(argv) 
  {
    realize(user,this);
    setProc(p);
  };
private:
  static void user(void *a) {
    StartThread *st = (StartThread*)a;
    pExit(pMain(st->_argc,st->_argv));
  };

  int          _argc;
  const char **_argv;
};

#ifdef __CYGWIN__

// This calls an external dummy function in order to ensure that it doesn't
// get inlined so that the main program can overload it.
extern "C" void default_pSetup(ConfigParms &) 
{
  dummy();
}

void pSetup (ConfigParms &) __attribute__ ((weak, alias ("default_pSetup")));

#else 

// Default setup function- does nothing.  User may
// override this in the main program b/c it's defined
// as using weak binding.
void pSetup(ConfigParms &) __attribute__ ((weak));

// This calls an external dummy function in order to ensure that it doesn't
// get inlined so that the main program can overload it.
void pSetup(ConfigParms &) 
{
  dummy();
}

#endif


// Setup function.  We try to hook this up so that it's called
// before anything that needs it, even if that stuff is also
// global/static.  The general steps of this function are:
// 1.  Run setup function.
// 2.  Execute any necessary static init functions.
int setupInternal()
{
  try {
    ConfigParms configParms;

    // Run setup function.
    pSetup(configParms);
    
    // Initialize process system.
    Proc::init(configParms);
    
    thesystem.init(configParms);
    thecluster.init(configParms);
  }
  catch (exception &err) {
    pPanic(err.what());
  }
  return 0;
}

// This wraps the setup function within a static variable so that
// we can ensure that the setup function is only called once.
// In general, any class with a static init function that could
// be used by the user (i.e. Cluster isn't currently) should have a
// call to setup() (not setupInternal!) in its constructor to ensure
// that setup is performed.
void setup()
{
  static int dummy = setupInternal();
  dummy = dummy;
}

// This executes the function pMain, calling it with the usual
// argc and argv.
int main(int argc,const char *argv[])
{
  // Continuation state for program exit.
  if (setjmp(caller)) {
    return(code); // return to caller        
  }

  try {

    // If we haven't done setup yet, do it now.
    setup();

    // Default processor.  Created after call to init so that we
    // have the correct number of priorities set.  We allocate from
    // stack b/c for some reason this got the garbage collector to
    // properly scan it.
    Proc *processor = new Proc;

    thecluster.reset(processor);

    // Note:  You can't catch SIGSEGV or SEGBUS b/c the GC uses those
    // signals during marking.
    sig_int  = (void*)signal(SIGINT, SA_HANDLER(shutdown));  // ctrl c
    sig_term = (void*)signal(SIGTERM, SA_HANDLER(shutdown)); // kill
    sig_chld = (void*)signal(SIGCHLD, SA_HANDLER(shutdown)); // death of child
    sig_hup  = (void*)signal(SIGHUP, SA_HANDLER(shutdown));  // termination triggered

    StartThread *st = new StartThread(argc,argv,processor);
    processor->add_ready(st);
    thecluster.scheduler();             // execute thread scheduler 
    return (thesystem.retcode());
  }
  catch (exception &err) {
    pPanic(err.what());
  }
}
