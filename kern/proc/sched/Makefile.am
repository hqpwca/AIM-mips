# Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
#
# This file is part of AIM.
#
# AIM is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# AIM is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

include $(top_srcdir)/env.am

noinst_LTLIBRARIES = libsched.la
noinst_PROGRAMS =

noinst_PROGRAMS += scheduler.ko
scheduler_ko_SOURCES = scheduler.c
scheduler_ko_LDFLAGS = $(AM_LDFLAGS) -Wl,-r
# Remember to add your module to modules.am
# You don't have to apply configure options to your module.

libsched_la_SOURCES = \
	sched.c

