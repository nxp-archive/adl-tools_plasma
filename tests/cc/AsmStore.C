//
// Simple class for storing assembly code.
// Rather than having an AST for assembly, we just
// have a list of strings that we print out at the
// end.
//

#include <sstream>
#include <iomanip>
#include <iterator>

#include "AsmStore.h"

using namespace std;

AsmStore::AsmStore(bool print_comments) :
  _print_comments(print_comments)
{
}

void AsmStore::o(const std::string &str,const char *comment)
{
  if (_print_comments && comment) {
    ostringstream ss;
    ss << setw(35) << setiosflags(ios_base::left) << 
      str << "# " << comment;
    _asm.push_back(ss.str());
  } else {
    if (!str.empty()) {
       _asm.push_back(str);
    }
  }
}

// Add code to front.
void AsmStore::splice_front(AsmStore &as)
{
  _asm.splice(_asm.begin(),as._asm,as._asm.begin(),as._asm.end());
}

// Add code to back.
void AsmStore::splice_back(AsmStore &as)
{
  _asm.splice(_asm.end(),as._asm,as._asm.begin(),as._asm.end());
}

void AsmStore::c(const std::string &str,int indent_amt)
{
  if (_print_comments) {
    _asm.push_back("# " + str);
  }
}

// Write data to the specified output stream.
void AsmStore::write(ostream &o) const
{
  ostream_iterator<string> iter(o,"\n");
  copy(_asm.begin(),_asm.end(),iter);
}
