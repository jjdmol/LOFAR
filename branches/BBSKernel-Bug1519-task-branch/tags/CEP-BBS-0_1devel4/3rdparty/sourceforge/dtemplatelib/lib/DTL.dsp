# Microsoft Developer Studio Project File - Name="DTL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DTL - Win32 Unicode Debug STLPort
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DTL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DTL.mak" CFG="DTL - Win32 Unicode Debug STLPort"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DTL - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DTL - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "DTL - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "DTL - Win32 Debug STLPort" (based on "Win32 (x86) Static Library")
!MESSAGE "DTL - Win32 Unicode Debug STLPort" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DTL - Win32 Release"

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
# ADD CPP /nologo /W3 /GR /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "DTL_UC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DTL - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "DTL_UC" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DTL - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DTL___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "DTL___Win32_Unicode_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DTL___Win32_Unicode_Debug"
# PROP Intermediate_Dir "DTL___Win32_Unicode_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Gm /GR /GX /ZI /Od /I "." /I "C:\stlport" /I "C:\stlport\old_hp" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__STL_USE_OWN_NAMESPACE" /D "__STL_USE_NAMESPACES" /D "__STL_USE_NEW_C_HEADERS" /D "__STL_USE_NEW_IOSTREAMS" /D "_UNICODE" /D "UNICODE" /FR /YX /FD /GZ /c
# ADD CPP /nologo /Gm /GR /GX /ZI /Od /I "." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_UNICODE" /D "UNICODE" /D "DTL_UC" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DTL - Win32 Debug STLPort"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DTL___Win32_Debug_STLPort"
# PROP BASE Intermediate_Dir "DTL___Win32_Debug_STLPort"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DTL___Win32_Debug_STLPort"
# PROP Intermediate_Dir "DTL___Win32_Debug_STLPort"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gm /GR /GX /ZI /Od /I "." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD CPP /nologo /Gm /GR /GX /ZI /Od /I "." /I "C:\stlport" /I "C:\stlport\old_hp" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__STL_USE_OWN_NAMESPACE" /D "__STL_USE_NAMESPACES" /D "__STL_USE_NEW_C_HEADERS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DTL - Win32 Unicode Debug STLPort"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DTL___Win32_Unicode_Debug_STLPort"
# PROP BASE Intermediate_Dir "DTL___Win32_Unicode_Debug_STLPort"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DTL___Win32_Unicode_Debug_STLPort"
# PROP Intermediate_Dir "DTL___Win32_Unicode_Debug_STLPort"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gm /GR /GX /ZI /Od /I "." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_UNICODE" /D "UNICODE" /FR /YX /FD /GZ /c
# ADD CPP /nologo /Gm /GR /GX /ZI /Od /I "." /I "C:\stlport" /I "C:\stlport\old_hp" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_UNICODE" /D "UNICODE" /D "__STL_USE_OWN_NAMESPACE" /D "__STL_USE_NAMESPACES" /D "__STL_USE_NEW_C_HEADERS" /FR /YX /FD /GZ /c
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

# Name "DTL - Win32 Release"
# Name "DTL - Win32 Debug"
# Name "DTL - Win32 Unicode Debug"
# Name "DTL - Win32 Debug STLPort"
# Name "DTL - Win32 Unicode Debug STLPort"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bind_basics.cpp
# End Source File
# Begin Source File

SOURCE=.\BoundIO.cpp
# End Source File
# Begin Source File

SOURCE=.\clib_fwd.cpp
# End Source File
# Begin Source File

SOURCE=.\CountedPtr.cpp
# End Source File
# Begin Source File

SOURCE=.\date_util.cpp
# End Source File
# Begin Source File

SOURCE=.\DB_Base.cpp
# End Source File
# Begin Source File

SOURCE=.\DBConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\DBException.cpp
# End Source File
# Begin Source File

SOURCE=.\DBStmt.cpp
# End Source File
# Begin Source File

SOURCE=.\DBView.cpp
# End Source File
# Begin Source File

SOURCE=.\dtl_base_types.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalBCA.cpp
# End Source File
# Begin Source File

SOURCE=.\RootException.cpp
# End Source File
# Begin Source File

SOURCE=.\string_util.cpp
# End Source File
# Begin Source File

SOURCE=.\validate.cpp
# End Source File
# Begin Source File

SOURCE=.\variant_row.cpp
# End Source File
# Begin Source File

SOURCE=.\VariantException.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\array_string.h
# End Source File
# Begin Source File

SOURCE=.\bind_basics.h
# End Source File
# Begin Source File

SOURCE=.\BoundIO.h
# End Source File
# Begin Source File

SOURCE=.\Callback.h
# End Source File
# Begin Source File

SOURCE=.\clib_fwd.h
# End Source File
# Begin Source File

SOURCE=.\CountedPtr.h
# End Source File
# Begin Source File

SOURCE=.\date_util.h
# End Source File
# Begin Source File

SOURCE=.\DB_Base.h
# End Source File
# Begin Source File

SOURCE=.\DB_iterator.h
# End Source File
# Begin Source File

SOURCE=.\DBConnection.h
# End Source File
# Begin Source File

SOURCE=.\DBDefaults.h
# End Source File
# Begin Source File

SOURCE=.\DBException.h
# End Source File
# Begin Source File

SOURCE=.\DBIndex.h
# End Source File
# Begin Source File

SOURCE=.\DBStmt.h
# End Source File
# Begin Source File

SOURCE=.\DBView.h
# End Source File
# Begin Source File

SOURCE=.\delete_iterator.h
# End Source File
# Begin Source File

SOURCE=.\DTL.h
# End Source File
# Begin Source File

SOURCE=.\dtl_algo.h
# End Source File
# Begin Source File

SOURCE=.\dtl_base_types.h
# End Source File
# Begin Source File

SOURCE=.\dtl_config.h
# End Source File
# Begin Source File

SOURCE=.\DynaDBView.h
# End Source File
# Begin Source File

SOURCE=.\IndexedDBView.h
# End Source File
# Begin Source File

SOURCE=.\insert_iterator.h
# End Source File
# Begin Source File

SOURCE=.\LocalBCA.h
# End Source File
# Begin Source File

SOURCE=.\minimax.h
# End Source File
# Begin Source File

SOURCE=.\random_select_iterator.h
# End Source File
# Begin Source File

SOURCE=.\random_select_update_iterator.h
# End Source File
# Begin Source File

SOURCE=.\RandomDBView.h
# End Source File
# Begin Source File

SOURCE=.\RootException.h
# End Source File
# Begin Source File

SOURCE=.\select_insert_iterator.h
# End Source File
# Begin Source File

SOURCE=.\select_iterator.h
# End Source File
# Begin Source File

SOURCE=.\select_update_iterator.h
# End Source File
# Begin Source File

SOURCE=.\sql_iterator.h
# End Source File
# Begin Source File

SOURCE=.\starit.h
# End Source File
# Begin Source File

SOURCE=.\std_warn_off.h
# End Source File
# Begin Source File

SOURCE=.\std_warn_on.h
# End Source File
# Begin Source File

SOURCE=.\string_util.h
# End Source File
# Begin Source File

SOURCE=.\table.h
# End Source File
# Begin Source File

SOURCE=.\update_iterator.h
# End Source File
# Begin Source File

SOURCE=.\validate.h
# End Source File
# Begin Source File

SOURCE=.\variant_row.h
# End Source File
# Begin Source File

SOURCE=.\VariantException.h
# End Source File
# Begin Source File

SOURCE=.\vec_multiset.h
# End Source File
# End Group
# End Target
# End Project
