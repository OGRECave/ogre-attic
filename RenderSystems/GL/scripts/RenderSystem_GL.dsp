# Microsoft Developer Studio Project File - Name="RenderSystem_GL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=RenderSystem_GL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RenderSystem_GL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RenderSystem_GL.mak" CFG="RenderSystem_GL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RenderSystem_GL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "RenderSystem_GL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RenderSystem_GL - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RENDERSYSTEM_GL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob0 /I "..\include" /I "..\src\win32" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /I "..\src\nvparse" /I "..\src\nvparse\winheaders" /I "..\src\atifs\include" /I "..\src\glsl\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RENDERSYSTEM_GL_EXPORTS" /D "OGRE_GL_USE_MULTITEXTURING" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x418 /d "NDEBUG"
# ADD RSC /l 0x418 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OPENGL32.lib GLU32.lib OgreMain.lib /nologo /dll /machine:I386 /libpath:"..\..\..\OgreMain\lib\Release" /libpath:"..\..\..\Dependencies\lib\Release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\bin\Release\*.dll ..\..\..\Samples\Common\bin\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "RenderSystem_GL - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RENDERSYSTEM_GL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\src\win32" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /I "..\src\nvparse" /I "..\src\nvparse\winheaders" /I "..\src\atifs\include" /I "..\src\glsl\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RENDERSYSTEM_GL_EXPORTS" /D "OGRE_GL_USE_MULTITEXTURING" /YX /FD /GZ /Zm500 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x418 /d "_DEBUG"
# ADD RSC /l 0x418 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain_d.lib OPENGL32.lib GLU32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\OgreMain\lib\Debug" /libpath:"..\..\..\Dependencies\lib\Debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\bin\Debug\*.dll ..\..\..\Samples\Common\bin\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "RenderSystem_GL - Win32 Release"
# Name "RenderSystem_GL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\glprocs.c
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLATIFSInit.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLContext.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLDefaultHardwareBufferManager.cpp
# End Source File
# Begin Source File

SOURCE=..\misc\OgreGLEngine.def
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLEngineDll.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLGpuNvparseProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLGpuProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLGpuProgramManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLHardwareBufferManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLHardwareIndexBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLHardwareOcclusionQuery.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLHardwarePixelBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLHardwareVertexBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLPixelFormat.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLRenderSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLSupport.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLTexture.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGLTextureManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\OgreWin32Context.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreWin32GLSupport.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreWin32RenderTexture.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreWin32Window.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\glprocs.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLContext.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLDefaultHardwareBufferManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLGpuNvparseProgram.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLGpuProgram.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLGpuProgramManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLHardwareBufferManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLHardwareIndexBuffer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLHardwareOcclusionQuery.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLHardwarePixelBuffer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLHardwareVertexBuffer.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLPixelFormat.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLPrerequisites.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLRenderSystem.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLSupport.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLTexture.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGLTextureManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreWin32Context.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreWin32GLSupport.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreWin32Prerequisites.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreWin32RenderTexture.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreWin32Window.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "nvparse"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\nvparse\_ps1.0_lexer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_ps1.0_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_ps1.0_parser.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_rc1.0_lexer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_rc1.0_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_rc1.0_parser.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_ts1.0_lexer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_ts1.0_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_ts1.0_parser.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_vs1.0_lexer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_vs1.0_parser.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\_vs1.0_parser.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\avp1.0_impl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\macro.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\nvparse.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\nvparse.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\nvparse_errors.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\nvparse_errors.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\nvparse_externs.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\ps1.0_program.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\ps1.0_program.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\rc1.0_combiners.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\rc1.0_combiners.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\rc1.0_final.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\rc1.0_final.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\rc1.0_general.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\rc1.0_general.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\rc1.0_register.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\ts1.0_inst.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\ts1.0_inst.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\ts1.0_inst_list.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\ts1.0_inst_list.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\winheaders\unistd.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\vcp1.0_impl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\vp1.0_impl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\vs1.0_inst.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\vs1.0_inst.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\vs1.0_inst_list.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\vs1.0_inst_list.h
# End Source File
# Begin Source File

SOURCE=..\src\nvparse\vsp1.0_impl.cpp
# End Source File
# End Group
# Begin Group "atifs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\atifs\src\ATI_FS_GLGpuProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\src\atifs\include\ATI_FS_GLGpuProgram.h
# End Source File
# Begin Source File

SOURCE=..\src\atifs\src\Compiler2Pass.cpp
# End Source File
# Begin Source File

SOURCE=..\src\atifs\include\Compiler2Pass.h
# End Source File
# Begin Source File

SOURCE=..\src\atifs\src\ps_1_4.cpp
# End Source File
# Begin Source File

SOURCE=..\src\atifs\include\ps_1_4.h
# End Source File
# End Group
# Begin Group "glsl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\GLSL\src\OgreGLSLExtSupport.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\include\OgreGLSLExtSupport.h
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\src\OgreGLSLGpuProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\include\OgreGLSLGpuProgram.h
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\src\OgreGLSLLinkProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\include\OgreGLSLLinkProgram.h
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\src\OgreGLSLLinkProgramManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\include\OgreGLSLLinkProgramManager.h
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\src\OgreGLSLProgram.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\include\OgreGLSLProgram.h
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\src\OgreGLSLProgramFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GLSL\include\OgreGLSLProgramFactory.h
# End Source File
# End Group
# End Target
# End Project
