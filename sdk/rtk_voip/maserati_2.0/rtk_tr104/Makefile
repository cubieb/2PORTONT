include ../.linux_config

# parameter structure (VoIP)
VOIP_TR104_LIB=voip_tr104.a
LINK_LIBS = ../flash/libvoip_flash_client.a
OBJS = prmt_voiceservice.o prmt_capabilities.o prmt_voice_profile.o prmt_phyinterface.o\
       prmt_voice_profile_line.o prmt_line_codec_list.o 
OBJS += porting_tr104.o

#for ipc
OBJS +=cwmpevt.o cwmp_main_tr104.o 

# other assistant 
OBJS += str_utility.o str_mib.o mib_tr104.o 

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_865X), y)
	MIB_INCLUDE=-I../../../include -I../../mib
endif
 
TR069_DIR=$(DIR_USERS)/cwmp-tr069
#TR069_INCLUDE=-I$(TR069_DIR)/cwmpClient/ -I$(TR069_DIR)/include/ -I../../boa/apmib/
TR069_INCLUDE=-I$(TR069_DIR)/cwmpClient/ -I$(TR069_DIR)/include/ -I$(ROOTDIR)/include/rtk

CFLAGS += -I../ -I../includes/ $(TR069_INCLUDE) $(MIB_INCLUDE) -Wall -O 
#CFLAGS += -I../ -I../includes/ -I../../cwmp-tr069/include/ -I../../cwmp-tr069/cwmpClient/ \
          -I../../boa/src-project/LINUX  $(MIB_INCLUDE) -Wall -O 

ifeq ($(CONFIG_RTK_VOIP_IP_PHONE), y)
CFLAGS      +=  -I../ui/include/
endif

all: $(VOIP_TR104_LIB)

$(VOIP_TR104_LIB): $(OBJS) 
	$(AR) rcs $(VOIP_TR104_LIB) $(OBJS) 

%.o: %.c $(DEPH)
	$(CC) $(CFLAGS) -c -o $@ $< 

dep:
	$(CC) $(CFLAGS) -MM *.c > .depend 

clean:
	-rm -f $(EXEC) *.o
	-rm -f $(EXEC) *.a
	-rm -f .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
