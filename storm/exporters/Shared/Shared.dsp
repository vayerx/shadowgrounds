# Microsoft Developer Studio Project File - Name="Shared" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Shared - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Shared.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Shared.mak" CFG="Shared - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Shared - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Shared - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Shared - Win32 Fast" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Shared - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /Ob2 /I "../../include" /D "FB_EXPORT" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /ZI /Od /I "../../include" /D "FB_EXPORT" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /D "_WINDOWS" /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Shared___Win32_Fast"
# PROP BASE Intermediate_Dir "Shared___Win32_Fast"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Shared___Win32_Fast"
# PROP Intermediate_Dir "Shared___Win32_Fast"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Zi /O2 /Ob2 /I "../../include" /D "FB_EXPORT" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT BASE CPP /Fr /YX
# ADD CPP /nologo /W3 /GX /Zi /O2 /Ob2 /I "../../include" /D "FB_FAST_BUILD" /D "FB_EXPORT" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Fast\Shared.lib"

!ENDIF 

# Begin Target

# Name "Shared - Win32 Release"
# Name "Shared - Win32 Debug"
# Name "Shared - Win32 Fast"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Export_Bone.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Dialogs.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Exporter.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Face.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Helper.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Lightobject.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Lod.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Material.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Model.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Object.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Object_Chopper.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Types.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Export_Vertex.cpp

!IF  "$(CFG)" == "Shared - Win32 Release"

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

!ELSEIF  "$(CFG)" == "Shared - Win32 Fast"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Export_Bone.h
# End Source File
# Begin Source File

SOURCE=.\Export_Dialogs.h
# End Source File
# Begin Source File

SOURCE=.\Export_Exporter.h
# End Source File
# Begin Source File

SOURCE=.\Export_Face.h
# End Source File
# Begin Source File

SOURCE=.\Export_Helper.h
# End Source File
# Begin Source File

SOURCE=.\Export_Lightobject.h
# End Source File
# Begin Source File

SOURCE=.\Export_Lod.h
# End Source File
# Begin Source File

SOURCE=.\Export_Material.h
# End Source File
# Begin Source File

SOURCE=.\Export_Model.h
# End Source File
# Begin Source File

SOURCE=.\Export_Object.h
# End Source File
# Begin Source File

SOURCE=.\Export_Object_Chopper.h
# End Source File
# Begin Source File

SOURCE=.\Export_Singleton.h
# End Source File
# Begin Source File

SOURCE=.\Export_Types.h
# End Source File
# Begin Source File

SOURCE=.\Export_Vertex.h
# End Source File
# End Group
# Begin Group "Resource files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Resources\bp.ico
# End Source File
# Begin Source File

SOURCE=.\Resources\resource.h
# End Source File
# End Group
# End Target
# End Project
