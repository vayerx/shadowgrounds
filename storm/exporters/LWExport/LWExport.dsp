# Microsoft Developer Studio Project File - Name="LWExport" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=LWExport - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LWExport.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LWExport.mak" CFG="LWExport - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LWExport - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "LWExport - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "LWExport - Win32 Fast" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LWExport - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LWEXPORT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /Ob2 /I "C:\LightWave_3D\SDK\include" /I "../../include" /D "FB_EXPORT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LWEXPORT_EXPORTS" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ..\shared\release\shared.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"libcmt.lib" /out:"Release/FrozenbyteLWExport.dll" /libpath:"../../lib"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LWEXPORT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /ZI /Od /I "../../include" /I "C:\LightWave_3D\SDK\include" /D "FB_EXPORT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LWEXPORT_EXPORTS" /D "WIN32_LEAN_AND_MEAN" /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\shared\debug\shared.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"Debug/FrozenbyteLWExportDebug.dll" /pdbtype:sept /libpath:"../../lib"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "LWExport___Win32_Fast"
# PROP BASE Intermediate_Dir "LWExport___Win32_Fast"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Fast"
# PROP Intermediate_Dir "Fast"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Zi /O2 /Ob2 /I "C:\LightWave_3D\SDK\include" /I "../../include" /D "FB_EXPORT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LWEXPORT_EXPORTS" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT BASE CPP /Fr /YX
# ADD CPP /nologo /W3 /GX /Zi /O2 /Ob2 /I "C:\LightWave_3D\SDK\include" /I "../../include" /D "FB_FAST_BUILD" /D "FB_EXPORT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LWEXPORT_EXPORTS" /D "WIN32_LEAN_AND_MEAN" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\shared\release\shared.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"libcmt.lib" /out:"Release/FrozenbyteLWExport.dll" /libpath:"../../lib"
# ADD LINK32 ..\shared\fast\shared.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /nodefaultlib:"libcmt.lib" /out:"Fast/FrozenbyteLWExportFast.dll" /libpath:"../../lib"
# SUBTRACT LINK32 /debug

!ENDIF 

# Begin Target

# Name "LWExport - Win32 Release"
# Name "LWExport - Win32 Debug"
# Name "LWExport - Win32 Fast"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Lighwave"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\..\libs\LW_SDK\source\serv.def
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\libs\LW_SDK\source\servmain.c

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\libs\LW_SDK\source\shutdown.c

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\libs\LW_SDK\source\startup.c

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\libs\LW_SDK\source\username.c

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# End Group
# Begin Group "external"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\util\Debug_MemoryManager.cpp

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\LWExport_Bone.cpp

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LWExport_Helper.cpp

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LWExport_Lightwave.cpp

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LWExport_Manager.cpp

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LWExport_Material.cpp

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LWExport_Object.cpp

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LWExport_Scene.cpp

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LWExport_Transform.cpp

!IF  "$(CFG)" == "LWExport - Win32 Release"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Debug"

!ELSEIF  "$(CFG)" == "LWExport - Win32 Fast"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\LWExport_Bone.h
# End Source File
# Begin Source File

SOURCE=.\LWExport_Helper.h
# End Source File
# Begin Source File

SOURCE=.\LWExport_Manager.h
# End Source File
# Begin Source File

SOURCE=.\LWExport_Material.h
# End Source File
# Begin Source File

SOURCE=.\LWExport_Object.h
# End Source File
# Begin Source File

SOURCE=.\LWExport_Scene.h
# End Source File
# Begin Source File

SOURCE=.\LWExport_Transform.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\Resources\shared.rc
# End Source File
# End Group
# Begin Source File

SOURCE=..\Shared\Resources\bp.ico
# End Source File
# End Target
# End Project
