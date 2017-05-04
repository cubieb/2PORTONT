CC=$(CROSS_COMPILE)gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=wlan_server.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=wlan_server
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

romfs:
	$(ROMFSINST) ./$(EXECUTABLE) /bin/$(EXECUTABLE)

.PHONY: rcX
rcX:
	$(ROMFSINST) -a "::respawn:/bin/wlan_server" /etc/inittab

clean: 
	rm -rf $(EXECUTABLE) *.o
