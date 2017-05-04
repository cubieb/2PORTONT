################################################################################
#
#	Copyright 2009 DOSCH & AMAND Research GmbH&Co.KG
#
#	Author	:	Ralf Stein, Dirk Kelbch
#	Date	:	09-Feb-2009
#
#	History
#	----------------------------------------------------------------------------
#
#################################################################################

.KEEP_STATE:
.SUFFIXES:
.SUFFIXES: .c .cpp .h .hpp .asm .s .inc .o .elf .d .dep .def .lex .y .ypp

libraries :=
symfiles  :=
objects   :=
linkpath  :=

APPNAME := cmbs_tcx

ifeq ($(OS),Windows_NT)
HOST_OS = WIN32
else
HOST_OS = UNIX
endif

ifeq ($(notdir $(basename $(SHELL))),cmd)
### SHELL=cmd.exe
RM := del /Q
SL := \$(EMPTY)
else
### SHELL=sh
RM := rm -f
SL := /
endif

################################################################################
#
# Directory
#
# define output,intermediate, object, project and base directory
#
BASE :=.
PROJDIR := ..

OUTDIR := $(BASE)$(SL)out
OBJDIR := $(BASE)$(SL)obj
LIBDIR := $(PROJDIR)$(SL)lib
################################################################################
#
# DNA C-Option definition
#
#
#
coptions := -DCMBS_API -DDNA_TAG

################################################################################
#
# DNA include
#
#
includes := -I. -I$(PROJDIR)$(SL)include -I$(PROJDIR)$(SL)cmbs

################################################################################

objects := $(OBJDIR)/tcx_cmbs.o
objects += $(OBJDIR)/tcx_keyb.o
objects += $(OBJDIR)/tcx_log.o
objects += $(OBJDIR)/keyb_srv.o
objects += $(OBJDIR)/keyb_call.o
objects += $(OBJDIR)/keyb_swup.o

vpath %.c $(BASE)

ifeq ($(HOST_OS),WIN32)
CMBS_LIBNAME := cmbs_host_win32.lib
CMBS_LIB := $(LIBDIR)$(SL)$(CMBS_LIBNAME)
else
CMBS_LIBNAME := cmbs_host_lnx
CMBS_LIB := $(LIBDIR)/lib$(CMBS_LIBNAME).a
linkpath += -L$(LIBDIR)
endif

################################################################################
# tcx core
ifeq ($(TC),XML)
include $(CORE)$(SL)tcx$(SL)tcx.mak
endif

################################################################################
# host cmbs application
include $(BASE)$(SL)appcmbs$(SL)happcmbs.mak


################################################################################
# framework
ifeq ($(HOST_OS),WIN32)
include $(PROJDIR)$(SL)frame$(SL)win32$(SL)frame.mak
else
include $(PROJDIR)/frame/linux/frame.mak
endif

################################################################################
# CMBS
include $(PROJDIR)$(SL)cmbs$(SL)cmbs.mak

################################################################################
# rules

ifeq ($(HOST_OS),WIN32)
include  win32.mak
else
include  linux.mak
endif

all:  dirs $(OUTDIR)/$(APPNAME)

dirs: $(OBJDIR) $(OUTDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(CMBS_LIB):	$(cfr_objects) $(cmbs_objects)
	$(ARCHIEVE) $(cfr_objects) $(cmbs_objects)

clean:
	$(RM) $(OBJDIR)$(SL)*
ifeq ($(HOST_OS),WIN32)
	$(RM) $(OUTDIR)$(SL)$(APPNAME).exe
else
	$(RM) $(OUTDIR)$(SL)$(APPNAME)
endif

$(OUTDIR)/$(APPNAME): $(CMBS_LIB) $(objects) 
ifeq ($(HOST_OS),WIN32)
	$(LINK)  $(LFLAGS) $(objects) $(linkpath)
else
	$(LINK)  $(objects) $(LFLAGS) $(linkpath)
endif


################################################################################
#
# END
