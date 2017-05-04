#
# Copyright (C) 2014 Realtek Semiconductor Corp.
#
# Makefile for debugging tool
#

VERSION=1.1.0

EXEC = dbg_tool
BIN = bin

# copy & change file name
all:
	cp $(EXEC).$(VERSION) $(BIN)/$(EXEC)

install: all
	chmod 775 $(BIN)/$(EXEC)
	cp -af $(BIN)/$(EXEC) $(BINDIR)

clean:
	rm -f $(BIN)/$(EXEC)
