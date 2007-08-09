#!/bin/bash

# TODO - invoke xcode build

# frameworks
rm -rf sdk_frameworks 
mkdir sdk_frameworks

cp -R /Library/Frameworks/Ogre.framework sdk_frameworks/
cp -R /Library/Frameworks/OgreCEGUIRenderer.framework sdk_frameworks/

# dependencies
cp -R /Library/Frameworks/Cg.framework sdk_frameworks/
cp -R /Library/Frameworks/CEGUI.framework sdk_frameworks/

/Developer/Tools/packagemaker -build -proj sdk_frameworks.pmproj

# Contents
rm -rf sdk_contents
mkdir sdk_contents

# invoke doxygen
pushd ../../Docs/src
doxygen html.cfg
popd

cp -R ../../Docs sdk_contents/

# do samples
cp -R ../../Mac/Samples sdk_contents/

# TODO - anything to fix up?

/Developer/Tools/packagemaker -build -proj sdk_contents.pmproj

# TODO - make mpkg or dmg?

