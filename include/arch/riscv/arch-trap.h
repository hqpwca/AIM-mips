// by ZBY

#ifndef _ARCH_TRAP_H
#define _ARCH_TRAP_H

#define SIZEOF_TRAPFRAME (8*32+8*4)
#define REGBYTES 8
# define STORE    sd
# define LOAD     ld

#ifndef __ASSEMBLER__

struct trapframe {
    uint64_t reg[32];
    uint64_t sscratch; // off:8*32 zero if from kernel, non-zero(kernel stack) if from user
    uint64_t sepc;     // off:8*33
    uint64_t scause;   // off:8*34
    uint64_t stval;    // off:8*35
};
static_assert(sizeof(struct trapframe)==SIZEOF_TRAPFRAME, "trapframe");


extern void trap_entry(void);


#endif	/* !__ASSEMBLER__ */

#endif /* _ARCH_TRAP_H */
