@echo on

set CONFIGURATION=release
set EXPORTERDIR=LEXIExporter

set SDKSUBPATH_OGRE=..\..\SDK\Ogre\1.4.1\bin\bin

set OUTDIR=.\LexiExportInstall\InstallData
set COPY_PARAMS=/F /H /R /Y

xcopy %COPY_PARAMS% .\Config\*.cfg "%OUTDIR%"
xcopy /D /S .\Config\shaders "%OUTDIR%\%EXPORTERDIR%\shaders\"
xcopy %COPY_PARAMS% .\LexiExport\bin\LEXIExport.dlu "%OUTDIR%\plugins\"
del "%OUTDIR%\plugins\LEXIExport(debug).dlu"
del "%OUTDIR%\OgreMain_d.dll"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\ILUT.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\OIS.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\cg.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\OgreMain.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\Plugin_CgProgramManager.dll "%OUTDIR%\%EXPORTERDIR%\"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\RenderSystem_GL.dll "%OUTDIR%\%EXPORTERDIR%\"
pause