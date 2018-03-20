// by ZBY

#ifndef _ARCH_IRQ_H
#define _ARCH_IRQ_H

#ifndef __ASSEMBLER__

#define local_irq_enable() do { \
while(1);\
} while (0)

#define local_irq_disable() do { \
while(1);\
} while (0)

#define local_irq_save(flags) do { \
(void)flags;while(1);\
} while (0)

#define local_irq_restore(flags) do { \
(void)flags;while(1);\
} while (0)

#endif /* !__ASSEMBLER__ */

#endif /* _ARCH_IRQ_H */
