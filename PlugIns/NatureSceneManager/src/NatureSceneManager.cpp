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
#include "OgreEntity.h"
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
	// If already instanstiated then flush existing world geometry out of memory
	flushWorldGeometry();

	if (!mNatureRoot)
    {
	    mNatureRoot = getRootSceneNode()->createChildSceneNode("NaturePluginRoot");
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

void NatureSceneManager::clearScene(void)
{
	flushWorldGeometry();

	SceneManager::clearScene();
}
//----------------------------------------------------------------------------

void NatureSceneManager::flushWorldGeometry()
{
    if (mNaturePatchManager != 0)
	{
		delete mNaturePatchManager;
		mNaturePatchManager = 0;
	}

    if (mNaturePatchLoader != 0)
	{
		delete mNaturePatchLoader;
		mNaturePatchLoader = 0;
	}
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
IntersectionSceneQuery* 
NatureSceneManager::createIntersectionQuery(unsigned long mask)
{
    
    NatureIntersectionSceneQuery* q = new NatureIntersectionSceneQuery(this);
    q->setQueryMask(mask);
    return q;
}

//----------------------------------------------------------------------------
void NatureIntersectionSceneQuery::execute(IntersectionSceneQueryListener* listener)
{
    // Do movables to movables as before
    DefaultIntersectionSceneQuery::execute(listener);
    SceneQuery::WorldFragment frag;

    // Do entities to world
	SceneManager::EntityList::const_iterator a, theEnd;
    NatureSceneManager *sceneMgr = static_cast<NatureSceneManager*>(mParentSceneMgr);
    theEnd = sceneMgr->getEntities().end();
    for (a = sceneMgr->getEntities().begin();a != theEnd; ++a)
    {
        // Apply mask 
        if ( a->second->getQueryFlags() & mQueryMask)
        {
            const AxisAlignedBox& box = a->second->getWorldBoundingBox();
            std::list<RenderOperation> opList;
            sceneMgr->mNaturePatchManager->getPatchRenderOpsInBox(box, opList);
            
            std::list<RenderOperation>::iterator i, iend;
            iend = opList.end();
            for (i = opList.begin(); i != iend; ++i)
            {
                frag.fragmentType = SceneQuery::WFT_RENDER_OPERATION;
                frag.renderOp = &(*i);
                listener->queryResult(a->second, &frag);
            }
        }

    }
}

} // namespace Ogre

