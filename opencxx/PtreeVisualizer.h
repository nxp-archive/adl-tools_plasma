#include <iostream>
#include <vector>
#include <string>
#include <typeinfo>
#include <cassert>
#include <opencxx/parser/NonLeaf.h>

namespace Opencxx {

class PtreeVisualizer
{
public:
    PtreeVisualizer(std::ostream& output) 
      : output_(output) 
      , counter_(0)
    {
        output << "digraph Ptree {" << std::endl;
        output << "  node[shape=box,width=.1,height=.1];" << std::endl;
    }
    
   ~PtreeVisualizer()
    {
        output_ << "}" << std::endl;
    }
    
    void Visualize(Opencxx::Ptree* p, Opencxx::Ptree* parent = 0, 
                   const char* label = 0)
    {
        using std::endl;
        using std::string;
        
        if (p == 0)
        {
            output_ << "  ";
            if (parent) {
                assert(label);
                output_ << "_" << (void*)parent 
                        << " -> "
                        << "_null" << counter_
                        << "[taillabel=\"" << label << "\"];" << endl;
            }
            output_ << "  _null" << counter_ << "[shape=point,label=\"\"];" 
                    << endl;
            ++counter_;
            return;
        }
        if (parent)
        {
            assert(label);
            output_ << "  _" << (void*)parent 
                    << " -> "
                    << " _" << (void*)p 
                    << "[taillabel=\"" << label << "\"];"
                    << endl;
        }
        string text;
        if (p->IsLeaf()) {
            char* position = p->GetPosition();
            if (position) {
                text = string(position, position + p->GetLength());
            }
        }
        string demangled(Demangle(typeid(*p).name()));
        output_ << "  _" << (void*)p 
                << "[label=\"" << demangled;
        if (text.length()) {
            output_ << "\\n" << Escaped(text);
        }
        output_ << "\"";
        if (p->IsLeaf()) {
            output_ << ",style=filled,fillcolor=lightgray";
        }
        else {
            if (demangled == "NonLeaf") {
                output_ << ",color=lightgray,fontcolor=lightgray";
            }
            else {
                output_ << ",style=bold";
            }
        }
        output_ << "];" << endl;
        if (! p->IsLeaf())
        {
            Opencxx::NonLeaf* nonLeaf = static_cast<Opencxx::NonLeaf*>(p);
            Visualize(p->Car(), p, "CAR");
            Visualize(p->Cdr(), p, "CDR");
        }
    }
    
private:
    static std::string Escaped(const std::string& s)
    {
        std::vector<char> result;
        for(int i = 0; i < s.size(); ++i)
        {
            switch (s[i])
            {
                case '\n' : 
                case '\"' :
                case '\\' :
                    result.push_back('\\');
                default:    
                    result.push_back(s[i]);
            }
        }
        return std::string(result.begin(), result.end());
    }
    
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
    std::ostream& output_;
    int           counter_;
};

}
