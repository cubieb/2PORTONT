
# -*- sh -*-

# $Id: Makefile,v 1.2 2011/08/12 09:45:45 kaohj Exp $
MY_CFLAGS = -I$(ROOTDIR)/$(LINUXDIR)/include
BUILD_CFLAGS += $(MY_CFLAGS)
ifdef CONFIG_USE_RSDK_WRAPPER
BUILD_CC = $(CC)
else
BUILD_CC = /opt/toolchain_mips_nofpu/bin/mips-linux-gcc
endif

CFLAGS += $(INCZ) -D__USE_BSD
#CFLAGS += $(INCZ) 
EXTRALIBS = $(LIBCRYPT) $(LIBMIB) $(LIBVOIP)
#LDFLAGS = -L. -L/lib

TARGETS = ftl_format flash_erase flash_eraseall nanddump doc_loadbios \
	mkfs.jffs ftl_check mkfs.jffs2 flash_lock flash_unlock flash_info \
	flash_otp_info flash_otp_dump mtd_debug flashcp nandwrite \
	jffs2dump \
	nftldump nftl_format docfdisk \
	rfddump rfdformat \
	sumtool #jffs2reader

#compr_zlib.o: compr_zlib.c zlib.c zlib.h

%.o: %.c
	$(BUILD_CC) $(CFLAGS) $(BUILD_CFLAGS) -g -c -o $@ $< -g -Wp,-MD,.$<.dep

%: %.o
	$(BUILD_CC) $(LDFLAGS) $(CFLAGS) $(BUILD_CFLAGS) -g -o $@ $^ $(EXTRALIBS) $(LDLIBS)

.SUFFIXES:

all: $(TARGETS)



romfs:
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_ERASE        /bin/flash_erase
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_ERASEALL     /bin/flash_eraseall
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_FTL_CHECK    /bin/ftl_check
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_FTL_FORMAT   /bin/ftl_format
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_MKFSJFFS     /bin/mkfs.jffs
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_MKFSJFFS2    /bin/mkfs.jffs2
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NFTLDUMP     /bin/nftldump
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NFTL_FORMAT  /bin/nftl_format
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NANDDUMP     /bin/nanddump
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NANDWRITE    /bin/nandwrite
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_DOC_LOADBIOS /bin/doc_loadbios


mkfs.jffs2: crc32.o compr_rtime.o mkfs.jffs2.o compr_zlib.o compr.o zlib.o
	$(BUILD_CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

flash_eraseall: crc32.o flash_eraseall.o
	$(BUILD_CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

jffs2reader: jffs2reader.o
	$(BUILD_CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

jffs2dump: jffs2dump.o crc32.o
	$(BUILD_CC) $(LDFLAGS) -o $@ $^

sumtool: sumtool.o crc32.o
	$(BUILD_CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGETS) .*.c.dep 
