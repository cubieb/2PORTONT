.EXPORT_ALL_VARIABLES:
TARGET = xmlconfig
INCLUDES = $(wildcard ./includes/*.h)
#CFLAGS += -Wall -g
CFLAGS += -g -Os
CFLAGS += -I./includes -I$(ROOTDIR)/include/rtk
LIBS = $(LIBMIB)
ifdef CONFIG_E8B
OBJS_DATA = xml_data_mib_table_e8.o xml_data_mib_chain_e8.o
else
OBJS_DATA = xml_data_mib_table.o xml_data_mib_chain.o
endif
OBJS = xml_config_main.o xml_config_gramma.o xml_config_parser.o xml_config_generator.o
#OBJS += sysconfig.o

# voip data section
ifdef CONFIG_USER_RTK_VOIP
CFLAGS += -DVOIP_SUPPORT
VOIP_APP = $(ROOTDIR)/user/rtk_voip
CFLAGS += -I$(VOIP_APP)/includes
LDFLAGS += -L$(VOIP_APP)/voip_manager -lvoip_manager
ifdef CONFIG_E8B
LDFLAGS += $(LIBSSL)
endif
OBJS_DATA += xml_data_voip.o
endif
#this is the end voip data section

CFLAGS += -DLUNA_EXTRA_CHK=$(LUNA_EXTRA_CHK)

OBJS_END = xml_data_end.o

OBJS += $(OBJS_DATA) $(OBJS_END)
RC_LEVEL=/etc/init.d/rc20
ifdef CONFIG_E8B
DIFF := "$(shell diff ./includes/mib_e8.h $(ROOTDIR)/include/rtk/mib.h)"
else
DIFF := "$(shell diff ./includes/mib.h $(ROOTDIR)/include/rtk/mib.h)"
endif
all: chk_struct 
	make $(TARGET)

.PHONY: romfs
romfs:
	$(ROMFSINST) /bin/$(TARGET)
	$(ROMFSINST) config_xmlconfig.sh /etc/scripts/config_xmlconfig.sh
	$(ROMFSINST) flash.sh /etc/scripts/flash
	$(ROMFSINST) $(ROOTDIR)/config/config_default.xml /etc/config_default.xml
	$(ROMFSINST) $(ROOTDIR)/config/config_default_hs.xml /etc/config_default_hs.xml

.PHONY: rcX
rcX:
	$(ROMFSINST) -a "/bin/sh /etc/scripts/config_xmlconfig.sh -b" $(RC_LEVEL)

.PHONY: chk_struct
chk_struct:
	@if [ $(DIFF) != "" ]; then \
	  echo "---------------------------------------------------------------"; \
	  echo $(DIFF); echo; \
	  echo "Please sync mib.h/mibtbl.c and xml_data_mib_table/chain.c"; \
	  echo "---------------------------------------------------------------"; \
	  exit $(LUNA_EXTRA_CHK); \
	fi


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS) 

%.o: %.c ${INCLUDES}
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf $(OBJS) $(TARGET) 

