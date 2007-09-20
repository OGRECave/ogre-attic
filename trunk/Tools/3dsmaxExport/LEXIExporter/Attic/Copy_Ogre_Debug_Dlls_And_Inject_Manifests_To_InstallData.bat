@echo on

set CONFIGURATION=debug
set EXPORTERDIR=LEXIExporter

set SDKSUBPATH_OGRE=%OGRE_HOME%\bin

set OUTDIR=.\LexiExportInstall\InstallData
set DLL_OUTDIR=%OUTDIR%\%EXPORTERDIR%\Dlls\
set COPY_PARAMS=/F /H /R /Y

rmdir /S /Q %OUTDIR%
mkdir %OUTDIR%
mkdir %OUTDIR%\%EXPORTERDIR%\
mkdir %OUTDIR%\%EXPORTERDIR%\Logs

xcopy %COPY_PARAMS% .\Config\*.cfg "%DLL_OUTDIR%"
del "%DLL_OUTDIR%\plugins.cfg"
xcopy /D /S .\Config\shaders "%OUTDIR%\%EXPORTERDIR%\shaders\"
xcopy %COPY_PARAMS% .\LexiExport\bin\LEXIExporter_d.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% .\LexiExportProxy\bin\LEXIExportProxy_d.dlu "%OUTDIR%\plugins\"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\ILUT_d.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\OIS_d.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\cg_d.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\OgreMain_d.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\Plugin_CgProgramManager_d.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\RenderSystem_GL_d.dll "%DLL_OUTDIR%"

set VS8MTDIR=%VS80COMNTOOLS%\Bin

"%VS8MTDIR%\mt.exe" -inputresource:"%OUTDIR%"\OgreMain_d.dll;#2 -outputresource:"%DLL_OUTDIR%"\Plugin_CgProgramManager_d.dll;#2
"%VS8MTDIR%\mt.exe" -inputresource:"%OUTDIR%"\OgreMain_d.dll;#2 -outputresource:"%DLL_OUTDIR%"\RenderSystem_Direct3D9_d.dll;#2
"%VS8MTDIR%\mt.exe" -inputresource:"%OUTDIR%"\OgreMain_d.dll;#2 -outputresource:"%DLL_OUTDIR%"\RenderSystem_GL_d.dll;#2
pause