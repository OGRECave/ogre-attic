#!/bin/bash

# determine the project file extensions to use for each SDK 
case "$1" in
"") echo "Usage: `basename $0` [VC7, VC8, CB, CB_STLP]"; exit $E_NOARGS ;;

"CBMINGW") PROJEXT=".cbp" ; RMEXT="_stlp.cbp" ;;

"CBMINGW_STLP") PROJEXT="_stlp.cbp" ; RMEXT= ;;

"VC7") PROJEXT=".vcproj" ; RMEXT="_vc8.vcproj" ;;

"VC8") PROJEXT="_vc8.vcproj" ; RMEXT= ;;
esac

echo "copying and processing sample scripts for SDK: $1"

rm -R samples/refapp
rm -R samples/scripts
rm -R samples/src
rm -R samples/include
mkdir samples/scripts
mkdir samples/refapp
mkdir samples/refapp/scripts
mkdir samples/src
mkdir samples/include

# process the project files but only do the ones required for a specific SDK
/bin/find ../../samples -iname *$PROJEXT -exec cp \{\} samples/scripts \;
cp ../../ReferenceApplication/BspCollision/scripts/*$PROJEXT samples/scripts
cp ../../ReferenceApplication/ReferenceAppLayer/scripts/*$PROJEXT samples/refapp/scripts
# only proces file deletions if RMEXT was set
if [ -n "$RMEXT" ]
then
 # remove unwanted scripts that got copied over
 rm samples/scripts/*$RMEXT
fi
rm samples/scripts/OgreGUIRenderer$PROJEXT
/bin/find samples/scripts/ -iname *$PROJEXT -exec sed -i -f altersamples.sed \{\} \;
/bin/find samples/refapp/scripts/ -iname *$PROJEXT -exec sed -i -f alterrefapp.sed \{\} \;

# Combine the include / src folders; easier to do here than in setup
/bin/find ../../samples -iname *.cpp -exec cp \{\} samples/src \;
/bin/find ../../samples -iname *.h -exec cp \{\} samples/include \;
cp ../../ReferenceApplication/BspCollision/src/*.cpp samples/src

# Copy and alter resources.cfg
cp ../../Samples/Common/bin/Release/resources.cfg samples/
sed -i -e 's/\.\.\/\.\.\/\.\.\/Media/..\/..\/media/i' samples/resources.cfg
