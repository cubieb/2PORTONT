include ../.linux_config
include ../Makefile.rcm

DIR_FLASH = $(shell pwd)

HOSTCC = gcc
CFLAGS = -g -Wall -I../includes

ifdef RCM_SIMULATION
CFLAGS += -DRCM_SIMULATION
endif

all: $(EXE) 
	make -f Makefile-client 
#	make -f Makefile-tool-mips
#	make -f Makefile-tool-x86

clean:
	make -f Makefile-client clean
	make -f Makefile-tool-mips clean
	make -f Makefile-tool-x86 clean
	make w32-clean

w32-clean:
	rm -f WIN32/voip_flash.h
	rm -f WIN32/voip_flash_mib.c
	rm -f WIN32/voip_flash_mib.h
	rm -f WIN32/voip_flash_tool.c
	rm -f WIN32/voip_flash_tool.h
	rm -f WIN32/kernel_config.h
	rm -f WIN32/rtk_voip.h
	rm -f WIN32/apmib.h
	rm -f WIN32/mibdef.h
	rm -f WIN32/mibtbl.h
	rm -f WIN32/mibtbl.c
	rm -f WIN32/cvcfg.c
	@#rm -f w32_define
	@#rm -f cvvoip_w32.tgz

w32: 
	ln -fs $(DIR_FLASH)/voip_flash_mib.c WIN32/
	ln -fs $(DIR_FLASH)/voip_flash_tool.c WIN32/
	ln -fs $(DIR_FLASH)/../includes/voip_flash.h WIN32/
	ln -fs $(DIR_FLASH)/../includes/voip_flash_mib.h WIN32/
	ln -fs $(DIR_FLASH)/../includes/voip_flash_tool.h WIN32/
	ln -fs $(DIR_FLASH)/../includes/kernel_config.h WIN32/
	ln -fs $(DIR_FLASH)/../includes/rtk_voip.h WIN32/
	ln -fs $(DIR_FLASH)/../../boa/apmib/apmib.h WIN32/
	ln -fs $(DIR_FLASH)/../../boa/apmib/mibdef.h WIN32/
	ln -fs $(DIR_FLASH)/../../boa/apmib/mibtbl.h WIN32/
	ln -fs $(DIR_FLASH)/../../boa/apmib/mibtbl.c WIN32/
	ln -fs $(DIR_FLASH)/../../boa/apmib/cvcfg.c WIN32/
	./w32_flags.sh
	@#tar cvfz cvvoip_w32.tgz WIN32

w32_define: w32_define.o
	$(HOSTCC) -o $@ $(CFLAGS) $<

-include $(DEPENDENCY)
