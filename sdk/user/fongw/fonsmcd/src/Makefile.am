#
# standalone fonsmcd Makefile.am
#
# This file is part of FONUCS. Copyright (C) 2006, 2007, 2008 FON Wireless Ltd.
#

bin_PROGRAMS=fonsmcd fonsmcd_debug
lib_LTLIBRARIES=libfonrsa.la

EXTRA_DIST = @top_srcdir@/client/etc/*

fonsmcd_CFLAGS = -I@top_srcdir@/common -DFONTEST -DFONGW -DNETGEAR

#fonsmcd_LDFLAGS = /usr/lib/libdjbdns.a
fonsmcd_LDFLAGS = $(LDFLAGS)/libdjbdns.a

fonsmcd_LDADD = -ldjbdns -ldaemon -lfonrsa

fonsmcd_SOURCES = \
	@top_srcdir@/client/fonsmcd/fonsmcd.c \
	@top_srcdir@/common/fonsm/log.c \
	@top_srcdir@/common/fonsm/log.h \
	@top_srcdir@/common/fonsm/protocol.c \
	@top_srcdir@/common/fonsm/protocol.h \
	@top_srcdir@/common/poption.c \
	@top_srcdir@/common/poption.h \
	@top_srcdir@/common/base/base16.c \
	@top_srcdir@/common/base/base16.h \
	@top_srcdir@/common/base/base32.c \
	@top_srcdir@/common/base/base32.h \
	@top_srcdir@/common/base/base64.c \
	@top_srcdir@/common/base/base64.h \
	@top_srcdir@/common/base/base85.c \
	@top_srcdir@/common/base/base85.h \
	@top_srcdir@/common/fonsm/urandom.c \
	@top_srcdir@/common/fonsm/urandom.h \
	@top_srcdir@/common/fonsm/aes.c \
	@top_srcdir@/common/fonsm/aes.h

fonsmcd_debug_CFLAGS = -I@top_srcdir@/common -DLOGPRINTF -DFONTEST -DFONERADEBUG -DDEBUG -DFONGW -DNETGEAR

fonsmcd_debug_LDFLAGS = $(fonsmcd_LDFLAGS)

fonsmcd_debug_LDADD = $(fonsmcd_LDADD)

fonsmcd_debug_SOURCES = $(fonsmcd_SOURCES)

libfonrsa_la_CFLAGS = -I@top_srcdir@/common -I@top_srcdir@/fonrsa -DFONTEST

libfonrsa_la_SOURCES = \
	@top_srcdir@/common/fonrsa/bigint.c \
	@top_srcdir@/common/fonrsa/bigint.h \
	@top_srcdir@/common/fonrsa/bigint_impl.h \
	@top_srcdir@/common/fonrsa/rmd160.c \
	@top_srcdir@/common/fonrsa/rmd160.h \
	@top_srcdir@/common/fonrsa/fonrsa.c \
	@top_srcdir@/common/fonrsa/fonrsa.h

