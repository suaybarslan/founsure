#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

$PACKAGE = libfounsure
$VERSION =  1.0

LDFLAGS="$LDFLAGS -L/usr/local/lib"

AC_PREREQ([2.69])
AC_INIT([founsure], [1.0], arslans@mef.edu.tr, [], [http://suaybarslan.com/founsure])

AC_CONFIG_SRCDIR([src/main_enc.c])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])


#AM_INIT_AUTOMAKE($PACKAGE, $VERSION) # old - depreceated.
AM_INIT_AUTOMAKE([1.13 -Wall -Wno-extra-portability])


LT_INIT([disable-static])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# AC_PROG_RANLIB
#AC_CHECK_LIB([metis], [METIS_PartGraphKway], [], 
#             [AC_MSG_FAILURE(
#               [You need to have METIS library installed.
#                  metis is available from http://glaros.dtc.umn.edu/gkhome/metis/metis/download])
#             ])


# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h stdint.h stdlib.h stdbool.h string.h sys/time.h unistd.h omp.h])
AC_CHECK_HEADERS([metis.h defs.h rename.h struct.h macros.h proto.h])

AC_SEARCH_LIBS([pow], [m])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
# AX_EXT

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor getcwd gettimeofday memset mkdir sqrt strchr strdup strrchr])

AC_CONFIG_FILES(Makefile src/Makefile man/Makefile)
AC_OUTPUT
