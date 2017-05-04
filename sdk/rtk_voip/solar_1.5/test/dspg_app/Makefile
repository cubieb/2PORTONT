include ../../.linux_config

CFLAGS += -I../../includes -Itest -Itest/appcmbs -Icmbs -Iframe -Iinclude -Iutil -Ibus -Wall -lpthread -D__linux__
#CFLAGS += -O2
#CFLAGS += -g --static

TARGET = dspg_app

#objs = bus.o dect_test.o

objs = $(TEST_OBJ) $(CMBS_OBJ) $(FRAME_OBJ) $(UTILITY_OBJ) $(BUS_OBJ)

TEST_OBJ = test/tcx_log.o test/tcx_keyb.o test/tcx_cmbs.o test/keyb_swup.o test/keyb_srv.o test/keyb_call.o \
		test/appcmbs/cmbs_str.o test/appcmbs/appswup.o test/appcmbs/appsrv.o test/appcmbs/appcmbs.o test/appcmbs/appcall.o

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
