
.EXPORT_ALL_VARIABLES:

EXTRA_CFLAGS += -I$(ROOTDIR)/freeswan/libcrypto/libaes
EXTRA_CFLAGS += -I$(ROOTDIR)/freeswan/libcrypto/include
EXTRA_CFLAGS += -DEMBED -I$(LIBC_INCLUDE) 
LDLIBS += ../libc/libc.a

VPATH = $(ROOTDIR)/freeswan/libcrypto/libaes

first_target: libaes.a

include $(ROOTDIR)/freeswan/libcrypto/libaes/Makefile

romfs:

