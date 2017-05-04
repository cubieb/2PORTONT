include ../build/Makeparm.cfg
include ../build/Makeparm.def

OBJS		=	dialplan.o dialplanhelp.o

LIB_OBJ		= $(foreach obj,$(OBJS),$(OBJ_DIR)/$(obj))
DEPENDENCY	= $(LIB_OBJ:.o=.d)

lib: $(OBJ_DIR) $(DIALPLAN_LIB)

clean:
	rm -f $(DIALPLAN_LIB) $(LIB_OBJ) $(DEPENDENCY)

$(OBJ_DIR):
	mkdir -p $@

$(DIALPLAN_LIB): $(LIB_OBJ)
	$(AR) rcs $(DIALPLAN_LIB) $^

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
