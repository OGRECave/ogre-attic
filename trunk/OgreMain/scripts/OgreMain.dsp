# Microsoft Developer Studio Project File - Name="OgreMain" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=OgreMain - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "OgreMain.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OgreMain.mak" CFG="OgreMain - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OgreMain - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "OgreMain - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "OgreMain - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib\Release"
# PROP Intermediate_Dir "..\obj\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OGREMAIN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\include" /I "..\..\Dependencies\zlib\include" /I "..\..\Dependencies\libpng\include" /I "..\..\Dependencies\jpeglib\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OGREMAIN_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x418 /d "NDEBUG"
# ADD RSC /l 0x418 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  zlib.lib libpng.lib jpeglib.lib /nologo /dll /machine:I386 /libpath:"..\..\Dependencies\zlib\lib\Release" /libpath:"..\..\Dependencies\libpng\lib\Release" /libpath:"..\..\Dependencies\jpeglib\lib\Release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Desc=Building dependencies
PreLink_Cmds=cd ..\..\Dependencies 	makeall_vc6.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "OgreMain - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib\Debug"
# PROP Intermediate_Dir "..\obj\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OGREMAIN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\include" /I "..\..\Dependencies\zlib\include" /I "..\..\Dependencies\libpng\include" /I "..\..\Dependencies\jpeglib\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OGREMAIN_EXPORTS" /D "_STLP_USE_DYNAMIC_LIB" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x418 /d "_DEBUG"
# ADD RSC /l 0x418 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib zlib.lib libpng.lib jpeglib.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\..\Dependencies\zlib\lib\Debug" /libpath:"..\..\Dependencies\libpng\lib\Debug" /libpath:"..\..\Dependencies\jpeglib\lib\Debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Desc=Building dependencies
PreLink_Cmds=cd ..\..\Dependencies 	makeall_vc6.bat
PostBuild_Cmds=copy ..\lib\Debug\OgreMain.dll ..\..\Samples\Common\bin\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "OgreMain - Win32 Release"
# Name "OgreMain - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\OgreArchiveEx.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreArchiveManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreAxisAlignedBox.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreBillboard.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreBillboardSet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreBitwise.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreCamera.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreColourValue.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreConfigFile.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreConfigOptionMap.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreController.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreControllerManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreDataChunk.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreDynLib.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreDynLibManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreEntity.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreException.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreImage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreInput.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreLight.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreLog.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreLogManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMaterial.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMaterialManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMath.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMatrix3.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMatrix4.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMeshManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMovableObject.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreOofModelFile.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreParticleEmitter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreParticleEmitterCommands.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreParticleIterator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreParticleSystemManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePatchSurface.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePlane.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePlatformManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePredefinedControllers.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreQuaternion.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderWindowMap.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreResourceManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreResourceMap.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRoot.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSceneManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSceneManagerEnumerator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSceneNode.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSDDataChunk.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSimpleRenderable.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSingleton.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreString.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreStringConverter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreStringInterface.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreStringVector.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSubEntity.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSubMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTextureLayer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTextureManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreVector3.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreViewport.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreZip.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreZipArchiveFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\src\unzip.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\asm_math.h
# End Source File
# Begin Source File

SOURCE=..\include\Ogre.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreArchiveEx.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreArchiveFactory.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreArchiveManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreAxisAlignedBox.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreBillboard.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreBillboardSet.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreBitwise.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreBlendMode.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreCamera.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreCodecFactory.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreColourValue.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreCommon.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreConfig.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreConfigDialog.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreConfigFile.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreConfigOptionMap.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreController.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreControllerManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreCustomizable.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreDataChunk.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreDynLib.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreDynLibManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreEntity.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreErrorDialog.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreException.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreFrameListener.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGeometryData.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreImage.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreInput.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreLight.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreLog.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreLogManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMaterial.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMaterialManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMath.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMatrix3.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMatrix4.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMesh.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMeshManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMovableObject.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreOofFile.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreOofModelFile.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreParticle.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreParticleAffector.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreParticleAffectorFactory.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreParticleEmitter.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreParticleEmitterCommands.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreParticleEmitterFactory.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreParticleIterator.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreParticleSystem.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreParticleSystemManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePatchSurface.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePlane.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePlatform.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePlatformManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePredefinedControllers.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePrerequisites.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreQuaternion.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderable.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderOperation.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderQueue.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderSystem.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderTarget.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderWindow.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderWindowMap.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreResource.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreResourceManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreResourceMap.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRoot.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSceneManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSceneManagerEnumerator.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSceneNode.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSDDataChunk.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSimpleRenderable.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSingleton.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSphere.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreStaticFaceGroup.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreStdHeaders.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreString.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreStringConverter.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreStringInterface.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreStringVector.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSubEntity.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSubMesh.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTexture.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTextureManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreUnzip.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreVector3.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreViewport.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreZip.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreZipArchiveFactory.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
