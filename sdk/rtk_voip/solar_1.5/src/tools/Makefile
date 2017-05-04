include ../build/Makeparm.cfg
include ../build/Makeparm.def
CFLAGS	= -I../../includes/
CFLAGS	+= -I../../ui/include/

all:
	$(XCC) -o solar_monitor $(CFLAGS) watchdog.c
clean:
	-rm solar_monitor
