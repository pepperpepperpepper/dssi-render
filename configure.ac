AC_INIT(src/dssi-render.c)

LT_INIT

PACKAGE=dssi-render
VERSION=0.0.1

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

PKG_CHECK_MODULES(DSSI, dssi >= 0.9)
PKG_CHECK_MODULES(SNDFILE, sndfile >= 1.0.9)

dnl Require ALSA or libdssialsacompat
PKG_CHECK_MODULES(ALSA, alsa >= 0.9, with_alsa=ALSA, with_alsa=no)
if test "x${with_alsa}" = xno ; then
  PKG_CHECK_MODULES(ALSA, libdssialsacompat, with_alsa=libdssialsacompat,
                    AC_MSG_ERROR([neither ALSA nor libdssialsacompat found]))
fi
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)

AC_SUBST(DSSI_CFLAGS)
AC_SUBST(DSSI_LIBS)

AC_SUBST(SNDFILE_CFLAGS)
AC_SUBST(SNDFILE_LIBS)

AC_PROG_CC

AC_OUTPUT(Makefile src/Makefile src/midi/Makefile)
