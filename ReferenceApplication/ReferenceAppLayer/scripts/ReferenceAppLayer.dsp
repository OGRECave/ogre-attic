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
!MESSAGE "ReferenceAppLayer - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ReferenceAppLayer - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ReferenceAppLayer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\lib\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "..\obj\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\$(ConfigurationName)"
# PROP Intermediate_Dir "..\obj\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /I "..\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "REFERENCEAPPLAYER_EXPORTS" /D "_MBCS" /Gm /Gy /YX /GZ /c /GX 
# ADD CPP /nologo /MDd /I "..\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "REFERENCEAPPLAYER_EXPORTS" /D "_MBCS" /Gm /Gy /YX /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 
# ADD RSC /l 1033 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib ode.lib /nologo /dll /out:"..\lib\$(ConfigurationName)\ReferenceAppLayer.dll" /version:0.99 /incremental:yes /libpath:""..\..\..\OgreMain\lib\$(ConfigurationName)"" /libpath:""..\..\..\Dependencies\lib\$(ConfigurationName)"" /debug /pdb:"..\lib\$(ConfigurationName)\ReferenceAppLayer.pdb" /pdbtype:sept /subsystem:windows /implib:"$(OutDir)/ReferenceAppLayer.lib" /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib ode.lib /nologo /dll /out:"..\lib\$(ConfigurationName)\ReferenceAppLayer.dll" /version:0.99 /incremental:yes /libpath:""..\..\..\OgreMain\lib\$(ConfigurationName)"" /libpath:""..\..\..\Dependencies\lib\$(ConfigurationName)"" /debug /pdb:"..\lib\$(ConfigurationName)\ReferenceAppLayer.pdb" /pdbtype:sept /subsystem:windows /implib:"$(OutDir)/ReferenceAppLayer.lib" /machine:ix86 
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\$(TargetFileName) ..\..\..\Samples\Common\bin\$(ConfigurationName)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ReferenceAppLayer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\lib\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "..\obj\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\$(ConfigurationName)"
# PROP Intermediate_Dir "..\obj\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /I "..\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /Zi /W3 /Ox /Og /Ob2 /Oi /Op /Ot /Oy /GT /G6 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "REFERENCEAPPLAYER_EXPORTS" /D "_MBCS" /GF /Gy PRECOMP_VC7_TOBEREMOVED /c /GX 
# ADD CPP /nologo /MD /I "..\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /Zi /W3 /Ox /Og /Ob2 /Oi /Op /Ot /Oy /GT /G6 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "REFERENCEAPPLAYER_EXPORTS" /D "_MBCS" /GF /Gy PRECOMP_VC7_TOBEREMOVED /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 
# ADD RSC /l 1033 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib ode.lib /nologo /dll /out:"..\lib\$(ConfigurationName)\ReferenceAppLayer.dll" /version:0.99 /incremental:no /libpath:""..\..\..\OgreMain\lib\$(ConfigurationName)"" /libpath:""..\..\..\Dependencies\lib\$(ConfigurationName)"" /debug /pdbtype:sept /subsystem:windows /opt:ref /opt:icf /implib:"$(OutDir)/ReferenceAppLayer.lib" /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib ode.lib /nologo /dll /out:"..\lib\$(ConfigurationName)\ReferenceAppLayer.dll" /version:0.99 /incremental:no /libpath:""..\..\..\OgreMain\lib\$(ConfigurationName)"" /libpath:""..\..\..\Dependencies\lib\$(ConfigurationName)"" /debug /pdbtype:sept /subsystem:windows /opt:ref /opt:icf /implib:"$(OutDir)/ReferenceAppLayer.lib" /machine:ix86 
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OutDir)\$(TargetFileName) ..\..\..\Samples\Common\bin\$(ConfigurationName)
# End Special Build Tool

!ENDIF

# Begin Target

# Name "ReferenceAppLayer - Win32 Debug"
# Name "ReferenceAppLayer - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm"
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

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc"
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
# Begin Group "ODE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\collision.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\common.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\contact.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\error.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\geom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\mass.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\matrix.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\memory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\misc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\objects.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\ode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\odecpp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\odecpp_collision.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\odemath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Dependencies\include\ode\rotation.h
# End Source File
# End Group
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

