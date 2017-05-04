
COMMAND_EDITING = 1
COMMAND_TAB_COMPLETION = 1

SH = msh
ifdef COMMAND_EDITING
SHOBJS = msh.o cmdedit.o test.o
CFLAGS += -DBB_FEATURE_COMMAND_EDITING
ifdef COMMAND_TAB_COMPLETION
CFLAGS += -DBB_FEATURE_COMMAND_TAB_COMPLETION
endif
else
SHOBJS = msh.o test.o
endif

all: $(SH)

$(SH): $(SHOBJS)
	$(CC) $(LDFLAGS) -o $@ $(SHOBJS) $(LDLIBS$(LDLIBS-$(@)))

romfs:
	$(ROMFSINST) msh /bin/msh
#	$(ROMFSINST) -s /bin/msh /bin/sh
#	$(ROMFSINST) -s /bin/msh /bin/[
	ln -f $(ROMFSDIR)/bin/msh $(ROMFSDIR)/bin/sh
	ln -f $(ROMFSDIR)/bin/msh $(ROMFSDIR)/bin/[

clean:
	-rm -f $(SH) *.elf *.gdb *.o

