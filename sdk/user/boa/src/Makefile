# Generated automatically from Makefile.in by configure.  

#KERNEL_HEADERS=-I$(ROOTDIR)/linux-2.4.x

ifdef CONFIG_IPV6
CFLAGS += -DIPV6
endif 
#CFLAGS += -D_DEBUG_MESSAGE

ifndef UCLINUX_BUILD_USER
LDFLAGS = -LLINUX
LIBCRYPT = -lcrypt
endif

GCC_FLAGS = -Wstrict-prototypes -Wpointer-arith -Wcast-align -Wcast-qual\
  -Wtraditional\
  -Wshadow\
  -Wconversion\
  -Waggregate-return\
  -Wmissing-prototypes\
  -Wnested-externs\
  -Wall

# Add -DNO_DAEMON if you do not want the server to fork upon launch
#LDFLAGS = -lnsl 
#CFLAGS = -O -DLINUX -I$(ROOTDIR)/lib/include -I$(ROOTDIR)/lib/include/sys -I$(ROOTDIR)/$(LINUXDIR)/include -I$(ROOTDIR)/$(LINUXDIR)/include/asm-mips -I$(ROOTDIR)/include -I. -I./rtl865x 

#CFLAGS += -DSERVER_SSL -I$(ROOTDIR)/lib/libssl/include
#SSLLIB =$(ROOTDIR)/lib/libssl/libssl.a $(ROOTDIR)/lib/libssl/libcrypto.a
#LDFLAGS	+= $(SSLLIB) $(IFMODLIB)
MANDIR = /usr/local/man
BINDIR = /usr/local/sbin


###### Merge for VoIP project ######
ifeq ($(CONFIG_USER_RTK_VOIP),y)
-include $(ROOTDIR)/$(LINUXDIR)/.config
ifdef CONFIG_USER_RTK_VOIP
PWD := $(shell pwd)
TOPDIR := $(shell pwd)/../../..
VOIP_APP = $(TOPDIR)/user/rtk_voip
CFLAGS += -DVOIP_SUPPORT -I$(VOIP_APP)/includes -I$(VOIP_APP)/web \
	-I$(PWD) -I$(PWD)/..
VOIPLIB=$(VOIP_APP)/web/libweb_voip.a
VOIPLIB += -L$(VOIP_APP)/voip_manager -lvoip_manager 
endif
################end#################
endif

## --- HTTPS server support --- ##
ifeq ($(CONFIG_USER_BOA_WITH_SSL),y)
# ---- HTTPS with Matrix SSL library ---
ifeq ($(CONFIG_USER_MATRIXSSL),y)
LIBMATRIXSSL_INC_DIR=$(ROOTDIR)/lib/libmatrixssl/
CFLAGS +=  -DLINUX -I$(LIBMATRIXSSL_INC_DIR)
LDFLAGS += -L$(ROOTDIR)/lib/libmatrixssl/
SSLLIB = -lmatrixssl
else
ifeq ($(CONFIG_USER_MATRIXSSL_NEW),y)
LIBMATRIXSSL_INC_DIR=$(ROOTDIR)/user/cwmp-tr069/matrixssl-3-6-2-open
CFLAGS +=  -DLINUX -I$(LIBMATRIXSSL_INC_DIR) -I$(LIBMATRIXSSL_INC_DIR)/matrixssl
LDFLAGS += -L$(ROOTDIR)/lib/libmatrixssl_new/
SSLLIB = -lcore -lpscrypto -lmatrixssl_new
else
LIBSSL_INC_DIR=$(ROOTDIR)/lib/libssl/include/
CFLAGS +=  -DLINUX -I$(LIBSSL_INC_DIR)
SSLLIB = -lssl -lcrypto
endif
endif
endif
# --- End of HTTPS ---

CFLAGS += -fPIC
ifeq ($(CONFIG_RTL_WPS2_SUPPORT),y)
CFLAGS += -DWPS20
endif

ifeq ($(CONFIG_RTK_OMCI_V1), y)
CFLAGS += -I$(APOLLODIR)/sdk/src/app/omci_v1/API -I$(APOLLODIR)/sdk/src/app/omci_v1/GOS/inc
LDFLAGS += -L$(APOLLODIR)/sdk/src/app/omci_v1/LIB
OMCILIB = -lomci_api
endif	#CONFIG_RTK_OMCI_V1

ifeq ($(CONFIG_RTL_WPS_VERSION_USER_CONFIG),y)
CFLAGS += -DWPS_VERSION_CONFIGURABLE
endif

CFLAGS += -DMULTI_LANG_DL=$(MULTI_LANG_DL)

# Change these if necessary

SUBDIRS=LINUX
WEBLIB=LINUX/libwebs.a
MIBLIB=$(LIBMIB)
#MIBLIB=LINUX/libmib.a $(LIBRTK)

# Web file 
ORI_WEB=./web_default
ORI_SUB=$(ORI_WEB)/admin
PARSED_WEB=./web_parse_ml
PARSED_SUB=$(PARSED_WEB)/admin
ifeq ($(CONFIG_USER_RTK_VOIP),y)
VOIP_PARSED_WEB=$(VOIP_APP)/voip_web_parse_ml
endif
ASP_HDR_PATH=$(ROOTDIR)/user/boa/src/LINUX/form_src/asp.h
ASPFLAG=-E -x c -P -C
# Web file end

SOURCES = alias.c boa.c cgi.c config.c get.c hash.c log.c \
    queue.c read.c request.c response.c signals.c util.c \
    cgi_header.c pipe.c nls.c auth.c globals.c \
    asp_page.c port.c msgutil.c
ifdef UCLINUX_BUILD_USER
ifndef CONFIG_LIB_LIBMD5_FORCE
SOURCES += md5.c
endif
else
SOURCES += md5.c
endif

    
PAGE_ADMIN_DEP1 = mtmcode.js share.js acl.asp date.asp fw-macfilter.asp parental-ctrl.asp reboot.asp \
	saveconf.asp status.asp upgrade.asp upgrade2.asp upgrade3.asp toupgrade.asp index2.asp wladvanced.asp \
	wlbasic.asp wlstatbl.asp wlwpa.asp wlactrl.asp wlwds.asp wlsurvey.asp wlwps.asp connlimit.asp wlmultipleap.asp wlstatbl_vap.asp 
# add for 11n wagem logo

PAGE_ADMIN_DEP2 = graphics/del.gif graphics/edit.gif graphics/sagemlogo2.gif graphics/sagemlogo1.gif \
	graphics/topbar.gif menu-images/menu_bar.gif \
	menu-images/menu_corner.gif menu-images/menu_corner_minus.gif \
	menu-images/menu_corner_plus.gif menu-images/menu_folder_closed.gif \
	menu-images/menu_folder_open.gif menu-images/menu_link_default.gif \
	menu-images/menu_pixel.gif menu-images/menu_root.gif \
	menu-images/menu_tee.gif menu-images/menu_tee_minus.gif \
	menu-images/menu_tee_plus.gif
# add for 11n wagem logo

OBJS = y.tab.o lex.yy.o ${SOURCES:.c=.o} timestamp.o

.PHONY: clean mrclean depend all
 
ifdef UCLINUX_BUILD_USER
all:	${OBJS} subdir boa
else
all:	${OBJS} subdir boa webs_link
endif

subdir:
	for x in $(SUBDIRS); do (cd $$x && $(MAKE) ); done

boa.o: $(ROOTDIR)/include/linux/autoconf.h

boa:	${OBJS} $(WEBLIB)
ifeq ($(MULTI_LANG_DL),1)
	${CC} $(LDFLAGS) -o $@ ${OBJS} ${WEBLIB} $(MIBLIB) $(LDLIBS$(LDLIBS-$(@))) $(VOIPLIB) $(OMCILIB) -lpthread $(SSLLIB) $(LIBCRYPT) $(LIBMD5) -ldl
else
	${CC} $(LDFLAGS) -o $@ ${OBJS} ${WEBLIB} $(MIBLIB) $(LDLIBS$(LDLIBS-$(@))) $(VOIPLIB) $(OMCILIB) -lpthread $(SSLLIB) $(LIBCRYPT) $(LIBMD5)
endif
webs_link:
	for i in $(PAGE_ADMIN_DEP1) ; do \
		[ -e web/admin/$$i ] || ln -s ../$$i web/admin/$$i; \
	done
	for i in $(PAGE_ADMIN_DEP2) ; do \
		[ -e web/admin/$$i ] || ln -s ../../$$i web/admin/$$i; \
	done

clean:
	find . -name '*.o' | xargs rm -f
	find . -name '*.so' | xargs rm -f
	rm -rf $(PARSED_WEB) $(VOIP_PARSED_WEB)
	rm -f boa
	for i in $(PAGE_ADMIN_DEP1) $(PAGE_ADMIN_DEP2); do \
		rm -f web/admin/$$i; \
	done
	rm -f 
	for x in $(SUBDIRS); do (cd $$x && $(MAKE) clean ); done
#	rm -f ${OBJS} boa core lex.yy.c y.tab.c y.tab.h rtl865x/*.o LINUX/*.o LINUX/*.a
	
mrclean:	clean
	rm -f config.status config.cache config.h Makefile config.log

#escape.h:	escape.pl
#	perl escape.pl >escape.h

%.d: %.c
	$(CC) -E $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $< >/dev/null
	
depend:
	rm *.o rtl865x/*.o -f
	${CC} $(CFLAGS) -E -MM ${SOURCES} > .depend
	
-include .depend

timestamp.c:	${SOURCES} boa_grammar.y boa_lexer.l
	touch timestamp.c

install:	boa
	install -c -m 755 boa ${BINDIR} 
	install -c -m 755 ../utils/boa_indexer ${BINDIR}
	install -c -m 644 ../docs/boa.1 ${MANDIR}/man1

SOCKET = socket localhost 80
ECHO = /bin/echo

.PHONY: test dne cgi_bin dir normal

test:	dne cgi_bin dir normal

dne:
	${ECHO} 'GET /file_that_does_not_exit HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'GET /file_that_does_not_exit\n' | ${SOCKET}
	${ECHO} 'HEAD /file_that_does_not_exit HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'HEAD /file_that_does_not_exit\n' | ${SOCKET}

cgi_bin:
	${ECHO} 'GET /cgi-bin/cgi-test.cgi HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'GET /cgi-bin/cgi-test.cgi\n' | ${SOCKET}
	${ECHO} 'HEAD /cgi-bin/cgi-test.cgi HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'HEAD /cgi-bin/cgi-test.cgi\n' | ${SOCKET}

DIR = /doc/

dir:
	${ECHO} 'GET ${DIR} HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'GET ${DIR}\n' | ${SOCKET}
	${ECHO} 'HEAD ${DIR} HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'HEAD ${DIR}\n' | ${SOCKET}

NORMAL = /test_file.gz

normal:
	${ECHO} 'GET ${NORMAL} HTTP/1.0\n\n' | ${SOCKET} | wc -c
	${ECHO} 'GET ${NORMAL}\n' | ${SOCKET} | wc -c
	${ECHO} 'HEAD ${NORMAL} HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'HEAD ${NORMAL}' | ${SOCKET}

GZ = /test_file

# Note: to test automatic gunzipping a file named "test_file.gz" must be in
# the document root.

gz:	
	${ECHO} 'GET ${GZ} HTTP/1.0\n\n' | ${SOCKET} | wc -c
	${ECHO} 'GET ${GZ}\n' | ${SOCKET} | wc -c
	${ECHO} 'HEAD ${GZ} HTTP/1.0\n\n' | ${SOCKET}
	${ECHO} 'HEAD ${GZ}\n' | ${SOCKET}


romfs:
	for x in $(SUBDIRS); do make -C $$x $@; done
	
# general web
	if [ ! -d $(PARSED_WEB) ]; then \
		mkdir $(PARSED_WEB); \
	fi
	test -d $(PARSED_WEB) && rm -rf $(PARSED_WEB)

	find $(ORI_WEB) -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/\"/DOUBLE_QUOTE_TMP/g";
	find $(ORI_WEB) -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/'/SINGLE_QUOTE_TMP/g";
	
	cp -R $(ORI_WEB) $(PARSED_WEB)
	-for f in $(ORI_WEB)/*.asp; do \
		name=$$(basename $$f); \
		$(CC) $(ASPFLAG) -include $(ASP_HDR_PATH) $$f > $(PARSED_WEB)/$$name; \
	done
	-for f in $(ORI_SUB)/*.asp; do \
		name=$$(basename $$f); \
		$(CC) $(ASPFLAG) -include $(ASP_HDR_PATH) $$f > $(PARSED_SUB)/$$name; \
	done

	-for f in $(ORI_WEB)/*.js; do \
		name=$$(basename $$f); \
		$(CC) $(ASPFLAG) -include $(ASP_HDR_PATH) $$f > $(PARSED_WEB)/$$name; \
	done
	-for f in $(ORI_SUB)/*.js; do \
		name=$$(basename $$f); \
		$(CC) $(ASPFLAG) -include $(ASP_HDR_PATH) $$f > $(PARSED_SUB)/$$name; \
	done
	
	find $(ORI_WEB) -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/DOUBLE_QUOTE_TMP/\"/g";
	find $(ORI_WEB) -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/SINGLE_QUOTE_TMP/'/g";
	find $(PARSED_WEB) -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/DOUBLE_QUOTE_TMP/\"/g";
	find $(PARSED_WEB) -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/SINGLE_QUOTE_TMP/'/g";
# voip web
ifeq ($(CONFIG_USER_RTK_VOIP),y)
	if [ ! -d $(VOIP_PARSED_WEB) ]; then \
		mkdir $(VOIP_PARSED_WEB); \
	fi
	test -d $(VOIP_PARSED_WEB) && rm -rf $(VOIP_PARSED_WEB)

	find $(VOIP_APP)/web -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/\"/DOUBLE_QUOTE_TMP/g";
	find $(VOIP_APP)/web -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/'/SINGLE_QUOTE_TMP/g";
	
	cp -R $(VOIP_APP)/web $(VOIP_PARSED_WEB)
	-for f in $(VOIP_APP)/web/*.asp; do \
		name=$$(basename $$f); \
		$(CC) $(ASPFLAG) -include $(ASP_HDR_PATH) $$f > $(VOIP_PARSED_WEB)/$$name; \
	done

	-for f in $(VOIP_APP)/web/*.js; do \
		name=$$(basename $$f); \
		$(CC) $(ASPFLAG) -include $(ASP_HDR_PATH) $$f > $(VOIP_PARSED_WEB)/$$name; \
	done
	find $(VOIP_APP)/web -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/DOUBLE_QUOTE_TMP/\"/g";
	find $(VOIP_APP)/web -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/SINGLE_QUOTE_TMP/'/g";
	find $(VOIP_PARSED_WEB) -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/DOUBLE_QUOTE_TMP/\"/g";
	find $(VOIP_PARSED_WEB) -regextype posix-extended -regex '.*[js or asp]' -type f | xargs sed -i "s/SINGLE_QUOTE_TMP/'/g";
endif
	$(ROMFSINST) $(PARSED_WEB)  /home/httpd/web
	
	$(ROMFSINST) /bin/boa
	$(ROMFSINST) /home/httpd/boa.conf
	$(ROMFSINST) -e CONFIG_USER_Y1731 $(PARSED_WEB)/ethoam.asp /home/httpd/web
ifdef CONFIG_USER_RTK_VOIP
	$(ROMFSINST) $(VOIP_PARSED_WEB)/voip_script.js /home/httpd/web
	$(ROMFSINST) $(VOIP_PARSED_WEB)/voip_general.asp /home/httpd/web
	$(ROMFSINST) $(VOIP_PARSED_WEB)/voip_tone.asp /home/httpd/web
	$(ROMFSINST) $(VOIP_PARSED_WEB)/voip_ring.asp /home/httpd/web
	$(ROMFSINST) $(VOIP_PARSED_WEB)/voip_other.asp /home/httpd/web
#	$(ROMFSINST) $(VOIP_PARSED_WEB)/voip_config.asp /home/httpd/web
	$(ROMFSINST) $(VOIP_PARSED_WEB)/voip_fxo.asp /home/httpd/web
	$(ROMFSINST) $(VOIP_PARSED_WEB)/voip_network.asp /home/httpd/web
ifeq ($(CONFIG_RTK_VOIP_IVR),y)
	$(ROMFSINST) $(VOIP_PARSED_WEB)/voip_ivr_req.asp /home/httpd/web
endif
	$(ROMFSINST) $(VOIP_PARSED_WEB)/voip_sip_status.asp /home/httpd/web
	$(ROMFSINST) -s /var/config_voip.dat  /home/httpd/web/config_voip.dat

endif
ifeq ($(CONFIG_USER_BOA_WITH_SSL),y)
	$(ROMFSINST) /etc/ssl_key.pem
	$(ROMFSINST) /etc/ssl_cert.pem
endif
