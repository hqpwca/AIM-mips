// by ZBY

#ifndef _ARCH_TRAP_H
#define _ARCH_TRAP_H

#define SIZEOF_TRAPFRAME (8*32+8*5)
#define REGBYTES 8
# define STORE    sd
# define LOAD     ld

#ifndef __ASSEMBLER__

struct trapframe {
    union {
        struct {
            uint64_t reg[32];
        };
        struct {
            uint64_t zero;
            uint64_t ra;
            uint64_t sp;
            uint64_t gp;
            uint64_t tp;
            uint64_t t0;
            uint64_t t1;
            uint64_t t2;
            union {
                uint64_t s0;
                uint64_t fp;
            };
            uint64_t s1;
            uint64_t a0;
            uint64_t a1;
            uint64_t a2;
            uint64_t a3;
            uint64_t a4;
            uint64_t a5;
            uint64_t a6;
            uint64_t a7;
            uint64_t s2;
            uint64_t s3;
            uint64_t s4;
            uint64_t s5;
            uint64_t s6;
            uint64_t s7;
            uint64_t s8;
            uint64_t s9;
            uint64_t s10;
            uint64_t s11;
            uint64_t t3;
            uint64_t t4;
            uint64_t t5;
            uint64_t t6;
        };
    };
    uint64_t sscratch; // off:8*32 zero if from kernel, non-zero(kernel stack) if from user
    uint64_t sepc;     // off:8*33
    uint64_t scause;   // off:8*34
    uint64_t stval;    // off:8*35
    union {
        uint64_t raw;
        struct {
            uint64_t UIE  : 1;
            uint64_t SIE  : 1;
            uint64_t      : 2;
            uint64_t UPIE : 1;
            uint64_t SPIE : 1;
            uint64_t      : 2;
            uint64_t SPP  : 1;
            uint64_t      : 4;
            uint64_t FS   : 2;
            uint64_t XS   : 2;
            uint64_t      : 1;
            uint64_t SUM  : 1;
            uint64_t MXR  : 1;
            uint64_t      : 12;
            uint64_t UXL  : 2;
            uint64_t      : 64-35;
            uint64_t SD   : 1;
        };
    } sstatus;
};
static_assert(sizeof(struct trapframe)==SIZEOF_TRAPFRAME, "trapframe");


extern void trap_entry(void);


#endif	/* !__ASSEMBLER__ */

#endif /* _ARCH_TRAP_H */
