# Microsoft Developer Studio Project File - Name="missioncreator" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=missioncreator - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "missioncreator.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "missioncreator.mak" CFG="missioncreator - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "missioncreator - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "missioncreator - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "missioncreator - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "missioncreator - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "missioncreator - Win32 Release"
# Name "missioncreator - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\missioncreator.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Other"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\filesystem\convert_type.h
# End Source File
# Begin Source File

SOURCE=..\..\Debug_MemoryManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Debug_MemoryManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\empty_buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\file_list.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\file_list.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\file_package_manager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\file_package_manager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\ifile_list.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\ifile_package.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\input_file_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\input_file_stream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\input_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\input_stream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\input_stream_wrapper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\input_stream_wrapper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\detail\ioapi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\memory_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\memory_stream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\output_file_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\output_file_stream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\output_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\output_stream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\standard_package.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\filesystem\standard_package.h
# End Source File
# Begin Source File

SOURCE=..\..\TextFileModifier.cpp
# End Source File
# Begin Source File

SOURCE=..\..\TextFileModifier.h
# End Source File
# End Group
# End Target
# End Project
