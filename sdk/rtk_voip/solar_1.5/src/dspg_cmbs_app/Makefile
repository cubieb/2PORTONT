include ../../.linux_config

CMBS_PATH = cmbs_tcx

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

OBJS = $(MAIN_OBJ) $(TEST_OBJ) $(CMBS_OBJ) $(FRAME_OBJ) $(UTILITY_OBJ) $(TEST_OBJ2) #$(BUS_OBJ)
OBJS += $(FRAME_OBJ_UART)
# For CMBS V2.81
OBJS += $(TEST_OBJ3) #$(KEYB_OBJ)

################################################################################################################

MAIN_OBJ = $(CMBS_PATH)/test/dect_test_main.o $(CMBS_PATH)/test/dect_test_scenario.o $(CMBS_PATH)/test/tcx_cmbs.o

KEYB_OBJ = $(CMBS_PATH)/test/tcx_keyb.o $(CMBS_PATH)/test/keyb_swup.o $(CMBS_PATH)/test/keyb_srv.o $(CMBS_PATH)/test/keyb_call.o

TEST_OBJ = $(CMBS_PATH)/test/tcx_log.o $(CMBS_PATH)/test/appcmbs/cmbs_str.o $(CMBS_PATH)/test/appcmbs/appswup.o \
		$(CMBS_PATH)/test/appcmbs/appsrv.o $(CMBS_PATH)/test/appcmbs/appcmbs.o $(CMBS_PATH)/test/appcmbs/appcall.o

CMBS_OBJ = $(CMBS_PATH)/cmbs/cmbs_int.o $(CMBS_PATH)/cmbs/cmbs_api.o $(CMBS_PATH)/cmbs/cmbs_dsr.o $(CMBS_PATH)/cmbs/cmbs_dee.o \
			$(CMBS_PATH)/cmbs/cmbs_dem.o $(CMBS_PATH)/cmbs/cmbs_ie.o $(CMBS_PATH)/cmbs/cmbs_cmd.o

FRAME_OBJ = $(CMBS_PATH)/frame/cfr_ie.o $(CMBS_PATH)/frame/linux/cfr_cmbs.o 
FRAME_OBJ_UART = $(CMBS_PATH)/frame/linux/cfr_uart.o
FRAME_OBJ_SPI = $(CMBS_PATH)/frame/linux/cfr_spi.o

UTILITY_OBJ = #$(CMBS_PATH)/util/cmbs_pram_app.o $(CMBS_PATH)/util/cmbs_pram_aslave.o $(CMBS_PATH)/util/cmbs_pram_booter.o \
			$(CMBS_PATH)/util/crc16.o util/pflash.o
# For CMBS V2.40. 2.50
TEST_OBJ += $(CMBS_PATH)/test/appcmbs/appla2.o

TEST_OBJ2 = $(CMBS_PATH)/test/appcmbs/appdata.o $(CMBS_PATH)/test/appcmbs/appfacility.o $(CMBS_PATH)/test/appcmbs/applistacc.o \
			$(CMBS_PATH)/test/keyb_data.o $(CMBS_PATH)/test/keyb_facility.o

# For CMBS V2.81
TEST_OBJ3 = $(CMBS_PATH)/test/tcx_eep.o $(CMBS_PATH)/test/ListAccess/LASessionMgr.o $(CMBS_PATH)/test/ListAccess/ListChangeNotif.o \
		$(CMBS_PATH)/test/ListAccess/ListsApp.o $(CMBS_PATH)/test/ListAccess/sqlite3.o $(CMBS_PATH)/test/ListAccess/SQLiteWrapper.o

BUS_OBJ = $(CMBS_PATH)/bus/bus.o

DEPENDENCY  = $(OBJS:.o=.d)
LDFLAGS     =   -lpthread -ldl
CFLAGS += -I../../includes -I$(CMBS_PATH)/test -I$(CMBS_PATH)/test/appcmbs -I$(CMBS_PATH)/cmbs -I$(CMBS_PATH)/frame -I$(CMBS_PATH)/frame/linux -I$(CMBS_PATH)/include -I$(CMBS_PATH)/util -I$(CMBS_PATH)/bus -I$(CMBS_PATH)/test/ListAccess -Wall
CFLAGS += -D__linux__ -DCMBS_CATIQ
CFLAGS += -DRTK_SIP_INTEGRATED
CFLAGS += -fPIC
#CFLAGS += -O2
#CFLAGS += -g --static	# For GDB debugger

DSPG_DECT_LIB = dspg_cmbs_app_release.a

all: $(OBJS)
	$(AR) rcs $(DSPG_DECT_LIB) $^

voip_manager.o: ../../voip_manager/voip_manager.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(DEPENDENCY)
	rm -f $(DSPG_DECT_LIB)
	rm -f voip_manager.o

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<
	$(CC) -M $(CFLAGS) $< | sed -e "s#^$(*F).o[ :]*#$(@D)\/$(*F).o : #" > $(@:.o=.d)
	
