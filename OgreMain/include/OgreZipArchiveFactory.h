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
        virtual ~ZipArchiveFactory();

        String getArchiveType(void) const;       
        ArchiveEx *createObj( const String& name );
        String getType() const;
    };

}

#endif
