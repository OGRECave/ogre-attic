#!/bin/bash

OGRE_VERSION="v1.4.7"
# Only build for i386, halves the size
ARCH="i386"
REMOVE_ARCH="ppc"

# invoke xcode build
xcodebuild -project ../../Mac/Ogre/Ogre.xcodeproj -alltargets -configuration Release
xcodebuild -project ../../Mac/Ogre/Ogre.xcodeproj -alltargets -configuration Debug

# frameworks
echo Copying frameworks...
mkdir sdk_contents/Dependencies

# Stuff we've built
ditto -arch $ARCH ../../Mac/build/Release/Ogre.framework sdk_contents/Dependencies/Ogre.framework

# dependencies
ditto -arch $ARCH ../../Dependencies/Cg.framework sdk_contents/Dependencies/Cg.framework
ditto -arch $ARCH ../../Dependencies/CEGUI.framework sdk_contents/Dependencies/CEGUI.framework

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
rm -f sdk_docs/api/html/*.hhk
rm -f sdk_docs/api/html/*.map
rm -f sdk_docs/api/html/*.md5
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

# Copy project location
cp -R ../../Mac/Samples/* sdk_samples/
# copy source
mkdir sdk_samples/src
mkdir sdk_samples/include

find ../../samples -iname *.cpp -exec cp \{\} sdk_samples/src \;
find ../../samples -iname *.h -exec cp \{\} sdk_samples/include \;
cp ../../ReferenceApplication/BspCollision/src/*.cpp sdk_samples/src

# Copy dependencies
mkdir sdk_contents/Dependencies/include
mkdir sdk_contents/Dependencies/lib
#mkdir sdk_contents/Dependencies/lib/Debug
mkdir sdk_contents/Dependencies/lib/Release
cp -R ../../Dependencies/include/OIS sdk_contents/Dependencies/include
#cp ../../Dependencies/lib/Debug/libois.a sdk_contents/Dependencies/lib/Debug/
cp ../../Dependencies/lib/Release/libois.a sdk_contents/Dependencies/lib/Release/

# Fix up project references (2 stage rather than in-place since in-place only works for single sed commands)
sed -f editsamples.sed sdk_contents/Samples/Samples.xcodeproj/project.pbxproj > tmp.xcodeproj
mv tmp.xcodeproj sdk_contents/Samples/Samples.xcodeproj/project.pbxproj
# Fix up architecture
sed -i -e "s/$REMOVE_ARCH,//g" sdk_contents/Samples/Samples.xcodeproj/project.pbxproj

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
