//
// Tests to make sure that duplicate symbols cause an error.
//

int main(int argc,char **argv) {
  int a;
  {
    int b;
    int x;
    int y;
    int z;
    int b;
    {
      yy = 10;
      b = 1;
      a = 2;
    }
  }
}
