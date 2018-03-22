# Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
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

# RAIM_SET_ARG([feature], [macro], [desc], [default-value])
AC_DEFUN([RAIM_SET_ARG],
  [
    AC_MSG_CHECKING([$3])
    AC_ARG_WITH([$1], [AS_HELP_STRING([--with-$1=ARG],
    				      [Set $3 (default=$4)])],
		[],
		[with_]m4_bpatsubst([$1], -, _)=$4)
    AC_DEFINE_UNQUOTED([$2], [$with_]m4_bpatsubst([$1], -, _), [$3])
    AC_MSG_RESULT([$with_]m4_bpatsubst([$1], -, _))
  ]
)

# RAIM_SET_ARGSTR([feature], [macro], [desc], [default-value])
AC_DEFUN([RAIM_SET_ARGSTR],
  [
    AC_MSG_CHECKING([$3])
    AC_ARG_WITH([$1], [AS_HELP_STRING([--with-$1=ARG],
    				      [Set $3 (default=$4)])],
		[],
		[with_]m4_bpatsubst([$1], -, _)=$4)
    AC_DEFINE_UNQUOTED([$2], "[$with_]m4_bpatsubst([$1], -, _)", [$3])
    AC_MSG_RESULT([$with_]m4_bpatsubst([$1], -, _))
  ]
)

# RAIM_SUBST_MULTILINE([var], [content])
AC_DEFUN([RAIM_SUBST_MULTILINE],
  [
    AC_SUBST($1, $2)
    AM_SUBST_NOTMAKE($1)
  ]
)

# RAIM_FUNC_ATTRIBUTE([attribute])
AC_DEFUN([RAIM_GCC_FUNC_ATTRIBUTE],
  [
    AS_VAR_PUSHDEF([raim_var], [ax_cv_have_func_attribute_$1])
    AS_VAR_PUSHDEF([raim_defined_var], [raim_defined_attribute_$1])
    AX_GCC_FUNC_ATTRIBUTE($1)
    AS_IF(
      [test xyes = [x]AS_VAR_GET([raim_defined_var])], [],
      [test xyes = [x]AS_VAR_GET([raim_var])], [
        AS_VAR_SET([raim_defined_var], [yes])
	AC_DEFINE_UNQUOTED(
	  [__$1], [__attribute__(($1))],
	  [pseudo-keyword for attribute $1]
	)
      ]
    )
    AS_VAR_POPDEF([raim_defined_var])
    AS_VAR_POPDEF([raim_var])
  ]
)

# RAIM_VAR_ATTRIBUTE([attribute])
AC_DEFUN([RAIM_GCC_VAR_ATTRIBUTE],
  [
    AS_VAR_PUSHDEF([raim_var], [ax_cv_have_var_attribute_$1])
    AS_VAR_PUSHDEF([raim_defined_var], [raim_defined_attribute_$1])
    AX_GCC_VAR_ATTRIBUTE($1)
    AS_IF(
      [test xyes = [x]AS_VAR_GET([raim_defined_var])], [],
      [test xyes = [x]AS_VAR_GET([raim_var])], [
        AS_VAR_SET([raim_defined_var], [yes])
	AC_DEFINE_UNQUOTED(
	  [__$1], [__attribute__(($1))],
	  [pseudo-keyword for attribute $1]
	)
      ]
    )
    AS_VAR_POPDEF([raim_defined_var])
    AS_VAR_POPDEF([raim_var])
  ]
)

# RAIM_HELP_ENABLE([feature], [desc])
AC_DEFUN(
	[RAIM_HELP_ENABLE],[]dnl
[AS_HELP_STRING([--enable-$1], [enable $2])
AS_HELP_STRING([--disable-$1], [disable $2])[]dnl
])
dnl FIXME The indent above looks bad, but don't change it.

# RAIM_ARG_VAR([variable], [desc])
AC_DEFUN([RAIM_ARG_VAR], [
	AC_ARG_VAR([$1], [set $2])
	AC_SUBST([$1])
	AC_DEFINE_UNQUOTED([$1], [$][$1], [$2])
])

# RAIM_ARG_WITH([feature], [macro], [desc], [value_if_empty])
# serves a variable, a substitution, and a macro
AC_DEFUN([RAIM_ARG_WITH], [
	AS_VAR_PUSHDEF([default_var], [with_]m4_bpatsubst([$1], -, _))
	AS_VAR_PUSHDEF([preset_var], [__with_]m4_bpatsubst([$1], -, _))
	AC_ARG_WITH([$1], [AS_HELP_STRING([--with-$1=ARG], [set $3])])
	AS_VAR_IF([default_var], [], [AS_VAR_COPY([default_var], [preset_var])])
	AS_VAR_IF([default_var], [], [AS_VAR_SET([default_var], [$4])])
	AC_DEFINE_UNQUOTED([$2], [$default_var], [$3])
	AC_SUBST([default_var])
	AS_VAR_POPDEF([preset_var])
	AS_VAR_POPDEF([default_var])
])

# RAIM_ARG_ENABLE_FEATURE([feature], [conditional], [desc])
# serves a variable, a substitution, a conditional, and a macro
AC_DEFUN([RAIM_ARG_ENABLE_FEATURE], [
	AS_VAR_PUSHDEF([default_var], [enable_]m4_bpatsubst([$1], -, _))
	AS_VAR_PUSHDEF([preset_var], [__enable_]m4_bpatsubst([$1], -, _))
	AC_ARG_ENABLE([$1], [RAIM_HELP_ENABLE([$1], [$3])])
	AS_VAR_IF([default_var], [], [AS_VAR_COPY([default_var], [preset_var])])
	AS_VAR_IF([default_var], [], [AS_VAR_SET([default_var], [$4])])
	AC_SUBST([default_var])
	AM_CONDITIONAL([$2], [test x$default_var = xyes])
	AS_IF(
	  [test xyes = [x]AS_VAR_GET([default_var])], [
	    AC_DEFINE_UNQUOTED([$2], [], [$3])
	  ]
	)
	AS_VAR_POPDEF([preset_var])
	AS_VAR_POPDEF([default_var])
])

# RAIM_ARG_ENABLE([feature], [conditional], [desc])
# serves a variable, a substitution, and a conditional
AC_DEFUN([RAIM_ARG_ENABLE], [
	AS_VAR_PUSHDEF([default_var], [enable_]m4_bpatsubst([$1], -, _))
	AS_VAR_PUSHDEF([preset_var], [__enable_]m4_bpatsubst([$1], -, _))
	AC_ARG_ENABLE([$1], [RAIM_HELP_ENABLE([$1], [$3])])
	AS_VAR_IF([default_var], [], [AS_VAR_COPY([default_var], [preset_var])])
	AS_VAR_IF([default_var], [], [AS_VAR_SET([default_var], [$4])])
	AC_SUBST([default_var])
	AM_CONDITIONAL([$2], [test x$default_var = xyes])
	AS_VAR_POPDEF([preset_var])
	AS_VAR_POPDEF([default_var])
])

# RAIM_SUPPORT_ARCH([space_separated_list])
# E.g. if ARCH=armv7a, define an automake conditional ARCH_ARMV7A
AC_DEFUN([RAIM_SUPPORT_ARCH], [
	m4_foreach_w([var], [$1], [
		AM_CONDITIONAL([ARCH_]m4_toupper(var), [test x$ARCH = x]var)
	])
])

# RAIM_SUPPORT_MACH([space_separated_list])
# E.g. if MACH=zynq, define an automake conditional MACH_ZYNQ
AC_DEFUN([RAIM_SUPPORT_MACH], [
	m4_foreach_w([var], [$1], [
		AM_CONDITIONAL([MACH_]m4_toupper(var), [test x$MACH = x]var)
	])
])

# RAIM_REQUIRE_COMPILE_FLAG([cflags])
# Require compiler to support all of cflags, or fail immediately.
AC_DEFUN([RAIM_REQUIRE_COMPILE_FLAG], [
	AX_CHECK_COMPILE_FLAG(
		$1,
		[AS_VAR_APPEND([RAIM_CFLAGS], [$1][" "])],
		[echo "cc does not support $1" && exit 1]
	)
])

