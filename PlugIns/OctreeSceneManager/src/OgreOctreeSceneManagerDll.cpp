
#include <OgreOctreeSceneManager.h>
#include <OgreTerrainSceneManager.h>
#include <OgreRoot.h>

namespace Ogre
{
OctreeSceneManager* octreePlugin;
TerrainSceneManager* terrainPlugin;

extern "C" void dllStartPlugin( void )
{
    // Create new scene manager
    octreePlugin = new OctreeSceneManager();
    terrainPlugin = new TerrainSceneManager();

    // Register
    Root::getSingleton().setSceneManager( ST_GENERIC, octreePlugin );
    Root::getSingleton().setSceneManager( ST_EXTERIOR_CLOSE, terrainPlugin );
    //Root::getSingleton().setSceneManager( ST_EXTERIOR_FAR, terrainPlugin );
}

extern "C" void dllStopPlugin( void )
{
    delete octreePlugin;
    delete terrainPlugin;
}
}
