# -- updatedd: configure.ac --
#
# Copyright (C) 2002, 2003, 2004, 2005 Philipp Benner
#
# This file is part of UpdateDD - http://updatedd.philipp-benner.de.
#
# UpdateDD is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# any later version.
#
# UpdateDD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with UpdateDD; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

AC_INIT([UpdateDD], [2.5], [updatedd@philipp-benner.de])
AC_CONFIG_SRCDIR(libexception_handle/libexception_handle.c)
AC_CONFIG_HEADER(config.h:config.h.in)
AC_PROG_CC
AC_PROG_INSTALL

OUTPUT="Makefile libexception_handle/Makefile src/Makefile
	src/plugins/Makefile scripts/Makefile Documentation/Makefile
	version.h"


# ------------------] LIBRARIES [

AC_CHECK_LIB(ucb,		sprintf)
AC_CHECK_LIB(socket,		connect)
AC_CHECK_LIB(resolv,		gethostbyname,,
				AC_CHECK_LIB(resolv, hstrerror) # Solaris
)
AC_CHECK_LIB(nsl,		gethostbyname)
AC_CHECK_LIB(c,			dlopen)
AC_CHECK_LIB(dl,		dlopen) # Linux
AC_CHECK_LIB(gnugetopt,		getopt_long) # FreeBSD

# ------------------] DEBUG [

AC_ARG_ENABLE(debug,
    [  --enable-debug          enable debug (no default)])
if test "$enable_debug" = "yes"; then
	CFLAGS="$CFLAGS -g -ggdb"
	AC_DEFINE(DEBUG)
fi

# ------------------] DEFINES [
#

AC_DEFINE(_GNU_SOURCE)
AC_DEFINE(LIBPATH, [$libdir/updatedd])

# ------------------] HEADERS [
#

HEADERS_TO_CHECK="stdarg.h dlfcn.h dirent.h netinet/in.h sys/ioctl.h
		 arpa/inet.h netdb.h syslog.h signal.h"

AC_CHECK_HEADERS($HEADERS_TO_CHECK,,
	AC_MSG_ERROR([this header is required]) )

AC_CHECK_HEADER(net/if.h,,
	AC_MSG_ERROR([this header is required]),
	[[#include <sys/types.h>
	  #include <sys/socket.h>]] )

AC_CHECK_HEADER(error.h, AC_DEFINE(HAVE_ERROR_H)) # OpenBSD

# ------------------] FUNCTIONS [
#

AC_CHECK_FUNCS(getopt_long,,
	AC_MSG_ERROR([this function is required]) )
AC_CHECK_FUNCS(dprintf)

# ------------------] CFLAGS [
#

CFLAGS="-Wall -Wwrite-strings -Wstrict-prototypes -Wno-trigraphs $CFLAGS"

# ------------------] PERL [
#

AC_PATH_PROG(PERL, perl, "no")

SUBDIRS="libexception_handle src src/plugins Documentation"
if test "$PERL" != "no"; then
	SUBDIRS="$SUBDIRS scripts updatedd-wrapper"
	OUTPUT="$OUTPUT updatedd-wrapper/Makefile
		updatedd-wrapper/create-config
		updatedd-wrapper/updatedd-wrapper
		scripts/interface.pl
		scripts/ipserv.pl"
fi

AC_SUBST(SUBDIRS)

# ------------------] AC_OUTPUT [
#

AC_OUTPUT([$OUTPUT])

# ------------------] MESSAGE [
#

if test "$PERL" = "no"; then
cat << EOF

WARNING: Perl is not installed!
-> UpdateDD-Wrapper will not be installed, which
   provides support for configuration files and
   web-based ip-detection.
EOF

else

chmod 755 updatedd-wrapper/create-config
chmod 755 updatedd-wrapper/updatedd-wrapper
chmod 755 scripts/ipserv.pl
chmod 755 scripts/interface.pl

fi

cat << EOF

	now run: make
		 make install

	BSD users have to use gmake.

EOF
