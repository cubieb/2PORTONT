# Generated automatically from Makefile.in by configure.
#
#  Makefile for the Squid Object Cache server
#
#  $Id: Makefile,v 1.1.1.1 2003/08/18 05:40:23 kaohj Exp $
#
#  Uncomment and customize the following to suit your needs:
#

prefix		= /usr/squid
exec_prefix	= ${prefix}
exec_suffix	= 
cgi_suffix	= .cgi
top_srcdir	= ..
bindir		= ${exec_prefix}/bin
libexecdir      = ${bindir}
sysconfdir	= ${prefix}/etc
localstatedir   = ${prefix}
srcdir		= .

# Gotta love the DOS legacy
#
SQUID_EXE	= squid$(exec_suffix)
CLIENT_EXE	= client$(exec_suffix)
DNSSERVER_EXE	= dnsserver$(exec_suffix)
UNLINKD_EXE	= unlinkd$(exec_suffix)
PINGER_EXE	= pinger$(exec_suffix)
CACHEMGR_EXE	= cachemgr$(cgi_suffix)
IPCPROG		= ipcprog$(exec_suffix)

DEFAULT_PREFIX		= $(prefix)
DEFAULT_CONFIG_FILE     = $(sysconfdir)/squid.conf
DEFAULT_MIME_TABLE	= $(sysconfdir)/mime.conf
DEFAULT_DNSSERVER       = $(libexecdir)/$(DNSSERVER_EXE)
DEFAULT_CACHE_LOG       = $(localstatedir)/logs/cache.log
DEFAULT_ACCESS_LOG      = $(localstatedir)/logs/access.log
DEFAULT_STORE_LOG       = $(localstatedir)/logs/store.log
DEFAULT_PID_FILE        = /var/run/squid.pid
DEFAULT_SWAP_DIR        = $(localstatedir)/cache
DEFAULT_PINGER		= $(libexecdir)/$(PINGER_EXE)
DEFAULT_UNLINKD		= $(libexecdir)/$(UNLINKD_EXE)
DEFAULT_ICON_DIR	= $(prefix)/icons
DEFAULT_ERROR_DIR	= $(prefix)/errors
DEFAULT_MIB_PATH	= $(prefix)/mib.txt

# CC		= gcc
MAKEDEPEND	= /usr/bin/makedepend
INSTALL		= /usr/bin/install -c
INSTALL_BIN 	= ${INSTALL}
INSTALL_FILE 	= ${INSTALL} -m 644
INSTALL_SUID	= ${INSTALL} -o root -m 4755
# RANLIB		= ranlib
LN_S		= ln -s
PERL            = /usr/bin/perl
CRYPTLIB	= $(LIBCRYPT)
REGEXLIB	= -lregex
PTHREADLIB	= 
SNMPLIB		= 
MALLOCLIB	= 
AC_CFLAGS	=
#LDFLAGS		+= -T $(LDSCRIPT) -g
# XTRA_LIBS	= 
XTRA_LIBS	= $(LIBM)
XTRA_OBJS 	= 
MV		= /bin/mv
RM		= /bin/rm
SHELL		= /bin/sh


INCLUDE		= -I. -I../include -I$(top_srcdir)/include
CFLAGS 	       += $(AC_CFLAGS) $(INCLUDE) $(DEFINES)
SQUID_LIBS	= -L../lib $(REGEXLIB) $(CRYPTLIB) \
		  $(SNMPLIB) $(MALLOCLIB) -lmiscutil $(XTRA_LIBS)
CLIENT_LIBS	= -L../lib -lmiscutil $(XTRA_LIBS)
DNSSERVER_LIBS	= -L../lib -lmiscutil $(XTRA_LIBS)
PINGER_LIBS	= -L../lib -lmiscutil $(XTRA_LIBS)
STD_APP_LIBS    = -L../lib -lmiscutil $(XTRA_LIBS)
HOST_APP_LIBS    = -L../lib -lhostutil -lm

PROGS           = $(SQUID_EXE) $(CLIENT_EXE)
UTILS           = $(DNSSERVER_EXE) $(UNLINKD_EXE) $(IPCPROG)
SUID_UTILS	= $(PINGER_EXE)
CGIPROGS	= $(CACHEMGR_EXE)
OBJS	 	= \
		access_log.o \
		acl.o \
		asn.o \
		 \
		authenticate.o \
		cache_cf.o \
		CacheDigest.o \
		cache_manager.o \
		carp.o \
		cbdata.o \
		client_db.o \
		client_side.o \
		comm.o \
		comm_select.o \
		debug.o \
		 \
		disk.o \
		dns.o \
		dns_internal.o \
		errorpage.o \
		ETag.o \
		event.o \
		fd.o \
		filemap.o \
		forward.o \
		fqdncache.o \
		ftp.o \
		globals.o \
		gopher.o \
		helper.o \
		 \
		http.o \
		HttpStatusLine.o \
		HttpHdrCc.o \
		HttpHdrRange.o \
		HttpHdrContRange.o \
		HttpHeader.o \
		HttpHeaderTools.o \
		HttpBody.o \
		HttpMsg.o \
		HttpReply.o \
		HttpRequest.o \
		icmp.o \
		icp_v2.o \
		icp_v3.o \
		ident.o \
		internal.o \
		ipc.o \
		ipcache.o \
		 \
		main.o \
		mem.o \
		MemPool.o \
		MemBuf.o \
		mime.o \
		multicast.o \
		neighbors.o \
		net_db.o \
		Packer.o \
		pconn.o \
		peer_digest.o \
		peer_select.o \
		pump.o \
		redirect.o \
		refresh.o \
		send-announce.o \
		 \
		ssl.o \
		stat.o \
		StatHist.o \
		String.o \
		stmem.o \
		store.o \
		store_io.o \
		store_io_ufs.o \
		store_client.o \
		store_digest.o \
		store_dir.o \
		store_dir_ufs.o \
		store_key_md5.o \
		store_log.o \
		store_rebuild.o \
		store_swapin.o \
		store_swapmeta.o \
		store_swapout.o \
		string_arrays.o \
		tools.o \
		unlinkd.o \
		url.o \
		urn.o \
		useragent.o \
		wais.o \
		wccp.o \
		whois.o \
		$(XTRA_OBJS)

SNMP_OBJS	= \
		snmp_core.o \
		snmp_agent.o

HTCP_OBJS	= htcp.o

DELAY_OBJS	= delay_pools.o

ASYNC_OBJS	= \
		aiops.o \
		async_io.o \
		store_io_asyncufs.o

LEAKFINDER_OBJS	= \
		leakfinder.o

DEFAULTS        = \
	-DDEFAULT_CONFIG_FILE=\"$(DEFAULT_CONFIG_FILE)\"
FLTFLAGS += -s 1000000

all:    squid.conf $(PROGS) $(UTILS) $(SUID_UTILS) $(CGIPROGS)

$(OBJS): $(top_srcdir)/include/version.h

$(SNMP_OBJS): ../snmplib/libsnmp.a $(top_srcdir)/include/cache_snmp.h

$(SQUID_EXE): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(SQUID_LIBS) $(LDLIBS)

globals.o: globals.c Makefile
	$(CC) -c $< $(CFLAGS) -I$(srcdir) $(DEFAULTS)

globals.c: globals.h mk-globals-c.pl
	$(PERL) $(srcdir)/mk-globals-c.pl < $(srcdir)/globals.h > $@

string_arrays.c: enums.h mk-string-arrays.pl
	$(PERL) $(srcdir)/mk-string-arrays.pl < $(srcdir)/enums.h > $@

$(CLIENT_EXE): client.o
	$(CC) $(LDFLAGS) -o $@ client.o $(CLIENT_LIBS) $(LDLIBS)

$(DNSSERVER_EXE): dnsserver.o
	$(CC) $(LDFLAGS) -o $@ dnsserver.o $(DNSSERVER_LIBS) $(LDLIBS)

$(CACHEMGR_EXE): cachemgr.o
	$(CC) $(LDFLAGS) -o $@ cachemgr.o $(CLIENT_LIBS) $(LDLIBS)

$(PINGER_EXE): pinger.o debug.o globals.o
	$(CC) $(LDFLAGS) -o $@ pinger.o debug.o globals.o $(PINGER_LIBS) $(LDLIBS)

$(UNLINKD_EXE): unlinkd-daemon.o
	$(CC) $(LDFLAGS) -o $@ unlinkd-daemon.o $(LDLIBS)

unlinkd-daemon.o: unlinkd.c
	$(CC) -c $(CFLAGS) -DUNLINK_DAEMON $(srcdir)/unlinkd.c -o $@

$(IPCPROG): ipcprog.o
	$(CC) $(LDFLAGS) -o $@ ipcprog.o $(LDLIBS)

ipcprog.o: ipcprog.c
	$(CC) -c $(CFLAGS) $(srcdir)/ipcprog.c -o $@

cache_diff: cache_diff.o debug.o globals.o store_key_md5.o
	$(CC) $(LDFLAGS) -o $@ debug.o globals.o store_key_md5.o $(STD_APP_LIBS) $(LDLIBS)

test_cache_digest: test_cache_digest.o CacheDigest.o debug.o globals.o store_key_md5.o
	$(CC) $(LDFLAGS) -o $@ CacheDigest.o debug.o globals.o store_key_md5.o $(STD_APP_LIBS) $(LDLIBS)

cache_cf.o: cf_parser.c

squid.conf cf_parser.c: cf.data cf_gen
	./cf_gen cf.data

cf_gen: cf_gen.c
	gcc -o $@ $(AC_CFLAGS) $(INCLUDE) $(DEFINES) cf_gen.c $(HOST_APP_LIBS)

cf.data: cf.data.pre Makefile
	sed "\
	s%@DEFAULT_MIME_TABLE@%$(DEFAULT_MIME_TABLE)%g;\
	s%@DEFAULT_DNSSERVER@%$(DEFAULT_DNSSERVER)%g;\
	s%@DEFAULT_UNLINKD@%$(DEFAULT_UNLINKD)%g;\
	s%@DEFAULT_PINGER@%$(DEFAULT_PINGER)%g;\
	s%@DEFAULT_CACHE_LOG@%$(DEFAULT_CACHE_LOG)%g;\
	s%@DEFAULT_ACCESS_LOG@%$(DEFAULT_ACCESS_LOG)%g;\
	s%@DEFAULT_STORE_LOG@%$(DEFAULT_STORE_LOG)%g;\
	s%@DEFAULT_PID_FILE@%$(DEFAULT_PID_FILE)%g;\
	s%@DEFAULT_SWAP_DIR@%$(DEFAULT_SWAP_DIR)%g;\
	s%@DEFAULT_ICON_DIR@%$(DEFAULT_ICON_DIR)%g;\
	s%@DEFAULT_MIB_PATH@%$(DEFAULT_MIB_PATH)%g;\
	s%@DEFAULT_ERROR_DIR@%$(DEFAULT_ERROR_DIR)%g;\
	s%@DEFAULT_PREFIX@%$(DEFAULT_PREFIX)%g;"\
	< $(srcdir)/cf.data.pre >$@

install-mkdirs:
	-@if test ! -d $(ROMFSDIR)/$(prefix); then \
		echo "mkdir $(prefix)"; \
		mkdir $(ROMFSDIR)/$(prefix); \
	fi
	-@if test ! -d $(ROMFSDIR)/$(exec_prefix); then \
		echo "mkdir $(exec_prefix)"; \
		mkdir $(ROMFSDIR)/$(exec_prefix); \
	fi
	-@if test ! -d $(ROMFSDIR)/$(bindir); then \
		echo "mkdir $(bindir)"; \
		mkdir $(ROMFSDIR)/$(bindir); \
	fi
	-@if test ! -d $(ROMFSDIR)/$(libexecdir); then \
		echo "mkdir $(libexecdir)"; \
		mkdir $(ROMFSDIR)/$(libexecdir); \
	fi
	-@if test ! -d $(ROMFSDIR)/$(sysconfdir); then \
		echo "mkdir $(sysconfdir)"; \
		mkdir $(ROMFSDIR)/$(sysconfdir); \
	fi
	-@if test ! -d $(ROMFSDIR)/$(localstatedir); then \
		echo "mkdir $(localstatedir)"; \
		mkdir $(ROMFSDIR)/$(localstatedir); \
	fi
	-@if test ! -d $(ROMFSDIR)/$(localstatedir)/logs; then \
		echo "mkdir $(localstatedir)/log"; \
		mkdir $(ROMFSDIR)/$(localstatedir)/logs; \
	fi

# Michael Lupp <mike@nemesis.saar.de> wants to know about additions
# to the install target.
install: all install-mkdirs
	@for f in $(PROGS); do \
		if test -f $(ROMFSDIR)/$(bindir)/$$f; then \
			echo $(MV) $(bindir)/$$f $(bindir)/-$$f; \
			$(MV) $(ROMFSDIR)/$(bindir)/$$f $(ROMFSDIR)/$(bindir)/-$$f; \
		fi; \
		echo $(INSTALL_BIN) $$f $$(bindir); \
		$(INSTALL_BIN) $$f $(ROMFSDIR)/$(bindir); \
		if test -f $(ROMFSDIR)/$(bindir)/-$$f; then \
			echo $(RM) -f $(bindir)/-$$f; \
			$(RM) -f $(ROMFSDIR)/$(bindir)/-$$f; \
		fi; \
	done
	@for f in $(UTILS); do \
		if test -f $(ROMFSDIR)/$(libexecdir)/$$f; then \
			echo $(MV) $(libexecdir)/$$f $(libexecdir)/-$$f; \
			$(MV) $(ROMFSDIR)/$(libexecdir)/$$f $(ROMFSDIR)/$(libexecdir)/-$$f; \
		fi; \
		echo $(INSTALL_BIN) $$f $(libexecdir); \
		$(INSTALL_BIN) $$f $(ROMFSDIR)/$(libexecdir); \
		if test -f $(ROMFSDIR)/$(libexecdir)/-$$f; then \
			echo $(RM) -f $(libexecdir)/-$$f; \
			$(RM) -f $(ROMFSDIR)/$(libexecdir)/-$$f; \
		fi; \
	done
	@for f in $(CGIPROGS); do \
		if test -f $(ROMFSDIR)/$(libexecdir)/$$f; then \
			echo $(MV) $(libexecdir)/$$f $(libexecdir)/-$$f; \
			$(MV) $(ROMFSDIR)/$(libexecdir)/$$f $(ROMFSDIR)/$(libexecdir)/-$$f; \
		fi; \
		echo $(INSTALL_BIN) $$f $(libexecdir); \
		$(INSTALL_BIN) $$f $(ROMFSDIR)/$(libexecdir); \
		if test -f $(ROMFSDIR)/$(libexecdir)/-$$f; then \
			echo $(RM) -f $(libexecdir)/-$$f; \
			$(RM) -f $(ROMFSDIR)/$(libexecdir)/-$$f; \
		fi; \
	done
	$(INSTALL_FILE) $(srcdir)/mib.txt $(ROMFSDIR)$(DEFAULT_MIB_PATH)
	echo "$(INSTALL_FILE) squid.conf $(DEFAULT_CONFIG_FILE)";
	$(INSTALL_FILE) squid.conf $(ROMFSDIR)$(sysconfdir)/squid.conf;
	echo "$(INSTALL_FILE) $(srcdir)/mime.conf $(DEFAULT_MIME_TABLE)";
	$(INSTALL_FILE) $(srcdir)/mime.conf $(ROMFSDIR)$(DEFAULT_MIME_TABLE);

install-pinger:
	@f=pinger; \
	if test -f $(ROMFSDIR)/$(libexecdir)/$$f; then \
		echo $(MV) $(libexecdir)/$$f $(libexecdir)/-$$f; \
		$(MV) $(ROMFSDIR)/$(libexecdir)/$$f $(ROMFSDIR)/$(libexecdir)/-$$f; \
	fi; \
	echo $(INSTALL_SUID) $$f $(libexecdir); \
	$(INSTALL_SUID) $$f $(ROMFSDIR)/$(libexecdir) || exit 1; \
	if test -f $(ROMFSDIR)/$(libexecdir)/-$$f; then \
		echo $(RM) -f $(libexecdir)/-$$f; \
		$(RM) -f $(ROMFSDIR)/$(libexecdir)/-$$f; \
	fi

clean: 
	-rm -rf *.o *pure_* core $(PROGS) $(UTILS) $(CGIPROGS) $(SUID_UTILS)
	-rm -f globals.c string_arrays.c
	-touch cf_gen.o
	-touch cf_gen
# The last two lines were:
#	-rm -f cf_gen cf_parser.c cf.data globals.c string_arrays.c
# Removed this to preserve cf_gen's stuff which doesn't cross compile at all well.

distclean:	clean
	-rm -f Makefile squid.conf squid.conf.pre
	-rm -f Makefile.bak

tags:
	ctags *.[ch] ../include/*.h ../lib/*.[ch]

depend:
	$(MAKEDEPEND) -I../include -I. -fMakefile *.c
