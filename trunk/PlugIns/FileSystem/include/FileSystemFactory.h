#ifndef _FileSystemFactory_H__
#define _FileSystemFactory_H__

#include "OgreArchiveFactory.h"

namespace Ogre {
    /** Specialisation of ArchiveFactory for folders. */

    class FileSystemFactory : public ArchiveFactory
    {
    public:
        virtual ~FileSystemFactory();

        String getArchiveType(void) const;
        ArchiveEx* createArchive(const String& name);

        ArchiveEx *createObj( const String& name );
        String getType() const;        
    };

}

#endif
