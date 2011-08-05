# Microsoft Developer Studio Project File - Name="viewer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=viewer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "viewer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "viewer.mak" CFG="viewer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "viewer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "viewer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "viewer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../../storm/include" /I "../../../storm/keyb3" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /libpath:"../../../storm/lib"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "viewer - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /ZI /Od /I "../../../storm/include" /I "../../../storm/keyb3" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "FROZENBYTE_DEBUG_MEMORY" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../../../storm/lib"

!ENDIF 

# Begin Target

# Name "viewer - Win32 Release"
# Name "viewer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "editor source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\align_units.cpp
# End Source File
# Begin Source File

SOURCE=..\..\camera.cpp
# End Source File
# Begin Source File

SOURCE=..\..\color_component.cpp
# End Source File
# Begin Source File

SOURCE=..\..\color_picker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\command_list.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common_dialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\dialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\dialog_utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\mouse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\parser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\storm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\storm_model_utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\window.cpp
# End Source File
# End Group
# Begin Group "filesystem source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\filesystem\input_file_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\input_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\output_file_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\output_stream.cpp
# End Source File
# End Group
# Begin Group "external"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\util\AreaMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\util\assert.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\util\ColorMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\util\Debug_MemoryManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\file_list.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\file_package_manager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ui\LightManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\util\LightMap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\system\Logger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\util\SelfIlluminationChanger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\standard_package.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\convert\str2int.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\zip_package.cpp
# End Source File
# End Group
# Begin Group "sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\sound\AmplitudeArray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\sound\LipsyncManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\sound\LipsyncProperties.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\sound\SoundLib.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\sound\WaveReader.cpp
# End Source File
# End Group
# Begin Group "util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\util\procedural_applier.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\util\procedural_properties.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\viewer\application.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\viewer\bone_items.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\viewer\model.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\viewer\viewer_main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\viewer\application.h
# End Source File
# Begin Source File

SOURCE=..\..\..\viewer\bone_items.h
# End Source File
# Begin Source File

SOURCE=..\..\..\viewer\model.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\..\viewer\icon1.ico
# End Source File
# Begin Source File

SOURCE=..\..\..\viewer\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\viewer\viewer.rc
# End Source File
# End Group
# End Target
# End Project
