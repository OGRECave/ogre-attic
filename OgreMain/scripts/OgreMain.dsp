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
# ADD CPP /nologo /MD /W3 /GX /Ot /Oi /Oy /Ob1 /I "..\include" /I "..\..\Dependencies\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OGRE_NONCLIENT_BUILD" /Yu"OgreStableHeaders.h" /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x418 /d "NDEBUG"
# ADD RSC /l 0x418 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib zdll.lib zziplib.lib freetype2110.lib  devil.lib ilu.lib /nologo /dll /machine:I386 /libpath:"..\..\Dependencies\lib\Release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\lib\Release\OgreMain.dll ..\..\Samples\Common\bin\Release	copy ..\lib\Release\OgreMain.dll ..\..\Tools\Common\bin\Release
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
# ADD CPP /nologo /G6 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\include" /I "..\..\Dependencies\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OGRE_NONCLIENT_BUILD" /Yu"OgreStableHeaders.h" /FD /GZ /Zm500 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x418 /d "_DEBUG"
# ADD RSC /l 0x418 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib zdll.lib zziplibd.lib freetype2110_D.lib devil.lib ilu.lib /nologo /dll /debug /machine:I386 /out:"..\lib\Debug/OgreMain_d.dll" /pdbtype:sept /libpath:"..\..\Dependencies\lib\Debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\lib\Debug\OgreMain_d.dll ..\..\Samples\Common\bin\Debug	copy ..\lib\Debug\OgreMain_d.dll ..\..\Tools\Common\bin\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "OgreMain - Win32 Release"
# Name "OgreMain - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\OgreAnimation.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreAnimationState.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreAnimationTrack.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreArchiveManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreAutoParamDataSource.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreAxisAlignedBox.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreBillboard.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreBillboardParticleRenderer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreBillboardSet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreBitwise.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreBone.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreBorderPanelOverlayElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreCamera.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreCodec.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreColourValue.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreCommon.cpp
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

SOURCE=..\src\OgreCursor.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreDataStream.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreDefaultHardwareBufferManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreDynLib.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreDynLibManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreEdgeListBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreEntity.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreEventDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreEventProcessor.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreEventQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreException.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreExternalTextureSource.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreExternalTextureSourceManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreFont.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreFontManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreFrustum.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGpuProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGpuProgramManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGpuProgramUsage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreHardwareBufferManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreHardwareIndexBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreHardwareOcclusionQuery.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreHardwarePixelBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreHardwareVertexBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreHighLevelGpuProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreHighLevelGpuProgramManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreILCodecs.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreILImageCodec.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreILUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreImage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreImageResampler.h
# End Source File
# Begin Source File

SOURCE=..\src\OgreInput.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreInputEvent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreKeyEvent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreKeyFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreKeyTarget.cpp
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

SOURCE=..\src\OgreMaterialSerializer.cpp
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

SOURCE=..\src\OgreMemoryManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMeshManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMeshSerializer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMeshSerializerImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMouseEvent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMouseMotionTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMouseTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMovableObject.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreMovablePlane.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreNode.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreOverlay.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreOverlayContainer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreOverlayElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreOverlayElementCommands.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreOverlayManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePanelOverlayElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreParticle.cpp
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

SOURCE=..\src\OgrePass.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePatchMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePatchSurface.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePixelConversions.h
# End Source File
# Begin Source File

SOURCE=..\src\OgrePixelFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePlane.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePlatformManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePrecompiledHeaders.cpp
# ADD CPP /Yc"OgreStableHeaders.h"
# End Source File
# Begin Source File

SOURCE=..\src\OgrePredefinedControllers.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreProfiler.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreProgressiveMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreQuaternion.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRectangle2D.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderQueueSortingGrouping.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderSystemCapabilities.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderTexture.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRenderWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreResource.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreResourceBackgroundQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreResourceGroupManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreResourceManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRoot.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreRotationSpline.cpp
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

SOURCE=..\src\OgreSceneQuery.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSerializer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreShadowCaster.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreShadowVolumeExtrudeProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSimpleRenderable.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSimpleSpline.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSingleton.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSkeleton.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSkeletonInstance.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSkeletonManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreSkeletonSerializer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreStaticGeometry.cpp
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

SOURCE=..\src\OgreTagPoint.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTechnique.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTextAreaOverlayElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTexture.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTextureManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTextureUnitState.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTimer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreUserDefinedObject.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreVector2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreVector3.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreVertexIndexData.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreViewport.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreWireBoundingBox.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreZip.cpp
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

SOURCE=..\include\OgreAnimation.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreAnimationState.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreAnimationTrack.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreArchive.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreArchiveFactory.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreArchiveManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreAutoParamDataSource.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreAxisAlignedBox.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreBillboard.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreBillboardParticleRenderer.h
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

SOURCE=..\include\OgreBone.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreBorderPanelOverlayElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreCamera.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreCodec.h
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

SOURCE=..\include\OgreCursor.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreDataStream.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreDefaultHardwareBufferManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreDynLib.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreDynLibManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreEdgeListBuilder.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreEntity.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreErrorDialog.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreEventDispatcher.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreEventListeners.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreEventProcessor.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreEventQueue.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreEventTarget.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreException.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreExternalTextureSource.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreExternalTextureSourceManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreFactoryObj.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreFileSystem.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreFont.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreFontManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreFrameListener.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreFrustum.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGpuProgram.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGpuProgramManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGpuProgramUsage.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreHardwareBuffer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreHardwareBufferManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreHardwareIndexBuffer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreHardwareOcclusionQuery.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreHardwarePixelBuffer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreHardwareVertexBuffer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreHighLevelGpuProgram.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreHighLevelGpuProgramManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreILCodecs.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreILImageCodec.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreILUtil.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreImage.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreImageCodec.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreInput.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreInputEvent.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreIteratorWrappers.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreKeyEvent.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreKeyFrame.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreKeyTarget.h
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

SOURCE=..\include\OgreMaterialSerializer.h
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

SOURCE=..\include\OgreMemoryMacros.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMemoryManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMesh.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMeshFileFormat.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMeshManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMeshSerializer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMeshSerializerImpl.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMouseEvent.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMouseMotionTarget.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMouseTarget.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMovableObject.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreMovablePlane.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreNode.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreNoMemoryMacros.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreOverlay.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreOverlayContainer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreOverlayElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreOverlayElementCommands.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreOverlayElementFactory.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreOverlayManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePanelOverlayElement.h
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

SOURCE=..\include\OgreParticleSystemRenderer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePass.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePatchMesh.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePatchSurface.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePixelFormat.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePlane.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePlaneBoundedVolume.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePlatform.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePlatformManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePositionTarget.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePredefinedControllers.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePrerequisites.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreProfiler.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreProgressiveMesh.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreQuaternion.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRay.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRectangle.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRectangle2D.h
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

SOURCE=..\include\OgreRenderQueueListener.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderQueueSortingGrouping.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderSystem.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderSystemCapabilities.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderTarget.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderTargetListener.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderTargetTexture.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderTexture.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRenderWindow.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreResource.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreResourceBackgroundQueue.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreResourceGroupManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreResourceManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRoot.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreRotationalSpline.h
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

SOURCE=..\include\OgreSceneQuery.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreScriptLoader.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSearchOps.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSerializer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreShadowCaster.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreShadowVolumeExtrudeProgram.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSharedPtr.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSimpleRenderable.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSimpleSpline.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSingleton.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSkeleton.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSkeletonFileFormat.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSkeletonInstance.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSkeletonManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSkeletonSerializer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSphere.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreSpotShadowFadePng.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreStableHeaders.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreStaticFaceGroup.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreStaticGeometry.h
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

SOURCE=..\include\OgreTagPoint.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTargetManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTechnique.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTextAreaOverlayElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTexture.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTextureManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTextureUnitState.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTimer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreUserDefinedObject.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreVector2.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreVector3.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreVector4.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreVertexBoneAssignment.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreVertexIndexData.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreViewport.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreWireBoundingBox.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreZip.h
# End Source File
# Begin Source File

SOURCE=..\include\stl_user_config.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
