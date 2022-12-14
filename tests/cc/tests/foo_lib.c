/*******************************************************************
 * foo_lib.c
 * Atul Varma - 5/24/2004
 * CS Independent Study
 * $Id$
 *
 * Contains external library functions/variables for foo.c.
 *******************************************************************
*/

/* Test global variable. */
int stuff_count;

/* Test of static function definition, to make sure it
   doesn't conflict with fib() defined in foo.c. */
static int fib()
{
  return stuff_count += 1;
}

/* Increment global variable. */
int increment_stuff_count()
{
  fib();
  return 0;
}
