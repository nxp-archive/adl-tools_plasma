#ifndef guard_opencxx_Ptree_h
#define guard_opencxx_Ptree_h

//@beginlicenses@
//@license{chiba_tokyo}{}@
//@license{contributors}{}@
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
//  Other Contributors (see file AUTHORS) make(s) no representations about the suitability of this
//  software for any purpose. It is provided "as is" without express or implied
//  warranty.
//  
//  Copyright (C)  Other Contributors (see file AUTHORS)
//
//@endlicenses@

#include <iosfwd>
#include <opencxx/parser/GC.h>
#include <opencxx/parser/deprecated.h>
#include <opencxx/parser/PtreeUtil.h>

namespace Opencxx
{

  class AbstractTranslatingWalker;
  class AbstractTypingWalker;
  class TypeInfo;

#ifdef __opencxx
  metaclass QuoteClass Ptree;           // get qMake() available
#endif

  class Ptree : public LightObject {
  public:

    virtual ~Ptree() {};

    /** This returns true if the metaobject indicates a lexical token. */
    virtual bool IsLeaf() const = 0;

    /** Prints the metaobject on the </code>os</code> for debugging. Sublists are surrounded by [ and ]. */
    void Display(std::ostream&);
    void Display();

  public:

    /** @todo document */
    virtual void Print(std::ostream&, int, int) = 0;

    /** @todo document */
    int Write(std::ostream&);

    /** Writes the metaobject to the file specified by out. Unlike Display() , sublists are not surrounded by [ and ]. This member function returns the number of written lines. */
    virtual int Write(std::ostream&, int) = 0;

    /** @todo document */
    void PrintIndent(std::ostream&, int);


    /** Converts the parse tree into a character string and returns it. */
    char* ToString();

    /** @todo document */
    virtual void PrintOn(std::ostream&) const = 0;

    /** Returns position in the char buffer when the
        text parsed as <code>this</code> starts.
    */
    const char* GetPosition() { return data.leaf.position; }

    /** Returns the length of the text parsed as <code>this</code>. */
    int GetLength() { return data.leaf.length; }


    /**  */
    Ptree* Car()       { return data.nonleaf.child; }

    /**  */
    const Ptree* Car() const { return data.nonleaf.child; }

    /**  */
    Ptree* Cdr()       { return data.nonleaf.next; }

    /**  */
    const Ptree* Cdr() const { return data.nonleaf.next; }

    Ptree* Cadr() 
    { 
      return Cdr()->Car();
    }

    Ptree* Cddr() 
    { 
      return Cdr()->Cdr();
    }

  public:

    /**  */
    void SetCar(Ptree* p) { data.nonleaf.child = p; }

    /**  */
    void SetCdr(Ptree* p) { data.nonleaf.next = p; }

  public:
    /**  */
    bool IsA(int);

    /** Returns identifier of a node. 
        @see token-names.h */
    virtual int What();

    /** Returns encoded type for nodes that support it. */
    virtual const char* GetEncodedType();

    /** Returns encoded name for nodes that support it. */
    virtual const char* GetEncodedName();

    /** Calls appropriate Translate... method in <code>walker</code>,
        depending on dynamic type of <code>this</code>.
        
        @note This is <i>Visit</i> function of <i>Visitor Pattern</i>.
    */
    virtual Ptree* Translate(AbstractTranslatingWalker* walker);

    /** Calls appropriate Typeof... method in <code>walker</code>
        passing <code>ti</code> to it.
        
        @note This is <i>Visit</i> function of <i>Visitor Pattern</i>.
    */
    virtual void Typeof(AbstractTypingWalker* walker, TypeInfo& ti);

  public:

    /** 
        Makes a Ptree metaobject according to the format. The format is
        a null-terminated string. All occurrences of %c (character), %d
        (integer), %s (character string), and %p (Ptree) in the format are
        replaced with the values following the format. %% in the format is
        replaced with %. 
    */
    static Ptree* Make(const char* pat, ...);

    /** Makes a Ptree metaobject that represents the text. Any C++
        expression surrounded by backquotes can appear in text. Its
        occurrence is replaced with the value denoted by the expression. The
        type of the expression must be Ptree*, int, or char*.
        
        @deprecated Use PtreeUtil 
    */
    static Ptree* qMake(char*);

  public:

    /** Returns true if the metaobject represents a boolean.  We allow 'true'
        and 'false', as well as an unsigned integer.
    */
    bool Reify(bool &value);

    /** Returns true if the metaobject represents an integer constant. The
        denoted value is stored in value. Note that the denoted value is
        always a positive number because a negative number such as -4
        generates two distinct tokens such as - and 4.
    */
    bool Reify(unsigned int& value);

    /** Returns true if the metaobject represents a string literal. A
        string literal is a sequence of character surrounded by double
        quotes ". The denoted null-terminated string is stored in string. It
        does not include the double quotes at the both ends. Also, the
        escape sequences are not expanded.
    */
    bool Reify(char*& string);

  protected:
    union {
      struct {
	    Ptree* child;
	    Ptree* next;
      } nonleaf;
      struct {
	    const char* position;
	    int  length;
      } leaf;
    }data;

    friend class NonLeaf;
  };

  /** The operator << can be used to write a Ptree object to an output stream. It is equivalent to Write() in terms of the result.
   */
  inline std::ostream& operator << (std::ostream& s, Ptree* p)
  {
    p->Write(s);
    return s;
  }

}

#endif /* ! guard_opencxx_Ptree_h */

