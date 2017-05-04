x86 = n

ifeq ($(x86),y)
DNS_LIB = libdns.a
CFLAGS = -g -Wall
else
include ../build/Makeparm.cfg
include ../build/Makeparm.def
endif

OBJS	=	dns_task.o dns_api.o dns_test.o memwatch.o
LDFLAGS	=	
DEPENDENCY	= $(OBJS:.o=.d)

all: $(OBJS)
#	$(CC) $(LDFLAGS) -o dns_task dns_task.o memwatch.o -lpthread
#	$(CC) $(LDFLAGS) -o dns_test dns_api.o dns_test.o memwatch.o
	$(CC) $(LDFLAGS) -o dns_task dns_task.o -lpthread
	$(CC) $(LDFLAGS) -o dns_test dns_api.o dns_test.o
	ln -fs dns_test dns_test1
	ln -fs dns_test dns_test2 
	ln -fs dns_test dns_test3
	$(AR) rcs $(DNS_LIB) dns_api.o
#	make -f resolv.mak

clean: 
	rm -f $(OBJS) $(DEPENDENCY)
	rm -f dns_task dns_test1 dns_test2 dns_test3 $(DNS_LIB)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
