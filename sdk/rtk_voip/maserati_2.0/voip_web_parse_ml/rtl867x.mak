include ../.linux_config

ifndef WEB_FLAGS
target = error
else
target = build
endif

LIBA = libweb_voip.a
OBJS = web_voip.o web_voip_general.o web_voip_tone.o web_voip_ring.o web_voip_other.o web_voip_config.o web_voip_network.o web_voip_e8c.o web_voip_history.o
CFLAGS += $(WEB_FLAGS)
DEPENDENCY	= $(OBJS:.o=.d)

ifeq ($(CONFIG_RTK_VOIP_IVR),y)
	OBJS += web_voip_ivr_req.o
endif

ifneq ($(BUILD_DIALPLAN), 0)
CFLAGS += -DCONFIG_RTK_VOIP_DIALPLAN
endif
ifeq ($(CONFIG_USER_RTK_WAN_CTYPE), y)
	CFLAGS += -DCONFIG_RTK_VOIP_MULTI_WAN_CTYPE
endif

VOIP_SRC		=	$(VOIP_APP)/src
all: $(target)

error: 
	@echo "Please make in WEB directory."

build: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBA) $(OBJS)
	
clean:
	-rm -f $(LIBA) $(OBJS) *.d
	-rm -f $(WEB_DIR)/voip_script.js
	-rm -f $(WEB_DIR)/voip_general.asp
	-rm -f $(WEB_DIR)/voip_tone.asp
	-rm -f $(WEB_DIR)/voip_ring.asp
	-rm -f $(WEB_DIR)/voip_other.asp
	-rm -f $(WEB_DIR)/voip_config.asp
	-rm -f $(WEB_DIR)/voip_network.asp
  ifeq ($(CONFIG_RTK_VOIP_IVR),y)
	-rm -f $(WEB_DIR)/voip_ivr_req.asp
  endif
	-rm -f $(WEB_DIR)/voip_sip_status.asp
	rm -f $(DEPENDENCY)
	
%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)

-include $(DEPENDENCY)
