#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <libc/string.h>

// copied from dietlibc

void *
memcpy (void *dst, const void *src, size_t n)
{
    void           *res = dst;
    unsigned char  *c1, *c2;
    c1 = (unsigned char *) dst;
    c2 = (unsigned char *) src;
    while (n--) *c1++ = *c2++;
    return (res);
}
void* memset(void * dst, int s, size_t count) {
    register char * a = dst;
    count++;	/* this actually creates smaller code than using count-- */
    while (--count)
	*a++ = s;
    return dst;
}
