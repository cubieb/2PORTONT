#! /usr/bin/make

include ../../config/.config

EXT_FUNC+=802_3 nat arp arpreply ip standard log redirect vlan mark_m mark \
          pkttype stp among limit ulog
ifdef CONFIG_IPV6
EXT_FUNC+=ip6
endif
ifdef CONFIG_BRIDGE_EBT_DHCP_OPT
EXT_FUNC+=dhcp_opt
endif

#ifeq ($(CONFIG_BOA_WEB_E8B_CH),y)
EXT_FUNC+=ftos
#endif
EXT_TABLES+=filter nat broute
EXT_OBJS+=$(foreach T,$(EXT_FUNC), extensions/ebt_$(T).o)
EXT_OBJS+=$(foreach T,$(EXT_TABLES), extensions/ebtable_$(T).o)
EXT_OBJS+= extensions/initext.o

extensions/ebtable.a: $(EXT_OBJS)
	rm -f $@
	$(CROSS)ar crv $@ $(EXT_OBJS)
	$(CROSS)ranlib $@

extensions/initext.o: extensions/initext.c
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $< -I$(KERNEL_INCLUDES)	
extensions/initext.c: extensions/Makefile $(LINUX_CONFIG)
	echo "" > $@
	for i in $(EXT_FUNC); do \
		echo "extern void ebt_$${i}_init(void);" >> $@; \
	done
	for i in $(EXT_TABLES); do \
		echo "extern void ebt_$${i}_init(void);" >> $@; \
	done
	echo "void init_extensions(void) {" >> $@
	for i in $(EXT_FUNC); do \
		echo "	ebt_$${i}_init();" >> $@; \
	done
	for i in $(EXT_TABLES); do \
		echo "	ebtable_$${i}_init();" >> $@; \
	done
	echo "}" >> $@

extensions/ebt_%.o: extensions/ebt_%.c include/ebtables_u.h
	$(CC) $(CFLAGS) $(PROGSPECS)  -D_INIT=ebt_$*_init -c -o $@ $< -I$(KERNEL_INCLUDES)

extensions/ebtable_%.o: extensions/ebtable_%.c
	$(CC) $(CFLAGS) $(PROGSPECS)  -D_INIT=ebtable_$*_init -c -o $@ $< -I$(KERNEL_INCLUDES)

