# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "ncgen.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
OUTDIR=.
INTDIR=.

ALL : $(OUTDIR)/ncgen.exe $(OUTDIR)/ncgen.bsc

# ADD BASE CPP /nologo /ML /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /ML /W3 /GX /O2 /I "..\xdr" /I "..\libsrc" /I "..\..\hdf\src" /I "..\..\hdf\jpeg" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "HDF" /D "NO_SYS_XDR_INC" /c
# SUBTRACT CPP /YX /Fr
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\xdr" /I "..\libsrc" /I "..\..\hdf\src"\
 /I "..\..\hdf\jpeg" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "HDF" /D\
 "NO_SYS_XDR_INC" /Fo$(INTDIR)/ /c 
CPP_OBJS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
BSC32_SBRS= \
	
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"ncgen.bsc" 

$(OUTDIR)/ncgen.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/init.obj \
	$(INTDIR)/generate.obj \
	$(INTDIR)/main.obj \
	$(INTDIR)/escapes.obj \
	$(INTDIR)/load.obj \
	$(INTDIR)/close.obj \
	$(INTDIR)/genlib.obj \
	$(INTDIR)/getfill.obj \
	$(INTDIR)/msofttab.obj
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\xdr\win32xdr.lib ..\libsrc\win32cdf.lib ..\util\win32utl.lib ..\..\hdf\src\win32hdf.lib ..\..\hdf\jpeg\win32jpg.lib /NOLOGO /SUBSYSTEM:console /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\xdr\win32xdr.lib ..\libsrc\win32cdf.lib ..\util\win32utl.lib\
 ..\..\hdf\src\win32hdf.lib ..\..\hdf\jpeg\win32jpg.lib /NOLOGO\
 /SUBSYSTEM:console /INCREMENTAL:no /PDB:$(OUTDIR)/"ncgen.pdb" /MACHINE:I386\
 /OUT:$(OUTDIR)/"ncgen.exe" 

$(OUTDIR)/ncgen.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
OUTDIR=.
INTDIR=.

ALL : $(OUTDIR)/ncgen.exe $(OUTDIR)/ncgen.bsc

# ADD BASE CPP /nologo /ML /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /c
# ADD CPP /nologo /ML /W3 /GX /Zi /Od /I "..\xdr" /I "..\libsrc" /I "..\..\hdf\src" /I "..\..\hdf\jpeg" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "HDF" /D "NO_SYS_XDR_INC" /c
# SUBTRACT CPP /YX /Fr
CPP_PROJ=/nologo /ML /W3 /GX /Zi /Od /I "..\xdr" /I "..\libsrc" /I\
 "..\..\hdf\src" /I "..\..\hdf\jpeg" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "HDF" /D "NO_SYS_XDR_INC" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"ncgen.pdb" /c 
CPP_OBJS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
BSC32_SBRS= \
	
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"ncgen.bsc" 

$(OUTDIR)/ncgen.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/init.obj \
	$(INTDIR)/generate.obj \
	$(INTDIR)/main.obj \
	$(INTDIR)/escapes.obj \
	$(INTDIR)/load.obj \
	$(INTDIR)/close.obj \
	$(INTDIR)/genlib.obj \
	$(INTDIR)/getfill.obj \
	$(INTDIR)/msofttab.obj
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\xdr\win32xdr.lib ..\libsrc\win32cdf.lib ..\util\win32utl.lib ..\..\hdf\src\win32hdf.lib ..\..\hdf\jpeg\win32jpg.lib /NOLOGO /SUBSYSTEM:console /DEBUG /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\xdr\win32xdr.lib ..\libsrc\win32cdf.lib ..\util\win32utl.lib\
 ..\..\hdf\src\win32hdf.lib ..\..\hdf\jpeg\win32jpg.lib /NOLOGO\
 /SUBSYSTEM:console /INCREMENTAL:yes /PDB:$(OUTDIR)/"ncgen.pdb" /DEBUG\
 /MACHINE:I386 /OUT:$(OUTDIR)/"ncgen.exe" 

$(OUTDIR)/ncgen.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\ncgen.h
# End Source File
################################################################################
# Begin Source File

SOURCE=.\init.c

$(INTDIR)/init.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\generate.c

$(INTDIR)/generate.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\main.c

$(INTDIR)/main.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\escapes.c

$(INTDIR)/escapes.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\generic.h
# End Source File
################################################################################
# Begin Source File

SOURCE=.\load.c

$(INTDIR)/load.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\close.c

$(INTDIR)/close.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\genlib.c

$(INTDIR)/genlib.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\genlib.h
# End Source File
################################################################################
# Begin Source File

SOURCE=.\getfill.c

$(INTDIR)/getfill.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\msofttab.c

$(INTDIR)/msofttab.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\msofttab.h
# End Source File
# End Group
# End Project
################################################################################
