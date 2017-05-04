include ../build/Makeparm.cfg
include ../build/Makeparm.def

OBJS		=	eXosip.o eXutils.o jauth.o jcallback.o jcall.o jdialog.o jevents.o		\
				jfreinds.o jidentity.o jnotify.o jpipe.o jpublish.o jreg.o jrequest.o	\
				jresponse.o jsubscribe.o jsubscribers.o misc.o sdp_offans.o udp.o

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
