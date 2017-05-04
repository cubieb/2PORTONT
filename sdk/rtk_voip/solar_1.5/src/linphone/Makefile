include ../build/Makeparm.cfg
include ../build/Makeparm.def

OBJS		=	authentication.o commands.o enum.o exevents.o	\
				glist.o gshell.o gspawn.o linphonec.o linphonecore.o	\
				lpconfig.o misc.o proxy.o sdphandler.o uglib.o 

ifeq ($(CONFIG_RTK_VOIP_IP_PHONE),y)
OBJS		+= uihandler.o
endif

LIB_OBJ		= $(foreach obj,$(OBJS),$(OBJ_DIR)/$(obj))
DEPENDENCY	= $(LIB_OBJ:.o=.d)

lib: $(OBJ_DIR) $(LINPHONE_LIB)

clean:
	rm -f $(LINPHONE_LIB) $(LIB_OBJ) $(DEPENDENCY)

$(OBJ_DIR):
	mkdir -p $@

$(LINPHONE_LIB): $(LIB_OBJ)
	$(AR) rcs $(LINPHONE_LIB) $^

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
