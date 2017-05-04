CC=$(CROSS_COMPILE)gcc
CFLAGS=-c -Wall
LDFLAGS=-L../tmp/lib
SOURCES=sample_util.c md_ctrlpt.c md_ctrlpt_main.c
OBJECTS=$(SOURCES:.c=.o)
#OBJECTS=sample_util.o md_ctrlpt.o md_ctrlpt_main.o
EXECUTABLE=upnpmd_cp
CFLAGS += -I$(ROOTDIR)/include -I$(ROOTDIR) -I../tmp/include/upnp -pthread -DEMBED
LIBS=-pthread -lupnp -lthreadutil -lixml

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)
#	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)
	#$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $@ $(LIBS)

#sample_util.o: sample_util.c

#md_ctrlpt.o: md_ctrlpt.c sample_util.c
#	$(CC) $(CFLAGS) $< -o $@

#md_ctrlpt_main.o: md_ctrlpt_main.c

#md_ctrlpt.o: md_ctrlpt.c sample_util.c
#	$(CC) -c $< -o $@ $(CFLAGS)

#md_ctrlpt.o: sample_util.c
#md_ctrlpt_main.o: md_ctrlpt_main.c

#.SUFFIXES: .c .o

.c.o:
#	$(CC) -c $< -o $@ $(CFLAGS)
#	$(CC) $(CFLAGS) -o $@ -c $<
	$(CC) $(CFLAGS) $< -o $@

romfs:
	$(ROMFSINST) ./$(EXECUTABLE) /bin/$(EXECUTABLE)

clean:
	rm -rf $(EXECUTABLE) *.o

