#
# Copyright (C) 2014 Realtek Semiconductor Corp.
#
# Makefile for SDK EPON OAM protocol Source
#

VERSION=1.1.2
ifeq ($(CONFIG_RTL9601B_SERIES), y)
DIR = 9601B_series
else
ifeq ($(CONFIG_RTL9600_SERIES), y)
DIR = 9600_series
else
ifeq ($(CONFIG_RTL9602C_SERIES), y)
DIR = 9602C_series
endif
endif
endif

EXEC = eponoamd
EXECLI = oamcli
BIN = bin

# copy & change file name
all:
	cp $(DIR)/$(EXEC).$(VERSION) $(BIN)/$(EXEC)
	cp $(DIR)/$(EXECLI).$(VERSION) $(BIN)/$(EXECLI)

install: all
	chmod 775 $(BIN)/$(EXEC)
	chmod 775 $(BIN)/$(EXECLI)
	cp -af $(BIN)/$(EXEC) $(BINDIR)
	cp -af $(BIN)/$(EXECLI) $(BINDIR)

clean:
	rm -f $(OAMCORE) $(USEROAM) $(BIN)/$(EXEC) $(BIN)/$(EXECLI) *.gdb *.elf *.elf2flt
