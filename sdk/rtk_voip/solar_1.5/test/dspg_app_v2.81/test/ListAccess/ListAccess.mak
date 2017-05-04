ifdef CMBSDIR
LISTACCESSBASE:=$(CMBSDIR)/test/ListAccess
else
LISTACCESSBASE:=$(BASE)/ListAccess
endif

####################################################################
# settle includes
includes += -I$(LISTACCESSBASE) 

####################################################################
# settle objects
objects += $(OBJDIR)/sqlite3.o
objects += $(OBJDIR)/SQLiteWrapper.o
objects += $(OBJDIR)/ListsApp.o
objects += $(OBJDIR)/LASessionMgr.o
objects += $(OBJDIR)/ListChangeNotif.o

####################################################################
# Libraries
ifeq ($(HOST_OS),UNIX)
LIBS += -ldl
endif

####################################################################
# settle vpath
vpath %.c 	$(LISTACCESSBASE)
