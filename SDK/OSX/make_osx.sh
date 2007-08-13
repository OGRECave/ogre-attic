#!/bin/bash

# TODO - invoke xcode build

# frameworks
echo Copying frameworks...
rm -rf sdk_frameworks 
mkdir sdk_frameworks

# Stuff we've built
cp -R ../../Mac/build/Release/*.framework sdk_frameworks/

# dependencies
cp -R /Library/Frameworks/Cg.framework sdk_frameworks/
cp -R /Library/Frameworks/CEGUI.framework sdk_frameworks/
# OgreCEGUIrenderer is currently in the precompiled deps, maybe move
cp -R /Library/Frameworks/OgreCEGUIRenderer.framework sdk_frameworks/

echo Frameworks copied.
echo Building frameworks package...

/Developer/Tools/packagemaker -build -proj sdk_frameworks.pmproj -p sdk_frameworks.pkg

echo Frameworks package built.

# Docs
echo Building API docs...
rm -rf sdk_docs
mkdir sdk_docs

# invoke doxygen
pushd ../../Docs/src
doxygen html.cfg
popd

cp -R ../../Docs/api sdk_docs/
# delete unnecessary files
rm -f sdk_docs/api/*.hhk
rm -f sdk_docs/api/*.map
rm -f sdk_docs/api/*.md5
cp -R ../../Docs/manual sdk_docs/
cp -R ../../Docs/licenses sdk_docs/
cp -R ../Win32/docs/ReadMe.html sdk_docs/
cp -R ../../Docs/ChangeLog.html sdk_docs/

echo API generation done.
echo Building docs package...
/Developer/Tools/packagemaker -build -proj sdk_docs.pmproj -p sdk_docs.pkg
echo Docs package done.
# do samples
echo Copying samples...
rm -rf sdk_samples
mkdir sdk_samples

cp -R ../../Mac/Samples/* sdk_samples/

# TODO - anything to fix up?

echo Samples copied.
echo Building samples package...
/Developer/Tools/packagemaker -build -proj sdk_samples.pmproj -p sdk_samples.pkg
echo Samples package built.

# Final install
echo Building final installer...
# NB can't use the alias for doing dist packages? Always fails!
# Also note - PackageManager fails when using relative paths to contained packages
# when building a distribution package. Therefore the paths in the pmproj are absolute
# but need updating to make sure they are correct
# Get current dir and escape it (NB backslashes have to be double-escaped here)
currDir=`pwd | sed -e 's/\//\\\\\\//g'`
sedCmd="s/\<string\>.*\/ogrenew\/SDK\/OSX\/sdk_frameworks\.pkg\<\/string\>/<string>$currDir\/sdk_frameworks.pkg<\/string>/"
echo $sedCmd > sed.cmd
sedCmd="s/\<string\>.*\/ogrenew\/SDK\/OSX\/sdk_docs\.pkg\<\/string\>/<string>$currDir\/sdk_docs.pkg<\/string>/"
echo $sedCmd >> sed.cmd
sedCmd="s/\<string\>.*\/ogrenew\/SDK\/OSX\/sdk_samples\.pkg\<\/string\>/<string>$currDir\/sdk_samples.pkg<\/string>/"
echo $sedCmd >> sed.cmd

# Need to make sure that project file is XML rather than binary
plutil -convert xml1 sdk_complete_in.pmproj
sed -f sed.cmd sdk_complete_in.pmproj > sdk_complete.pmproj
/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj sdk_complete.pmproj -p OgreSDKOSX_v1.4.4.mpkg

echo Done!
