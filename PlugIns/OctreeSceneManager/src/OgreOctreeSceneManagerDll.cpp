/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include <OgreOctreeSceneManager.h>
#include <OgreTerrainSceneManager.h>
#include <OgreRoot.h>

namespace Ogre
{
OctreeSceneManagerFactory* octreePlugin;
TerrainSceneManagerFactory* terrainPlugin;

extern "C" void _OgreOctreePluginExport dllStartPlugin( void )
{
    // Create new scene manager
    octreePlugin = new OctreeSceneManagerFactory();
    terrainPlugin = new TerrainSceneManagerFactory();
	// Construct listener manager singleton
	new TerrainPageSourceListenerManager();

    // Register
    Root::getSingleton().addSceneManagerFactory(octreePlugin);
    Root::getSingleton().addSceneManagerFactory(terrainPlugin);

}
extern "C" void _OgreOctreePluginExport dllShutdownPlugin( void )
{
	Root::getSingleton().removeSceneManagerFactory(terrainPlugin);
	Root::getSingleton().removeSceneManagerFactory(octreePlugin);
	// destroy listener manager
	delete TerrainPageSourceListenerManager::getSingletonPtr();
}

extern "C" void _OgreOctreePluginExport dllStopPlugin( void )
{
    delete octreePlugin;
    delete terrainPlugin;
}
}