//
// Make sure that we get a warning about missing a return statement.
//

int foo ()
{
  if (1) {
    return 5;
  } else {
    int a;
  }
}
