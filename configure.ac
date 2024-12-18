dnl
dnl The Webalizer - A web server log file analysis program
dnl Copyright (C) 1997-2013 by Bradford L. Barrett
dnl
dnl configure.in template for The Webalizer Version 2.23
dnl Process this file with autoconf to produce a configure script.
dnl

dnl AC_REVISION([$Id$])
AC_INIT([The Webalizer - A web server log file analysis program],
	m4_esyscmd_s([ver=$(git describe --dirty) && echo "${ver#*[_-]}"]),
	[vovcat@gmail.com],
	[webalizer],
	[https://github.com/vovcat/webalizer/])
dnl m4_warning([AC_PACKAGE_TARNAME = ]AC_PACKAGE_TARNAME)

AC_PREREQ([2.70])
AC_CONFIG_MACRO_DIR([tools/m4])
AC_CONFIG_AUX_DIR([tools/autoconf])
AC_CONFIG_HEADERS([config.h])
dnl AC_PREFIX_DEFAULT([/usr/local/AC_PACKAGE_TARNAME-AC_PACKAGE_VERSION])
AC_CANONICAL_BUILD()
AC_CANONICAL_HOST()
AC_CANONICAL_TARGET()

AM_INIT_AUTOMAKE([1.11 -Wall -Wno-portability no-define silent-rules foreign])
dnl AM_MAINTAINER_MODE()
AM_SILENT_RULES()

dnl Make build non-verbose by default
AM_DEFAULT_VERBOSITY=0

dnl ------------------------------------------
dnl default language (don't change!)
dnl ------------------------------------------

AC_SUBST([DEFAULT_LANG], ["english"])

dnl ------------------------------------------
dnl Checks for required programs.
dnl ------------------------------------------

AC_PROG_CC()
AC_PROG_LN_S()
AC_PROG_INSTALL()
AC_PROG_MAKE_SET()

if test "$ac_cv_prog_cc_c11" = "no"; then
   AC_MSG_ERROR([C11 compiler is required!])
fi

dnl ------------------------------------------
dnl Version info
dnl ------------------------------------------

AC_ARG_VAR([EXTRA_VER], [String appended to version ($EXTRA_VER)])
AC_SUBST([EXTRA_VER])

CONFIGURE_CMD="$0 $ac_configure_args"
AC_SUBST([CONFIGURE_CMD])

# CC version
for ac_option in --version -v -V -qversion; do
  ac_ver=`$CC $ac_option 2>&1 |grep -i -w version |head -1`
  test -n "$ac_ver" && break
done

CCVER="$CC ($ac_ver)"
AC_DEFINE_UNQUOTED([CCVER], ["$CCVER"], [C compiler info])

dnl ------------------------------------------
dnl check for platform specific settings
dnl ------------------------------------------

AC_ARG_ENABLE(static,
  [  --disable-static        Link gd, png statically          [[default=enable]]],
  [], [enable_static=yes])
AC_SUBST([enable_static])

AC_ARG_ENABLE(debug,
  [  --enable-debug          Compile with debugging code      [[default=no]]],
  [], [enable_debug=no])
AC_SUBST([enable_debug])

AC_SYS_LARGEFILE()

AC_C_CHAR_UNSIGNED()
if test "$ac_cv_c_char_unsigned" = "yes"; then
  CFLAGS="-fsigned-char ${CFLAGS}"
fi

AC_CHECK_TYPE(u_int64_t, unsigned long long)
AC_CHECK_DECL(altzone, [OPTS="-DHAVE_ALTZONE ${OPTS}"], [], [#include <time.h>])

dnl ------------------------------------------
dnl these are needed on some platforms
dnl ------------------------------------------

AC_CHECK_LIB(44bsd, main, [LIBS_WEBALIZER="-l44bsd ${LIBS_WEBALIZER}"])
AC_CHECK_LIB(m, main, [LIBS_WEBALIZER="-lm ${LIBS_WEBALIZER}"])
if test "$ac_cv_lib_m_main" = "yes"; then
  AC_CHECK_HEADERS(math.h)
fi
AC_CHECK_HEADERS(getopt.h)

dnl ------------------------------------------
dnl ensure current libz is present - required!
dnl ------------------------------------------

AC_CHECK_LIB(z, main, [LIBS_WEBALIZER="-lz ${LIBS_WEBALIZER}"])
if test "$ac_cv_lib_z_main" != "yes"; then
  AC_MSG_ERROR([z library not found.. please install libz])
fi
AC_CHECK_LIB(z, gzrewind)
if test "$ac_cv_lib_z_gzrewind" != "yes"; then
  AC_MSG_ERROR([Old version of libz found.. please upgrade!])
fi
AC_CHECK_HEADER(zlib.h)
if test "$ac_cv_header_zlib_h" != "yes"; then
  AC_MSG_ERROR([zlib.h header not found.. please install])
fi

dnl ------------------------------------------
dnl BZip2 code specific tests
dnl ------------------------------------------

AC_ARG_ENABLE(bz2,
  [  --enable-bz2            Enable BZip2 decompression code  [[default=no]]],
  [USE_BZIP="${enableval}", USE_BZIP="no"])

AC_ARG_WITH(bz2,
  [  --with-bz2=DIR          Alternate location for bz2 header files],
  [CPPFLAGS="${CPPFLAGS} -I${withval}"])
AC_ARG_WITH(bz2lib,
  [  --with-bz2lib=DIR       Alternate location for bz2 library],
  [LDFLAGS="-L${withval} ${LDFLAGS}"])

if test "${USE_BZIP}" = "yes"; then
  AC_CHECK_LIB(bz2, main, [USE_BZIP="yes"],
    [USE_BZIP="no"; AC_MSG_WARN([libbz2 not found.. bzip2 code will will be disabled!])])
fi

if test "${USE_BZIP}" = "yes"; then
  AC_CHECK_LIB(bz2, BZ2_bzopen, [USE_BZIP="yes"],
    [USE_BZIP="no"; AC_MSG_WARN([Old bz2 library found.. bzip2 code will be disabled!])])
fi

if test "${USE_BZIP}" = "yes"; then
  AC_CHECK_HEADER(bzlib.h, [USE_BZIP="yes"],
    [USE_BZIP="no"; AC_MSG_WARN([bzlib.h not found.. bzip2 code will be disabled!])])
fi

if test "${USE_BZIP}" = "yes"; then
  # we have both the library and the header, proceed...
  OPTS="-DUSE_BZIP ${OPTS}"
  LIBS_WEBALIZER="-lbz2 ${LIBS_WEBALIZER}"
fi

dnl ------------------------------------------
dnl Berkeley DB library location
dnl ------------------------------------------

AC_ARG_WITH(db,
  [  --with-db=DIR           Alternate location for libdb header files],
  [CPPFLAGS="${CPPFLAGS} -I${withval}"])
AC_ARG_WITH(dblib,
  [  --with-dblib=DIR        Alternate location for db library],
  [LDFLAGS="-L${withval} ${LDFLAGS}"])

dnl ------------------------------------------
dnl DNS/GeoDB lookup specific tests
dnl ------------------------------------------

AC_ARG_ENABLE(dns,
  [  --enable-dns            Enable DNS/GeoDB lookup code     [[default=yes]]],
  [USE_DNS="${enableval}"], [USE_DNS="yes"])

if test "${USE_DNS}" = "yes"; then
  AC_CHECK_LIB(db, main, [USE_DNS="yes"],
    [USE_DNS="no"; AC_MSG_WARN([libdb not found.. DNS/GeoDB code disabled!])])
fi

if test "${USE_DNS}" = "yes"; then
  AC_CHECK_HEADER(db.h, [USE_DNS="yes"],
    [USE_DNS="no"; AC_MSG_WARN([db.h not found.. DNS/GeoDB code disabled!])])
fi

if test "${USE_DNS}" = "yes"; then
  dnl we have both library and header.. proceed
  OPTS="-DUSE_DNS ${OPTS}"
  LIBS_WEBALIZER="-ldb ${LIBS_WEBALIZER}"
  LIBS_WCMGR="-ldb ${LIBS_WCMGR}"

  AC_CHECK_FUNC(fdatasync)
  if test "$ac_cv_func_fdatasync" = "no"; then
    AC_CHECK_LIB(rt, fdatasync, [
      LIBS_WEBALIZER="-lrt ${LIBS_WEBALIZER}"
      LIBS_WCMGR="-lrt ${LIBS_WCMGR}"
    ])
  fi

  AC_CHECK_FUNC(socket)
  if test "$ac_cv_func_socket" = "no"; then
    AC_CHECK_LIB(socket, main, [LIBS_WEBALIZER="-lsocket ${LIBS_WEBALIZER}"])
  fi
  AC_CHECK_HEADERS(sys/socket.h)

  AC_CHECK_FUNC(inet_pton)
  if test "$ac_cv_func_inet_pton" = "no"; then
    AC_CHECK_LIB(nsl, [inet_pton], [LIBS_WEBALIZER="-lnsl ${LIBS_WEBALIZER}"])
  fi
fi

AC_ARG_WITH(geodb,
  [  --with-geodb=DIR        Default GeoDB data dir [[DATAROOTDIR/GeoDB]]],
  [GEODB_LOC="${withval}"], [GEODB_LOC="${datarootdir}/GeoDB"])
AC_SUBST([GEODB_LOC])

dnl ------------------------------------------
dnl ensure libpng is present - required!
dnl ------------------------------------------

AC_ARG_WITH(png,
  [  --with-png=DIR          Alternate location for png header files],
  [CPPFLAGS="${CPPFLAGS} -I${withval}"])
AC_ARG_WITH(pnglib,
  [  --with-pnglib=DIR       Alternate location for png library],
  [LDFLAGS="-L${withval} ${LDFLAGS}"])

AC_CHECK_LIB(png, main)
if test "$ac_cv_lib_png_main" != "yes"; then
  AC_MSG_ERROR([png library not found.. please install libpng])
fi

dnl ------------------------------------------
dnl ensure libgd is present - required!
dnl ------------------------------------------

AC_ARG_WITH(gd,
  [  --with-gd=DIR           Alternate location for gd header files],
  [CPPFLAGS="${CPPFLAGS} -I${withval}"])
AC_ARG_WITH(gdlib,
  [  --with-gdlib=DIR        Alternate location for gd library],
  [LDFLAGS="-L${withval} ${LDFLAGS}"])

AC_CHECK_HEADER(gd.h)
if test "$ac_cv_header_gd_h" != "yes"; then
  AC_MSG_ERROR([gd.h header not found.. please install])
fi
AC_CHECK_LIB(gd, main)
if test "$ac_cv_lib_gd_main" != "yes"; then
  AC_MSG_ERROR([gd library not found.. please install libgd])
fi
AC_CHECK_LIB(gd, gdImagePng)
if test "$ac_cv_lib_gd_gdImagePng" != "yes"; then
  AC_MSG_ERROR([Old version of libgd found.. please upgrade!])
fi
AC_CHECK_LIB(gd, gdImageStringFT,
  [LIBS_GD="-lgd -lpng -lfontconfig -lfreetype -lexpat -lbrotlidec -lbrotlicommon"],
  [LIBS_GD="-lgd -lpng"])
if test "$ac_cv_lib_gd_gdImageStringFT" = "yes" ; then
  AC_DEFINE([HAVE_LIBGD_TTF], [1], [Does GD support TTF fonts?])
fi
AC_SUBST([LIBS_GD])

dnl ------------------------------------------
dnl GeoIP code specific tests
dnl ------------------------------------------

AC_ARG_ENABLE(geoip,
  [  --enable-geoip          Enable GeoIP geolocation code    [[default=yes]]],
  [USE_GEOIP="${enableval}"], [USE_GEOIP="yes"])

AC_ARG_WITH(geoip,
  [  --with-geoip=DIR        Alternate location for libGeoIP header files],
  [CPPFLAGS="${CPPFLAGS} -I${withval}"])
AC_ARG_WITH(geoiplib,
  [  --with-geoiplib=DIR     Alternate location for geoip library],
  [LDFLAGS="-L${withval} ${LDFLAGS}"])

if test "${USE_GEOIP}" = "yes"; then
  AC_CHECK_LIB(GeoIP, main, [USE_GEOIP="yes"],
    [USE_GEOIP="no"; AC_MSG_WARN([libGeoIP not found.. GeoIP code will be disabled!])])
fi

if test "${USE_GEOIP}" = "yes"; then
  AC_CHECK_HEADER(GeoIP.h, [USE_GEOIP="yes"],
    [USE_GEOIP="no"; AC_MSG_WARN([GeoIP.h not found.. GeoIP code will be disabled!])])
fi

if test "${USE_GEOIP}" = "yes"; then
  dnl we have both library and header.. proceed
  OPTS="-DUSE_GEOIP ${OPTS}"
  LIBS_WEBALIZER="-lGeoIP ${LIBS_WEBALIZER}"
fi

dnl ------------------------------------------
dnl check which hash function to use
dnl ------------------------------------------

AC_ARG_ENABLE(oldhash,
  [  --enable-oldhash        Use old hash function (slower)   [[default=no]]],
  [OPTS="$OPTS -DUSE_OLDHASH"], [])

dnl ------------------------------------------
dnl language with gettext
dnl ------------------------------------------

dnl AM_GNU_GETTEXT

dnl ------------------------------------------
dnl Use package-specific localedir to decouple HTML output language from
dnl system locale set wich is affected by localepurge
dnl ------------------------------------------

AC_ARG_WITH(pkglocaledir,
  [  --with-pkglocaledir=DIR Default package locale files directory @<:@[DATAROOTDIR/]AC_PACKAGE_TARNAME[/locale]@:>@],
  [pkglocaledir="${withval}"], [pkglocaledir='${datarootdir}/${PACKAGE}/locale'])
AC_SUBST(pkglocaledir)

dnl ------------------------------------------
dnl etcdir variable
dnl ------------------------------------------

AC_ARG_WITH(etcdir,
  [  --with-etcdir=DIR       Default config directory @<:@[SYSCONFDIR/]AC_PACKAGE_TARNAME@:>@],
  [etcdir="${withval}"], [etcdir='${sysconfdir}/${PACKAGE}'])
AC_SUBST(etcdir)

dnl ------------------------------------------
dnl done.. write out our Makefile
dnl ------------------------------------------

# Do not use configure's LIBS, it contains trash
LIBS=

# Move CFLAGS to CFLAGS_SYS
AC_SUBST([CFLAGS_SYS], [$CFLAGS])
CFLAGS=

AC_SUBST([OPTS])
AC_SUBST([LIBS_GD])
AC_SUBST([LIBS_WEBALIZER])
AC_SUBST([LIBS_WCMGR])
dnl AC_SUBST([subst_vars], [`echo $ac_subst_vars`])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT()

AC_MSG_NOTICE([Done. Type 'make' to continue with build.])
