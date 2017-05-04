
INCLUDES= 
#LIBS= -lpthread -lupnp -lixml -lthreadutil

LIBS= ../../bin/$(TARGET)/libupnp.a \
../../bin/$(TARGET)/libixml.a \
../../bin/$(TARGET)/libthreadutil.a \
-lpthread

CC = $(TARGET)gcc

CFLAGS += -Wall -g $(OPT) $(LDFLAGS)

all: upnpd

upnpd: main.o gatedevice.o pmlist.o util.o config.o
	$(CC)  $(CFLAGS) main.o gatedevice.o pmlist.o util.o config.o $(LIBS) -o  $@ 
	@echo "make $@ finished on `date`"

%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o upnpd

install: upnpd
	@install -d /etc/linuxigd
	@install etc/gatedesc.xml /etc/linuxigd
	@install etc/gateconnSCPD.xml  /etc/linuxigd
	@install etc/gateicfgSCPD.xml /etc/linuxigd
	@install etc/dummy.xml /etc/linuxigd
	@install upnpd /usr/sbin
	@install etc/upnpd.conf /etc	
