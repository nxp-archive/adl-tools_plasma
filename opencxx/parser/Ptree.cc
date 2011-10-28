//@beginlicenses@
//@license{chiba_tokyo}{}@
//@license{chiba_tsukuba}{}@
//@license{chiba_tokyo}{}@
//
//  Permission to use, copy, distribute and modify this software and its  
//  documentation for any purpose is hereby granted without fee, provided that
//  the above copyright notice appears in all copies and that both that copyright
//  notice and this permission notice appear in supporting documentation.
// 
//  1997-2001 Shigeru Chiba, Tokyo Institute of Technology. make(s) no representations about the suitability of this
//  software for any purpose. It is provided "as is" without express or implied
//  warranty.
//  
//  Copyright (C)  1997-2001 Shigeru Chiba, Tokyo Institute of Technology.
//
//  -----------------------------------------------------------------
//
//  Permission to use, copy, distribute and modify this software and its  
//  documentation for any purpose is hereby granted without fee, provided that
//  the above copyright notice appears in all copies and that both that copyright
//  notice and this permission notice appear in supporting documentation.
// 
//  1997-2001 Shigeru Chiba, University of Tsukuba. make(s) no representations about the suitability of this
//  software for any purpose. It is provided "as is" without express or implied
//  warranty.
//  
//  Copyright (C)  1997-2001 Shigeru Chiba, University of Tsukuba.
//
//  -----------------------------------------------------------------
//
//  Permission to use, copy, distribute and modify this software and its  
//  documentation for any purpose is hereby granted without fee, provided that
//  the above copyright notice appears in all copies and that both that copyright
//  notice and this permission notice appear in supporting documentation.
// 
//  1997-2001 Shigeru Chiba, Tokyo Institute of Technology. make(s) no representations about the suitability of this
//  software for any purpose. It is provided "as is" without express or implied
//  warranty.
//  
//  Copyright (C)  1997-2001 Shigeru Chiba, Tokyo Institute of Technology.
//
//@endlicenses@

#include <iostream>
#include <cstdarg>
#include <stdio.h>
#include <cstring>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <opencxx/parser/ErrorLog.h>
#include <opencxx/parser/MopMsg.h>
#include <opencxx/parser/TheErrorLog.h>
#include <opencxx/parser/AbstractTranslatingWalker.h>
#include <opencxx/parser/AbstractTypingWalker.h>
#include <opencxx/parser/NonLeaf.h> // :TODO: cyclic dependency!!! get rid of it!
#include <opencxx/parser/Ptree.h>
#include <opencxx/parser/token-names.h>
#include <opencxx/parser/ErrorLog.h>
#include <opencxx/parser/Lex.h>
#include <opencxx/parser/MopMsg.h>
#include <opencxx/parser/TheErrorLog.h>
#include <opencxx/parser/GC.h>
#include <opencxx/parser/ptreeAll.h>
#include <opencxx/parser/auxil.h>

namespace Opencxx
{

using std::ostream;

void Ptree::Display(std::ostream& s)
{
    if(this == 0)
	s << "0\n";
    else{
	Print(s, 0, 0);
	s << '\n';
    }
}

void Ptree::Display()
{
    Display(std::cerr);
}

int Ptree::Write(ostream& s)
{
    if(this == 0)
	return 0;
    else
	return Write(s, 0);
}

void Ptree::PrintIndent(std::ostream& out, int indent)
{
    out << '\n';
    for(int i = 0; i < indent; ++i)
	out << "    ";
}

char* Ptree::ToString()
{
    if(this == 0)
    {
	return 0;  // :TODO: why? shouldn't we assert(this) ???
    }
    else{
	std::ostringstream buf;
	PrintOn(buf);
	const std::string& s = buf.str();
	char* result = new (GC) char[s.length() + 1];
	std::strcpy(result, s.c_str());
	return result;
    }
}

int Ptree::What()
{
    return BadToken;
}

bool Ptree::IsA(int kind)
{
    if(this == 0)
	return false;
    else
	return bool(What() == kind);
}

Ptree* Ptree::Translate(AbstractTranslatingWalker* w)
{
    return w->TranslatePtree(this);
}

void Ptree::Typeof(AbstractTypingWalker* w, TypeInfo& t)
{
    w->TypeofPtree(this, t);
}

const char* Ptree::GetEncodedType()
{
    return 0;
}

const char* Ptree::GetEncodedName()
{
    return 0;
}

Ptree* Ptree::Make(const char* pat, ...)
{
  va_list args;
  const int N = 4096;
  static char buf[N];
  char c;
  int len;
  char* ptr;
  Ptree* p;
  Ptree* q;
  int i = 0, j = 0;
  Ptree* result = 0;

  va_start(args, pat);
  while((c = pat[i++]) != '\0')
    if(c == '%'){
	    c = pat[i++];
	    if(c == '%')
        buf[j++] = c;
	    else if(c == 'd'){
        ptr = IntegerToString(va_arg(args, int), len);
        memmove(&buf[j], ptr, len);
        j += len;
	    }
	    else if(c == 's'){
        ptr = va_arg(args, char*);
        len = strlen(ptr);
        memmove(&buf[j], ptr, len);
        j += len;
	    }
	    else if(c == 'c')
        buf[j++] = va_arg(args, int);
	    else if(c == 'p'){
        p = va_arg(args, Ptree*);
        if(p == 0)
          /* ignore */;
        else if(p->IsLeaf()){
          memmove(&buf[j], p->GetPosition(), p->GetLength());
          j += p->GetLength();
        }
        else{   
          if(j > 0)
            q = PtreeUtil::List(new DupLeaf(buf, j), p);
          else
            q = PtreeUtil::List(p);

          j = 0;
          result = PtreeUtil::Nconc(result, q);
        }
	    }
	    else
        TheErrorLog().Report(MopMsg(Msg::Fatal, "Ptree::Make()", "invalid format"));
    }
    else
	    buf[j++] = c;

  va_end(args);

  assert(j < 4096);

  if(j > 0) {
    if(result == 0) {
      result = new DupLeaf(buf, j);
    } else {
      result = PtreeUtil::Snoc(result, new DupLeaf(buf, j));
    }
  }

  return result;
}

bool Ptree::Reify(bool& value)
{
    return Lex::Reify(this, value);
}

bool Ptree::Reify(unsigned int& value)
{
    return Lex::Reify(this, value);
}

bool Ptree::Reify(char*& str)
{
    return Lex::Reify(this, str);
}

Ptree* Ptree::qMake(char*)
{
    TheErrorLog().Report(MopMsg(Msg::Fatal, "PtreeUtil::qMake()", "the metaclass must be compiled by OpenC++."));
    return 0;
}

}

