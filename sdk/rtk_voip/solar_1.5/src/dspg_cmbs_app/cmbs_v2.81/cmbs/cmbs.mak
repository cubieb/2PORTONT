ifdef CMBSDIR 
CMBSBASE:=$(CMBSDIR)/cmbs
includes += -I$(CMBSDIR)/include 
else
CMBSBASE:=$(PROJDIR)/cmbs
endif

includes += -I$(CMBSBASE)
cmbs_objects:=
####################################################################
# settle includes

#includes += -I$(FRAMEBASE) -I$(FRAMEBASE)/vone 
####################################################################
# settle objects

cmbs_objects += $(OBJDIR)/cmbs_int.o
cmbs_objects += $(OBJDIR)/cmbs_api.o
cmbs_objects += $(OBJDIR)/cmbs_dsr.o
cmbs_objects += $(OBJDIR)/cmbs_dee.o
cmbs_objects += $(OBJDIR)/cmbs_dem.o
cmbs_objects += $(OBJDIR)/cmbs_ie.o
cmbs_objects += $(OBJDIR)/cmbs_cmd.o

####################################################################
# settle vpath

vpath %.c 	$(CMBSBASE)
