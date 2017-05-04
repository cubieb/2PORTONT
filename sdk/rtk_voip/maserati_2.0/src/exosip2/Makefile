include ../build/Makeparm.cfg
include ../build/Makeparm.def

OBJS		=	eXcall_api.o eXconf.o eXinsubscription_api.o eXmessage_api.o eXoptions_api.o eXosip.o	\
				eXpublish_api.o eXrefer_api.o eXregister_api.o eXsubscription_api.o eXtl.o eXtl_tcp.o 	\
				eXtl_udp.o eXtransport.o eXutils.o	inet_ntop.o jauth.o jcallback.o jcall.o jdialog.o 	\
				jevents.o jnotify.o jpipe.o jpublish.o jreg.o jrequest.o								\
				jresponse.o jsubscribe.o milenage.o misc.o rijndael.o sdp_offans.o udp.o 				\
				

ifeq ($(CONFIG_RTK_VOIP_SIP_TLS),y)
OBJS		+=	eXosip_tls.o
endif

LIB_OBJ		= $(foreach obj,$(OBJS),$(OBJ_DIR)/$(obj))
DEPENDENCY	= $(LIB_OBJ:.o=.d)

lib: $(OBJ_DIR) $(EXOSIP_LIB)

clean:
	rm -f $(EXOSIP_LIB) $(LIB_OBJ) $(DEPENDENCY)

$(OBJ_DIR):
	mkdir -p $@

$(EXOSIP_LIB): $(LIB_OBJ)
	$(AR) rcs $(EXOSIP_LIB) $^

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
