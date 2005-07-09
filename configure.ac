# arch-tag: df04c3d3-9a02-453e-a652-db57a2269f13

AC_INIT
AC_CONFIG_SRCDIR([configure.ac])

#put ac helpers into ac-helpers
AC_CONFIG_AUX_DIR([ac-helpers])

#information on package
AM_INIT_AUTOMAKE(gtk-engine-experience, 0.9.6)


#checks for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_YACC

AM_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(EXPERIENCE, gtk+-2.0 >= 2.6.0 gdk-pixbuf-2.0)

AC_C_CONST
AC_TYPE_SIZE_T

AC_SUBST(EXPERIENCE_CFLAGS)
AC_SUBST(EXPERIENCE_LIBS)

GTK_VERSION="`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`"
AC_SUBST(GTK_VERSION)

CFLAGS="-Wall -g -Wmissing-prototypes -Werror"

#If this is byacc, we need some other cflags.
if test "$YACC" = "byacc" ; then
	CFLAGS="$CFLAGS -Wno-parentheses -Dlint"
fi

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
