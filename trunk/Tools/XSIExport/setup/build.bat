@echo off
if "%1" == "4" goto xsi4
if "%1" =="5" goto xsi5
:usage
echo build [version]
goto end

:xsi5
candle xsi5install.wxs
light -out OgreXSIExporter5.msi xsi5install.wixobj %WIX_ROOT%\ui\wixui_featuretree.wixlib
goto end

:xsi4:
candle xsi4install.wxs
light -out OgreXSIExporter4.msi xsi4install.wixobj %WIX_ROOT%\ui\wixui_featuretree.wixlib
goto end



:end

