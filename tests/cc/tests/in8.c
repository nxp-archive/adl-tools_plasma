//
// Make sure that assignment operators require a
// valid lvalue.
//

int foo ()
{
  1 += 2;
  return 0;
}
