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

#include <opencxx/parser/PtreeClassSpec.h>
#include <opencxx/parser/AbstractTranslatingWalker.h>
#include <opencxx/parser/token-names.h>

namespace Opencxx
{

PtreeClassSpec::PtreeClassSpec(Ptree* p, Ptree* q, Ptree* c)
: NonLeaf(p, q)
{
    encoded_name = 0;
    comments = c;
}

PtreeClassSpec::PtreeClassSpec(Ptree* car, Ptree* cdr,
		Ptree* c, const char* encode) : NonLeaf(car, cdr)
{
    encoded_name = encode;
    comments = c;
}

int PtreeClassSpec::What()
{
    return ntClassSpec;
}

Ptree* PtreeClassSpec::Translate(AbstractTranslatingWalker* w)
{
    return w->TranslateClassSpec(this);
}

const char* PtreeClassSpec::GetEncodedName()
{
    return encoded_name;
}

Ptree* PtreeClassSpec::GetComments()
{
    return comments;
}

}
