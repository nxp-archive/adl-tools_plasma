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
#include <string>
#include <vector>
#include <opencxx/parser/Msg.h>
#include <opencxx/parser/Parser.h>
#include <opencxx/parser/ProgramFile.h>
#include <opencxx/parser/ErrorLog.h>
#include <opencxx/parser/Lex.h>
#include <opencxx/parser/Ptree.h>
#include <opencxx/parser/Encoding.h>
    //< :TODO: remove once dependency of Ptree on Encoding is removed

//#include <opencxx/PtreeDumper.h>

using namespace Opencxx;

class MockErrorLog : public ErrorLog
{
public:
    void Report(const Msg& msg) 
    {
        msg.PrintOn(std::cerr);
        std::cerr << std::endl;
    }
};


class NodeIdentity
{
public:
    enum Kind { Nil, Leaf, NonLeaf };
    
    NodeIdentity(Ptree* p) {
        if (p) {
            type_ = Demangle(typeid(*p).name());
            if (p->IsLeaf()) {
                if (p->GetPosition()) {
                    value_ = std::string(p->GetPosition()
                                       , p->GetPosition() + p->GetLength());
                }
                kind_ = Leaf;
            }
            else {
                kind_ = NonLeaf;
            }
        }
        else {
            type_ = "nil";
            kind_ = Nil;
        }
    }
    
    Kind GetKind() const { return kind_; }
    bool IsGenericNonLeaf() const { return type_ == "NonLeaf"; }
    const std::string& GetType() const { return type_; }
    const std::string& GetValue() const { return value_; }
    
    bool operator<(const NodeIdentity& that) const
    {
        if (kind_ < that.kind_) return true;
        if (kind_ > that.kind_) return false;
        if (type_ < that.type_) return true;
        if (type_ > that.type_) return false;
        if (value_ < that.value_) return true;
        return false;
    }

    void PrintOn(std::ostream& os) const
    {
        os << type_;
        if (value_ != "") {
            os << "(\"" << value_ << "\")"; // :TODO: escaped
        }
    }

private:
    /** Kludge, kludge, kludge. But works. */
    static std::string Demangle(const std::string& s)
    {
        using std::string;
        string result(s);
        StripPrefix("N7Opencxx", result);
        StripLeadingDigits(result);
        string::iterator end = result.end();
        assert(end != result.begin());
        --end;
        return string(result.begin(), end);
    }

    static void StripPrefix(const std::string& prefix, std::string& s)
    {
        using std::string;
        string::const_iterator prefIter = prefix.begin();
        string::const_iterator prefEnd  = prefix.end();
        string::iterator iter = s.begin();
        string::iterator end  = s.end();
        
        while (iter != end  &&  prefIter != prefEnd  &&  *iter == *prefIter)
        {
            ++iter;
            ++prefIter;
        }
        
        if (prefIter == prefEnd) {
            s = string(iter, s.end());
        }
    }

    static void StripLeadingDigits(std::string& s)
    {
        using std::string;
        string::iterator iter = s.begin();
        string::iterator end  = s.end();
        while (iter != end  &&  '0' <= *iter &&  *iter <= '9') {
            ++iter;
        }
        s = string(iter, s.end());
    }

private:
    Kind        kind_;
    std::string type_;
    std::string value_;
};

std::ostream& operator<<(std::ostream& os, const NodeIdentity& ni)
{
    ni.PrintOn(os);
    return os;
}

class Topology
{
public:
    static Topology* Create(Ptree* root, std::vector<Ptree*>& queue)
    {
        NodeIdentity identity(root);
        assert(root);
        assert(identity.GetType() != "NonLeaf");
        assert(identity.GetKind() != NodeIdentity::Leaf);
        
        Topology* result = new Topology;
        result->type_ = identity.GetType();
        result->shape_.push_back('R');
        WalkNonLeaves(root->Car(), result->shape_, result->leaves_, queue);
        WalkNonLeaves(root->Cdr(), result->shape_, result->leaves_, queue);
        return result;
    }
    
    static void WalkNonLeaves(
        Ptree* node
      , std::vector<char>& shape
      , std::vector<NodeIdentity>& leaves
      , std::vector<Ptree*>& queue
    )
    {
        if (! node) {
            shape.push_back('.');
        }
        else {
            NodeIdentity identity(node);
            if (identity.GetKind() == NodeIdentity::Leaf) {
                shape.push_back('L');
                leaves.push_back(identity);
            }
            else if (identity.IsGenericNonLeaf()) {
                shape.push_back('*');
                WalkNonLeaves(node->Car(), shape, leaves, queue);
                WalkNonLeaves(node->Cdr(), shape, leaves, queue);
            }
            else {
                shape.push_back('N');
                leaves.push_back(identity);
                queue.push_back(node);
            }
        }
    }
    
    void PrintOn(std::ostream& os) const
    {
        os << type_ << " "
           << std::string(shape_.begin(), shape_.end());
        for(std::vector<NodeIdentity>::const_iterator 
                begin = leaves_.begin()
              , iter  = leaves_.begin()
              , end   = leaves_.end();
            iter != end;
            ++iter)
        {
            os << " " << *iter;
        }
    }

private:
    Topology() {}
        
    std::string               type_;
    std::vector<char>         shape_;
    std::vector<NodeIdentity> leaves_;
};

std::ostream& operator<<(std::ostream& os, const Topology& t)
{
    t.PrintOn(os);
    return os;
}

void Process(Ptree* root)
{
    using std::vector;
    
    vector<Ptree*> queue;
    vector<Topology*> topologies;
    queue.push_back(root);
    while (queue.size())
    {
        Ptree* current = *(queue.rbegin());
        queue.pop_back();
        Topology* t = Topology::Create(current, queue);
        topologies.push_back(t);
    }
    
    for(vector<Topology*>::const_iterator 
            iter = topologies.begin()
          , end  = topologies.end();
        iter != end;
        ++iter)
    {
        std::cout << **iter << std::endl;
    }
}

int main(int ac, char* av[])
{
    using std::string;
    using std::endl;
    if (ac <= 1) {
        std::cerr << 
        "USAGE: dump-ast FILE...\n"
        "\n"
        "  Parses FILEs and dumps topologies rooted in nodes derived from NonLeaf,\n"
        "  one node per line.\n"
        "\n"
        "OUTPUT FORMAT:\n"
        "  First word on the line is node name. Second word is a tree topology\n"
        "  encoded as preorder traversal, where R is root, L is Leaf or\n"
        "  derived, * is NonLeaf, '.' is NULL, and N is a node derived\n"
        "  from NonLeaf. After a space there is a space-separated list\n"
        "  describing L and N nodes shown in topology.\n"
        "\n";
        std::exit(1);
    }

    for(int i = 1; i < ac; ++i)
    {
        MockErrorLog errorLog;
        std::ifstream inputFile(av[i]);
        Opencxx::ProgramFile program(inputFile, av[i]);
        Lex lexer(&program);
        Parser parser(&lexer, errorLog);
        
        Ptree* result;
        bool ok = parser.rProgram(result);
        if (! ok) {
            std::cerr << "ERROR: syntax error" << std::endl;
            std::exit(1);
        }
        
        //Opencxx::PtreeDumper dumper(std::cout);
        //dumper.Dump(result);
        Process(result);
    }
}
