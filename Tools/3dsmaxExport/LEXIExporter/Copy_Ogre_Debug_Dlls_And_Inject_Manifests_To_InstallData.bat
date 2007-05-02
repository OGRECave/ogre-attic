@echo on

set CONFIGURATION=debug
set EXPORTERDIR=LEXIExporter

set SDKSUBPATH_OGRE=..\..\SDK\Ogre\1.2.5\bin\bin

set OUTDIR=.\LexiExportInstall\InstallData
set COPY_PARAMS=/F /H /R /Y

xcopy %COPY_PARAMS% .\Config\*.cfg "%OUTDIR%"
xcopy /D /S .\Config\shaders "%OUTDIR%\%EXPORTERDIR%\shaders\"
xcopy %COPY_PARAMS% .\LexiExport\bin\LEXIExport(debug).dlu "%OUTDIR%\plugins\"
del "%OUTDIR%\plugins\LEXIExport.dlu"
del "%OUTDIR%\OgreMain.dll"
del "%OUTDIR%\OgrePlatform.dll"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\zlib1.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\DevIL.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\ILU.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\ILUT.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\cg.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\OgreMain_d.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\OgrePlatform_d.dll "%OUTDIR%"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\Plugin_CgProgramManager.dll "%OUTDIR%\%EXPORTERDIR%\"
xcopy %COPY_PARAMS% %SDKSUBPATH_OGRE%\%CONFIGURATION%\RenderSystem_GL.dll "%OUTDIR%\%EXPORTERDIR%\"

set VS8MTDIR=%VS80COMNTOOLS%\Bin

"%VS8MTDIR%\mt.exe" -inputresource:"%OUTDIR%"\OgreMain_d.dll;#2 -outputresource:"%OUTDIR%"\OgrePlatform_d.dll;#2
"%VS8MTDIR%\mt.exe" -inputresource:"%OUTDIR%"\OgreMain_d.dll;#2 -outputresource:"%OUTDIR%\%EXPORTERDIR%"\Plugin_CgProgramManager.dll;#2
"%VS8MTDIR%\mt.exe" -inputresource:"%OUTDIR%"\OgreMain_d.dll;#2 -outputresource:"%OUTDIR%\%EXPORTERDIR%"\RenderSystem_Direct3D9.dll;#2
"%VS8MTDIR%\mt.exe" -inputresource:"%OUTDIR%"\OgreMain_d.dll;#2 -outputresource:"%OUTDIR%\%EXPORTERDIR%"\RenderSystem_GL.dll;#2
