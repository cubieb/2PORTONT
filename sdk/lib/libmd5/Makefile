CFLAGS += -fPIC

all: libmd5.so

libmd5.o: libmd5.h libmd5.c
	$(CC) -Os -c -o $@ $(CFLAGS) libmd5.c

libmd5.a: libmd5.o
	$(AR) $(ARFLAGS) $@ $?

libmd5.so: libmd5.a
	$(LD) -shared --warn-common --warn-once -z combreloc -soname=$@.0 \
		-o $@ --whole-archive $?
  ifdef CONFIG_USE_RSDK_WRAPPER
	install -m 644 libmd5.so $(ROOTDIR)/lib/librtk/libmd5.so
	install -m 644 libmd5.a $(ROOTDIR)/lib/librtk/libmd5.a
  endif #CONFIG_USE_RSDK_WRAPPER

clean:
	rm -rf libmd5.o libmd5.a libmd5.so
  ifdef CONFIG_USE_RSDK_WRAPPER
	rm -rf $(ROOTDIR)/lib/librtk/libmd5.so
	rm -rf $(ROOTDIR)/lib/librtk/libmd5.a
  endif #CONFIG_USE_RSDK_WRAPPER

romfs:
	$(ROMFSINST) /lib/libmd5.so
  ifdef CONFIG_USE_RSDK_WRAPPER
	$(ROMFSINST) -s libmd5.so /lib/libmd5.so.0
  else #CONFIG_USE_RSDK_WRAPPER
	$(ROMFSINST) -s /lib/libmd5.so /lib/libmd5.so.0
  endif #CONFIG_USE_RSDK_WRAPPER

