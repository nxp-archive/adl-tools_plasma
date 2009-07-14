//@beginlicenses@
//@license{Grzegorz Jakacki}{2004}@
//
//  Permission to use, copy, distribute and modify this software and its  
//  documentation for any purpose is hereby granted without fee, provided that
//  the above copyright notice appears in all copies and that both that copyright
//  notice and this permission notice appear in supporting documentation.
// 
//  Grzegorz Jakacki make(s) no representations about the suitability of this
//  software for any purpose. It is provided "as is" without express or implied
//  warranty.
//  
//  Copyright (C) 2004 Grzegorz Jakacki
//
//@endlicenses@

#include <set>
#include <string.h>

#include <opencxx/OpencxxConfiguration.h>
#include <cassert>
#include <cstdlib>
#include <opencxx/Class.h>
#include <opencxx/CliErrorMsg.h>
#include <opencxx/GenericMsg.h>
#include <opencxx/InfoMsg.h>
#include <opencxx/UnknownCliOptionException.h>
#include "opencxx/parser/CerrErrorLog.h"
#include <opencxx/parser/ErrorLog.h>

namespace Opencxx
{

  OpencxxConfiguration::OpencxxConfiguration(Opencxx::ErrorLog& errorLog)
    : errorLog_(errorLog)
    , verboseMode_(false)
    , libtoolPlugins_(false)
    , doPreprocess_(true)
    , preprocessTwice_(false)
    , makeExecutable_(true)
    , makeSharedLibrary_(false)
    , recognizeOccExtensions_(true)
    , wcharSupport_(false)
    , staticInitialization_(true)
    , doCompile_(true)
    , showProgram_(false)
    , externalDriver_(false)
    , doTranslate_(true)
    , showVersion_(false)
    , printMetaclasses_(false)
    , numOfObjectFiles_(0)
    , sharedLibraryName_()
    , compilerCommand_("g++")
    , preprocessorCommand_("g++")
    , linkerCommand_("g++")
  {
    setupValidExtensions();
  }

  /**
     @todo Figure out who owns the <code>option</code> c-string
     and make this method side-effect free.
  */
  void OpencxxConfiguration::RecordCmdOption(char* option)
  {
    if(option == 0 || *option == '\0')
      return;

    char* value = strchr(option, '=');
    if(value != 0)
      *value++ = '\0';

    if(!Class::RecordCmdLineOption(option, value)){
      errorLog_.Report(CliErrorMsg("too many `-M' options"));
      assert(! "CliErrorMsg should throw");
    }
  }

  static bool streq(const char* s1, const char* s2)
  {
    return strcmp(s1, s2) == 0;
  }

  static bool strpref(const char* prefix, const char* text)
  {
    if (strlen(prefix) > strlen(text)) return false;
    return strncmp(prefix, text, strlen(prefix)) == 0;
  }

  static std::set<std::string> ValidExtensions;

  /*
    File Naming convention

    .c, .C, .cc, .cpp, .cxx : C++ source files
    .mc : metaclass files
    .occ, .ii, .i (IRIX_CC): temporary files used by OpenC++
  */
  void setupValidExtensions()
  {
    if (ValidExtensions.empty()) {
      ValidExtensions.insert(".cc");
      ValidExtensions.insert(".C");
      ValidExtensions.insert(".c");
      ValidExtensions.insert(".mc");
      ValidExtensions.insert(".cxx");
      ValidExtensions.insert(".cpp");
      ValidExtensions.insert(".ii");
      ValidExtensions.insert(".i");
      ValidExtensions.insert(".occ");
    }
  }

  // Add a new valid extension.  This should include the ".".
  // Returns true if added, false it not added b/c it already existed.
  bool addValidExtension(const std::string &str)
  {
    return ValidExtensions.insert(str).second;
  }

  // Remove an extension.  Returns true if removed, false if it didn't exist.
  void removeValidExtension(const std::string &str)
  {
    ValidExtensions.erase(ValidExtensions.find(str));
  }

  // Remove all valid extensions.
  void removeAllValidExtensions()
  {
    ValidExtensions.clear();
  }

  // Returns true if the file name matches a valid extension.
  static bool IsCxxSource(const char* fname)
  {
    using namespace std;

    char* ext = strrchr(fname, '.');
    if(ext == 0)
      return false;

    return ValidExtensions.count(ext);
  }

  void ParseCcOptions(
                      const std::string& arg
                      , OpencxxConfiguration& config)
  {
    if(arg[0] != '-') {
      if(IsCxxSource(arg.c_str())) {
        if (config.SourceFileName().empty()) {
          config.SetSourceFileName(arg);
          return;
        }
        else {
          config.ErrorLog().Report(
                                   GenericMsg(
                                              Msg::Fatal, SourceLocation(), 
                                              "multiple source files not allowed"
                                              )
                                   );
          assert(! "fatal message should throw");
        }
      }
    }

    config.AddObjectFile();
    if (config.SourceFileName().empty()) {
      config.AddCcOption(arg);
    }
    else {
      config.AddCc2Option(arg);
    }
  }



  void ParseCommandLine(int argc, char** argv, OpencxxConfiguration& config)
    /* throws UnknownCliOption */
  {
    int i;
    for(i = 1; i < argc; ++i) {
      if (streq("--", argv[i])) break;
      else if(streq("--private--external-driver", argv[i])) {
        config.SetExternalDriver(true);
      }
      else if(streq("--private--libtool-plugins", argv[i])) {
        config.SetLibtoolPlugins(true);
      }
      else if(streq("--private--no-static-initialization", argv[i])) {
        config.SetStaticInitialization(false);
      }
      else if (strpref("--static-mc=", argv[i])) {
        config.SetSharedLibraryName(&argv[i][12]);
      }
      else if (streq("-E", argv[i])) config.SetDoCompile(false);
      else if (streq("--private--output", argv[i])) {
        ++i;
        if (argc <= i) {
          config.ErrorLog().Report(CliErrorMsg("missing argument to `--output'"));
          assert(! "CLI error should throw");
        }
        else {
          config.SetOutputFileName(argv[i]);
        }
      }
      else if (strpref("--comp=",argv[i])) {
        const char *compilerName = &argv[i][7];
        if (! *compilerName ) {
          config.ErrorLog().Report(CliErrorMsg("missing argument to '--comp'"));
          assert(!"CLI error should throw");
        }
        // Change arg0 of cpp and cc options.
        config.SetCompilerCommand(compilerName);
        config.SetPreprocessorCommand(compilerName);
      }
      else if(strncmp("-u", argv[i],2) == 0 && argv[i][2] != '\0') {
        // User-specified file extension to be recognized as OpenC++ source.
        addValidExtension(&argv[i][2]);
      }
      else if (streq("-g", argv[i])) config.AddCcOption(argv[i]);
      else if (streq("-n", argv[i])) config.SetDoPreprocess(false);
      else if (strpref("-m", argv[i])) {
        config.SetMakeSharedLibrary(true);
        config.SetSharedLibraryName(&argv[i][2]);
      }
      else if (streq("-P", argv[i])) config.SetPreprocessTwice(true);
      else if (streq("-p", argv[i])) config.SetDoTranslate(false);
      else if (streq("-C", argv[i])) {
        config.AddCppOption("-C");
        config.SetPreprocessTwice(true);
      }
      else if(streq("-c", argv[i])) config.SetMakeExecutable(false);
      else if(streq("-l", argv[i])) {
	    config.SetPrintMetaclasses(true);
      }
      else if(strpref("-S", argv[i])) {
        if (argv[i][2] == 0) {
          config.AddMetaclass(argv[++i]);
        }
        else {
          config.AddMetaclass(&argv[i][2]);
        }
      }
      else if(streq("-s", argv[i])) config.SetShowProgram(true);
      else if(streq("-v", argv[i]) || streq("--verbose", argv[i])) config.SetVerboseMode(true);
      else if(streq("-V", argv[i])) {
        config.SetShowVersion(true);
        return;
      }
      else if (streq("--regular-c++", argv[i]))
        config.SetRecognizeOccExtensions(false);
      else if (strpref("-D", argv[i])) config.AddCppOption(argv[i]);
      else if (strpref("-I", argv[i])) config.AddCppOption(argv[i]);
      else if (strpref("-d", argv[i]) && !streq("-d", argv[i])) {
        config.AddCppOption(&argv[i][2]);
      }
      else if (strpref("-M", argv[i])) config.RecordCmdOption(&argv[i][2]);
      else if (streq("-w", argv[i])) config.SetWcharSupport(true);
      else if (strpref("-", argv[i])) {
        throw UnknownCliOptionException(argv[i]);
      }
      else ParseCcOptions(argv[i], config);
    }
    while (++i < argc) {
      ParseCcOptions(argv[i], config);
    }
    
    if (! config.DoTranslate()) {
      config.SetDoCompile(false);
    }
  }

}


