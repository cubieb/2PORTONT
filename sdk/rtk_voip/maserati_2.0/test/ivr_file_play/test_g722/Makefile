CFLAGS = -I../../../includes/ -I../../../ui/include/
CFLAGS += -DCONFIG_RTK_VOIP_IVR

LDFLAGS += -L../../../voip_manager/ -lvoip_manager 
LDFLAGS += -ldl -lpthread

all: 
	$(CC) $(CFLAGS) $(LDFLAGS) -o test_g722.o test_g722.c

romfs:
	$(ROMFSINST) test_g722.o /bin/test_ivr_g722
	$(ROMFSINST) 722_raw /usr/

clean:
	rm -rf *.o
