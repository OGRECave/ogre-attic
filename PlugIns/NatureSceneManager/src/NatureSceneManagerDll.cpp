/*****************************************************************************

	File: NatureSceneManagerDll.cpp
	Desc: 
	Date: 2003/02/23

	Author: Martin Persson

*****************************************************************************/

#include <OgreRoot.h>

#include "NatureSceneManager.h"

namespace Ogre
{

NatureSceneManager* naturePlugin;

extern "C" void dllStartPlugin(void)
{
    // Create new scene manager
    naturePlugin = new NatureSceneManager();

    // Register
    Root::getSingleton().setSceneManager(ST_EXTERIOR_FAR, naturePlugin);
}

extern "C" void dllStopPlugin(void)
{
    delete naturePlugin;
}

}
