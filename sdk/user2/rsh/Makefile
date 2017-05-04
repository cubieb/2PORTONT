BIN = rlogin rsh
SBIN = in.rlogind in.rshd
MAN1 = rlogin.1 rsh.1
MAN5 = rhosts.5
MAN8 = rlogind.8 rshd.8
PAM = pam/rlogin pam/rsh

CC ?= gcc
PREFIX ?= /usr
INSTALL ?= install
BINDIR ?= $(PREFIX)/bin
SBINDIR ?= $(PREFIX)/sbin
SHAREDIR ?= $(PREFIX)/share
SYSCONFDIR ?= $(PREFIX)/etc
MANDIR ?= $(SHAREDIR)/man
PAMDIR ?= $(SYSCONFDIR)/pam.d
CFLAGS ?= -Wall -g -O2 -pipe -DBINDIR=\"$(BINDIR)\"

all: $(BIN) $(SBIN)

rlogin: rlogin.c
	$(CC) $(CFLAGS) -o $@ $<

in.rlogind: rlogind.c
	$(CC) $(CFLAGS) -lutil -lpam -o $@ $<

rsh: rsh.c
	$(CC) $(CFLAGS) -o $@ $<

in.rshd: rshd.c
	$(CC) $(CFLAGS) -lpam -o $@ $<

install: install-bin install-sbin install-man install-pam

install-bin: $(BIN)
	mkdir -p $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 4711 $(BIN) $(DESTDIR)$(BINDIR)/

install-sbin: $(SBIN)
	mkdir -p $(DESTDIR)$(SBINDIR)
	$(INSTALL) $(SBIN) $(DESTDIR)$(SBINDIR)/

install-man: $(MAN1) $(MAN5) $(MAN8)
	mkdir -p $(DESTDIR)$(MANDIR)/man1/
	mkdir -p $(DESTDIR)$(MANDIR)/man5/
	mkdir -p $(DESTDIR)$(MANDIR)/man8/
	$(INSTALL) -m 644 $(MAN1) $(DESTDIR)$(MANDIR)/man1/
	$(INSTALL) -m 644 $(MAN5) $(DESTDIR)$(MANDIR)/man5/
	$(INSTALL) -m 644 $(MAN8) $(DESTDIR)$(MANDIR)/man8/

install-pam: $(PAM)
	mkdir -p $(DESTDIR)$(PAMDIR)
	$(INSTALL) -m 644 $(PAM) $(DESTDIR)$(PAMDIR)/

clean:
	rm -f $(BIN) $(SBIN)
