# Microsoft Developer Studio Project File - Name="Plugin_GuiElements" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Plugin_GuiElements - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Plugin_GuiElements.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Plugin_GuiElements.mak" CFG="Plugin_GuiElements - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Plugin_GuiElements - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Plugin_GuiElements - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/S&S/DEV/Ogrenew/Plugin_GuiElements", PKDAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Plugin_GuiElements - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PLUGIN_GuiElements_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob2 /I "../include" /I "../../../OgreMain/include" /I "..\..\..\Dependencies\include" /D "_USRDLL" /D "PLUGIN_GUIELEMENTS_EXPORTS" /D "NDEBUG" /D "_MBCS" /D "OGRE_WIN32" /D "_WINDOWS" /D "WIN32" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x418 /d "NDEBUG"
# ADD RSC /l 0x418 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain.lib /nologo /dll /machine:I386 /libpath:"..\..\..\OgreMain\lib\Release"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\bin\Release\*.dll ..\..\..\Samples\Common\bin\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Plugin_GuiElements - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PLUGIN_GuiElements_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../include" /I "../../../OgreMain/include" /I "..\..\..\Dependencies\include" /D "_USRDLL" /D "PLUGIN_GUIELEMENTS_EXPORTS" /D "_MBCS" /D "OGRE_WIN32" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /D "OGRE_DYNAMIC_LINKAGE" /YX /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x418 /d "_DEBUG"
# ADD RSC /l 0x418 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OgreMain_d.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"LIBCMT" /pdbtype:sept /libpath:"..\..\..\OgreMain\lib\Debug"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\bin\Debug\*.dll ..\..\..\Samples\Common\bin\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Plugin_GuiElements - Win32 Release"
# Name "Plugin_GuiElements - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\OgreBorderButtonGuiElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreBorderPanelGuiElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreButtonGuiElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreCursorGuiElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreGuiElements.cpp
# End Source File
# Begin Source File

SOURCE=..\misc\OgreGuiElements.def
# End Source File
# Begin Source File

SOURCE=..\src\OgreListGuiElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePanelGuiElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgrePopupMenuGuiElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreScrollBarGuiElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTextAreaGuiElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTextBoxGuiElement.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OgreTTYGuiElement.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\OgreBorderButtonGuiElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreBorderPanelGuiElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreButtonGuiElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreCursorGuiElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreCursorGuiManager.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGuiElementFactories.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreGuiElementPrerequisites.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreListGuiElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePanelGuiElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgrePopupMenuGuiElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreScrollBarGuiElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTextAreaGuiElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTextBoxGuiElement.h
# End Source File
# Begin Source File

SOURCE=..\include\OgreTTYGuiElement.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
