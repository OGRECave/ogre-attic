#include "OgreZipArchiveFactory.h"


namespace Ogre {

//-----------------------------------------------------------------------
String ZipArchiveFactory::getArchiveType(void)
{
    return "Zip";
}

//-----------------------------------------------------------------------
ArchiveEx* ZipArchiveFactory::createArchive(const String& name)
{
    return new Zip(name);
}

}
