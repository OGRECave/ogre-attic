bash copysamples.sh
pushd ..\..\Docs\src
doxygen chm.cfg
popd
makensis ogresdk_vc6.nsi
pause
