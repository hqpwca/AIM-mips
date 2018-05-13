
#include <string.h>

#define __likely(x) (x)
#define __unlikely(x) (x)

size_t strlen(const char *s) {
  register size_t i;
  //if (__unlikely(!s)) return 0;
  for (i=0; __likely(*s); ++s) ++i;
  return i;
}

