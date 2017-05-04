EXEC = usbmount 
OBJS = usbmount.o

all: $(EXEC)

ifeq ($(CONFIG_ARCH_LUNA_SLAVE),y)
$(EXEC): 
else
$(EXEC): LDFLAGS += $(LIBMIB) $(LIBVOIP)
endif
romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.o

