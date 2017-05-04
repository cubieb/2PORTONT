#uc makefile
UC_ROOT=$(shell pwd)
UC_BOA_LINUX_PATH=$(UC_ROOT)/../../boa/src-project/LINUX


CLIENT_EXEC = ucc
CLIENT_OBJS = uc_udp.o uc_mib.o uc_client.o

RELAY_EXEC = ucrelay
RELAY_OBJS = uc_udp.o uc_relay.o

SERVER_EXEC = ucd
SERVER_OBJS = uc_udp.o uc_mib.o uc_cmd.o uc_xdsl.o uc_sys.o uc_server.o

STARTSLV_EXEC = ucstartslv
STARTSLV_OBJS = uc_startslv.o
STARTSLV_DEPH = $(UC_BOA_LINUX_PATH)/utility.h $(UC_BOA_LINUX_PATH)/mib.h


DEPH = uc_udp.h uc_mib.h uc_cmd.h uc_sys.h uc_adslif.h uc_xdsl.h
CFLAGS += -Wall

PROG_ALL = $(CLIENT_EXEC)
ifdef CONFIG_USER_NFBI_MASTER
PROG_ALL += $(RELAY_EXEC) 
ifdef CONFIG_USER_XDSL_SLAVE
PROG_ALL += $(STARTSLV_EXEC)
endif #CONFIG_USER_XDSL_SLAVE
endif #CONFIG_USER_NFBI_SLAVE


ifdef CONFIG_USER_NFBI_SLAVE
PROG_ALL += $(SERVER_EXEC) 
SERVER_OBJS += uc_mdio.o
CFLAGS += -DWITH_MDIO
DEPH += uc_mdio.h
DEPH += ../../../linux-2.6.x/drivers/char/rtl867x_nfbi/rtl867x_mdio.h
endif #CONFIG_USER_NFBI_SLAVE


all: $(PROG_ALL)

%.o: %.c $(DEPH)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(SERVER_OBJS)

$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(CLIENT_OBJS)

$(RELAY_EXEC): $(RELAY_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(RELAY_OBJS)

uc_startslv.o: uc_startslv.c $(DEPH)
	$(CC) $(CFLAGS) -I$(UC_BOA_LINUX_PATH) -c -o uc_startslv.o uc_startslv.c

$(STARTSLV_EXEC): $(STARTSLV_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(STARTSLV_OBJS) $(LIBMIB)

romfs:
	$(ROMFSINST) /bin/$(CLIENT_EXEC)
ifdef CONFIG_USER_NFBI_MASTER
	$(ROMFSINST) /bin/$(RELAY_EXEC)
ifdef CONFIG_USER_XDSL_SLAVE
	$(ROMFSINST) /bin/$(STARTSLV_EXEC)
endif #CONFIG_USER_XDSL_SLAVE
endif #CONFIG_USER_NFBI_MASTER
ifdef CONFIG_USER_NFBI_SLAVE
	$(ROMFSINST) /bin/$(SERVER_EXEC)
endif #CONFIG_USER_NFBI_SLAVE

	
clean:
	-rm -f $(STARTSLV_EXEC) $(SERVER_EXEC) $(CLIENT_EXEC) $(RELAY_EXEC) *.elf *.o
