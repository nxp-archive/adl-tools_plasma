//
// This should cause a parse error.
//
static int fib(int i)
{
  fib_count += 1;
  if (i == 1) {
    return !!!;
  } else {
    if (i == 0) {
      return 0;
    } else {
      return fib(i-1) + fib(i-2);
    }
  }
}
