include ../../.linux_config

CFLAGS += -I../../includes -Itest -Itest/appcmbs -Icmbs -Iframe -Iinclude -Iutil -Ibus -Wall -lpthread -D__linux__ -DCMBS_CATIQ
#CFLAGS += -DMY_TEST_MAIN
#CFLAGS += -O2
#CFLAGS += -g --static	# For GDB debugger

TARGET = dspg_app

objs = $(MAIN_OBJ) $(TEST_OBJ) $(CMBS_OBJ) $(FRAME_OBJ) $(UTILITY_OBJ) $(BUS_OBJ)
objs += $(TEST_OBJ2)

MAIN_OBJ = test/dect_test_main.o test/dect_test_scenario.o test/tcx_cmbs.o

TEST_OBJ = test/tcx_log.o test/tcx_keyb.o test/keyb_swup.o test/keyb_srv.o test/keyb_call.o \
		test/appcmbs/cmbs_str.o test/appcmbs/appswup.o test/appcmbs/appsrv.o test/appcmbs/appcmbs.o test/appcmbs/appcall.o

TEST_OBJ2 = test/appcmbs/appdata.o test/appcmbs/appfacility.o test/appcmbs/applistacc.o test/keyb_data.o test/keyb_facility.o

CMBS_OBJ = cmbs/cmbs_int.o cmbs/cmbs_api.o cmbs/cmbs_dsr.o cmbs/cmbs_dee.o cmbs/cmbs_dem.o cmbs/cmbs_ie.o cmbs/cmbs_cmd.o

FRAME_OBJ = frame/cfr_spi.o frame/cfr_uart.o frame/cfr_ie.o frame/cfr_cmbs.o

UTILITY_OBJ = #util/cmbs_pram_app.o util/cmbs_pram_aslave.o util/cmbs_pram_booter.o util/crc16.o util/pflash.o

BUS_OBJ = bus/bus.o

all:	$(objs)
	$(CC) -o $(TARGET) $(objs) $(CFLAGS)


clean:
	rm -f $(objs)
	rm -f $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
