// by ZBY

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <aim/boot.h>
#include <util.h>
#include <elf.h>
#include <libc/string.h>
#include <mach-platform.h>

typedef elf64hdr_t Elf_Ehdr;
typedef elf64_phdr_t Elf_Phdr;
#define die(x) do { \
    bputs("ERROR: "); \
    bputs(x); \
    bputs("\n"); \
    SBI_CALL(SBI_SHUTDOWN, 0, 0, 0); \
    while (1); \
} while (0)



void bputc(int c)
{
    sbi_console_putchar(c);
}





// copied from riscv-pk/pk/elf.h
#define IS_ELF(hdr) \
  ((hdr).e_ident[0] == 0x7f && (hdr).e_ident[1] == 'E' && \
   (hdr).e_ident[2] == 'L'  && (hdr).e_ident[3] == 'F')

#define IS_ELF64(hdr) (IS_ELF(hdr) && (hdr).e_ident[4] == 2)




// copied from riscv-pk/machine/encoding.h
# define RISCV_PGLEVEL_BITS 9
#define RISCV_PGSHIFT 12
#define RISCV_PGSIZE (1 << RISCV_PGSHIFT)




// copied from riscv-pk/machine/vm.h
#define MEGAPAGE_SIZE ((uintptr_t)(RISCV_PGSIZE << RISCV_PGLEVEL_BITS))




// modified from riscv-pk/bbl/kernel_elf.c

uintptr_t load_kernel_elf(void* blob, size_t size)
{
  Elf_Ehdr* eh = blob;
  if (sizeof(*eh) > size ||
      !(eh->e_ident[0] == '\177' && eh->e_ident[1] == 'E' &&
        eh->e_ident[2] == 'L'    && eh->e_ident[3] == 'F'))
    goto fail;

  if (IS_ELF64(*eh) != (sizeof(uintptr_t) == 8))
    goto fail;

  size_t phdr_size = eh->e_phnum * sizeof(Elf_Ehdr);
  Elf_Phdr* ph = blob + eh->e_phoff;
  if (eh->e_phoff + phdr_size > size)
    goto fail;

  for (int i = eh->e_phnum - 1; i >= 0; i--) {
    if(ph[i].p_type == PT_LOAD && ph[i].p_memsz) {
      uintptr_t paddr = ph[i].p_paddr;
      bputs("  PADDR ");
      bputh64(paddr);
      bputs(" FILESZ ");
      bputh64(ph[i].p_filesz);
      bputs(" MEMSZ ");
      bputh64(ph[i].p_memsz);
      bputs("\n");
      
      if (ph[i].p_offset + ph[i].p_filesz > size)
        goto fail;
      memcpy((void*)paddr, blob + ph[i].p_offset, ph[i].p_filesz);
      memset((void*)paddr + ph[i].p_filesz, 0, ph[i].p_memsz - ph[i].p_filesz);
    }
  }

  return eh->e_entry;

fail:
    die("failed to load payload");
}


void bootmain(void)
{
    bputs("\n");
    bputs("================= WELCOME TO AIM BOOT-LOADER FOR RISCV-VIRT =================\n");
    bputs("LOADING KERNEL ELF ...\n");
    
    uintptr_t entry = load_kernel_elf(PTRCAST(KERN_ELF_ADDR), (size_t)-1);

    bputs("JUMP TO KERNEL ENTRY ...\n");

    ((void (*)(void))entry)();
    die("entry returned");
}
