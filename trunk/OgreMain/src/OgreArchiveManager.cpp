/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreArchiveManager.h"

#include "OgreArchiveFactory.h"
#include "OgreArchiveEx.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {
    typedef void (*createFunc)( ArchiveEx**, const String& );

    //-----------------------------------------------------------------------
    template<> ArchiveManager* Singleton<ArchiveManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    ArchiveEx* ArchiveManager::load( const String& filename, const String& archiveType, int priority /* =1 */ )
    {
        ArchiveEx* pArch = (ArchiveEx*)(getByName(filename));
        if (!pArch)
        {
            // Search factories
            ArchiveFactoryMap::iterator it = mArchFactories.find(archiveType);
            if (it == mArchFactories.end())
                // Factory not found
                Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot find an archive factory "
                    "to deal with archive of type " + archiveType, "ArchiveManager::load");

            pArch = it->second->createObj( filename );

            ResourceManager::load(pArch, priority);
        }
        return pArch;
    }

    //-----------------------------------------------------------------------
    Resource* ArchiveManager::create( const String& name ) {
        return NULL;
    }

    //-----------------------------------------------------------------------
    ArchiveManager& ArchiveManager::getSingleton(void)
    {
        return Singleton<ArchiveManager>::getSingleton();
    }

    //-----------------------------------------------------------------------
    ArchiveManager::~ArchiveManager()
    {
        // Unload & delete resources in turn
        for( ResourceMap::iterator it = mResources.begin(); it != mResources.end(); ++it )
        {
            it->second->unload();
        }

        // Empty the list
        mResources.clear();
    }
    //-----------------------------------------------------------------------
    void ArchiveManager::addArchiveFactory(ArchiveFactory* factory)
    {        
        mArchFactories.insert( ArchiveFactoryMap::value_type( factory->getType(), factory ) );
        LogManager::getSingleton().logMessage("ArchiveFactory for archive type " +     factory->getType() + " registered.");
    }

}

