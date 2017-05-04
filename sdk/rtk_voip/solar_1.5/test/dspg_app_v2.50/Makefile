include ../../.linux_config

CMBS_PATH = .

LDFLAGS = -lpthread
ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
CFLAGS += -DCONFIG_RTK_VOIP_DECT_SPI_SUPPORT
endif
ifdef CONFIG_RTK_VOIP_DECT_UART_SUPPORT
CFLAGS += -DCONFIG_RTK_VOIP_DECT_UART_SUPPORT
endif
CFLAGS += -I../../includes -I$(CMBS_PATH)/test -I$(CMBS_PATH)/test/appcmbs -I$(CMBS_PATH)/cmbs -I$(CMBS_PATH)/frame -I$(CMBS_PATH)/frame/linux -I$(CMBS_PATH)/include -I$(CMBS_PATH)/util -I$(CMBS_PATH)/bus -Wall
CFLAGS += -lpthread -D__linux__ -DCMBS_CATIQ
#CFLAGS += -DMY_TEST_MAIN
#CFLAGS += -O2
#CFLAGS += -g --static	# For GDB debugger
CFLAGS += --static


TARGET = dspg_cmbs_tcx

OBJS = $(TEST_OBJ) $(CMBS_OBJ) $(FRAME_OBJ) $(UTILITY_OBJ) $(BUS_OBJ) $(TEST_OBJ2) $(KEYB_OBJ)
OBJS += $(MAIN_OBJ)
#OBJS += $(MAIN2_OBJ)

MAIN_OBJ = $(CMBS_PATH)/test/tcx_cmbs.o
MAIN2_OBJ = $(CMBS_PATH)/test/dect_test_main.o $(CMBS_PATH)/test/dect_test_scenario.o

TEST_OBJ = $(CMBS_PATH)/test/tcx_log.o $(CMBS_PATH)/test/appcmbs/cmbs_str.o $(CMBS_PATH)/test/appcmbs/appswup.o $(CMBS_PATH)/test/appcmbs/appsrv.o \
		$(CMBS_PATH)/test/appcmbs/appcmbs.o $(CMBS_PATH)/test/appcmbs/appcall.o

KEYB_OBJ = $(CMBS_PATH)/test/tcx_keyb.o $(CMBS_PATH)/test/keyb_swup.o $(CMBS_PATH)/test/keyb_srv.o $(CMBS_PATH)/test/keyb_call.o

TEST_OBJ += $(CMBS_PATH)/test/appcmbs/appla2.o

TEST_OBJ2 = $(CMBS_PATH)/test/appcmbs/appdata.o $(CMBS_PATH)/test/appcmbs/appfacility.o $(CMBS_PATH)/test/appcmbs/applistacc.o \
			$(CMBS_PATH)/test/keyb_data.o $(CMBS_PATH)/test/keyb_facility.o

CMBS_OBJ = $(CMBS_PATH)/cmbs/cmbs_int.o $(CMBS_PATH)/cmbs/cmbs_api.o $(CMBS_PATH)/cmbs/cmbs_dsr.o $(CMBS_PATH)/cmbs/cmbs_dee.o \
			$(CMBS_PATH)/cmbs/cmbs_dem.o $(CMBS_PATH)/cmbs/cmbs_ie.o $(CMBS_PATH)/cmbs/cmbs_cmd.o

FRAME_OBJ = $(CMBS_PATH)/frame/cfr_spi.o $(CMBS_PATH)/frame/cfr_uart.o $(CMBS_PATH)/frame/cfr_ie.o $(CMBS_PATH)/frame/cfr_cmbs.o

UTILITY_OBJ = #$(CMBS_PATH)/util/cmbs_pram_app.o $(CMBS_PATH)/util/cmbs_pram_aslave.o $(CMBS_PATH)/util/cmbs_pram_booter.o \
			$(CMBS_PATH)/util/crc16.o util/pflash.o

BUS_OBJ = $(CMBS_PATH)/bus/bus.o


all:	$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)



clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
