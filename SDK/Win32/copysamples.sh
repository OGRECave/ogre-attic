#!/bin/bash
rm -R Samples
mkdir Samples
mkdir Samples/scripts
mkdir Samples/ReferenceApplication
mkdir Samples/ReferenceApplication/scripts

# Do the project files
/bin/find ../../Samples -iname *.dsp -exec cp \{\} Samples/scripts \;
/bin/find ../../Samples -iname *.vcproj -exec cp \{\} Samples/scripts \;
/bin/find ../../ReferenceApplication -iname *.dsp -exec cp \{\} Samples/ReferenceApplication/scripts \;
/bin/find ../../ReferenceApplication -iname *.vcproj -exec cp \{\} Samples/ReferenceApplication/scripts \;
rm Samples/scripts/OgreGUIRenderer.dsp
rm Samples/scripts/OgreGUIRenderer.vcproj
/bin/find Samples/scripts/ -iname *.dsp -exec sed -i -f altersamples.sed \{\} \;
/bin/find Samples/scripts/ -iname *.vcproj -exec sed -i -f altersamples.sed \{\} \;
# do reference app too

# Source & headers just referenced in setup
