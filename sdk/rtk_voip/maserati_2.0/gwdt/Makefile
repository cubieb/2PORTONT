TARGET = voip_gwdt
OBJS = gwdt_main.o
CFLAGS = -Wall -Os -g -DGWDT_DEBUG
LIBS = 
LDFLAGS = 

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

romfs:
	$(ROMFSINST) /bin/$(TARGET)
	$(ROMFSINST) app.gwdt /etc/app.gwdt
	$(ROMFSINST) rc_voip /etc/rc_voip
	
clean:
	-rm -rf $(TARGET) $(OBJS)


	
