include ../build/Makeparm.cfg
include ../build/Makeparm.def

OBJS		=	rcm_bmm.o

LIB_OBJ		= $(foreach obj,$(OBJS),$(OBJ_DIR)/$(obj))
DEPENDENCY	= $(LIB_OBJ:.o=.d)

lib: $(OBJ_DIR) $(BUDDYMEMORY_LIB)

clean:
	rm -f $(BUDDYMEMORY_LIB) $(LIB_OBJ) $(DEPENDENCY)

$(OBJ_DIR):
	mkdir -p $@

$(BUDDYMEMORY_LIB): $(LIB_OBJ)
	$(AR) rcs $(BUDDYMEMORY_LIB) $^

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
