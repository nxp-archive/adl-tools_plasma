//
// Main program- initializes resources and starts threads.
//

#include <iostream>
#include <setjmp.h>
#include <signal.h>

#include "Machine.h"
#include "System.h"
#include "Processor.h"

using namespace std;

namespace plasma {

  static jmp_buf caller; // state for non-local exit
  static int code;       // return code

  static void *sig_int;  // save buffers for original signal handlers
  static void *sig_term;
  static void *sig_segv;
  static void *sig_bus;
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
    signal(SIGSEGV, SA_HANDLER(sig_segv)); // segmentation violation
    signal(SIGBUS, SA_HANDLER(sig_bus));   // bus error
    signal(SIGCHLD, SA_HANDLER(sig_chld)); // death of child
    signal(SIGHUP, SA_HANDLER(sig_hup));   // termination triggered
    signal(SIGUSR1, SA_HANDLER(sig_usr1)); // garbage collection
    longjmp(caller, 1);                    // non-local jump to saved state
  }

  Processor   processor;
  System      thesystem;

  // Terminate program with return code.
  void pExit(int code)
  {
    pLock();
    thesystem.shutdown(code); // shutdown system and deliver exit code
    processor.runscheduler();
  }

  // Abort program with error message.
  void pAbort(char *msg)
  {
    pLock();
    cout << "\nPlasma aborted: " << msg << ".\n";
    thesystem.shutdown(-1);              // shutdown system and deliver exit code -1
    processor.runscheduler();
  }

  // Abort program with error message and immediate exit.
  void pPanic(char *msg)
  {
    cout << "\nPlasma panic: " << msg << ".\n";
    abort();                        // produce core
  }

}

using namespace plasma;

// Main entry routine that must be supplied by the user.
extern int pMain(int,const char *[]);

// This class holds the main starting point and runs the pMain
// routine with argc and argv.
class StartThread : public Thread {
public:
  StartThread(int argc,const char **argv) :
    _argc(argc), _argv(argv) 
  {
    realize(user,this);
  };
private:
  static void user(void *a) {
    StartThread *st = (StartThread*)a;
    pExit(pMain(st->_argc,st->_argv));
  };

  int          _argc;
  const char **_argv;
};

// Default setup function- does nothing.  User may
// override this in the main program b/c it's defined
// as using weak binding.
void pSetup(ConfigParms &) __attribute__ ((weak));

void pSetup(ConfigParms &) 
{
}

// This executes the function pMain, calling it with the usual
// argc and argv.
int main(int argc,const char *argv[])
{
  ConfigParms configParms;

  // Continuation state for program exit.
  if (setjmp(caller)) {
    return(code); // return to caller        
  }

  // Run setup function.
  pSetup(configParms);

  // Initialize process system.
  thesystem.init(configParms);
  processor.init(configParms);

  sig_int  = (void*)signal(SIGINT, SA_HANDLER(shutdown));  // ctrl c
  sig_term = (void*)signal(SIGTERM, SA_HANDLER(shutdown)); // kill
  sig_segv = (void*)signal(SIGSEGV, SA_HANDLER(shutdown)); // segmentation violation
  sig_bus  = (void*)signal(SIGBUS, SA_HANDLER(shutdown));  // bus error
  sig_chld = (void*)signal(SIGCHLD, SA_HANDLER(shutdown)); // death of child
  sig_hup  = (void*)signal(SIGHUP, SA_HANDLER(shutdown));  // termination triggered

  StartThread *st = new StartThread(argc,argv);
  thesystem.add_ready(st);
  processor.scheduler();             // execute thread scheduler 
  delete st;
  return (thesystem.retcode());
}
