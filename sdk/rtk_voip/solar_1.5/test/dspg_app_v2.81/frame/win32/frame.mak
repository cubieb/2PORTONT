ifdef CMBSDIR 
FRAMEBASE:=$(CMBSDIR)/frame
else
FRAMEBASE:=$(PROJDIR)/frame
endif

cfr_objects:=
####################################################################
# settle includes

includes += -I$(FRAMEBASE) -I$(FRAMEBASE)/win32

####################################################################
# settle objects

ifeq ($(HOST_DLL_SUPPORT),0)
cfr_objects += $(OBJDIR)/cfr_uart.o
cfr_objects += $(OBJDIR)/cphysicalport.o
cfr_objects += $(OBJDIR)/cfr_mssg.o
endif
cfr_objects += $(OBJDIR)/cfr_cmbs.o
cfr_objects += $(OBJDIR)/cfr_ie.o

####################################################################
# settle vpath

vpath %.c 	$(FRAMEBASE)
# win32
vpath %.c   $(FRAMEBASE)/win32
vpath %.cpp $(FRAMEBASE)/win32