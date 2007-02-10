# alter the version number
# required format ./bumpversion.sh <major> <minor> <patch> <codename>

MAJOR=$1
MINOR=$2
PATCH=$3
CODENAME=$4

if [ -e $MAJOR ]
then 
	echo Major version cannot be blank
	exit 1
fi
if [ -e $MINOR ]
then 
	echo Minor version cannot be blank
	exit 1
fi
if [ -e $PATCH ]
then 
	echo Patch version cannot be blank
	exit 1
fi
if [ -e $CODENAME ]
then 
	echo Codename cannot be blank
	exit 1
fi

# configure.in
#	AC_INIT([OGRE], [1.4.0])
#	AM_INIT_AUTOMAKE([OGRE], 1.4.0)
sed -i -e "s/AC_INIT(.*$/AC_INIT([OGRE], [$MAJOR.$MINOR.$PATCH])/i" configure.in
sed -i -e "s/AC_INIT_AUTOMAKE.*$/AM_INIT_AUTOMAKE([OGRE], $MAJOR.$MINOR.$PATCH)/i" configure.in

# Docs/src/html.cfg
#	PROJECT_NUMBER         = 1.4.0
sed -i -e "s/PROJECT_NUMBER.*$/PROJECT_NUMBER         = $MAJOR.$MINOR.$PATCH/i" Docs/src/html.cfg

# Docs/src/manual.texi 
#	@settitle OGRE Manual v1.4.0 ('Eihort')
sed -i -e "s/@settitle.*$/@settitle OGRE Manual v$MAJOR.$MINOR.$PATCH ('$CODENAME')/i" Docs/src/manual.texi

# Mac/XCode/Ogre/config.h
# Mac/XCode/Samples/config.h 
#	#define PACKAGE_STRING "OGRE 1.4.0"
#	#define PACKAGE_VERSION "1.4.0"
#	#define VERSION "1.4.0"
sed -i -e "s/#define PACKAGE_STRING.*$/#define PACKAGE_STRING \"OGRE $MAJOR.$MINOR.$PATCH\"/i" Mac/XCode/Ogre/config.h
sed -i -e "s/#define PACKAGE_VERSION.*$/#define PACKAGE_VERSION \"$MAJOR.$MINOR.$PATCH\"/i" Mac/XCode/Ogre/config.h
sed -i -e "s/#define VERSION.*$/#define VERSION \"$MAJOR.$MINOR.$PATCH\"/i" Mac/XCode/Ogre/config.h
sed -i -e "s/#define PACKAGE_STRING.*$/#define PACKAGE_STRING \"OGRE $MAJOR.$MINOR.$PATCH\"/i" Mac/XCode/Samples/config.h
sed -i -e "s/#define PACKAGE_VERSION.*$/#define PACKAGE_VERSION \"$MAJOR.$MINOR.$PATCH\"/i" Mac/XCode/Samples/config.h
sed -i -e "s/#define VERSION.*$/#define VERSION \"$MAJOR.$MINOR.$PATCH\"/i" Mac/XCode/Samples/config.h


# OgreMain/include/OgrePrerequisites.h 
#	#define OGRE_VERSION_MAJOR 1
#	#define OGRE_VERSION_MINOR 4
#	#define OGRE_VERSION_PATCH 0
#	#define OGRE_VERSION_NAME "Eihort"
sed -i -e "s/define OGRE_VERSION_MAJOR.*$/define OGRE_VERSION_MAJOR $MAJOR/i" OgreMain/include/OgrePrerequisites.h
sed -i -e "s/define OGRE_VERSION_MINOR.*$/define OGRE_VERSION_MINOR $MINOR/i" OgreMain/include/OgrePrerequisites.h
sed -i -e "s/define OGRE_VERSION_PATCH.*$/define OGRE_VERSION_PATCH $PATCH/i" OgreMain/include/OgrePrerequisites.h
sed -i -e "s/define OGRE_VERSION_NAME.*$/define OGRE_VERSION_NAME \"$CODENAME\"/i" OgreMain/include/OgrePrerequisites.h

# Samples/Common/setup/demos.wxs
#	Name='OGRE Demos 1.2.5' 
#	Version='1.2.5'
sed -i -e "s/Name='OGRE Demos [0-9]*\.[0-9]*\.[0-9]*'/Name='OGRE Demos $MAJOR.$MINOR.$PATCH'/i" Samples/Common/setup/demos.wxs
sed -i -e "s/Version='[0-9]*\.[0-9]*\.[0-9]*'/Version='$MAJOR.$MINOR.$PATCH'/i" Samples/Common/setup/demos.wxs

# SDK/Win32/ogresdk.nsh
#	!define PRODUCT_VERSION "1.2.5"
sed -i -e "s/PRODUCT_VERSION \".*$/PRODUCT_VERSION \"$MAJOR.$MINOR.$PATCH\"/i" SDK/Win32/ogresdk.nsh

# Tools/Common/setup/commandlinetools.wxs
#	Version='1.2.5'
sed -i -e "s/Version='[0-9]*\.[0-9]*\.[0-9]*'/Version='$MAJOR.$MINOR.$PATCH'/i" Tools/Common/setup/commandlinetools.wxs

# Tools/MilkshapeExport/setup/milkshapeinstall.wxs
#	Version='1.2.5'
sed -i -e "s/Version='[0-9]*\.[0-9]*\.[0-9]*'/Version='$MAJOR.$MINOR.$PATCH'/i" Tools/MilkshapeExport/setup/milkshapeinstall.wxs

# Tools/XSIExport/setup/xsi5install.wxs
#	Version='1.2.5'
sed -i -e "s/Version='[0-9]*\.[0-9]*\.[0-9]*'/Version='$MAJOR.$MINOR.$PATCH'/i" Tools/XSIExport/setup/xsi5install.wxs

