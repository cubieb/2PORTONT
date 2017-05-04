.EXPORT_ALL_VARIABLES:

ifdef CONFIG_IPV6
CFLAGS += -DIPV6
endif

DIRS = ftp

all:
	for i in $(DIRS) ; do make -C $$i $@ ; done

romfs:
	for i in $(DIRS) ; do make -C $$i $@ ; done

clean:
	-for i in $(DIRS) ; do make -C $$i clean ; done


