bash copysamples.sh
pushd ..\..\Docs\src
doxygen chm.cfg
popd
makensis ogresdk_vc7.nsi
pause
