
EXEC = osgi
OBJS = osgi_cmd.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	-rm -f $(EXEC) *.elf *.o

