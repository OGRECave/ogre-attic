
#include <OgreOctreeSceneManager.h>
#include <OgreRoot.h>

namespace Ogre
{
    OctreeSceneManager* octreePlugin;

    extern "C" void dllStartPlugin( void )
    {
        // Create new scene manager
        octreePlugin = new OctreeSceneManager();

        // Register
        Root::getSingleton().setSceneManager( ST_GENERIC, octreePlugin );
    }

    extern "C" void dllStopPlugin( void )
    {
        delete octreePlugin;
    }
}
