# Microsoft Developer Studio Project File - Name="ReferenceAppLayer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ReferenceAppLayer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ReferenceAppLayer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ReferenceAppLayer.mak" CFG="ReferenceAppLayer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ReferenceAppLayer - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ReferenceAppLayer - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ReferenceAppLayer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin\Release"
# PROP Intermediate_Dir "..\obj\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PLUGIN_BSPSCENEMANAGER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "REFERENCEAPPLAYER_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x418 /d "NDEBUG"
# ADD RSC /l 0x418 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib ode.lib /nologo /dll /pdb:"..\lib\Release/ReferenceAppLayer.pdb" /machine:I386 /out:"..\lib\Release/ReferenceAppLayer.dll" /implib:"..\lib\Release/ReferenceAppLayer.lib" /libpath:"..\..\..\OgreMain\lib\Release" /libpath:"..\..\..\Dependencies\lib\Release"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\lib\Release\*.dll ..\..\..\Samples\Common\bin\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ReferenceAppLayer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin\Debug"
# PROP Intermediate_Dir "..\obj\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PLUGIN_BSPSCENEMANAGER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "REFERENCEAPPLAYER_EXPORTS" /YX /FD /GZ /Zm500 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x418 /d "_DEBUG"
# ADD RSC /l 0x418 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain_d.lib ode.lib /nologo /dll /pdb:"..\lib\Debug/ReferenceAppLayer.pdb" /debug /machine:I386 /out:"..\lib\Debug/ReferenceAppLayer.dll" /implib:"..\lib\Debug/ReferenceAppLayer.lib" /pdbtype:sept /libpath:"..\..\..\OgreMain\lib\Debug" /libpath:"..\..\..\Dependencies\lib\Debug"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\lib\Debug\*.dll ..\..\..\Samples\Common\bin\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ReferenceAppLayer - Win32 Release"
# Name "ReferenceAppLayer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\OgreRefAppApplicationObject.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRefAppBall.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRefAppBox.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRefAppCollideCamera.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRefAppJoint.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRefAppJointSubtypes.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRefAppOgreHead.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRefAppPlane.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRefAppWorld.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\OgreRefAppApplicationObject.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRefAppBall.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRefAppBox.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRefAppCollideCamera.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRefAppJoint.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRefAppJointSubtypes.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRefAppOgreHead.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRefAppPlane.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRefAppPrerequisites.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRefAppWorld.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreReferenceAppLayer.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
