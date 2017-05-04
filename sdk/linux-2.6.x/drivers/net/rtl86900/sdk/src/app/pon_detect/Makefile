#
# Copyright (C) 2011 Realtek Semiconductor Corp.
#
# Makefile for SDK EPON OAM protocol Source
#

APOLLO_SDK_PATH = $(SDKDIR)

#LIB += -L$(APOLLODIR)/object/src/app/lib -lrtk -lpthread -L$(ROOTDIR)/user/boa/src/LINUX -lmib $(LIBVOIP)
LIB += -L$(APOLLODIR)/object/src/app/lib -lrtk -lpthread

SDK_ROOT_DIR = $(APOLLO_SDK_PATH)
SDK_INCLUDE   = $(SDK_ROOT_DIR)/include
SYS_INCLUDE = $(SDK_ROOT_DIR)/system/include
BOA_INCLUDE = $(ROOTDIR)/user/boa/src/



INCLUDE += -I$(SDK_INCLUDE) \
           -I$(SYS_INCLUDE) \
	   -I$(BOA_INCLUDE)

PON_DETECT_DIR := .
PON_DETECT_INC_DIR  = $(PON_DETECT_DIR)/inc
PON_DETECT_SRC_DIR := $(PON_DETECT_DIR)/src
EXEC = pondetect
BIN = bin


OBJ_PONDETECT := $(PON_DETECT_SRC_DIR)/pon_detect_main.o


$(EXEC): $(OBJ_PONDETECT)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDE) $(OBJ_PONDETECT) $(LIB) -o $(BIN)/$@
	$(STRIP) $(BIN)/$@

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

# Force to rebuild all every time
all: clean  $(EXEC)

install: $(EXEC)
	chmod 775 $(BIN)/$(EXEC)
	cp -af $(BIN)/$(EXEC) $(BINDIR)

clean:
	rm -f $(OBJ_PONDETECT) $(BIN)/$(EXEC) *.gdb *.elf *.elf2flt

