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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <opencxx/parser/Parser.h>
#include <opencxx/parser/ProgramString.h>
#include <opencxx/parser/ProgramFile.h>
#include <opencxx/parser/ErrorLog.h>
#include <opencxx/parser/Lex.h>
#include <opencxx/parser/Ptree.h>
#include <opencxx/parser/Encoding.h>
    //< :TODO: remove once dependency of Ptree on Encoding is removed

#include <opencxx/PtreeVisualizer.h>

using namespace Opencxx;

class MockErrorLog : public ErrorLog
{
public:
    void Report(const Msg&) {}
};

bool Parse(const std::string& nterm, Parser& parser, Opencxx::Ptree*& result)
{
    Opencxx::Encoding encoding;
    
    if      (nterm == "rProgram") return parser.rProgram(result);
    else if (nterm == "rDefinition") return parser.rDefinition(result);
    else if (nterm == "rNullDeclaration") return parser.rNullDeclaration(result);
    else if (nterm == "rTypedef") return parser.rTypedef(result);
    else if (nterm == "rTypeSpecifier") return parser.rTypeSpecifier(result, false, encoding);
    else if (nterm == "rMetaclassDecl") return parser.rMetaclassDecl(result);
    else if (nterm == "rMetaArguments") return parser.rMetaArguments(result);
    else if (nterm == "rLinkageSpec") return parser.rLinkageSpec(result);
    else if (nterm == "rNamespaceSpec") return parser.rNamespaceSpec(result);
    else if (nterm == "rNamespaceAlias") return parser.rNamespaceAlias(result);
    else if (nterm == "rUsing") return parser.rUsing(result);
    else if (nterm == "rLinkageBody") return parser.rLinkageBody(result);
    else if (nterm == "rTemplateDecl") return parser.rTemplateDecl(result);
    else if (nterm == "rTempArgList") return parser.rTempArgList(result);
    else if (nterm == "rTempArgDeclaration") return parser.rTempArgDeclaration(result);
    else if (nterm == "rExternTemplateDecl") return parser.rExternTemplateDecl(result);

    else if (nterm == "rDeclaration") return parser.rDeclaration(result);
    else if (nterm == "rCondition") return parser.rCondition(result);
    else if (nterm == "rSimpleDeclaration") return parser.rSimpleDeclaration(result);

    else if (nterm == "optMemberSpec") return parser.optMemberSpec(result);
    else if (nterm == "optStorageSpec") return parser.optStorageSpec(result);
    else if (nterm == "optCvQualify") return parser.optCvQualify(result);
    else if (nterm == "optIntegralTypeOrClassSpec") return parser.optIntegralTypeOrClassSpec(result, encoding);
    else if (nterm == "rConstructorDecl") return parser.rConstructorDecl(result, encoding);
    else if (nterm == "optThrowDecl") return parser.optThrowDecl(result);

    else if (nterm == "optPtrOperator") return parser.optPtrOperator(result, encoding);
    else if (nterm == "rMemberInitializers") return parser.rMemberInitializers(result);
    else if (nterm == "rMemberInit") return parser.rMemberInit(result);

    else if (nterm == "rName") return parser.rName(result, encoding);
    else if (nterm == "rOperatorName") return parser.rOperatorName(result, encoding);
    else if (nterm == "rCastOperatorName") return parser.rCastOperatorName(result, encoding);
    else if (nterm == "rPtrToMember") return parser.rPtrToMember(result, encoding);
    else if (nterm == "rTemplateArgs") return parser.rTemplateArgs(result, encoding);

    else if (nterm == "rArgDeclList") return parser.rArgDeclList(result, encoding);
    else if (nterm == "rArgDeclaration") return parser.rArgDeclaration(result, encoding);

    else if (nterm == "rFunctionArguments") return parser.rFunctionArguments(result);
    else if (nterm == "rInitializeExpr") return parser.rInitializeExpr(result);

    else if (nterm == "rEnumSpec") return parser.rEnumSpec(result, encoding);
    else if (nterm == "rEnumBody") return parser.rEnumBody(result);
    else if (nterm == "rClassSpec") return parser.rClassSpec(result, encoding);
    else if (nterm == "rBaseSpecifiers") return parser.rBaseSpecifiers(result);
    else if (nterm == "rClassBody") return parser.rClassBody(result);
    else if (nterm == "rClassMember") return parser.rClassMember(result);
    else if (nterm == "rAccessDecl") return parser.rAccessDecl(result);
    else if (nterm == "rUserAccessSpec") return parser.rUserAccessSpec(result);

    else if (nterm == "rCommaExpression") return parser.rCommaExpression(result);

    else if (nterm == "rExpression") return parser.rExpression(result);
    else if (nterm == "rConditionalExpr") return parser.rConditionalExpr(result);
    else if (nterm == "rShiftExpr") return parser.rShiftExpr(result);
    else if (nterm == "rAdditiveExpr") return parser.rAdditiveExpr(result);
    else if (nterm == "rMultiplyExpr") return parser.rMultiplyExpr(result);
    else if (nterm == "rPmExpr") return parser.rPmExpr(result);
    else if (nterm == "rCastExpr") return parser.rCastExpr(result);
    else if (nterm == "rTypeName") return parser.rTypeName(result);
    else if (nterm == "rTypeName") return parser.rTypeName(result, encoding);
    else if (nterm == "rUnaryExpr") return parser.rUnaryExpr(result);
    else if (nterm == "rThrowExpr") return parser.rThrowExpr(result);
    else if (nterm == "rSizeofExpr") return parser.rSizeofExpr(result);
    else if (nterm == "rTypeidExpr") return parser.rTypeidExpr(result);
    else if (nterm == "rAllocateExpr") return parser.rAllocateExpr(result);
    else if (nterm == "rUserdefKeyword") return parser.rUserdefKeyword(result);
    else if (nterm == "rAllocateType") return parser.rAllocateType(result);
    else if (nterm == "rNewDeclarator") return parser.rNewDeclarator(result, encoding);
    else if (nterm == "rAllocateInitializer") return parser.rAllocateInitializer(result);
    else if (nterm == "rPostfixExpr") return parser.rPostfixExpr(result);
    else if (nterm == "rPrimaryExpr") return parser.rPrimaryExpr(result);
    else if (nterm == "rUserdefStatement") return parser.rUserdefStatement(result);
    else if (nterm == "rVarName") return parser.rVarName(result);
    else if (nterm == "rVarNameCore") return parser.rVarNameCore(result, encoding);

    else if (nterm == "rFunctionBody") return parser.rFunctionBody(result);
    else if (nterm == "rCompoundStatement") return parser.rCompoundStatement(result);
    else if (nterm == "rStatement") return parser.rStatement(result);
    else if (nterm == "rIfStatement") return parser.rIfStatement(result);
    else if (nterm == "rSwitchStatement") return parser.rSwitchStatement(result);
    else if (nterm == "rWhileStatement") return parser.rWhileStatement(result);
    else if (nterm == "rDoStatement") return parser.rDoStatement(result);
    else if (nterm == "rForStatement") return parser.rForStatement(result);
    else if (nterm == "rTryStatement") return parser.rTryStatement(result);

    else if (nterm == "rExprStatement") return parser.rExprStatement(result);
    else if (nterm == "rDeclarationStatement") return parser.rDeclarationStatement(result);
    else {
        std::cerr << "ERROR: unknown parser function `" << nterm << "'" 
                  << std::endl;
        std::exit(1);
    }
}

int main(int ac, char* av[])
{
    using std::string;
    using std::endl;
    if (ac <= 1) {
        std::cerr << "USAGE: show-ast PARSER_FUNCTION C++CODE" << std::endl
                  << "       show-ast PARSER_FUNCTION -f FILE" << std::endl
                  << std::endl
                  << "    Parses the code using given parser function" << std::endl
                  << "    and outputs `show-ast.ps' file with AST visualization" << std::endl
                  << "    Depends on GraphViz. For value of PARSER_FUNCTION" << std::endl
                  << "    see source code; for quick-start use 'rProgram'." << std::endl;
        std::exit(1);
    }

    MockErrorLog errorLog;
    Opencxx::Program* program;
    
    string parserName(av[1]);
    string programText;
    if (ac > 2  && av[2] == string("-f")) {
        if (ac <= 3) {
            std::cerr << "ERROR: no file after `-f'" << std::endl;
            std::exit(1);
        }
        std::ifstream* input = new std::ifstream(av[3]);
        if (! input) {
            std::cerr << "ERROR: cannot open `" << av[3] << "'" << std::endl;
            std::exit(1);
        }
        program = new Opencxx::ProgramFile(*input, av[3]);
    }
    else {
        Opencxx::ProgramString* ps = new Opencxx::ProgramString;
        for(int i = 2; i < ac; ++i) {
            programText += " ";
            programText += av[i];
        }
        *ps << programText.c_str();
        program = ps;
    }
    Lex lexer(program);
    Parser parser(&lexer, errorLog);
    
    Ptree* result;
    bool ok = Parse(parserName, parser, result);
    if (! ok) {
        std::cerr << "ERROR: syntax error" << std::endl;
        std::exit(1);
    }
    
    //result->Display();
    std::ofstream out("show-ast.dot");
    {
        Opencxx::PtreeVisualizer visualizer(out);
        visualizer.Visualize(result);
    }
    out.close();
    int res = std::system("dot show-ast.dot -Tps -o show-ast.ps");
    if (res == -1) {
        std::cerr << "ERROR: call to `system' failed" << endl;
        std::exit(1);
    }
}
