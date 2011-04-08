#include "const.h"
#include "int_to_string.h"

#include <climits>

// This takes a pointer pointing to the character after the end of
// a buffer, prints the number into the tail of the buffer,
// and returns a pointer to the first charachter in the number.
// Make sure your buffer's big enough, this doesn't check.
static char* DoIntToString(unsigned long val, char* bufhead)
{
  // deal with any possible encoding problems
  const char digits[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

  *(--bufhead) = '\0';

  if(val == 0)
    *(--bufhead) = '0';
  else do {
    *(--bufhead) = digits[val % 10];
    val /= 10;
  } while(val);

  return bufhead;
}

static const unsigned long ul_max_digits = (ULONG_MAX >> 32 == 0) ? 10 : 20;

std::string WFMath::IntToString(unsigned long val)
{
  static const unsigned bufsize = ul_max_digits + 1; // add one for \0
  char buffer[bufsize];

  return DoIntToString(val, buffer + bufsize);
}

// Deals with the fact that while, e.g. 0x80000000 (in 32 bit),
// is a valid (negative) signed value, the negative
// of it can only be expressed as an unsigned quantity.
static unsigned long SafeAbs(long val)
{
#if LONG_MAX + LONG_MIN >= 0
  // a signed variable can hold -LONG_MIN, we're completely safe
  return (val >= 0) ? val : -val;
#else
  if(val >= 0)
    return val;
  else if(val >= -LONG_MAX) // -LONG_MAX is a valid signed long
    return -val;
  else // LONG_MAX + val < 0
    return LONG_MAX + (unsigned long) (-(LONG_MAX + val));
#endif
}

std::string WFMath::IntToString(long val)
{
  static const unsigned bufsize = ul_max_digits + 2; // one for \0, one for minus sign
  char buffer[bufsize];

  char* bufhead = DoIntToString(SafeAbs(val), buffer + bufsize);

  if(val < 0)
    *(--bufhead) = '-';

  return bufhead;
}
