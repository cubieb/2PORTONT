include ../.linux_config

DIR_FLASH = $(shell pwd)

HOSTCC = gcc
EXE = cvvoip
OBJS = cvvoip.o voip_flash_mib.o voip_flash_tool.o

CFLAGS = -g -Wall -DCVVOIP -I../includes
LDFLAGS = #--static

ifneq ($(BUILD_DIALPLAN), 0)
CFLAGS += -DCONFIG_RTK_VOIP_DIALPLAN
endif

DEPENDENCY	= $(OBJS:.o=.d)

all: $(EXE) 
	make -f Makefile-client 
	make -f Makefile-tool-mips

$(EXE): $(OBJS)
	$(HOSTCC) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c
	$(HOSTCC) -c -o $@ $(CFLAGS) $<
	$(HOSTCC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

clean:
	rm -f $(EXE) $(OBJS) *.d
	rm -f WIN32/voip_flash.h
	rm -f WIN32/voip_flash_mib.c
	rm -f WIN32/voip_flash_mib.h
	rm -f WIN32/voip_flash_tool.c
	rm -f WIN32/voip_flash_tool.h
	rm -f WIN32/apmib.h
	rm -f WIN32/mibdef.h
	rm -f WIN32/mibtbl.h
	rm -f WIN32/mibtbl.c
	rm -f WIN32/cvcfg.c
	rm -f cvvoip_w32.tgz
	make -f Makefile-client clean
	make -f Makefile-tool-mips clean

w32: w32_define
	ln -fs $(DIR_FLASH)/voip_flash_mib.c WIN32/
	ln -fs $(DIR_FLASH)/voip_flash_tool.c WIN32/
	ln -fs $(DIR_FLASH)/../includes/voip_flash.h WIN32/
	ln -fs $(DIR_FLASH)/../includes/voip_flash_mib.h WIN32/
	ln -fs $(DIR_FLASH)/../includes/voip_flash_tool.h WIN32/
	ln -fs $(DIR_FLASH)/../../goahead-2.1.1/LINUX/apmib.h WIN32/
	ln -fs $(DIR_FLASH)/../../goahead-2.1.1/LINUX/mibdef.h WIN32/
	ln -fs $(DIR_FLASH)/../../goahead-2.1.1/LINUX/mibtbl.h WIN32/
	ln -fs $(DIR_FLASH)/../../goahead-2.1.1/LINUX/mibtbl.c WIN32/
	ln -fs $(DIR_FLASH)/../../goahead-2.1.1/LINUX/cvcfg.c WIN32/
	./w32_flags.sh
	#tar cvfz cvvoip_w32.tgz WIN32

w32_define: w32_define.o
	$(CC) -o $@ $(CFLAGS) $<

-include $(DEPENDENCY)
