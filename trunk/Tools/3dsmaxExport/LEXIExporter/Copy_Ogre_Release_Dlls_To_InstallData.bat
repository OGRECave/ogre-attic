@echo on

set CONFIGURATION=release
set EXPORTERDIR=LEXIExporter

set SDKSUBPATH_OGRE=D:\ITE5000-SDK\SDK\Ogre\1.2.2\bin\bin

set OUTDIR=.\LexiExportInstall\InstallData
set COPY_PARAMS=/F /H /R /Y

xcopy %COPY_PARAMS% .\Config\*.cfg "%OUTDIR%"
xcopy /D /S .\Config\shaders "%OUTDIR%\%EXPORTERDIR%\shaders\"
xcopy %COPY_PARAMS% .\LexiExport\bin\LEXIExport.dlu "%OUTDIR%\plugins\"
del "%OUTDIR%\plugins\LEXIExport(debug).dlu"
del "%OUTDIR%\OgreMain.dll"
del "%OUTDIR%\OgrePlatform.dll"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\zlib1.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\DevIL.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\ILU.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\ILUT.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\cg.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\OgreMain.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\OgrePlatform.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\Plugin_CgProgramManager.dll "%OUTDIR%\%EXPORTERDIR%\"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\RenderSystem_GL.dll "%OUTDIR%\%EXPORTERDIR%\"
