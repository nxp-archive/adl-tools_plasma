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

#include <cassert>
#include <iostream>
#include <opencxx/parser/ProgramString.h>
#include <opencxx/parser/Lex.h>
#include <opencxx/parser/Parser.h>
#include <opencxx/parser/Ptree.h>
#include <opencxx/parser/ErrorLog.h>
#include <opencxx/parser/Msg.h>

using namespace Opencxx;

class MockErrorLog : public ErrorLog
{
public:
    void Report(const Msg& m) 
    { 
        assert(m.GetSeverity() != Msg::Error);
        assert(m.GetSeverity() != Msg::Fatal);
    }
};

int main()
{
    ProgramString program;
    program << "int main() {}\n";
    MockErrorLog errorLog;
    Lex lexer(&program);
    Parser parser(&lexer, errorLog);
    
    Ptree* ptree;
    bool result = parser.rProgram(ptree);
    assert(result);
    ptree->Display(std::cerr);
}
