#!/bin/bash

OGRE_VERSION="v1.4.4"

# invoke xcode build
xcodebuild -project ../../Mac/Ogre/Ogre.xcodeproj -alltargets -configuration Release
# Just release mode, debug is too big
#xcodebuild -project ../../Mac/Ogre/Ogre.xcodeproj -alltargets -configuration Debug

rm -rf sdk_contents 
mkdir sdk_contents

# frameworks
echo Copying frameworks...
mkdir sdk_contents/Frameworks

# Stuff we've built
cp -R ../../Mac/build/Release/*.framework sdk_contents/Frameworks/

# dependencies
cp -R /Library/Frameworks/Cg.framework sdk_contents/Frameworks/
cp -R /Library/Frameworks/CEGUI.framework sdk_contents/Frameworks/
# OgreCEGUIrenderer is currently in the precompiled deps, maybe move
cp -R /Library/Frameworks/OgreCEGUIRenderer.framework sdk_contents/Frameworks/

echo Frameworks copied.

# Docs
echo Building API docs...
mkdir sdk_contents/docs

# invoke doxygen
pushd ../../Docs/src
doxygen html.cfg
popd

cp -R ../../Docs/api sdk_contents/docs/
# delete unnecessary files
rm -f sdk_contents/docs/api/html/*.hhk
rm -f sdk_contents/docs/api/html/*.map
rm -f sdk_contents/docs/api/html/*.md5
cp -R ../../Docs/manual sdk_contents/docs/
cp -R ../../Docs/licenses sdk_contents/docs/
cp -R ../Win32/docs/ReadMe.html sdk_contents/docs/
cp -R ../../Docs/ChangeLog.html sdk_contents/docs/

echo API generation done.

# do samples
echo Copying samples...
mkdir sdk_contents/Samples

# Copy project location
cp -R ../../Mac/Samples/* sdk_contents/Samples/
# copy source
mkdir sdk_contents/Samples/src
mkdir sdk_contents/Samples/include

find ../../samples -iname *.cpp -exec cp \{\} sdk_contents/Samples/src \;
find ../../samples -iname *.h -exec cp \{\} sdk_contents/Samples/include \;
cp ../../ReferenceApplication/BspCollision/src/*.cpp sdk_contents/Samples/src

# Copy dependencies
mkdir sdk_contents/Samples/Dependencies
mkdir sdk_contents/Samples/Dependencies/include
mkdir sdk_contents/Samples/Dependencies/lib
#mkdir sdk_contents/Samples/Dependencies/lib/Debug
mkdir sdk_contents/Samples/Dependencies/lib/Release
cp -R ../../Dependencies/include/OIS sdk_contents/Samples/Dependencies/include
#cp ../../Dependencies/lib/Debug/libois.a sdk_contents/Samples/Dependencies/lib/Debug/
cp ../../Dependencies/lib/Release/libois.a sdk_contents/Samples/Dependencies/lib/Release/

# Fix up project references (2 stage rather than in-place since in-place only works for single sed commands)
sed -f editsamples.sed sdk_contents/Samples/Samples.xcodeproj/project.pbxproj > tmp.xcodeproj
mv tmp.xcodeproj sdk_contents/Samples/Samples.xcodeproj/project.pbxproj

echo Samples copied.

echo Copying Media...

cp -R ../../Samples/Media sdk_contents/Samples/

# Fix up config files
sed -i -e "s/\.\.\/\.\.\/\.\.\/\.\.\/Samples/..\/..\/Samples/g" sdk_contents/samples/config/resources.cfg

echo Media copied.

#remove CVS files to avoid accidental commit of these copies!
find sdk_contents -d -iname CVS  -exec rm -rf \{\} \;

echo Building DMG...

bunzip2 -k -f template.dmg.bz2
mkdir tmp_dmg
hdiutil attach template.dmg -noautoopen -quiet -mountpoint tmp_dmg
ditto sdk_contents tmp_dmg/OgreSDK
hdiutil detach tmp_dmg
hdiutil convert -format UDBZ  -o OgreSDK_$OGRE_VERSION.dmg
rm -rf tmp_dmg
rm template.dmg

echo Done!
