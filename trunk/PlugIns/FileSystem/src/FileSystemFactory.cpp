#include "FileSystemFactory.h"
#include "FileSystem.h"

namespace Ogre {

    FileSystemFactory::~FileSystemFactory()
    {
    }

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

    String FileSystemFactory::getType()
    {
        return "FileSystem";
    }

    ArchiveEx *FileSystemFactory::createObj( int nA, ... )
    {
        va_list lst;
        va_start( lst, nA );

        String &name = va_arg( lst, String );
        return new FileSystem( name );

        va_end( lst );
    }

}
