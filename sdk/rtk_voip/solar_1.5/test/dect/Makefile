include ../../.linux_config

CFLAGS += -I../../includes -Wall

TARGET = dect

objs = bus.o dect_test.o 

all:	$(objs)
	$(CC) -o $(TARGET) $(objs) $(CFLAGS)


clean:
	rm -f $(objs)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
