include ../build/Makeparm.cfg
include ../build/Makeparm.def

#CFLAGS += -Wall -Os -mips16 
#CFLAGS := -Wall -Os

OBJS		= csv_api.o csv_parser.o
LIB_OBJ		= $(foreach obj,$(OBJS),$(OBJ_DIR)/$(obj))
DEPENDENCY	= $(LIB_OBJ:.o=.d)

#lib: $(OBJ_DIR) $(CSV_PARSER_LIB) $(CSV_PARSER_SO_LIB)
lib: $(OBJ_DIR) $(CSV_PARSER_LIB) 

$(OBJ_DIR):
	mkdir -p $@
	
$(CSV_PARSER_LIB): $(LIB_OBJ)
	$(AR) rcs $(CSV_PARSER_LIB) $^

$(CSV_PARSER_SO_LIB): $(LIB_OBJ)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $< 

romfs:
	$(ROMFSINST) config_in_dram.csv /etc/config.csv
#	$(ROMFSINST) config_in_flash.csv /etc/config.csv
#	$(ROMFSINST) ./reference/call_log_ref.csv /etc/call_log_ref.csv

clean:
	-rm -rf $(CSV_PARSER_LIB) $(CSV_PARSER_SO_LIB) $(LIB_OBJ) $(DEPENDENCY)

-include $(DEPENDENCY)

.PHONY: all clean 
