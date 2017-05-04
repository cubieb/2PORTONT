include ../../.linux_config

CFLAGS += -I../../includes -Wall
ifdef CONFIG_RTK_VOIP_IP_PHONE
CFLAGS 		+= -I../../ui/include
endif

objs = cli.o clishell.o cliprint.o cmdtree.o cmdcore.o cmdargs.o 

lib = cli.a

all: $(objs) 
	$(AR) rcs $(lib) $(objs) 

clean:
	rm -f $(objs) $(lib)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
