/*****************************************************************************

	File: NatureSceneManager.cpp
	Desc: 
	Date: 2003/02/23

	Author: Martin Persson

*****************************************************************************/

#include <iostream>

#include "OgreMaterialManager.h"

#include "NatureSceneManager.h"
#include "NaturePatchManager.h"

#include "HeightmapLoader.h"

namespace Ogre
{

//----------------------------------------------------------------------------

NatureSceneManager::NatureSceneManager() : SceneManager()
{
    mNaturePatchManager = 0;
    mNaturePatchLoader  = 0;

    mNatureRoot = 0;
}

//----------------------------------------------------------------------------

NatureSceneManager::~NatureSceneManager()
{
    if (mNaturePatchManager != 0)
	delete mNaturePatchManager;

    if (mNaturePatchLoader != 0)
	delete mNaturePatchLoader;
}

//----------------------------------------------------------------------------

void NatureSceneManager::setWorldGeometry (const String &filename)
{
    if (!mNatureRoot)
    {
	    mNatureRoot = static_cast<SceneNode*>(getRootSceneNode()->createChild("NaturePluginRoot"));
    }

    // create an instance of a patchloader
    // INFO: change patchloaders here
    mNaturePatchLoader = new HeightmapLoader(mNatureRoot);
    mNaturePatchLoader->initialise(filename);

    // create the patchmanager
    mNaturePatchManager = new NaturePatchManager();
    mNaturePatchManager->initialise(mNatureRoot, mNaturePatchLoader);
}

//----------------------------------------------------------------------------

bool NatureSceneManager::setOption(const String& strKey, const void *pValue)
{
    if (strKey == "TARGET_QUALITY")
    {
	Real value = *(reinterpret_cast<const Real *>(pValue));

	printf("setOption(\"%s\"): ", strKey.c_str());
	printf("%f\n", value);

	if (mNaturePatchManager != 0)
	    mNaturePatchManager->setTargetQuality(value);

	return true;
    }

    if (strKey == "MINIMUM_QUALITY")
    {
	Real value = *(reinterpret_cast<const Real *>(pValue));

	printf("setOption(\"%s\"): ", strKey.c_str());
	printf("%f\n", value);

	if (mNaturePatchManager != 0)
	    mNaturePatchManager->setMinimumQuality(value);

	return true;
    }

    return false;
}

//----------------------------------------------------------------------------

void NatureSceneManager::_renderVisibleObjects()
{
    if (mNaturePatchManager != 0)
	mNaturePatchManager->renderPatches();

    // Run the overloaded method
    SceneManager::_renderVisibleObjects();
}

//----------------------------------------------------------------------------

void NatureSceneManager::_updateSceneGraph(Camera *cam)
{
    if (mNaturePatchManager != 0)
	mNaturePatchManager->updatePatches(cam);

    // Run the overloaded method
    SceneManager::_updateSceneGraph(cam);
}

//----------------------------------------------------------------------------

} // namespace Ogre

