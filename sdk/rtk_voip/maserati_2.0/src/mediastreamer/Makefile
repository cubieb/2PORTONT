include ../build/Makeparm.cfg
include ../build/Makeparm.def

OBJS		=	audiostream.o ms.o mscodec.o msfilter.o

LIB_OBJ		= $(foreach obj,$(OBJS),$(OBJ_DIR)/$(obj))
DEPENDENCY	= $(LIB_OBJ:.o=.d)

lib: $(OBJ_DIR) $(MEDIA_LIB)

clean:
	rm -f $(MEDIA_LIB) $(LIB_OBJ) $(DEPENDENCY)

$(OBJ_DIR):
	mkdir -p $@

$(MEDIA_LIB): $(LIB_OBJ)
	$(AR) rcs $(MEDIA_LIB) $^

# pull in dependency info for existing .o files
-include $(DEPENDENCY)
