#include "FileSystemFactory.h"
#include "FileSystem.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    String FileSystemFactory::getArchiveType(void)
    {
        return "FileSystem";
    }

    //-----------------------------------------------------------------------
    ArchiveEx* FileSystemFactory::createArchive(const String& name)
    {
        return new FileSystem(name);
    }


}
