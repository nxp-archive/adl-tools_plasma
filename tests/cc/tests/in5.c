//
// Simple test of symbol table printing.
//

int xx;
extern int yy;

int main(int argc,char **argv) {
  int a;
  {
    int b;
    int c;
    {
      yy = 10;
      b = 1;
      a = 2;
    }
  }
}
