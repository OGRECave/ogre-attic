@echo on

set CONFIGURATION=release
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
del "%DLL_OUTDIR%\plugins_d.cfg"
xcopy /D /S .\Config\shaders "%OUTDIR%\%EXPORTERDIR%\shaders\"
xcopy %COPY_PARAMS% .\LexiExport\bin\LEXIExporter.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% .\LexiExportProxy\bin\LEXIExportProxy.dlu "%OUTDIR%\plugins\"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\ILUT.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\OIS.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\cg.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\OgreMain.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\Plugin_CgProgramManager.dll "%DLL_OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\RenderSystem_GL.dll "%DLL_OUTDIR%"

pause