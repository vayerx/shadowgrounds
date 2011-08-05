# Microsoft Developer Studio Project File - Name="particlefx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=particlefx - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "particlefx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "particlefx.mak" CFG="particlefx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "particlefx - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "particlefx - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "particlefx - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /I "../../../storm/include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "particlefx - Win32 Debug"

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
# ADD CPP /nologo /W3 /GX /ZI /Od /I "../../../storm/include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "particlefx - Win32 Release"
# Name "particlefx - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\particlefx\emitter_desc.cpp
DEP_CPP_EMITT=\
	"..\..\..\storm\include\c2_color.h"\
	"..\..\..\storm\include\c2_common.h"\
	"..\..\..\storm\include\c2_matrix.h"\
	"..\..\..\storm\include\c2_plane.h"\
	"..\..\..\storm\include\c2_ptrlist.h"\
	"..\..\..\storm\include\c2_quat.h"\
	"..\..\..\storm\include\c2_rect.h"\
	"..\..\..\storm\include\c2_sptr.h"\
	"..\..\..\storm\include\c2_sptrlist.h"\
	"..\..\..\storm\include\c2_string.h"\
	"..\..\..\storm\include\c2_UI.h"\
	"..\..\..\storm\include\c2_vectors.h"\
	"..\..\..\storm\include\DatatypeDef.h"\
	"..\..\..\storm\include\IStorm3D.h"\
	"..\..\..\storm\include\IStorm3D_Font.h"\
	"..\..\..\storm\include\IStorm3D_Helper.h"\
	"..\..\..\storm\include\IStorm3D_Light.h"\
	"..\..\..\storm\include\IStorm3D_Line.h"\
	"..\..\..\storm\include\IStorm3D_Material.h"\
	"..\..\..\storm\include\IStorm3D_Mesh.h"\
	"..\..\..\storm\include\IStorm3D_Model.h"\
	"..\..\..\storm\include\IStorm3D_Particle.h"\
	"..\..\..\storm\include\IStorm3D_Scene.h"\
	"..\..\..\storm\include\IStorm3D_Terrain.h"\
	"..\..\..\storm\include\IStorm3D_Texture.h"\
	"..\..\..\storm\include\Storm3D_Common.h"\
	"..\..\..\storm\include\Storm3D_Datatypes.h"\
	"..\..\..\storm\include\Storm3D_UI.h"\
	"..\..\parser.h"\
	"..\..\particlefx\emitter_desc.h"\
	"..\..\particlefx\float_track.h"\
	"..\..\particlefx\parsing.h"\
	"..\..\particlefx\particle.h"\
	"..\..\particlefx\particle_typedef.h"\
	"..\..\particlefx\vector_track.h"\
	{$(INCLUDE)}"boost\assert.hpp"\
	{$(INCLUDE)}"boost\checked_delete.hpp"\
	{$(INCLUDE)}"boost\config.hpp"\
	{$(INCLUDE)}"boost\config\posix_features.hpp"\
	{$(INCLUDE)}"boost\config\select_compiler_config.hpp"\
	{$(INCLUDE)}"boost\config\select_platform_config.hpp"\
	{$(INCLUDE)}"boost\config\select_stdlib_config.hpp"\
	{$(INCLUDE)}"boost\config\suffix.hpp"\
	{$(INCLUDE)}"boost\current_function.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_linux.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lightweight_mutex.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_irix.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_linux.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_nop.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32_cs.hpp"\
	{$(INCLUDE)}"boost\detail\shared_count.hpp"\
	{$(INCLUDE)}"boost\detail\shared_ptr_nmt.hpp"\
	{$(INCLUDE)}"boost\detail\winapi.hpp"\
	{$(INCLUDE)}"boost\scoped_ptr.hpp"\
	{$(INCLUDE)}"boost\shared_ptr.hpp"\
	{$(INCLUDE)}"boost\throw_exception.hpp"\
	{$(INCLUDE)}"config\_epilog.h"\
	{$(INCLUDE)}"config\_msvc_warnings_off.h"\
	{$(INCLUDE)}"config\_prolog.h"\
	{$(INCLUDE)}"config\stl_apcc.h"\
	{$(INCLUDE)}"config\stl_apple.h"\
	{$(INCLUDE)}"config\stl_as400.h"\
	{$(INCLUDE)}"config\stl_bc.h"\
	{$(INCLUDE)}"config\stl_como.h"\
	{$(INCLUDE)}"config\stl_confix.h"\
	{$(INCLUDE)}"config\stl_dec.h"\
	{$(INCLUDE)}"config\stl_dec_vms.h"\
	{$(INCLUDE)}"config\stl_fujitsu.h"\
	{$(INCLUDE)}"config\stl_gcc.h"\
	{$(INCLUDE)}"config\stl_hpacc.h"\
	{$(INCLUDE)}"config\stl_ibm.h"\
	{$(INCLUDE)}"config\stl_intel.h"\
	{$(INCLUDE)}"config\stl_kai.h"\
	{$(INCLUDE)}"config\stl_msvc.h"\
	{$(INCLUDE)}"config\stl_mwerks.h"\
	{$(INCLUDE)}"config\stl_mycomp.h"\
	{$(INCLUDE)}"config\stl_sco.h"\
	{$(INCLUDE)}"config\stl_select_lib.h"\
	{$(INCLUDE)}"config\stl_sgi.h"\
	{$(INCLUDE)}"config\stl_solaris.h"\
	{$(INCLUDE)}"config\stl_sunpro.h"\
	{$(INCLUDE)}"config\stl_symantec.h"\
	{$(INCLUDE)}"config\stl_watcom.h"\
	{$(INCLUDE)}"config\stl_wince.h"\
	{$(INCLUDE)}"config\stlcomp.h"\
	{$(INCLUDE)}"config\vc_select_lib.h"\
	{$(INCLUDE)}"pthread.h"\
	{$(INCLUDE)}"stl\_abbrevs.h"\
	{$(INCLUDE)}"stl\_config.h"\
	{$(INCLUDE)}"stl\_config_compat.h"\
	{$(INCLUDE)}"stl\_config_compat_post.h"\
	{$(INCLUDE)}"stl\_epilog.h"\
	{$(INCLUDE)}"stl\_prolog.h"\
	{$(INCLUDE)}"stl\_site_config.h"\
	{$(INCLUDE)}"stl_user_config.h"\
	
NODEP_CPP_EMITT=\
	"..\..\..\..\..\..\usr\include\pthread.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\float_track.cpp
DEP_CPP_FLOAT=\
	"..\..\..\storm\include\c2_color.h"\
	"..\..\..\storm\include\c2_common.h"\
	"..\..\..\storm\include\c2_matrix.h"\
	"..\..\..\storm\include\c2_plane.h"\
	"..\..\..\storm\include\c2_ptrlist.h"\
	"..\..\..\storm\include\c2_quat.h"\
	"..\..\..\storm\include\c2_rect.h"\
	"..\..\..\storm\include\c2_sptr.h"\
	"..\..\..\storm\include\c2_sptrlist.h"\
	"..\..\..\storm\include\c2_string.h"\
	"..\..\..\storm\include\c2_UI.h"\
	"..\..\..\storm\include\c2_vectors.h"\
	"..\..\..\storm\include\DatatypeDef.h"\
	"..\..\..\storm\include\IStorm3D.h"\
	"..\..\..\storm\include\IStorm3D_Font.h"\
	"..\..\..\storm\include\IStorm3D_Helper.h"\
	"..\..\..\storm\include\IStorm3D_Light.h"\
	"..\..\..\storm\include\IStorm3D_Line.h"\
	"..\..\..\storm\include\IStorm3D_Material.h"\
	"..\..\..\storm\include\IStorm3D_Mesh.h"\
	"..\..\..\storm\include\IStorm3D_Model.h"\
	"..\..\..\storm\include\IStorm3D_Particle.h"\
	"..\..\..\storm\include\IStorm3D_Scene.h"\
	"..\..\..\storm\include\IStorm3D_Terrain.h"\
	"..\..\..\storm\include\IStorm3D_Texture.h"\
	"..\..\..\storm\include\Storm3D_Common.h"\
	"..\..\..\storm\include\Storm3D_Datatypes.h"\
	"..\..\..\storm\include\Storm3D_UI.h"\
	"..\..\parser.h"\
	"..\..\particlefx\float_track.h"\
	"..\..\particlefx\parsing.h"\
	"..\..\particlefx\particle_typedef.h"\
	{$(INCLUDE)}"boost\assert.hpp"\
	{$(INCLUDE)}"boost\checked_delete.hpp"\
	{$(INCLUDE)}"boost\config.hpp"\
	{$(INCLUDE)}"boost\config\posix_features.hpp"\
	{$(INCLUDE)}"boost\config\select_compiler_config.hpp"\
	{$(INCLUDE)}"boost\config\select_platform_config.hpp"\
	{$(INCLUDE)}"boost\config\select_stdlib_config.hpp"\
	{$(INCLUDE)}"boost\config\suffix.hpp"\
	{$(INCLUDE)}"boost\current_function.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_linux.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lightweight_mutex.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_irix.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_linux.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_nop.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32_cs.hpp"\
	{$(INCLUDE)}"boost\detail\shared_count.hpp"\
	{$(INCLUDE)}"boost\detail\shared_ptr_nmt.hpp"\
	{$(INCLUDE)}"boost\detail\winapi.hpp"\
	{$(INCLUDE)}"boost\lexical_cast.hpp"\
	{$(INCLUDE)}"boost\scoped_ptr.hpp"\
	{$(INCLUDE)}"boost\shared_ptr.hpp"\
	{$(INCLUDE)}"boost\throw_exception.hpp"\
	{$(INCLUDE)}"config\_epilog.h"\
	{$(INCLUDE)}"config\_msvc_warnings_off.h"\
	{$(INCLUDE)}"config\_prolog.h"\
	{$(INCLUDE)}"config\stl_apcc.h"\
	{$(INCLUDE)}"config\stl_apple.h"\
	{$(INCLUDE)}"config\stl_as400.h"\
	{$(INCLUDE)}"config\stl_bc.h"\
	{$(INCLUDE)}"config\stl_como.h"\
	{$(INCLUDE)}"config\stl_confix.h"\
	{$(INCLUDE)}"config\stl_dec.h"\
	{$(INCLUDE)}"config\stl_dec_vms.h"\
	{$(INCLUDE)}"config\stl_fujitsu.h"\
	{$(INCLUDE)}"config\stl_gcc.h"\
	{$(INCLUDE)}"config\stl_hpacc.h"\
	{$(INCLUDE)}"config\stl_ibm.h"\
	{$(INCLUDE)}"config\stl_intel.h"\
	{$(INCLUDE)}"config\stl_kai.h"\
	{$(INCLUDE)}"config\stl_msvc.h"\
	{$(INCLUDE)}"config\stl_mwerks.h"\
	{$(INCLUDE)}"config\stl_mycomp.h"\
	{$(INCLUDE)}"config\stl_sco.h"\
	{$(INCLUDE)}"config\stl_select_lib.h"\
	{$(INCLUDE)}"config\stl_sgi.h"\
	{$(INCLUDE)}"config\stl_solaris.h"\
	{$(INCLUDE)}"config\stl_sunpro.h"\
	{$(INCLUDE)}"config\stl_symantec.h"\
	{$(INCLUDE)}"config\stl_watcom.h"\
	{$(INCLUDE)}"config\stl_wince.h"\
	{$(INCLUDE)}"config\stlcomp.h"\
	{$(INCLUDE)}"config\vc_select_lib.h"\
	{$(INCLUDE)}"pthread.h"\
	{$(INCLUDE)}"stl\_abbrevs.h"\
	{$(INCLUDE)}"stl\_config.h"\
	{$(INCLUDE)}"stl\_config_compat.h"\
	{$(INCLUDE)}"stl\_config_compat_post.h"\
	{$(INCLUDE)}"stl\_epilog.h"\
	{$(INCLUDE)}"stl\_prolog.h"\
	{$(INCLUDE)}"stl\_site_config.h"\
	{$(INCLUDE)}"stl_user_config.h"\
	
NODEP_CPP_FLOAT=\
	"..\..\..\..\..\..\usr\include\pthread.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\parsing.cpp
DEP_CPP_PARSI=\
	"..\..\..\storm\include\c2_color.h"\
	"..\..\..\storm\include\c2_common.h"\
	"..\..\..\storm\include\c2_matrix.h"\
	"..\..\..\storm\include\c2_plane.h"\
	"..\..\..\storm\include\c2_ptrlist.h"\
	"..\..\..\storm\include\c2_quat.h"\
	"..\..\..\storm\include\c2_rect.h"\
	"..\..\..\storm\include\c2_sptr.h"\
	"..\..\..\storm\include\c2_sptrlist.h"\
	"..\..\..\storm\include\c2_string.h"\
	"..\..\..\storm\include\c2_UI.h"\
	"..\..\..\storm\include\c2_vectors.h"\
	"..\..\..\storm\include\DatatypeDef.h"\
	"..\..\..\storm\include\IStorm3D.h"\
	"..\..\..\storm\include\IStorm3D_Font.h"\
	"..\..\..\storm\include\IStorm3D_Helper.h"\
	"..\..\..\storm\include\IStorm3D_Light.h"\
	"..\..\..\storm\include\IStorm3D_Line.h"\
	"..\..\..\storm\include\IStorm3D_Material.h"\
	"..\..\..\storm\include\IStorm3D_Mesh.h"\
	"..\..\..\storm\include\IStorm3D_Model.h"\
	"..\..\..\storm\include\IStorm3D_Particle.h"\
	"..\..\..\storm\include\IStorm3D_Scene.h"\
	"..\..\..\storm\include\IStorm3D_Terrain.h"\
	"..\..\..\storm\include\IStorm3D_Texture.h"\
	"..\..\..\storm\include\Storm3D_Common.h"\
	"..\..\..\storm\include\Storm3D_Datatypes.h"\
	"..\..\..\storm\include\Storm3D_UI.h"\
	"..\..\parser.h"\
	"..\..\particlefx\parsing.h"\
	"..\..\particlefx\particle_typedef.h"\
	"..\..\string_conversions.h"\
	{$(INCLUDE)}"boost\assert.hpp"\
	{$(INCLUDE)}"boost\checked_delete.hpp"\
	{$(INCLUDE)}"boost\config.hpp"\
	{$(INCLUDE)}"boost\config\posix_features.hpp"\
	{$(INCLUDE)}"boost\config\select_compiler_config.hpp"\
	{$(INCLUDE)}"boost\config\select_platform_config.hpp"\
	{$(INCLUDE)}"boost\config\select_stdlib_config.hpp"\
	{$(INCLUDE)}"boost\config\suffix.hpp"\
	{$(INCLUDE)}"boost\current_function.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_linux.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lightweight_mutex.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_irix.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_linux.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_nop.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32_cs.hpp"\
	{$(INCLUDE)}"boost\detail\shared_count.hpp"\
	{$(INCLUDE)}"boost\detail\shared_ptr_nmt.hpp"\
	{$(INCLUDE)}"boost\detail\winapi.hpp"\
	{$(INCLUDE)}"boost\lexical_cast.hpp"\
	{$(INCLUDE)}"boost\scoped_ptr.hpp"\
	{$(INCLUDE)}"boost\shared_ptr.hpp"\
	{$(INCLUDE)}"boost\throw_exception.hpp"\
	{$(INCLUDE)}"config\_epilog.h"\
	{$(INCLUDE)}"config\_msvc_warnings_off.h"\
	{$(INCLUDE)}"config\_prolog.h"\
	{$(INCLUDE)}"config\stl_apcc.h"\
	{$(INCLUDE)}"config\stl_apple.h"\
	{$(INCLUDE)}"config\stl_as400.h"\
	{$(INCLUDE)}"config\stl_bc.h"\
	{$(INCLUDE)}"config\stl_como.h"\
	{$(INCLUDE)}"config\stl_confix.h"\
	{$(INCLUDE)}"config\stl_dec.h"\
	{$(INCLUDE)}"config\stl_dec_vms.h"\
	{$(INCLUDE)}"config\stl_fujitsu.h"\
	{$(INCLUDE)}"config\stl_gcc.h"\
	{$(INCLUDE)}"config\stl_hpacc.h"\
	{$(INCLUDE)}"config\stl_ibm.h"\
	{$(INCLUDE)}"config\stl_intel.h"\
	{$(INCLUDE)}"config\stl_kai.h"\
	{$(INCLUDE)}"config\stl_msvc.h"\
	{$(INCLUDE)}"config\stl_mwerks.h"\
	{$(INCLUDE)}"config\stl_mycomp.h"\
	{$(INCLUDE)}"config\stl_sco.h"\
	{$(INCLUDE)}"config\stl_select_lib.h"\
	{$(INCLUDE)}"config\stl_sgi.h"\
	{$(INCLUDE)}"config\stl_solaris.h"\
	{$(INCLUDE)}"config\stl_sunpro.h"\
	{$(INCLUDE)}"config\stl_symantec.h"\
	{$(INCLUDE)}"config\stl_watcom.h"\
	{$(INCLUDE)}"config\stl_wince.h"\
	{$(INCLUDE)}"config\stlcomp.h"\
	{$(INCLUDE)}"config\vc_select_lib.h"\
	{$(INCLUDE)}"pthread.h"\
	{$(INCLUDE)}"stl\_abbrevs.h"\
	{$(INCLUDE)}"stl\_config.h"\
	{$(INCLUDE)}"stl\_config_compat.h"\
	{$(INCLUDE)}"stl\_config_compat_post.h"\
	{$(INCLUDE)}"stl\_epilog.h"\
	{$(INCLUDE)}"stl\_prolog.h"\
	{$(INCLUDE)}"stl\_site_config.h"\
	{$(INCLUDE)}"stl_user_config.h"\
	
NODEP_CPP_PARSI=\
	"..\..\..\..\..\..\usr\include\pthread.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\particle_desc.cpp
DEP_CPP_PARTI=\
	"..\..\..\storm\include\c2_color.h"\
	"..\..\..\storm\include\c2_common.h"\
	"..\..\..\storm\include\c2_matrix.h"\
	"..\..\..\storm\include\c2_plane.h"\
	"..\..\..\storm\include\c2_ptrlist.h"\
	"..\..\..\storm\include\c2_quat.h"\
	"..\..\..\storm\include\c2_rect.h"\
	"..\..\..\storm\include\c2_sptr.h"\
	"..\..\..\storm\include\c2_sptrlist.h"\
	"..\..\..\storm\include\c2_string.h"\
	"..\..\..\storm\include\c2_UI.h"\
	"..\..\..\storm\include\c2_vectors.h"\
	"..\..\..\storm\include\DatatypeDef.h"\
	"..\..\..\storm\include\IStorm3D.h"\
	"..\..\..\storm\include\IStorm3D_Font.h"\
	"..\..\..\storm\include\IStorm3D_Helper.h"\
	"..\..\..\storm\include\IStorm3D_Light.h"\
	"..\..\..\storm\include\IStorm3D_Line.h"\
	"..\..\..\storm\include\IStorm3D_Material.h"\
	"..\..\..\storm\include\IStorm3D_Mesh.h"\
	"..\..\..\storm\include\IStorm3D_Model.h"\
	"..\..\..\storm\include\IStorm3D_Particle.h"\
	"..\..\..\storm\include\IStorm3D_Scene.h"\
	"..\..\..\storm\include\IStorm3D_Terrain.h"\
	"..\..\..\storm\include\IStorm3D_Texture.h"\
	"..\..\..\storm\include\Storm3D_Common.h"\
	"..\..\..\storm\include\Storm3D_Datatypes.h"\
	"..\..\..\storm\include\Storm3D_UI.h"\
	"..\..\parser.h"\
	"..\..\particlefx\float_track.h"\
	"..\..\particlefx\parsing.h"\
	"..\..\particlefx\particle.h"\
	"..\..\particlefx\particle_desc.h"\
	"..\..\particlefx\particle_typedef.h"\
	"..\..\particlefx\vector_track.h"\
	{$(INCLUDE)}"boost\assert.hpp"\
	{$(INCLUDE)}"boost\checked_delete.hpp"\
	{$(INCLUDE)}"boost\config.hpp"\
	{$(INCLUDE)}"boost\config\posix_features.hpp"\
	{$(INCLUDE)}"boost\config\select_compiler_config.hpp"\
	{$(INCLUDE)}"boost\config\select_platform_config.hpp"\
	{$(INCLUDE)}"boost\config\select_stdlib_config.hpp"\
	{$(INCLUDE)}"boost\config\suffix.hpp"\
	{$(INCLUDE)}"boost\current_function.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_linux.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lightweight_mutex.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_irix.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_linux.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_nop.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32_cs.hpp"\
	{$(INCLUDE)}"boost\detail\shared_count.hpp"\
	{$(INCLUDE)}"boost\detail\shared_ptr_nmt.hpp"\
	{$(INCLUDE)}"boost\detail\winapi.hpp"\
	{$(INCLUDE)}"boost\scoped_ptr.hpp"\
	{$(INCLUDE)}"boost\shared_ptr.hpp"\
	{$(INCLUDE)}"boost\throw_exception.hpp"\
	{$(INCLUDE)}"config\_epilog.h"\
	{$(INCLUDE)}"config\_msvc_warnings_off.h"\
	{$(INCLUDE)}"config\_prolog.h"\
	{$(INCLUDE)}"config\stl_apcc.h"\
	{$(INCLUDE)}"config\stl_apple.h"\
	{$(INCLUDE)}"config\stl_as400.h"\
	{$(INCLUDE)}"config\stl_bc.h"\
	{$(INCLUDE)}"config\stl_como.h"\
	{$(INCLUDE)}"config\stl_confix.h"\
	{$(INCLUDE)}"config\stl_dec.h"\
	{$(INCLUDE)}"config\stl_dec_vms.h"\
	{$(INCLUDE)}"config\stl_fujitsu.h"\
	{$(INCLUDE)}"config\stl_gcc.h"\
	{$(INCLUDE)}"config\stl_hpacc.h"\
	{$(INCLUDE)}"config\stl_ibm.h"\
	{$(INCLUDE)}"config\stl_intel.h"\
	{$(INCLUDE)}"config\stl_kai.h"\
	{$(INCLUDE)}"config\stl_msvc.h"\
	{$(INCLUDE)}"config\stl_mwerks.h"\
	{$(INCLUDE)}"config\stl_mycomp.h"\
	{$(INCLUDE)}"config\stl_sco.h"\
	{$(INCLUDE)}"config\stl_select_lib.h"\
	{$(INCLUDE)}"config\stl_sgi.h"\
	{$(INCLUDE)}"config\stl_solaris.h"\
	{$(INCLUDE)}"config\stl_sunpro.h"\
	{$(INCLUDE)}"config\stl_symantec.h"\
	{$(INCLUDE)}"config\stl_watcom.h"\
	{$(INCLUDE)}"config\stl_wince.h"\
	{$(INCLUDE)}"config\stlcomp.h"\
	{$(INCLUDE)}"config\vc_select_lib.h"\
	{$(INCLUDE)}"pthread.h"\
	{$(INCLUDE)}"stl\_abbrevs.h"\
	{$(INCLUDE)}"stl\_config.h"\
	{$(INCLUDE)}"stl\_config_compat.h"\
	{$(INCLUDE)}"stl\_config_compat_post.h"\
	{$(INCLUDE)}"stl\_epilog.h"\
	{$(INCLUDE)}"stl\_prolog.h"\
	{$(INCLUDE)}"stl\_site_config.h"\
	{$(INCLUDE)}"stl_user_config.h"\
	
NODEP_CPP_PARTI=\
	"..\..\..\..\..\..\usr\include\pthread.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\particle_system.cpp
DEP_CPP_PARTIC=\
	"..\..\..\storm\include\c2_color.h"\
	"..\..\..\storm\include\c2_common.h"\
	"..\..\..\storm\include\c2_matrix.h"\
	"..\..\..\storm\include\c2_plane.h"\
	"..\..\..\storm\include\c2_ptrlist.h"\
	"..\..\..\storm\include\c2_quat.h"\
	"..\..\..\storm\include\c2_rect.h"\
	"..\..\..\storm\include\c2_sptr.h"\
	"..\..\..\storm\include\c2_sptrlist.h"\
	"..\..\..\storm\include\c2_string.h"\
	"..\..\..\storm\include\c2_UI.h"\
	"..\..\..\storm\include\c2_vectors.h"\
	"..\..\..\storm\include\DatatypeDef.h"\
	"..\..\..\storm\include\IStorm3D.h"\
	"..\..\..\storm\include\IStorm3D_Font.h"\
	"..\..\..\storm\include\IStorm3D_Helper.h"\
	"..\..\..\storm\include\IStorm3D_Light.h"\
	"..\..\..\storm\include\IStorm3D_Line.h"\
	"..\..\..\storm\include\IStorm3D_Material.h"\
	"..\..\..\storm\include\IStorm3D_Mesh.h"\
	"..\..\..\storm\include\IStorm3D_Model.h"\
	"..\..\..\storm\include\IStorm3D_Particle.h"\
	"..\..\..\storm\include\IStorm3D_Scene.h"\
	"..\..\..\storm\include\IStorm3D_Terrain.h"\
	"..\..\..\storm\include\IStorm3D_Texture.h"\
	"..\..\..\storm\include\Storm3D_Common.h"\
	"..\..\..\storm\include\Storm3D_Datatypes.h"\
	"..\..\..\storm\include\Storm3D_UI.h"\
	"..\..\parser.h"\
	"..\..\particlefx\emitter_desc.h"\
	"..\..\particlefx\float_track.h"\
	"..\..\particlefx\parsing.h"\
	"..\..\particlefx\particle.h"\
	"..\..\particlefx\particle_desc.h"\
	"..\..\particlefx\particle_system.h"\
	"..\..\particlefx\particle_system_manager.h"\
	"..\..\particlefx\particle_typedef.h"\
	"..\..\particlefx\vector_track.h"\
	{$(INCLUDE)}"boost\assert.hpp"\
	{$(INCLUDE)}"boost\checked_delete.hpp"\
	{$(INCLUDE)}"boost\config.hpp"\
	{$(INCLUDE)}"boost\config\posix_features.hpp"\
	{$(INCLUDE)}"boost\config\select_compiler_config.hpp"\
	{$(INCLUDE)}"boost\config\select_platform_config.hpp"\
	{$(INCLUDE)}"boost\config\select_stdlib_config.hpp"\
	{$(INCLUDE)}"boost\config\suffix.hpp"\
	{$(INCLUDE)}"boost\current_function.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_linux.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lightweight_mutex.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_irix.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_linux.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_nop.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32_cs.hpp"\
	{$(INCLUDE)}"boost\detail\shared_count.hpp"\
	{$(INCLUDE)}"boost\detail\shared_ptr_nmt.hpp"\
	{$(INCLUDE)}"boost\detail\winapi.hpp"\
	{$(INCLUDE)}"boost\scoped_ptr.hpp"\
	{$(INCLUDE)}"boost\shared_ptr.hpp"\
	{$(INCLUDE)}"boost\throw_exception.hpp"\
	{$(INCLUDE)}"config\_epilog.h"\
	{$(INCLUDE)}"config\_msvc_warnings_off.h"\
	{$(INCLUDE)}"config\_prolog.h"\
	{$(INCLUDE)}"config\stl_apcc.h"\
	{$(INCLUDE)}"config\stl_apple.h"\
	{$(INCLUDE)}"config\stl_as400.h"\
	{$(INCLUDE)}"config\stl_bc.h"\
	{$(INCLUDE)}"config\stl_como.h"\
	{$(INCLUDE)}"config\stl_confix.h"\
	{$(INCLUDE)}"config\stl_dec.h"\
	{$(INCLUDE)}"config\stl_dec_vms.h"\
	{$(INCLUDE)}"config\stl_fujitsu.h"\
	{$(INCLUDE)}"config\stl_gcc.h"\
	{$(INCLUDE)}"config\stl_hpacc.h"\
	{$(INCLUDE)}"config\stl_ibm.h"\
	{$(INCLUDE)}"config\stl_intel.h"\
	{$(INCLUDE)}"config\stl_kai.h"\
	{$(INCLUDE)}"config\stl_msvc.h"\
	{$(INCLUDE)}"config\stl_mwerks.h"\
	{$(INCLUDE)}"config\stl_mycomp.h"\
	{$(INCLUDE)}"config\stl_sco.h"\
	{$(INCLUDE)}"config\stl_select_lib.h"\
	{$(INCLUDE)}"config\stl_sgi.h"\
	{$(INCLUDE)}"config\stl_solaris.h"\
	{$(INCLUDE)}"config\stl_sunpro.h"\
	{$(INCLUDE)}"config\stl_symantec.h"\
	{$(INCLUDE)}"config\stl_watcom.h"\
	{$(INCLUDE)}"config\stl_wince.h"\
	{$(INCLUDE)}"config\stlcomp.h"\
	{$(INCLUDE)}"config\vc_select_lib.h"\
	{$(INCLUDE)}"pthread.h"\
	{$(INCLUDE)}"stl\_abbrevs.h"\
	{$(INCLUDE)}"stl\_config.h"\
	{$(INCLUDE)}"stl\_config_compat.h"\
	{$(INCLUDE)}"stl\_config_compat_post.h"\
	{$(INCLUDE)}"stl\_epilog.h"\
	{$(INCLUDE)}"stl\_prolog.h"\
	{$(INCLUDE)}"stl\_site_config.h"\
	{$(INCLUDE)}"stl_user_config.h"\
	
NODEP_CPP_PARTIC=\
	"..\..\..\..\..\..\usr\include\pthread.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\particle_system_manager.cpp
DEP_CPP_PARTICL=\
	"..\..\..\storm\include\c2_color.h"\
	"..\..\..\storm\include\c2_common.h"\
	"..\..\..\storm\include\c2_matrix.h"\
	"..\..\..\storm\include\c2_plane.h"\
	"..\..\..\storm\include\c2_ptrlist.h"\
	"..\..\..\storm\include\c2_quat.h"\
	"..\..\..\storm\include\c2_rect.h"\
	"..\..\..\storm\include\c2_sptr.h"\
	"..\..\..\storm\include\c2_sptrlist.h"\
	"..\..\..\storm\include\c2_string.h"\
	"..\..\..\storm\include\c2_UI.h"\
	"..\..\..\storm\include\c2_vectors.h"\
	"..\..\..\storm\include\DatatypeDef.h"\
	"..\..\..\storm\include\IStorm3D.h"\
	"..\..\..\storm\include\IStorm3D_Font.h"\
	"..\..\..\storm\include\IStorm3D_Helper.h"\
	"..\..\..\storm\include\IStorm3D_Light.h"\
	"..\..\..\storm\include\IStorm3D_Line.h"\
	"..\..\..\storm\include\IStorm3D_Material.h"\
	"..\..\..\storm\include\IStorm3D_Mesh.h"\
	"..\..\..\storm\include\IStorm3D_Model.h"\
	"..\..\..\storm\include\IStorm3D_Particle.h"\
	"..\..\..\storm\include\IStorm3D_Scene.h"\
	"..\..\..\storm\include\IStorm3D_Terrain.h"\
	"..\..\..\storm\include\IStorm3D_Texture.h"\
	"..\..\..\storm\include\Storm3D_Common.h"\
	"..\..\..\storm\include\Storm3D_Datatypes.h"\
	"..\..\..\storm\include\Storm3D_UI.h"\
	"..\..\parser.h"\
	"..\..\particlefx\emitter_desc.h"\
	"..\..\particlefx\float_track.h"\
	"..\..\particlefx\parsing.h"\
	"..\..\particlefx\particle.h"\
	"..\..\particlefx\particle_desc.h"\
	"..\..\particlefx\particle_system.h"\
	"..\..\particlefx\particle_system_manager.h"\
	"..\..\particlefx\particle_typedef.h"\
	"..\..\particlefx\vector_track.h"\
	{$(INCLUDE)}"boost\assert.hpp"\
	{$(INCLUDE)}"boost\checked_delete.hpp"\
	{$(INCLUDE)}"boost\config.hpp"\
	{$(INCLUDE)}"boost\config\posix_features.hpp"\
	{$(INCLUDE)}"boost\config\select_compiler_config.hpp"\
	{$(INCLUDE)}"boost\config\select_platform_config.hpp"\
	{$(INCLUDE)}"boost\config\select_stdlib_config.hpp"\
	{$(INCLUDE)}"boost\config\suffix.hpp"\
	{$(INCLUDE)}"boost\current_function.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_linux.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lightweight_mutex.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_irix.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_linux.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_nop.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32_cs.hpp"\
	{$(INCLUDE)}"boost\detail\shared_count.hpp"\
	{$(INCLUDE)}"boost\detail\shared_ptr_nmt.hpp"\
	{$(INCLUDE)}"boost\detail\winapi.hpp"\
	{$(INCLUDE)}"boost\scoped_ptr.hpp"\
	{$(INCLUDE)}"boost\shared_ptr.hpp"\
	{$(INCLUDE)}"boost\throw_exception.hpp"\
	{$(INCLUDE)}"config\_epilog.h"\
	{$(INCLUDE)}"config\_msvc_warnings_off.h"\
	{$(INCLUDE)}"config\_prolog.h"\
	{$(INCLUDE)}"config\stl_apcc.h"\
	{$(INCLUDE)}"config\stl_apple.h"\
	{$(INCLUDE)}"config\stl_as400.h"\
	{$(INCLUDE)}"config\stl_bc.h"\
	{$(INCLUDE)}"config\stl_como.h"\
	{$(INCLUDE)}"config\stl_confix.h"\
	{$(INCLUDE)}"config\stl_dec.h"\
	{$(INCLUDE)}"config\stl_dec_vms.h"\
	{$(INCLUDE)}"config\stl_fujitsu.h"\
	{$(INCLUDE)}"config\stl_gcc.h"\
	{$(INCLUDE)}"config\stl_hpacc.h"\
	{$(INCLUDE)}"config\stl_ibm.h"\
	{$(INCLUDE)}"config\stl_intel.h"\
	{$(INCLUDE)}"config\stl_kai.h"\
	{$(INCLUDE)}"config\stl_msvc.h"\
	{$(INCLUDE)}"config\stl_mwerks.h"\
	{$(INCLUDE)}"config\stl_mycomp.h"\
	{$(INCLUDE)}"config\stl_sco.h"\
	{$(INCLUDE)}"config\stl_select_lib.h"\
	{$(INCLUDE)}"config\stl_sgi.h"\
	{$(INCLUDE)}"config\stl_solaris.h"\
	{$(INCLUDE)}"config\stl_sunpro.h"\
	{$(INCLUDE)}"config\stl_symantec.h"\
	{$(INCLUDE)}"config\stl_watcom.h"\
	{$(INCLUDE)}"config\stl_wince.h"\
	{$(INCLUDE)}"config\stlcomp.h"\
	{$(INCLUDE)}"config\vc_select_lib.h"\
	{$(INCLUDE)}"pthread.h"\
	{$(INCLUDE)}"stl\_abbrevs.h"\
	{$(INCLUDE)}"stl\_config.h"\
	{$(INCLUDE)}"stl\_config_compat.h"\
	{$(INCLUDE)}"stl\_config_compat_post.h"\
	{$(INCLUDE)}"stl\_epilog.h"\
	{$(INCLUDE)}"stl\_prolog.h"\
	{$(INCLUDE)}"stl\_site_config.h"\
	{$(INCLUDE)}"stl_user_config.h"\
	
NODEP_CPP_PARTICL=\
	"..\..\..\..\..\..\usr\include\pthread.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\vector_track.cpp
DEP_CPP_VECTO=\
	"..\..\..\storm\include\c2_color.h"\
	"..\..\..\storm\include\c2_common.h"\
	"..\..\..\storm\include\c2_matrix.h"\
	"..\..\..\storm\include\c2_plane.h"\
	"..\..\..\storm\include\c2_ptrlist.h"\
	"..\..\..\storm\include\c2_quat.h"\
	"..\..\..\storm\include\c2_rect.h"\
	"..\..\..\storm\include\c2_sptr.h"\
	"..\..\..\storm\include\c2_sptrlist.h"\
	"..\..\..\storm\include\c2_string.h"\
	"..\..\..\storm\include\c2_UI.h"\
	"..\..\..\storm\include\c2_vectors.h"\
	"..\..\..\storm\include\DatatypeDef.h"\
	"..\..\..\storm\include\IStorm3D.h"\
	"..\..\..\storm\include\IStorm3D_Font.h"\
	"..\..\..\storm\include\IStorm3D_Helper.h"\
	"..\..\..\storm\include\IStorm3D_Light.h"\
	"..\..\..\storm\include\IStorm3D_Line.h"\
	"..\..\..\storm\include\IStorm3D_Material.h"\
	"..\..\..\storm\include\IStorm3D_Mesh.h"\
	"..\..\..\storm\include\IStorm3D_Model.h"\
	"..\..\..\storm\include\IStorm3D_Particle.h"\
	"..\..\..\storm\include\IStorm3D_Scene.h"\
	"..\..\..\storm\include\IStorm3D_Terrain.h"\
	"..\..\..\storm\include\IStorm3D_Texture.h"\
	"..\..\..\storm\include\Storm3D_Common.h"\
	"..\..\..\storm\include\Storm3D_Datatypes.h"\
	"..\..\..\storm\include\Storm3D_UI.h"\
	"..\..\parser.h"\
	"..\..\particlefx\parsing.h"\
	"..\..\particlefx\particle_typedef.h"\
	"..\..\particlefx\vector_track.h"\
	{$(INCLUDE)}"boost\assert.hpp"\
	{$(INCLUDE)}"boost\checked_delete.hpp"\
	{$(INCLUDE)}"boost\config.hpp"\
	{$(INCLUDE)}"boost\config\posix_features.hpp"\
	{$(INCLUDE)}"boost\config\select_compiler_config.hpp"\
	{$(INCLUDE)}"boost\config\select_platform_config.hpp"\
	{$(INCLUDE)}"boost\config\select_stdlib_config.hpp"\
	{$(INCLUDE)}"boost\config\suffix.hpp"\
	{$(INCLUDE)}"boost\current_function.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_linux.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\atomic_count_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lightweight_mutex.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_gcc.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_irix.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_linux.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_nop.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_pthreads.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32.hpp"\
	{$(INCLUDE)}"boost\detail\lwm_win32_cs.hpp"\
	{$(INCLUDE)}"boost\detail\shared_count.hpp"\
	{$(INCLUDE)}"boost\detail\shared_ptr_nmt.hpp"\
	{$(INCLUDE)}"boost\detail\winapi.hpp"\
	{$(INCLUDE)}"boost\lexical_cast.hpp"\
	{$(INCLUDE)}"boost\scoped_ptr.hpp"\
	{$(INCLUDE)}"boost\shared_ptr.hpp"\
	{$(INCLUDE)}"boost\throw_exception.hpp"\
	{$(INCLUDE)}"config\_epilog.h"\
	{$(INCLUDE)}"config\_msvc_warnings_off.h"\
	{$(INCLUDE)}"config\_prolog.h"\
	{$(INCLUDE)}"config\stl_apcc.h"\
	{$(INCLUDE)}"config\stl_apple.h"\
	{$(INCLUDE)}"config\stl_as400.h"\
	{$(INCLUDE)}"config\stl_bc.h"\
	{$(INCLUDE)}"config\stl_como.h"\
	{$(INCLUDE)}"config\stl_confix.h"\
	{$(INCLUDE)}"config\stl_dec.h"\
	{$(INCLUDE)}"config\stl_dec_vms.h"\
	{$(INCLUDE)}"config\stl_fujitsu.h"\
	{$(INCLUDE)}"config\stl_gcc.h"\
	{$(INCLUDE)}"config\stl_hpacc.h"\
	{$(INCLUDE)}"config\stl_ibm.h"\
	{$(INCLUDE)}"config\stl_intel.h"\
	{$(INCLUDE)}"config\stl_kai.h"\
	{$(INCLUDE)}"config\stl_msvc.h"\
	{$(INCLUDE)}"config\stl_mwerks.h"\
	{$(INCLUDE)}"config\stl_mycomp.h"\
	{$(INCLUDE)}"config\stl_sco.h"\
	{$(INCLUDE)}"config\stl_select_lib.h"\
	{$(INCLUDE)}"config\stl_sgi.h"\
	{$(INCLUDE)}"config\stl_solaris.h"\
	{$(INCLUDE)}"config\stl_sunpro.h"\
	{$(INCLUDE)}"config\stl_symantec.h"\
	{$(INCLUDE)}"config\stl_watcom.h"\
	{$(INCLUDE)}"config\stl_wince.h"\
	{$(INCLUDE)}"config\stlcomp.h"\
	{$(INCLUDE)}"config\vc_select_lib.h"\
	{$(INCLUDE)}"pthread.h"\
	{$(INCLUDE)}"stl\_abbrevs.h"\
	{$(INCLUDE)}"stl\_config.h"\
	{$(INCLUDE)}"stl\_config_compat.h"\
	{$(INCLUDE)}"stl\_config_compat_post.h"\
	{$(INCLUDE)}"stl\_epilog.h"\
	{$(INCLUDE)}"stl\_prolog.h"\
	{$(INCLUDE)}"stl\_site_config.h"\
	{$(INCLUDE)}"stl_user_config.h"\
	
NODEP_CPP_VECTO=\
	"..\..\..\..\..\..\usr\include\pthread.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\particlefx\emitter_desc.h
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\float_track.h
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\parsing.h
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\particle.h
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\particle_desc.h
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\particle_system.h
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\particle_system_manager.h
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\particle_typedef.h
# End Source File
# Begin Source File

SOURCE=..\..\particlefx\vector_track.h
# End Source File
# End Group
# End Target
# End Project
