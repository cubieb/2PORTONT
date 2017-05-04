include ../../.linux_config

CMBS_PATH = .
RSDK_GCC = mips-linux-gcc

# DECT COnfiguration
ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
CFLAGS += -DCONFIG_RTK_VOIP_DECT_SPI_SUPPORT
endif
ifdef CONFIG_RTK_VOIP_DECT_UART_SUPPORT
CFLAGS += -DCONFIG_RTK_VOIP_DECT_UART_SUPPORT
endif

ifdef CONFIG_RTK_VOIP_DECT_DSPG_UART_HW_FLOW_CTRL
CFLAGS += -DUART_DECT_CMBS_HW_FLOW_CTRL
endif
ifeq "$(CONFIG_RTK_VOIP_DECT_UART_BAUD_RATE)" "115200"
CFLAGS += -DUART_DECT_CMBS_UART_BAUD_B115200
endif
ifeq "$(CONFIG_RTK_VOIP_DECT_UART_BAUD_RATE)" "57600"
CFLAGS += -DUART_DECT_CMBS_UART_BAUD_B57600
endif
ifeq "$(CONFIG_RTK_VOIP_DECT_UART_BAUD_RATE)" "38400"
CFLAGS += -DUART_DECT_CMBS_UART_BAUD_B38400
endif
ifeq "$(CONFIG_RTK_VOIP_DECT_UART_BAUD_RATE)" "19200"
CFLAGS += -DUART_DECT_CMBS_UART_BAUD_B19200
endif
ifeq "$(CONFIG_RTK_VOIP_DECT_UART_BAUD_RATE)" "14400"
CFLAGS += -DUART_DECT_CMBS_UART_BAUD_B14400
endif
ifeq "$(CONFIG_RTK_VOIP_DECT_UART_BAUD_RATE)" "9600"
CFLAGS += -DUART_DECT_CMBS_UART_BAUD_B9600
endif

ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
CFLAGS += -DUART_DECT_TTYS0
else
CFLAGS += -DUART_DECT_TTYS1
endif


LDFLAGS = -lpthread -ldl
CFLAGS += -I../../includes -I$(CMBS_PATH)/test -I$(CMBS_PATH)/test/appcmbs -I$(CMBS_PATH)/cmbs -I$(CMBS_PATH)/frame -I$(CMBS_PATH)/frame/linux -I$(CMBS_PATH)/include -I$(CMBS_PATH)/util -I$(CMBS_PATH)/test/ListAccess -Wall
CFLAGS += -D__linux__ -DCMBS_CATIQ
#CFLAGS += -lpthread -ldl
CFLAGS += --static
CFLAGS += -fPIC
#CFLAGS += -O2
#CFLAGS += -g --static	# For GDB debugger


TARGET = dspg_cmbs_tcx

OBJS = $(TEST_OBJ) $(CMBS_OBJ) $(FRAME_OBJ) $(UTILITY_OBJ) $(TEST_OBJ2) $(TEST_OBJ3) $(KEYB_OBJ)
OBJS += $(MAIN_OBJ)
#OBJS += $(MAIN2_OBJ)

MAIN_OBJ = $(CMBS_PATH)/test/tcx_cmbs.o
MAIN2_OBJ = $(CMBS_PATH)/test/dect_test_main.o $(CMBS_PATH)/test/dect_test_scenario.o

TEST_OBJ = $(CMBS_PATH)/test/tcx_log.o $(CMBS_PATH)/test/tcx_eep.o $(CMBS_PATH)/test/appcmbs/cmbs_str.o $(CMBS_PATH)/test/appcmbs/appswup.o \
		$(CMBS_PATH)/test/appcmbs/appsrv.o $(CMBS_PATH)/test/appcmbs/appcmbs.o $(CMBS_PATH)/test/appcmbs/appcall.o

TEST_OBJ += $(CMBS_PATH)/test/appcmbs/appla2.o

TEST_OBJ2 = $(CMBS_PATH)/test/appcmbs/appdata.o $(CMBS_PATH)/test/appcmbs/appfacility.o $(CMBS_PATH)/test/appcmbs/applistacc.o \
			$(CMBS_PATH)/test/keyb_data.o $(CMBS_PATH)/test/keyb_facility.o

KEYB_OBJ = $(CMBS_PATH)/test/tcx_keyb.o $(CMBS_PATH)/test/keyb_swup.o $(CMBS_PATH)/test/keyb_srv.o $(CMBS_PATH)/test/keyb_call.o

# For CMBS V2.81
TEST_OBJ3 = $(CMBS_PATH)/test/ListAccess/LASessionMgr.o $(CMBS_PATH)/test/ListAccess/ListChangeNotif.o $(CMBS_PATH)/test/ListAccess/ListsApp.o \
                 $(CMBS_PATH)/test/ListAccess/sqlite3.o $(CMBS_PATH)/test/ListAccess/SQLiteWrapper.o


CMBS_OBJ = $(CMBS_PATH)/cmbs/cmbs_int.o $(CMBS_PATH)/cmbs/cmbs_api.o $(CMBS_PATH)/cmbs/cmbs_dsr.o $(CMBS_PATH)/cmbs/cmbs_dee.o \
			$(CMBS_PATH)/cmbs/cmbs_dem.o $(CMBS_PATH)/cmbs/cmbs_ie.o $(CMBS_PATH)/cmbs/cmbs_cmd.o

FRAME_OBJ = $(CMBS_PATH)/frame/linux/cfr_uart.o $(CMBS_PATH)/frame/linux/cfr_cmbs.o $(CMBS_PATH)/frame/cfr_ie.o

UTILITY_OBJ = #$(CMBS_PATH)/util/cmbs_pram_app.o $(CMBS_PATH)/util/cmbs_pram_aslave.o $(CMBS_PATH)/util/cmbs_pram_booter.o \
			$(CMBS_PATH)/util/crc16.o util/pflash.o


all:	$(OBJS)
	$(RSDK_GCC) -o $(TARGET) $(OBJS) $(CFLAGS) $(LDFLAGS)



clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

%.o: %.c
	$(RSDK_GCC) -c -o $@ $(CFLAGS) $<
