#include <string.h>

void *memccpy(void *dst, const void *src, int c, size_t count)
{
  char *a = dst;
  const char *b = src;
  while (count--)
  {
    *a++ = *b;
    if (*b==c)
    {
      return (void *)a;
    }
    b++;
  }
  return 0;
}

char *strncpy(char *dest, const char *src, size_t n) {
  memset(dest,0,n);
  memccpy(dest,src,0,n);
  return dest;
}
