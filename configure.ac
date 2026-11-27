dnl Process this file with autoconf to produce a configure script.
dnl GDAL stuff largely copied from rgdal and sf, (c) Roger Bivand and Edzer Pebesma
dnl
dnl rlas only uses GDAL's virtual file system (cpl_vsi.h) to read remote files, so it
dnl links GDAL alone: no PROJ, GEOS or sqlite3 of its own.
AC_INIT
AC_CONFIG_SRCDIR([src/readLAS.cpp])

: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
  echo "could not determine R_HOME"
  exit 1
fi

RBIN="${R_HOME}/bin/R"
CXX=`"${RBIN}" CMD config CXX`

case "${CXX}" in
  em++*) CXXFLAGS="${CXXFLAGS} -fwasm-exceptions" ;;
esac

# pick all flags for testing from R
: ${CC=`"${RBIN}" CMD config CC`}
: ${CFLAGS=`"${RBIN}" CMD config CFLAGS`}
: ${CPPFLAGS=`"${RBIN}" CMD config CPPFLAGS`}
: ${CXXFLAGS=`"${RBIN}" CMD config CXXFLAGS`}
: ${LDFLAGS=`"${RBIN}" CMD config LDFLAGS`}
AC_MSG_NOTICE([CC: ${CC}])
AC_MSG_NOTICE([CXX: ${CXX}])

GENERIC_INSTALL_MESSAGE="
*** Installing this package from source requires the prior
*** installation of GDAL, see for details
*** https://r-spatial.github.io/sf/#installing"

#
# GDAL
#

GDAL_CONFIG="gdal-config"
GDAL_CONFIG_SET="no"

AC_ARG_WITH([gdal-config],
    AS_HELP_STRING([--with-gdal-config=GDAL_CONFIG],[the location of gdal-config]),
           [gdal_config=$withval])
if test [ -n "$gdal_config" ] ; then
    GDAL_CONFIG_SET="yes"
    AC_SUBST([GDAL_CONFIG],["${gdal_config}"])
    AC_MSG_NOTICE(gdal-config set to $GDAL_CONFIG)
fi

if test ["$GDAL_CONFIG_SET" = "no"] ; then
  AC_PATH_PROG([GDAL_CONFIG], ["$GDAL_CONFIG"],["no"])
  if test ["$GDAL_CONFIG" = "no"] ; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([gdal-config not found or not executable. ${GENERIC_INSTALL_MESSAGE}])
  fi
else
  AC_MSG_CHECKING(gdal-config exists)
  if test -r "${GDAL_CONFIG}"; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([gdal-config not found - configure argument error. ${GENERIC_INSTALL_MESSAGE}])
  fi
  AC_MSG_CHECKING(gdal-config executable)
  if test -x "${GDAL_CONFIG}"; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([gdal-config not executable. ${GENERIC_INSTALL_MESSAGE}])
  fi
fi

AC_MSG_CHECKING(gdal-config usability)
if test `${GDAL_CONFIG} --version`;
then
	GDAL_CPPFLAGS=`${GDAL_CONFIG} --cflags`
	GDAL_VERSION=`${GDAL_CONFIG} --version`
	GDAL_LIBS=`${GDAL_CONFIG} --libs`
	GDAL_DEP_LIBS=`${GDAL_CONFIG} --dep-libs`
	AC_MSG_RESULT(yes)
else
	AC_MSG_ERROR([gdal-config not found. ${GENERIC_INSTALL_MESSAGE}])
fi

AC_MSG_NOTICE([GDAL: ${GDAL_VERSION}])
AC_MSG_CHECKING([GDAL version >= 2.0.1])

GDAL_MAJ_VER=`echo $GDAL_VERSION | cut -d "." -f1`
if test ${GDAL_MAJ_VER} -lt 2 ; then
  AC_MSG_RESULT(no)
  AC_MSG_ERROR([rlas is not compatible with GDAL versions below 2.0.1])
else
   AC_MSG_RESULT(yes)
fi

if test "${GDAL_VERSION}" = "3.6.0" ; then
  AC_MSG_ERROR([GDAL version 3.6.0 has been withdrawn, please update GDAL])
fi

INLIBS="${LIBS}"
INCPPFLAGS="${CPPFLAGS}"
INPKG_CPPFLAGS="${PKG_CPPFLAGS}"
INPKG_LIBS="${PKG_LIBS}"

PKG_CPPFLAGS="${INPKG_CPPFLAGS} ${GDAL_CPPFLAGS}"
PKG_LIBS="${INPKG_LIBS} ${GDAL_LIBS}"

# honor PKG_xx overrides
# for CPPFLAGS we will superfluously double R's flags
# since we'll set PKG_CPPFLAGS with this, but that shouldn't hurt
CPPFLAGS="${INCPPFLAGS} ${PKG_CPPFLAGS}"

gdalok=yes
AC_CHECK_HEADERS(cpl_vsi.h,,gdalok=no)
if test "${gdalok}" = no; then
   AC_MSG_ERROR([cpl_vsi.h not found in given locations. ${GENERIC_INSTALL_MESSAGE}])
fi

# Link test with the GDAL functions rlas actually calls

LIBS="${INLIBS} ${PKG_LIBS}"
[cat > gdal_test.cpp <<_EOCONF
#include <cpl_vsi.h>
int main(void) {
  VSIStatBufL st;
  VSIStatL("/vsimem/rlas_configure_test", &st);
  VSILFILE* f = VSIFOpenL("/vsimem/rlas_configure_test", "rb");
  if (f) VSIFCloseL(f);
  return 0;
}
_EOCONF]

AC_MSG_CHECKING(GDAL: linking with --libs only)
${CXX} ${CPPFLAGS} ${CXXFLAGS} -o gdal_test gdal_test.cpp ${LIBS} 2> errors.txt
if test `echo $?` -ne 0 ; then
  gdalok=no
  AC_MSG_RESULT(no)
else
  AC_MSG_RESULT(yes)
fi

if test "${gdalok}" = no; then
  AC_MSG_CHECKING(GDAL: linking with --libs and --dep-libs)
  LIBS="${LIBS} ${GDAL_DEP_LIBS}"
  gdalok=yes
  ${CXX} ${CPPFLAGS} ${CXXFLAGS} -o gdal_test gdal_test.cpp ${LIBS} 2>> errors.txt
  if test `echo $?` -ne 0 ; then
    gdalok=no
    AC_MSG_RESULT(no)
  else
    PKG_LIBS="${PKG_LIBS} ${GDAL_DEP_LIBS}"
    AC_MSG_RESULT(yes)
  fi
fi

if test "${gdalok}" = no; then
   cat errors.txt
   AC_MSG_NOTICE([Install failure: compilation and/or linkage problems.])
   AC_MSG_ERROR([VSIFOpenL not found in libgdal. ${GENERIC_INSTALL_MESSAGE}])
fi

rm -f gdal_test errors.txt gdal_test.cpp

#
# Partial link (macOS)
#
# When GDAL is linked statically (CRAN's macOS builds), libc++ template functions such as
# std::vector<T>::emplace_back exist both in rlas's objects and in the GDAL archives. The
# linker keeps one copy and both sides call it. If one side is built with ASAN and the
# other is not, the container annotations get out of sync (false container-overflow).
# rlas is compiled with -fvisibility=hidden; `ld -r` turns its hidden symbols into local
# symbols, so rlas's copies can no longer be merged with GDAL's: each side keeps its own.

RLAS_LINK_OBJECTS='$(RLAS_OBJECTS)'
case `uname -s` in
  Darwin)
    RLAS_LINK_OBJECTS='rlas_partial.o'
    AC_MSG_NOTICE([macOS: rlas objects are partially linked (ld -r) before the final link])
    ;;
esac

#
# concluding substitution
#
AC_SUBST([PKG_CPPFLAGS])
AC_SUBST([PKG_LIBS])
AC_SUBST([RLAS_LINK_OBJECTS])
AC_MSG_NOTICE([Package CPP flags: ${PKG_CPPFLAGS}])
AC_MSG_NOTICE([Package LIBS: ${PKG_LIBS}])

AC_CONFIG_FILES(src/Makevars)
AC_OUTPUT
