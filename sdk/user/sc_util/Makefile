EXEC = simctrl
OBJS = sc_status.o sc_fs.o simctrl.o sc_api.o

CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include
all: $(EXEC)

$(EXEC): $(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
		$(ROMFSINST) /bin/$(EXEC)
		
clean:
		-rm -f $(EXEC) *.o
