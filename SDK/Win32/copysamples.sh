#!/bin/bash
rm -R samples
mkdir samples
mkdir samples/scripts
mkdir samples/refapp
mkdir samples/refapp/scripts

# Do the project files
/bin/find ../../samples -iname *.dsp -exec cp \{\} samples/scripts \;
/bin/find ../../samples -iname *.vcproj -exec cp \{\} samples/scripts \;
cp ../../ReferenceApplication/BspCollision/scripts/*.dsp samples/scripts
cp ../../ReferenceApplication/BspCollision/scripts/*.vcproj samples/scripts
cp ../../ReferenceApplication/ReferenceAppLayer/scripts/*.dsp samples/refapp/scripts
cp ../../ReferenceApplication/ReferenceAppLayer/scripts/*.vcproj samples/refapp/scripts
rm samples/scripts/OgreGUIRenderer.dsp
rm samples/scripts/OgreGUIRenderer.vcproj
/bin/find samples/scripts/ -iname *.dsp -exec sed -i -f altersamples.sed \{\} \;
/bin/find samples/scripts/ -iname *.vcproj -exec sed -i -f altersamples.sed \{\} \;
/bin/find samples/refapp/scripts/ -iname *.dsp -exec sed -i -f alterrefapp.sed \{\} \;
/bin/find samples/refapp/scripts/ -iname *.vcproj -exec sed -i -f alterrefapp.sed \{\} \;
# Source & headers just referenced in setup
