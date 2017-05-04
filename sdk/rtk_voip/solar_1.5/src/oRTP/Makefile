include ../build/Makeparm.cfg
include ../build/Makeparm.def

OBJS		=	avprofile.o ortp.o payloadtype.o port_fct.o

LIB_OBJ		= $(foreach obj,$(OBJS),$(OBJ_DIR)/$(obj))
DEPENDENCY	= $(LIB_OBJ:.o=.d)

lib: $(OBJ_DIR) $(RTP_LIB)

clean:
	rm -f $(RTP_LIB) $(LIB_OBJ) $(DEPENDENCY)

$(OBJ_DIR):
	mkdir -p $@

$(RTP_LIB): $(LIB_OBJ)
	$(AR) rcs $(RTP_LIB) $^

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
