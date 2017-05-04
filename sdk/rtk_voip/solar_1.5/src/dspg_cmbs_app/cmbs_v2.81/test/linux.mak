CTARGET :=-D_REENTRANT

#--------------------------------------------------------------------
# "C" compiler flags
# -------------------------------

CMODE :=-W -Wall

#--------------------------------------------------------------------
# the target "C" compiler:
# ---------------------------------------

CC=gcc $(CTARGET) $(CMODE) $(includes) $(coptions)
CPP=gcc $(CTARGET) $(CMODE) $(includes) $(coptions)

ARCHIEVE=ar rcs $(CMBS_LIB)
################################################################################
# Linker

LIBS += -lpthread
LIBS += -l$(CMBS_LIBNAME)

LFLAGS=$(LINKCMD) $(LIBS) -o $@
LINK=gcc

################################################################################
# Rules

$(OBJDIR)/%.o:	%.c
	$(CC) -c $< -o$(OBJDIR)/$*.o

$(OBJDIR)/%.o:	%.cpp
	$(CPP) -c $< -o$(OBJDIR)/$*.o

