# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "mf3d.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/mf3d.lib $(OUTDIR)/mf3d.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"mf3d.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"mf3d.bsc" 
BSC32_SBRS= \
	$(INTDIR)/MF3D.SBR \
	$(INTDIR)/MFTEXTUT.SBR \
	$(INTDIR)/MFLABELS.SBR \
	$(INTDIR)/MFPRIMTV.SBR \
	$(INTDIR)/MFMEMORY.SBR \
	$(INTDIR)/MFBINRD.SBR \
	$(INTDIR)/MFOBJTYP.SBR \
	$(INTDIR)/MFFILE.SBR \
	$(INTDIR)/MFTEXTWR.SBR \
	$(INTDIR)/MFOBJECT.SBR \
	$(INTDIR)/MFGROUPS.SBR \
	$(INTDIR)/MFRSLNTN.SBR \
	$(INTDIR)/MFTEXTRD.SBR \
	$(INTDIR)/MFSTDCHK.SBR

$(OUTDIR)/mf3d.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO
LIB32_FLAGS=/NOLOGO /OUT:$(OUTDIR)\"mf3d.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	$(INTDIR)/MF3D.OBJ \
	$(INTDIR)/MFTEXTUT.OBJ \
	$(INTDIR)/MFLABELS.OBJ \
	$(INTDIR)/MFPRIMTV.OBJ \
	$(INTDIR)/MFMEMORY.OBJ \
	$(INTDIR)/MFBINRD.OBJ \
	$(INTDIR)/MFOBJTYP.OBJ \
	$(INTDIR)/MFFILE.OBJ \
	$(INTDIR)/MFTEXTWR.OBJ \
	$(INTDIR)/MFOBJECT.OBJ \
	$(INTDIR)/MFGROUPS.OBJ \
	$(INTDIR)/MFRSLNTN.OBJ \
	$(INTDIR)/MFTEXTRD.OBJ \
	$(INTDIR)/MFSTDCHK.OBJ

$(OUTDIR)/mf3d.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/mf3d.lib $(OUTDIR)/mf3d.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE CPP /nologo /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /W3 /GX /Z7 /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"mf3d.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinDebug/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"mf3d.bsc" 
BSC32_SBRS= \
	$(INTDIR)/MF3D.SBR \
	$(INTDIR)/MFTEXTUT.SBR \
	$(INTDIR)/MFLABELS.SBR \
	$(INTDIR)/MFPRIMTV.SBR \
	$(INTDIR)/MFMEMORY.SBR \
	$(INTDIR)/MFBINRD.SBR \
	$(INTDIR)/MFOBJTYP.SBR \
	$(INTDIR)/MFFILE.SBR \
	$(INTDIR)/MFTEXTWR.SBR \
	$(INTDIR)/MFOBJECT.SBR \
	$(INTDIR)/MFGROUPS.SBR \
	$(INTDIR)/MFRSLNTN.SBR \
	$(INTDIR)/MFTEXTRD.SBR \
	$(INTDIR)/MFSTDCHK.SBR

$(OUTDIR)/mf3d.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=lib.exe
# ADD BASE LIB32 /NOLOGO
# ADD LIB32 /NOLOGO
LIB32_FLAGS=/NOLOGO /OUT:$(OUTDIR)\"mf3d.lib" 
DEF_FLAGS=
DEF_FILE=
LIB32_OBJS= \
	$(INTDIR)/MF3D.OBJ \
	$(INTDIR)/MFTEXTUT.OBJ \
	$(INTDIR)/MFLABELS.OBJ \
	$(INTDIR)/MFPRIMTV.OBJ \
	$(INTDIR)/MFMEMORY.OBJ \
	$(INTDIR)/MFBINRD.OBJ \
	$(INTDIR)/MFOBJTYP.OBJ \
	$(INTDIR)/MFFILE.OBJ \
	$(INTDIR)/MFTEXTWR.OBJ \
	$(INTDIR)/MFOBJECT.OBJ \
	$(INTDIR)/MFGROUPS.OBJ \
	$(INTDIR)/MFRSLNTN.OBJ \
	$(INTDIR)/MFTEXTRD.OBJ \
	$(INTDIR)/MFSTDCHK.OBJ

$(OUTDIR)/mf3d.lib : $(OUTDIR)  $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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

SOURCE=.\MF3D.C
DEP_MF3D_=\
	.\MF3D.H\
	.\MFERRORS.H\
	.\MFINT64.H\
	.\MFSTDCHK.H\
	.\MFASSERT.H\
	.\MFFILE.H\
	.\MFGROUPS.H\
	.\MFINTOBJ.H\
	.\MFMACROS.H\
	.\MFMEMORY.H\
	.\MFOBJTYP.H\
	.\MFPRIMTV.H\
	.\MFRSLNTN.H\
	.\MFTEXTRD.H\
	.\MFTEXTWR.H\
	.\MFIOCALL.H\
	.\MFOBJCTS.H\
	.\MFTYPES.H\
	.\MFSYSTYP.H\
	.\MFSUBOBJ.H\
	.\MFOBJECT.H\
	.\MFLABELS.H

$(INTDIR)/MF3D.OBJ :  $(SOURCE)  $(DEP_MF3D_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFTEXTUT.C
DEP_MFTEX=\
	.\MFTEXTUT.H\
	.\MFSYSTYP.H\
	.\MFTYPES.H\
	.\MFASSERT.H\
	.\MFMEMORY.H

$(INTDIR)/MFTEXTUT.OBJ :  $(SOURCE)  $(DEP_MFTEX) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFLABELS.C
DEP_MFLAB=\
	.\MFLABELS.H\
	.\MFERRORS.H\
	.\MFTYPES.H\
	.\MFTEXTUT.H\
	.\MFSYSTYP.H

$(INTDIR)/MFLABELS.OBJ :  $(SOURCE)  $(DEP_MFLAB) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFPRIMTV.C
DEP_MFPRI=\
	.\MFPRIMTV.H\
	.\MF3D.H\
	.\MFERRORS.H\
	.\MFINT64.H\
	.\MFSYSTYP.H\
	.\MFTYPES.H\
	.\MFASSERT.H\
	.\MFBINRD.H\
	.\MFBINUTL.H\
	.\MFINTOBJ.H\
	.\MFLABELS.H\
	.\MFMACROS.H\
	.\MFMEMORY.H\
	.\MFOBJTYP.H\
	.\MFTEXTRD.H\
	.\MFTEXTST.H\
	.\MFTEXTUT.H\
	.\MFTEXTWR.H\
	.\MFIOCALL.H\
	.\MFOBJCTS.H\
	.\MFSUBOBJ.H\
	.\MFOBJECT.H

$(INTDIR)/MFPRIMTV.OBJ :  $(SOURCE)  $(DEP_MFPRI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFMEMORY.C
DEP_MFMEM=\
	.\MFMEMORY.H\
	.\MFASSERT.H

$(INTDIR)/MFMEMORY.OBJ :  $(SOURCE)  $(DEP_MFMEM) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFBINRD.C
DEP_MFBIN=\
	.\MFBINRD.H\
	.\MF3D.H\
	.\MFINT64.H\
	.\MFERRORS.H\
	.\MFMACROS.H\
	.\MFMEMORY.H\
	.\MFOBJECT.H\
	.\MFTYPES.H\
	.\MFINTOBJ.H\
	.\MFIOCALL.H\
	.\MFOBJCTS.H\
	.\MFSYSTYP.H\
	.\MFASSERT.H\
	.\MFSUBOBJ.H

$(INTDIR)/MFBINRD.OBJ :  $(SOURCE)  $(DEP_MFBIN) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFOBJTYP.C
DEP_MFOBJ=\
	.\MFOBJTYP.H\
	.\MFERRORS.H\
	.\MFOBJCTS.H\
	.\MFTYPES.H\
	.\MFASSERT.H\
	.\MFINTOBJ.H\
	.\MFTEXTRD.H\
	.\MFTEXTUT.H\
	.\MFOBJECT.H\
	.\MFSUBOBJ.H\
	.\MFSYSTYP.H\
	.\MFIOCALL.H

$(INTDIR)/MFOBJTYP.OBJ :  $(SOURCE)  $(DEP_MFOBJ) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFFILE.C
DEP_MFFIL=\
	.\MFFILE.H\
	.\MF3D.H\
	.\MFERRORS.H\
	.\MFOBJCTS.H\
	.\MFTYPES.H\
	.\MFASSERT.H\
	.\MFINTOBJ.H\
	.\MFMEMORY.H\
	.\MFPRIMTV.H\
	.\MFTEXTST.H\
	.\MFTEXTUT.H\
	.\MFTEXTWR.H\
	.\MFIOCALL.H\
	.\MFSUBOBJ.H\
	.\MFSYSTYP.H\
	.\MFLABELS.H\
	.\MFOBJTYP.H\
	.\MFOBJECT.H

$(INTDIR)/MFFILE.OBJ :  $(SOURCE)  $(DEP_MFFIL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFTEXTWR.C
DEP_MFTEXT=\
	.\MFTEXTWR.H\
	.\MFERRORS.H\
	.\MFASSERT.H\
	.\MFINTOBJ.H\
	.\MFTEXTST.H\
	.\MFTYPES.H\
	.\MFIOCALL.H\
	.\MFOBJCTS.H\
	.\MFSUBOBJ.H\
	.\MFSYSTYP.H

$(INTDIR)/MFTEXTWR.OBJ :  $(SOURCE)  $(DEP_MFTEXT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFOBJECT.C
DEP_MFOBJE=\
	.\MFOBJECT.H\
	.\MF3D.H\
	.\MFERRORS.H\
	.\MFINT64.H\
	.\MFOBJCTS.H\
	.\MFASSERT.H\
	.\MFGROUPS.H\
	.\MFINTOBJ.H\
	.\MFMACROS.H\
	.\MFMEMORY.H\
	.\MFOBJTYP.H\
	.\MFPRIMTV.H\
	.\MFRSLNTN.H\
	.\MFTEXTST.H\
	.\MFTEXTWR.H\
	.\MFTYPES.H\
	.\MFIOCALL.H\
	.\MFSYSTYP.H\
	.\MFSUBOBJ.H\
	.\MFLABELS.H

$(INTDIR)/MFOBJECT.OBJ :  $(SOURCE)  $(DEP_MFOBJE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFGROUPS.C
DEP_MFGRO=\
	.\MFGROUPS.H\
	.\MFERRORS.H\
	.\MFSUBOBJ.H\
	.\MFASSERT.H\
	.\MFINTOBJ.H\
	.\MFMACROS.H\
	.\MFMEMORY.H\
	.\MFTYPES.H\
	.\MFIOCALL.H\
	.\MFOBJCTS.H\
	.\MFSYSTYP.H

$(INTDIR)/MFGROUPS.OBJ :  $(SOURCE)  $(DEP_MFGRO) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFRSLNTN.C
DEP_MFRSL=\
	.\MFRSLNTN.H\
	.\MF3D.H\
	.\MFERRORS.H\
	.\MFINT64.H\
	.\MFTYPES.H\
	.\MFASSERT.H\
	.\MFINTOBJ.H\
	.\MFMACROS.H\
	.\MFMEMORY.H\
	.\MFIOCALL.H\
	.\MFOBJCTS.H\
	.\MFSYSTYP.H\
	.\MFSUBOBJ.H

$(INTDIR)/MFRSLNTN.OBJ :  $(SOURCE)  $(DEP_MFRSL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFTEXTRD.C
DEP_MFTEXTR=\
	.\MFTEXTRD.H\
	.\MF3D.H\
	.\MFERRORS.H\
	.\MFINT64.H\
	.\MFSYSTYP.H\
	.\MFASSERT.H\
	.\MFINTOBJ.H\
	.\MFMEMORY.H\
	.\MFOBJTYP.H\
	.\MFTEXTST.H\
	.\MFTEXTUT.H\
	.\MFTYPES.H\
	.\MFIOCALL.H\
	.\MFOBJCTS.H\
	.\MFSUBOBJ.H\
	.\MFOBJECT.H

$(INTDIR)/MFTEXTRD.OBJ :  $(SOURCE)  $(DEP_MFTEXTR) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MFSTDCHK.C
DEP_MFSTD=\
	.\MFSTDCHK.H\
	.\MFERRORS.H\
	.\MFINT64.H\
	.\MFTYPES.H\
	.\MFSYSTYP.H\
	.\MFASSERT.H

$(INTDIR)/MFSTDCHK.OBJ :  $(SOURCE)  $(DEP_MFSTD) $(INTDIR)

# End Source File
# End Group
# End Project
################################################################################
