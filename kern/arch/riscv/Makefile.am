# by ZBY

include $(top_srcdir)/env.am

SUBDIRS = \
	$(PREFIXED_MACH)
DIST_SUBDIRS = \
	mach-riscv-virt

dist_noinst_DATA = vmaim.lds.S

noinst_LTLIBRARIES = \
	libentry.la \
	libarch.la

libentry_la_SOURCES = \
    entry.S

libarch_la_SOURCES = \
    arch_mmu.c \
    arch_smp.c \
    arch_early_init.c

libarch_la_LIBADD = \
	mach-$(MACH)/libmach.la
