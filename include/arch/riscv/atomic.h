// by ZBY
#ifndef _ATOMIC_H
#define _ATOMIC_H


// FIXME: no atomic support in RISC-V

/* counter += val */
static inline void atomic_add(atomic_t *counter, uint32_t val)
{
	*counter += val;
}

/* counter -= val */
static inline void atomic_sub(atomic_t *counter, uint32_t val)
{
	*counter -= val;
}

/* counter++ */
static inline void atomic_inc(atomic_t *counter)
{
	(*counter)++;
}

/* counter-- */
static inline void atomic_dec(atomic_t *counter)
{
	(*counter)--;
}

static inline void atomic_set_bit(
	unsigned long nr,
	volatile unsigned long *addr)
{
    *addr |= (1UL << nr);
}

static inline void atomic_clear_bit(
	unsigned long nr,
	volatile unsigned long *addr)
{
    *addr &= ~(1UL << nr);
}

#endif

