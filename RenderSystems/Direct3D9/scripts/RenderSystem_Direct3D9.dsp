# Microsoft Developer Studio Project File - Name="RenderSystem_Direct3D9" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=RenderSystem_Direct3D9 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RenderSystem_Direct3D9.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RenderSystem_Direct3D9.mak" CFG="RenderSystem_Direct3D9 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RenderSystem_Direct3D9 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "RenderSystem_Direct3D9 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RenderSystem_Direct3D9 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin\Release"
# PROP Intermediate_Dir "..\obj\Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OGRED3DENGINEDLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G5 /MD /W3 /GX /O2 /Ob0 /I "..\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /D "_USRDLL" /D "OGRED3D9ENGINEDLL_EXPORTS" /D "NDEBUG" /D "_MBCS" /D "OGRE_WIN32" /D "_WINDOWS" /D "WIN32" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib d3d9.lib d3dx9.lib dxerr9.lib /nologo /dll /machine:I386 /nodefaultlib:"LIBC" /libpath:"..\..\..\OgreMain\lib\Release" /libpath:"..\..\..\Dependencies\lib\Release"
# SUBTRACT LINK32 /profile /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\bin\Release\*.dll ..\..\..\Samples\Common\bin\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "RenderSystem_Direct3D9 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin\Debug"
# PROP Intermediate_Dir "..\obj\Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OGRED3DENGINEDLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /D "_USRDLL" /D "OGRED3D9ENGINEDLL_EXPORTS" /D "_MBCS" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /FD /GZ /Zm500 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dxguid.lib OgreMain_d.lib d3d9.lib d3dx9.lib dxerr9.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBCD" /nodefaultlib:"LIBCMT" /pdbtype:sept /libpath:"../../../OgreMain/Lib/Debug" /libpath:"..\..\..\Dependencies\lib\Debug"
# SUBTRACT LINK32 /verbose
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\bin\Debug\*.dll ..\..\..\Samples\Common\bin\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "RenderSystem_Direct3D9 - Win32 Release"
# Name "RenderSystem_Direct3D9 - Win32 Debug"
# Begin Group "D3dEngine"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\OgreD3D9Driver.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9Driver.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9DriverList.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9DriverList.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9GpuProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9GpuProgram.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9GpuProgramManager.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9GpuProgramManager.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9HardwareBufferManager.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9HardwareBufferManager.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9HardwareIndexBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9HardwareIndexBuffer.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9HardwareOcclusionQuery.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9HardwareOcclusionQuery.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9HardwarePixelBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9HardwarePixelBuffer.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9HardwareVertexBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9HardwareVertexBuffer.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9HLSLProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9HLSLProgram.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9HLSLProgramFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9HLSLProgramFactory.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9Mappings.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9Mappings.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9Prerequisites.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9RenderSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9RenderSystem.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9RenderWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9RenderWindow.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9Texture.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9Texture.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9TextureManager.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9TextureManager.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9VertexDeclaration.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9VideoMode.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9VideoMode.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreD3D9VideoModeList.cpp
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9VideoModeList.h
# End Source File
# End Group
# Begin Group "Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\OgreD3D9EngineDll.cpp
# End Source File
# Begin Source File

SOURCE=..\misc\OgreD3D9RenderSystem.def
# End Source File
# Begin Source File

SOURCE=..\include\OgreD3D9VertexDeclaration.h
# End Source File
# End Group
# End Target
# End Project
