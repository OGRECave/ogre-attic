# Microsoft Developer Studio Project File - Name="Demo_BspCollision" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Demo_BspCollision - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Demo_BspCollision.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Demo_BspCollision.mak" CFG="Demo_BspCollision - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Demo_BspCollision - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Demo_BspCollision - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Demo_BspCollision - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\Samples\Common\bin\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "..\obj\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Samples\Common\bin\$(ConfigurationName)"
# PROP Intermediate_Dir "..\obj\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /I "..\include" /I "..\..\Common\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /I "..\..\ReferenceAppLayer\include" /ZI /W3 /Od /Op /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Gm /Gy PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MDd /I "..\include" /I "..\..\Common\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /I "..\..\ReferenceAppLayer\include" /ZI /W3 /Od /Op /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Gm /Gy PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 
# ADD RSC /l 1033 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib ReferenceAppLayer.lib /nologo /out:"..\..\..\Samples\Common\bin\$(ConfigurationName)\Demo_BspCollision.exe" /incremental:yes /libpath:""..\..\..\OgreMain\lib\$(ConfigurationName)"" /libpath:""..\..\Dependencies\lib\$(ConfigurationName)"" /libpath:""..\..\ReferenceAppLayer\lib\$(ConfigurationName)"" /debug /pdb:"..\..\..\Samples\Common\bin\$(ConfigurationName)\Demo_BspCollision.pdb" /pdbtype:sept /subsystem:windows /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib ReferenceAppLayer.lib /nologo /out:"..\..\..\Samples\Common\bin\$(ConfigurationName)\Demo_BspCollision.exe" /incremental:yes /libpath:""..\..\..\OgreMain\lib\$(ConfigurationName)"" /libpath:""..\..\Dependencies\lib\$(ConfigurationName)"" /libpath:""..\..\ReferenceAppLayer\lib\$(ConfigurationName)"" /debug /pdb:"..\..\..\Samples\Common\bin\$(ConfigurationName)\Demo_BspCollision.pdb" /pdbtype:sept /subsystem:windows /machine:ix86 

!ELSEIF  "$(CFG)" == "Demo_BspCollision - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\..\Samples\Common\bin\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "..\obj\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Samples\Common\bin\$(ConfigurationName)"
# PROP Intermediate_Dir "..\obj\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /I "..\include" /I "..\..\Common\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /I "..\..\ReferenceAppLayer\include" /Zi /W3 /O2 /Og /Ob1 /Op /Ot /Oy /G6 /GA /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /GF /Gm /Gy PRECOMP_VC7_TOBEREMOVED /c /GX 
# ADD CPP /nologo /MD /I "..\include" /I "..\..\Common\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /I "..\..\ReferenceAppLayer\include" /Zi /W3 /O2 /Og /Ob1 /Op /Ot /Oy /G6 /GA /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /GF /Gm /Gy PRECOMP_VC7_TOBEREMOVED /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 
# ADD RSC /l 1033 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib ReferenceAppLayer.lib /nologo /out:"..\..\..\Samples\Common\bin\$(ConfigurationName)\Demo_BspCollision.exe" /incremental:no /libpath:""..\..\..\OgreMain\lib\$(ConfigurationName)"" /libpath:""..\..\Dependencies\lib\$(ConfigurationName)"" /libpath:""..\..\ReferenceAppLayer\lib\$(ConfigurationName)"" /pdbtype:sept /subsystem:windows /opt:ref /opt:icf /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib ReferenceAppLayer.lib /nologo /out:"..\..\..\Samples\Common\bin\$(ConfigurationName)\Demo_BspCollision.exe" /incremental:no /libpath:""..\..\..\OgreMain\lib\$(ConfigurationName)"" /libpath:""..\..\Dependencies\lib\$(ConfigurationName)"" /libpath:""..\..\ReferenceAppLayer\lib\$(ConfigurationName)"" /pdbtype:sept /subsystem:windows /opt:ref /opt:icf /machine:ix86 

!ENDIF

# Begin Target

# Name "Demo_BspCollision - Win32 Debug"
# Name "Demo_BspCollision - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm"
# Begin Source File

SOURCE=..\src\BspCollision.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc"
# Begin Source File

SOURCE=..\..\Common\include\ExampleRefAppApplication.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\include\ExampleRefAppFrameListener.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

