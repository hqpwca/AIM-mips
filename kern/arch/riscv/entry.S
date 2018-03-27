// by ZBY

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

.text

.globl _start
_start:
    li sp, KERN_START
    j master_early_init

.globl high_address_entry
high_address_entry:
    li sp, KERN_BASE
    j master_init

.globl slave_upper_entry
slave_upper_entry:
