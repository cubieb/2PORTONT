GTK_FLAGS=`pkg-config gtk+-2.0 --cflags`
GTK_LIBS=`pkg-config gtk+-2.0 --libs`

KRB5LIBS=
LDAP_LIBS=
LIBS=-lcrypt -lresolv -ldl -lnetapi -ltdb -ltalloc
DEVELOPER_CFLAGS=-g -Wall -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -DDEBUG_PASSWORD -DDEVELOPER
FLAGS=-I../ -L../../../bin  -O -D_SAMBA_BUILD_=3 $(GTK_FLAGS)
CC=mips-linux-gcc
PICFLAG=-fPIE
LDFLAGS=-pie 
DYNEXP=-Wl,--export-dynamic
NETAPI_LIBS=$(LIBS) $(KRB5LIBS) $(LDAP_LIBS)
CMDLINE_LIBS=$(NETAPI_LIBS) $(POPT_OBJ)

# Compile a source file.
COMPILE_CC = $(CC) -I. $(FLAGS) $(PICFLAG) -c $< -o $@
COMPILE = $(COMPILE_CC)

PROGS = bin/getdc \
	bin/dsgetdc \
	bin/netdomjoin \
	bin/netdomjoin-gui \
	bin/getjoinableous \
	bin/user_add \
	bin/user_del \
	bin/user_enum \
	bin/user_dispinfo

all: $(PROGS)

MAKEDIR = || exec false; \
	  if test -d "$$dir"; then :; else \
	  echo mkdir "$$dir"; \
	  mkdir -p "$$dir" >/dev/null 2>&1 || \
	  test -d "$$dir" || \
	  mkdir "$$dir" || \
	  exec false; fi || exec false

BINARY_PREREQS = bin/.dummy

bin/.dummy:
	@if (: >> $@ || : > $@) >/dev/null 2>&1; then :; else \
	  dir=bin $(MAKEDIR); fi
	@: >> $@ || : > $@ # what a fancy emoticon!

.c.o:
	@if (: >> $@ || : > $@) >/dev/null 2>&1; then rm -f $@; else \
	 dir=`echo $@ | sed 's,/[^/]*$$,,;s,^$$,.,'` $(MAKEDIR); fi
	@echo Compiling $*.c
	@$(COMPILE) && exit 0;\
		echo "The following command failed:" 1>&2;\
		echo "$(COMPILE_CC)" 1>&2;\
		$(COMPILE_CC) >/dev/null 2>&1

CMDLINE_OBJ = common.o
GETDC_OBJ = getdc/getdc.o $(CMDLINE_OBJ)
DSGETDC_OBJ = dsgetdc/dsgetdc.o $(CMDLINE_OBJ)
NETDOMJOIN_OBJ = netdomjoin/netdomjoin.o $(CMDLINE_OBJ)
NETDOMJOIN_GUI_OBJ = netdomjoin-gui/netdomjoin-gui.o
GETJOINABLEOUS_OBJ = getjoinableous/getjoinableous.o $(CMDLINE_OBJ)
USERADD_OBJ = user/user_add.o $(CMDLINE_OBJ)
USERDEL_OBJ = user/user_del.o $(CMDLINE_OBJ)
USERENUM_OBJ = user/user_enum.o $(CMDLINE_OBJ)
USERDISPINFO_OBJ = user/user_dispinfo.o $(CMDLINE_OBJ)

bin/getdc: $(BINARY_PREREQS) $(GETDC_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS) -o $@ $(GETDC_OBJ) $(LDFLAGS) $(DYNEXP) $(CMDLINE_LIBS)

bin/dsgetdc: $(BINARY_PREREQS) $(DSGETDC_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS) -o $@ $(DSGETDC_OBJ) $(LDFLAGS) $(DYNEXP) $(CMDLINE_LIBS)

bin/getjoinableous: $(BINARY_PREREQS) $(GETJOINABLEOUS_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS) -o $@ $(GETJOINABLEOUS_OBJ) $(LDFLAGS) $(DYNEXP) $(CMDLINE_LIBS)

bin/netdomjoin: $(BINARY_PREREQS) $(NETDOMJOIN_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS) -o $@ $(NETDOMJOIN_OBJ) $(LDFLAGS) $(DYNEXP) $(CMDLINE_LIBS)

bin/netdomjoin-gui: $(BINARY_PREREQS) $(NETDOMJOIN_GUI_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS) $(GTK_FLAGS) -o $@ $(NETDOMJOIN_GUI_OBJ) $(LDFLAGS) $(DYNEXP) $(LIBS) $(KRB5LIBS) $(LDAP_LIBS) $(GTK_LIBS)

bin/user_add: $(BINARY_PREREQS) $(USERADD_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS) -o $@ $(USERADD_OBJ) $(LDFLAGS) $(DYNEXP) $(CMDLINE_LIBS)

bin/user_del: $(BINARY_PREREQS) $(USERDEL_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS) -o $@ $(USERDEL_OBJ) $(LDFLAGS) $(DYNEXP) $(CMDLINE_LIBS)

bin/user_enum: $(BINARY_PREREQS) $(USERENUM_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS) -o $@ $(USERENUM_OBJ) $(LDFLAGS) $(DYNEXP) $(CMDLINE_LIBS)

bin/user_dispinfo: $(BINARY_PREREQS) $(USERDISPINFO_OBJ)
	@echo Linking $@
	@$(CC) $(FLAGS) -o $@ $(USERDISPINFO_OBJ) $(LDFLAGS) $(DYNEXP) $(CMDLINE_LIBS)

clean:
	-rm -f $(PROGS)
	-rm -f core */*~ *~ \
		*/*.o */*/*.o */*/*/*.o
