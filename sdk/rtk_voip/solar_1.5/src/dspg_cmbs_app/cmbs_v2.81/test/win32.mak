CTARGET :=

#--------------------------------------------------------------------
# "C" compiler flags
# -------------------------------

CMODE := /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(OBJDIR)\$(APPNAME).pch" /YX /Fd"$(OBJDIR)\\" /FD /c

#--------------------------------------------------------------------
# the target "C" compiler:
# ---------------------------------------

CC=cl.exe $(CTARGET) $(CMODE) $(includes) $(coptions)
CPP=cl.exe $(CTARGET) $(CMODE) $(includes) $(coptions)

ARCHIEVE=lib /OUT:$(CMBS_LIB)
################################################################################
# Linker
LINKCMD:= /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\$(APPNAME).pdb" /machine:I386 /out:"$(OUTDIR)\$(APPNAME).exe"

LIBS += kernel32.lib user32.lib gdi32.lib 
LIBS += winspool.lib comdlg32.lib advapi32.lib shell32.lib 
LIBS += ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  
LIBS += kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib 
LIBS += advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  winmm.lib
LIBS += $(CMBS_LIB)

LFLAGS=$(LINKCMD) $(LIBS) 
LINK=link.exe

################################################################################
# Rules

$(OBJDIR)/%.o:	%.c
	$(CC) -c $< /Fo$(OBJDIR)/$*.o

$(OBJDIR)/%.o:	%.cpp
	$(CPP) -c $< /Fo$(OBJDIR)/$*.o

