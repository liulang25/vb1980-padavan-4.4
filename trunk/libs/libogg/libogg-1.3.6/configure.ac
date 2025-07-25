dnl Process this file with autoconf to produce a configure script.

AC_INIT([libogg],[1.3.6])

LT_INIT
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/framing.c)

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])

dnl Library versioning

LIB_CURRENT=8
LIB_REVISION=6
LIB_AGE=8
AC_SUBST(LIB_CURRENT)
AC_SUBST(LIB_REVISION)
AC_SUBST(LIB_AGE)

AC_PROG_CC
AM_PROG_CC_C_O

dnl Set some options based on environment

cflags_save="$CFLAGS"
if test -z "$GCC"; then
        case $host in
        *-*-irix*)
                DEBUG="-g -signed"
                CFLAGS="-O2 -w -signed"
                PROFILE="-p -g3 -O2 -signed"
		;;
        sparc-sun-solaris*)
                DEBUG="-v -g"
                CFLAGS="-xO4 -fast -w -fsimple -native -xcg92"
                PROFILE="-v -xpg -g -xO4 -fast -native -fsimple -xcg92 -Dsuncc"
		;;
        *)
                DEBUG="-g"
                CFLAGS="-O"
                PROFILE="-g -p"
		;;
        esac
else
        case $host in
        *-*-linux*)
                DEBUG="-g -Wall -Wextra -fsigned-char"
                CFLAGS="-O2 -Wall -Wextra -ffast-math -fsigned-char"
                PROFILE="-Wall -Wextra -W -pg -g -O2 -ffast-math -fsigned-char"
		;;
        sparc-sun-*)
                DEBUG="-g -Wall -Wextra -fsigned-char"
                CFLAGS="-O2 -ffast-math -fsigned-char"
                PROFILE="-pg -g -O2 -fsigned-char"
		;;
	*-*-darwin*)
		DEBUG="-fno-common -g -Wall -Wextra -fsigned-char"
		CFLAGS="-fno-common -O4 -Wall -Wextra -fsigned-char -ffast-math"
		PROFILE="-fno-common -O4 -Wall -Wextra -pg -g -fsigned-char -ffast-math"
		;;
        *)
                DEBUG="-g -Wall -Wextra -fsigned-char"
                CFLAGS="-O2 -fsigned-char"
                PROFILE="-O2 -g -pg -fsigned-char"
		;;
        esac
fi
CFLAGS="$CFLAGS $cflags_save"
DEBUG="$DEBUG $cflags_save"
PROFILE="$PROFILE $cflags_save"

dnl Checks for programs.

dnl Checks for libraries.

dnl Checks for header files.
INCLUDE_INTTYPES_H=0
INCLUDE_STDINT_H=0
INCLUDE_SYS_TYPES_H=0
AC_CHECK_HEADER(inttypes.h,INCLUDE_INTTYPES_H=1)
AC_CHECK_HEADER(stdint.h,INCLUDE_STDINT_H=1)
AC_CHECK_HEADER(sys/types.h,INCLUDE_SYS_TYPES_H=1)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Check for types

AC_CHECK_SIZEOF(int16_t)
AC_CHECK_SIZEOF(uint16_t)
AC_CHECK_SIZEOF(u_int16_t)
AC_CHECK_SIZEOF(int32_t)
AC_CHECK_SIZEOF(uint32_t)
AC_CHECK_SIZEOF(u_int32_t)
AC_CHECK_SIZEOF(int64_t)
AC_CHECK_SIZEOF(uint64_t)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)

case 2 in
     $ac_cv_sizeof_int16_t) SIZE16="int16_t";;
     $ac_cv_sizeof_short) SIZE16="short";;
     $ac_cv_sizeof_int) SIZE16="int";;
esac

case 2 in
     $ac_cv_sizeof_uint16_t) USIZE16="uint16_t";;
     $ac_cv_sizeof_short) USIZE16="unsigned short";;
     $ac_cv_sizeof_int) USIZE16="unsigned int";;
     $ac_cv_sizeof_u_int16_t) USIZE16="u_int16_t";;
esac

case 4 in
     $ac_cv_sizeof_int32_t) SIZE32="int32_t";;
     $ac_cv_sizeof_short) SIZE32="short";;
     $ac_cv_sizeof_int) SIZE32="int";;
     $ac_cv_sizeof_long) SIZE32="long";;
esac

case 4 in
     $ac_cv_sizeof_uint32_t) USIZE32="uint32_t";;
     $ac_cv_sizeof_short) USIZE32="unsigned short";;
     $ac_cv_sizeof_int) USIZE32="unsigned int";;
     $ac_cv_sizeof_long) USIZE32="unsigned long";;
     $ac_cv_sizeof_u_int32_t) USIZE32="u_int32_t";;
esac

case 8 in
     $ac_cv_sizeof_int64_t) SIZE64="int64_t";;
     $ac_cv_sizeof_int) SIZE64="int";;
     $ac_cv_sizeof_long) SIZE64="long";;
     $ac_cv_sizeof_long_long) SIZE64="long long";;
esac

case 8 in
     $ac_cv_sizeof_uint64_t) USIZE64="uint64_t";;
     $ac_cv_sizeof_unsigned_int) USIZE64="unsigned int";;
     $ac_cv_sizeof_unsigned_long) USIZE64="unsigned long";;
     $ac_cv_sizeof_unsigned_long_long) USIZE64="unsigned long long";;
esac

if test -z "$SIZE16"; then
        AC_MSG_ERROR(No 16 bit type found on this platform!)
fi
if test -z "$USIZE16"; then
        AC_MSG_ERROR(No unsigned 16 bit type found on this platform!)
fi
if test -z "$SIZE32"; then
        AC_MSG_ERROR(No 32 bit type found on this platform!)
fi
if test -z "$USIZE32"; then
        AC_MSG_ERROR(No unsigned 32 bit type found on this platform!)
fi
if test -z "$SIZE64"; then
        AC_MSG_WARN(No 64 bit type found on this platform!)
fi
if test -z "$USIZE64"; then
        AC_MSG_WARN(No unsigned 64 bit type found on this platform!)
fi

AC_ARG_ENABLE([crc],
    [AS_HELP_STRING([--disable-crc],
                    [Disable CRC in the demuxer])],,
    [enable_crc=yes])

AM_CONDITIONAL([DISABLE_CRC], [test "$enable_crc" = "no"])

AS_IF([test "$enable_crc" = "no"],[
  AC_DEFINE([DISABLE_CRC], [1], [Do not build with CRC])
])

dnl Checks for library functions.
AC_FUNC_MEMCMP

AC_ARG_ENABLE(gcc-sanitizers,
    AS_HELP_STRING([--enable-gcc-sanitizers], [Enable GCC sanitizers]),
    [ ac_enable_gcc_sanitizers=$enableval ], [ ac_enable_gcc_sanitizers=no] )

if test "$GCC" && test "x${ac_enable_gcc_sanitizers}" = xyes; then
  CFLAGS="${CFLAGS} -fsanitize=address -fsanitize=undefined -g"
  LDFLAGS="${CFLAGS} -fsanitize=address"

  TEST_ENV="env UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1"
  AC_SUBST(TEST_ENV)
fi

# Check for valgrind
AC_ARG_ENABLE(valgrind-testing,
     AS_HELP_STRING([--enable-valgrind-testing],[enable running of tests inside Valgrind (default disabled)]))
if test "x$enable_valgrind_testing" = "xyes"
then
  AC_PATH_PROG(VALGRIND,valgrind,)
  if test "x$VALGRIND" != "x"
  then
    VALGRIND="$VALGRIND -q --error-exitcode=99 --leak-check=full --show-reachable=yes --num-callers=50"
    TESTS_INFO="Test suite will be run under:
    ${VALGRIND}"
  else
    TESTS_INFO="Type 'make check' to run test suite (Valgrind not found)"
  fi
else
  TESTS_INFO="Type 'make check' to run test suite (Valgrind testing not enabled)"
fi

dnl Make substitutions

AC_SUBST(LIBTOOL_DEPS)
AC_SUBST(INCLUDE_INTTYPES_H)
AC_SUBST(INCLUDE_STDINT_H)
AC_SUBST(INCLUDE_SYS_TYPES_H)
AC_SUBST(SIZE16)
AC_SUBST(USIZE16)
AC_SUBST(SIZE32)
AC_SUBST(USIZE32)
AC_SUBST(SIZE64)
AC_SUBST(USIZE64)
AC_SUBST(OPT)
AC_SUBST(LIBS)
AC_SUBST(DEBUG)
AC_SUBST(CFLAGS)
AC_SUBST(PROFILE)


AC_CONFIG_FILES([
Makefile
src/Makefile
doc/Makefile doc/libogg/Makefile
include/Makefile include/ogg/Makefile include/ogg/config_types.h
libogg.spec
ogg.pc
ogg-uninstalled.pc
])
AC_CONFIG_HEADERS([config.h])

AC_OUTPUT
