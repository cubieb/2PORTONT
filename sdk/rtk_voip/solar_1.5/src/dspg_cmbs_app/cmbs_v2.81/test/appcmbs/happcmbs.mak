ifdef CMBSDIR
HAPPBASE:=$(CMBSDIR)/test/appcmbs
else
HAPPBASE:=$(BASE)/appcmbs
endif
####################################################################
# settle includes
includes += -I$(HAPPBASE) 
#linkpath += $(linkxml)

####################################################################
# settle objects

objects += $(OBJDIR)/appcmbs.o
objects += $(OBJDIR)/appsrv.o
objects += $(OBJDIR)/appcall.o
objects += $(OBJDIR)/appswup.o
objects += $(OBJDIR)/cmbs_str.o
objects += $(OBJDIR)/appfacility.o
objects += $(OBJDIR)/appdata.o
objects += $(OBJDIR)/applistacc.o
objects += $(OBJDIR)/appla2.o
####################################################################
# settle vpath

vpath %.c 	$(HAPPBASE)
