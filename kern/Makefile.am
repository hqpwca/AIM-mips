# Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
#
# This file is part of RAIM.
#
# RAIM is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# RAIM is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

include $(top_srcdir)/env.am
include $(top_srcdir)/modules.am

SUBDIRS = \
	arch \
	dev \
	init \
	lib \
	mm \
	panic \
	proc \
	trap

BUILT_SOURCES = vmraim.lds

noinst_LTLIBRARIES = libkernel.la
noinst_PROGRAMS = vmraim.elf
nodist_noinst_DATA = vmraim.s
CLEANFILES = boot.lds vmraim.s

# This list should always be ordered carefully
KERN_LIBS = \
	arch/$(ARCH)/libentry.la \
	dev/libdev.la \
	init/libearlyinit.la \
	lib/lib.la \
	mm/libmm.la \
	panic/libpanic.la \
	proc/libproc.la \
	trap/libtrap.la \
	arch/$(ARCH)/libarch.la \
	../drivers/libdrivers.la \
	../lib/libc/libc.la

libkernel_la_SOURCES =
libkernel_la_LIBADD = $(KERN_LIBS)

vmraim_elf_SOURCES =
vmraim_elf_LDADD = $(MODULES) libkernel.la
EXTRA_vmraim_elf_DEPENDENCIES = vmraim.lds
vmraim_elf_LDFLAGS = $(AM_LDFLAGS) -T vmraim.lds

# This file has double suffix, makefile rule cannot wildcard him
vmraim.lds: $(srcdir)/arch/$(ARCH)/vmraim.lds.S
	$(AM_V_GEN) $(COMPILE) -E -P $^ -o $@

