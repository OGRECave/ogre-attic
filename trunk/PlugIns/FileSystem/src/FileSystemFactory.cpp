#include "FileSystemFactory.h"
#include "FileSystem.h"

namespace Ogre {

    FileSystemFactory::~FileSystemFactory()
    {
    }

    //-----------------------------------------------------------------------
    String FileSystemFactory::getArchiveType(void) const
    {
        return "FileSystem";
    }

    //-----------------------------------------------------------------------
    ArchiveEx* FileSystemFactory::createArchive(const String& name)
    {
        return new FileSystem(name);
    }

    String FileSystemFactory::getType() const
    {
        return "FileSystem";
    }

    ArchiveEx *FileSystemFactory::createObj( const String& name)
    {
        return new FileSystem(name);
    }

}
