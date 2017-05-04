include ../build/Makeparm.cfg
include ../build/Makeparm.def

OBJS		=	stun.o stun_udp.o stun_api.o

LIB_OBJ		= $(foreach obj,$(OBJS),$(OBJ_DIR)/$(obj))
DEPENDENCY	= $(LIB_OBJ:.o=.d)

lib: $(OBJ_DIR) $(STUN_LIB)

clean:
	rm -f $(STUN_LIB) $(LIB_OBJ) $(DEPENDENCY)

$(OBJ_DIR):
	mkdir -p $@

$(STUN_LIB): $(LIB_OBJ)
	$(AR) rcs $(STUN_LIB) $^

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
