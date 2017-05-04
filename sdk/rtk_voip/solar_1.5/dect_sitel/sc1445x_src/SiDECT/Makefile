# Naming our phony targets
#.PHONY: clean install

PROJECT_SOURCES = cvmcon.c bus232.c busSPI.c hdlc.c 
EXECUTABLES      = dect_cvm
DISTR_DIR	= /home/manosd/src/ata_02_09/SC14450-uClinux-distr
ROMFS_DIR	= $(DISTR_DIR)/romfs
PROJECT_OBJECTS = $(PROJECT_SOURCES:.c=.o) 

#CROSSCOMPILE_FLAGS = -mcr16cplus -mdata-model=far -mint32 

CFLAGS		= $(CROSSCOMPILE_FLAGS) -c  -O0 -D__USE_POSIX199309 #-mno-align-double -w -Wall
#-I $(DISTR_DIR)/linux-2.6.x/drivers/char/sc14450_n_rtx_hdlc
#LDFLAGS	=
#LDFLAGS		= $(CROSSCOMPILE_FLAGS)  -Wl,-elf2flt,-static,--no-relax
LDFLAGS		= $(CROSSCOMPILE_FLAGS)  -Wl,-static,--no-relax
LIBS =  -lrt -lpthread
 

all: $(EXECUTABLES) 
$(EXECUTABLES): $(PROJECT_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(PROJECT_OBJECTS) $(LIBS)
	#$(CC) $(PROJECT_OBJECTS)  $(LDFLAGS) -o $@
#$(CC) $(LDFLAGS) $@.o -o $@

# Removing the executable and the object files
clean:
	rm -rf $(EXECUTABLES) $(PROJECT_OBJECTS) $(PROJECT_SOURCES:.c=.gdb)
 		  #echo clean: make complete

# Installing the final product
install:
		chmod a+x $(EXECUTABLES)
		cp $(EXECUTABLES) $(ROMFS_DIR)/bin/
		echo install: make complete

