// by ZBY

#ifndef _ARCH_TRAP_H
#define _ARCH_TRAP_H

#ifndef __ASSEMBLER__

struct trapframe {
// FIXME
};

extern void trap_entry(void);

#endif	/* !__ASSEMBLER__ */

#endif /* _ARCH_TRAP_H */
