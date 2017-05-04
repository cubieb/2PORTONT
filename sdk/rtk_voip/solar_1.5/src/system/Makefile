include ../build/Makeparm.cfg
include ../build/Makeparm.def

OBJS		= alarm.o abbreviated.o pstnroute.o  icmp.o

LIB_OBJ		= $(foreach obj,$(OBJS),$(OBJ_DIR)/$(obj))
DEPENDENCY	= $(LIB_OBJ:.o=.d)

lib: $(OBJ_DIR) $(ATA_SYSTEM_LIB)

clean:
	rm -f $(ATA_SYSTEM_LIB) $(LIB_OBJ) $(DEPENDENCY)

$(OBJ_DIR):
	mkdir -p $@

$(ATA_SYSTEM_LIB): $(LIB_OBJ)
	$(AR) rcs $(ATA_SYSTEM_LIB) $^

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
