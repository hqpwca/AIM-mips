
#include <string.h>

#define __likely(x) (x)
#define __unlikely(x) (x)

char *strchr(register const char *t, int c) {
  register char ch;

  ch = c;
  for (;;) {
    if (__unlikely(*t == ch)) break; 
    if (__unlikely(!*t)) return 0; 
    ++t;
  }
  return (char*)t;
}

