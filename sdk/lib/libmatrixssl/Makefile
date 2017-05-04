MATRIXSSL_DIR=$(ROOTDIR)/user/cwmp-tr069/matrixssl-1-8-3-open/
LIBRARY=libmatrixssl.so

.PHONY:	all romfs
all:
	if [ -d $(MATRIXSSL_DIR) ]; then \
		$(MAKE) -C $(MATRIXSSL_DIR)/src; \
		install -m 644 $(MATRIXSSL_DIR)/src/libmatrixssl.so .; \
	fi
  ifdef CONFIG_USE_RSDK_WRAPPER
	if [ -d $(MATRIXSSL_DIR) ]; then \
		install -m 644 $(MATRIXSSL_DIR)/src/libmatrixssl.so $(ROOTDIR)/lib/librtk; \
		install -m 644 $(MATRIXSSL_DIR)/src/libmatrixsslstatic.a $(ROOTDIR)/lib/librtk/libmatrixssl.a; \
	fi
  endif #CONFIG_USE_RSDK_WRAPPER

romfs:
	$(ROMFSINST) /lib/$(LIBRARY)
  ifdef CONFIG_USE_RSDK_WRAPPER
	$(ROMFSINST) -s $(LIBRARY) /lib/$(LIBRARY).0
  else #CONFIG_USE_RSDK_WRAPPER
	$(ROMFSINST) -s /lib/$(LIBRARY) /lib/$(LIBRARY).0	
  endif #CONFIG_USE_RSDK_WRAPPER

