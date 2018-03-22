#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <libc/string.h>
#include <raim/boot.h>


void bputs(const char *s)
{
    while (*s) { bputc(*s); s++; }
}

void bputh64(uint64_t x)
{
    const char *dict = "0123456789ABCDEF";
    bputc('0');
    bputc('x');
    int i;
    for (i = 60; i >= 0; i -= 4) {
        bputc(dict[(x >> i) & 0xF]);
    }
}


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
