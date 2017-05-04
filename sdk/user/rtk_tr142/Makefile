# Path for kernel module
TR142_PATH = $(APOLLODIR)/tr142

# Path for user space library
LIBTR142_PATH = libtr142

EXTRA_CFLAGS += -include $(KDIR)/include/linux/autoconf.h

all: 
	$(MAKE) -C $(TR142_PATH) all
ifneq ($(CONFIG_E8B),y)
	$(MAKE) -C $(LIBTR142_PATH) all
endif

clean:
	$(MAKE) -C $(TR142_PATH) clean
ifneq ($(CONFIG_E8B),y)
	$(MAKE) -C $(LIBTR142_PATH) clean
endif

romfs:
	$(MAKE) -C $(TR142_PATH) install
ifneq ($(CONFIG_E8B),y)
	$(MAKE) -C $(LIBTR142_PATH) romfs
endif
