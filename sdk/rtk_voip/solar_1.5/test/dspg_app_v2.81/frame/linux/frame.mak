FRAMEBASE:=$(PROJDIR)/frame

cfr_objects:=
####################################################################
# settle includes

includes += -I$(FRAMEBASE) -I$(FRAMEBASE)/linux 

####################################################################
# settle objects

cfr_objects += $(OBJDIR)/cfr_uart.o
cfr_objects += $(OBJDIR)/cfr_cmbs.o
cfr_objects += $(OBJDIR)/cfr_ie.o

####################################################################
# settle vpath

vpath %.c 	$(FRAMEBASE)
# linux
vpath %.c   $(FRAMEBASE)/linux
