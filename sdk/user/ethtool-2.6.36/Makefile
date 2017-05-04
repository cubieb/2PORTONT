#august 2011-1020
bin  = ethtool
OBJS = ethtool.o \
		  amd8111e.o de2104x.o e100.o e1000.o igb.o	\
		  fec_8xx.o ibm_emac.o ixgb.o ixgbe.o natsemi.o	\
		  pcnet32.o realtek.o tg3.o marvell.o vioc.o	\
		  smsc911x.o at76c50x-usb.o sfc.o stmmac.o
LDFLAGS += -I.

all: $(bin)

$(bin): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJS): ethtool-copy.h ethtool-util.h

romfs:
	$(ROMFSINST) /bin/$(bin)

clean:
	-rm -f $(bin) *.elf *.o

