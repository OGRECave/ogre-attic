s/\.\.\/\.\.\/Samples\/.*\/include/include/
s/\.\.\/\.\.\/Samples\/.*\/src/src/
s/\.\.\/\.\.\/Dependencies\/include\/CEGUI/..\/Frameworks\/CEGUI.framework\/Headers/
s/\.\.\/\.\.\/Dependencies/Dependencies/
s/\.\.\/\.\.\/OgreMain\/include/..\/Frameworks\/Ogre.framework\/Headers/
s/path = \/Library\/Frameworks\/CEGUI\.framework\; sourceTree = \"<absolute>\"/path = ..\/Frameworks\/CEGUI\.framework\; sourceTree = SOURCE_ROOT/
s/path = \/Library\/Frameworks\/OgreCEGUIRenderer\.framework\; sourceTree = \"<absolute>\"/path = ..\/Frameworks\/OgreCEGUIRenderer\.framework\; sourceTree = SOURCE_ROOT/
s/path = Ogre\.framework\; sourceTree = BUILT_PRODUCTS_DIR/path = ..\/Frameworks\/Ogre.framework\; sourceTree = SOURCE_ROOT/