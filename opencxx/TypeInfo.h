#ifndef guard_opencxx_TypeInfo_h
#define guard_opencxx_TypeInfo_h

//@beginlicenses@
//@license{chiba-tokyo}{}@
//@license{contributors}{}@
//
//  Copyright (C) 1997-2001 Shigeru Chiba, Tokyo Institute of Technology.
//
//  Permission to use, copy, distribute and modify this software and
//  its documentation for any purpose is hereby granted without fee,
//  provided that the above copyright notice appears in all copies and that
//  both that copyright notice and this permission notice appear in
//  supporting documentation.
//
//  Shigeru Chiba makes no representations about the suitability of this
//  software for any purpose.  It is provided "as is" without express or
//  implied warranty.
//
//  -----------------------------------------------------------------
//
//  Permission to use, copy, distribute and modify this software and its  
//  documentation for any purpose is hereby granted without fee, provided that
//  the above copyright notice appears in all copies and that both that copyright
//  notice and this permission notice appear in supporting documentation.
// 
//  Other Contributors (see file AUTHORS) make(s) no representations about the suitability of this
//  software for any purpose. It is provided "as is" without express or implied
//  warranty.
//  
//  Copyright (C)  Other Contributors (see file AUTHORS)
//
//@endlicenses@

#include <opencxx/parser/GC.h>

namespace Opencxx
{

class Ptree;
class Class;
class Environment;
class Bind;

enum TypeInfoId {
    UndefType,
    BuiltInType, ClassType, EnumType, TemplateType,
    PointerType, ReferenceType, PointerToMemberType,
    ArrayType, FunctionType
};

// These are used by TypeInfo::WhatBuiltinType()
//
enum {
    CharType = 1, IntType = 2, ShortType = 4, LongType = 8,
    LongLongType = 16, SignedType = 32, UnsignedType = 64, FloatType = 128,
    DoubleType = 256, LongDoubleType = 512, VoidType = 1024,
    BooleanType = 2048,
    WideCharType = 4096
};

/*
  TypeInfo interprets an encoded type name.  For details of the encoded
  type name, see class Encoding in encoding.h and encoding.cc.
*/
  class TypeInfo : public LightObject {
  public:
    TypeInfo();
    void Unknown();
    void Set(const char*, Environment*);
    void Set(Class*);
    void SetVoid();
    void SetInt();
    void SetMember(Ptree*);
    void SetTypeInfo(Environment *);

    TypeInfoId WhatIs();

    bool IsNoReturnType();

    bool IsConst();
    bool IsVolatile();

    unsigned int IsBuiltInType();
    bool IsFunction();
    bool IsEllipsis();
    bool IsPointerType();
    bool IsReferenceType();
    bool IsArray();
    bool IsPointerToMember();
    bool IsTemplateClass();
    Class* ClassMetaobject();
    bool IsClass(Class*&);
    bool IsEnum();
    bool IsEnum(Ptree*& spec);

    void Dereference() { --refcount; }
    void Dereference(TypeInfo&);
    void Reference() { ++refcount; }
    void Reference(TypeInfo&);
    bool NthArgument(int, TypeInfo&);
    int NumOfArguments();
    bool NthTemplateArgument(int, TypeInfo&);

    Ptree* FullTypeName();
    Ptree* MakePtree(Ptree* = 0);

    const char *Encoding() const { return encode; };

  private:
    static Ptree* GetQualifiedName(Environment*, Ptree*);
    static Ptree* GetQualifiedName2(Class*);
    void Normalize();
    bool ResolveTypedef(Environment*&, const char*&, bool);

    static const char* SkipCv(const char*, Environment*&);
    static const char* SkipName(const char*, Environment*);
    static const char* GetReturnType(const char*, Environment*);
    static const char* SkipType(const char*, Environment*);
    static const char* SkipChunk(const char *);

  private:
    int refcount;
    const char* encode;
    Class* metaobject;
    Environment* env;
  };

}

#endif /* ! guard_opencxx_TypeInfo_h */
