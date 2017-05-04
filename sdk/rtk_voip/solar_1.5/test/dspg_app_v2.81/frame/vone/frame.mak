ifdef CMBSDIR
FRAMEBASE:=$(CMBSDIR)/frame
else
FRAMEBASE:=$(PROJDIR)/frame
endif
####################################################################
# settle includes

includes += -I$(FRAMEBASE) -I$(FRAMEBASE)/vone 

####################################################################
# settle objects

objects += $(OBJDIR)/cfr_uart.otwi
objects += $(OBJDIR)/cfr_task.otwi
objects += $(OBJDIR)/cfr_cmbs.otwi
objects += $(OBJDIR)/cfr_ie.otwi

####################################################################
# settle vpath

vpath %.c 	$(FRAMEBASE)
vpath %.c   $(FRAMEBASE)/vone
