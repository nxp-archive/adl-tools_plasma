//
// Tests to make sure that we signal an error when an identifier
// is not found.
//

int xx;
extern int yy;

int main(int argc,char **argv) {
  int a;
  {
    int c;
    {
      yy = 10;
      b = 1;
      a = 2;
    }
  }
}
