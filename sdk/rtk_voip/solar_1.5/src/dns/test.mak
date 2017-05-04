OBJS	=	dns_task.o dns_api.o dns_test.o memwatch.o
CFLAGS	=	-g -Wall -DMALLOC_DEBUG -DMEMWATCH -DMEMWATCH_STDIO
DEPENDENCY	= $(OBJS:.o=.d)

all: $(OBJS)
	$(CC) $(LDFLAGS) -o dns_task dns_task.o memwatch.o -lpthread
	$(CC) $(LDFLAGS) -o dns_test dns_api.o dns_test.o memwatch.o
	ln -fs dns_test dns_test2 
	ln -fs dns_test dns_test3

clean: 
	rm -f $(OBJS) $(DEPENDENCY)
	rm -f dns_task dns_test dns_test2 dns_test3

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
