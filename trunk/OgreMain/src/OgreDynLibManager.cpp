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
#include "OgreStableHeaders.h"

#include "OgreDynLibManager.h"

#include "OgreDynLib.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    template<> DynLibManager* Singleton<DynLibManager>::ms_Singleton = 0;
    DynLibManager* DynLibManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    DynLibManager& DynLibManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-----------------------------------------------------------------------

	DynLibManager::DynLibManager()
	{
	}

    DynLib* DynLibManager::load( const String& filename, int priority /* = 1 */ )
    {        
        DynLib* pLib = static_cast<DynLib *>( getByName( filename ) );

        if( !pLib )
        {
            pLib = static_cast<DynLib *>( create( filename ) );
            ResourceManager::load(pLib, priority);
        }
        
        return pLib;
    }

    Resource* DynLibManager::create( const String& name )
    {
        return new DynLib( name );
    }

    DynLibManager::~DynLibManager()
    {
        // Unload & delete resources in turn
        for( ResourceMap::iterator it = mResources.begin(); it != mResources.end(); ++it )
        {
            it->second->unload();
            delete it->second;
        }

        // Empty the list
        mResources.clear();
    }
}
