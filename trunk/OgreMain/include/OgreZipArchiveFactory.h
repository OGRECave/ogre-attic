#ifndef _ZipArchiveFactory_H__
#define _ZipArchiveFactory_H__

#include "OgrePrerequisites.h"

#include "OgreArchiveFactory.h"
#include "OgreZip.h"

namespace Ogre {
    /** Specialisation of ArchiveFactory for Zip files. */

    class ZipArchiveFactory : public ArchiveFactory
    {
    public:
        String getArchiveType(void);
        ArchiveEx* createArchive(const String& name);
    };

}

#endif
