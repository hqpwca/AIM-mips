#ifndef _ADDRSPACE_H
#define _ADDRSPACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <util.h>

#define USEG 0x00000000
#define KSEG0 0x80000000
#define KSEG1 0xa0000000
#define KSSEG 0xc0000000
#define KSEG3 0xe0000000

#define USEG_END KSEG0

#define IO_CAC_BASE KSEG0
#define IO_UNCAC_BASE KSEG1

#define LOWRAM_TOP 0x10000000

#ifndef __ASSEMBLER__

#define TO_CAC(x) (IO_CAC_BASE + (x))
#define TO_UNCAC(x) (IO_UNCAC_BASE + (x))

#define is_user(x)	((ULCAST(x) >= USEG) && (ULCAST(x) < USEG_END))

static inline unsigned long kva2pa(void *x)
{
	unsigned long a = (unsigned long)x;
	if (a > KSEG1)
		return a - KSEG1;
	else if (a > KSEG0)
		return a - KSEG0;
	else if (a > KSSEG)
		return a - KSSEG + LOWRAM_TOP;
	else
		return (ulong)-1;	/* FIXME should be something like panic() */
}

static inline void *pa2kva(unsigned long x)
{
	if (x > LOWRAM_TOP)
		return (void *)(x - LOWRAM_TOP + KSSEG);
	else
		return (void *)TO_CAC(x);
}

#endif	/* !__ASSEMBLER__ */

#endif
