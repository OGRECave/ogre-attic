/*****************************************************************************

	File: NaturePatchLoader.h
	Desc: Skeleton for patch loading classes
	Date: 2003/03/02

	Author: Martin Persson

*****************************************************************************/

#ifndef __NATUREPATCHLOADER_H
#define __NATUREPATCHLOADER_H

#include <OgreString.h>

#include "NatureConfig.h"
#include "NaturePatch.h"

namespace Ogre
{

//----------------------------------------------------------------------------

class NaturePatchLoader
{
public:
    NaturePatchLoader(SceneNode *sceneRoot)
        : mSceneRoot(sceneRoot) { }

    virtual ~NaturePatchLoader() {}

    /// Called from NatureSceneManager::setWorldGeometry()
    virtual bool initialise(const String& filename) = 0;

    /// Called by NaturePatchManager when data is needed
    virtual NaturePatch::NaturePatchData *requestData(int x, int y, 
	Vector3 *worldPos, Vector3 *zonePos, Vector3 *scale) = 0;

    /// Called by NaturePatchManager when data can be released
    virtual void releaseData(NaturePatch::NaturePatchData *data) = 0;

    /// Returns the size of each zone (in unscaled units)
    virtual int getZoneSize() = 0;

    /// Returns the size of the map (in unscaled units)
    virtual int getMapSize() = 0;

    /// Gets the index of the patch at a set of world coords
    virtual void getPatchAtPosition(const Vector3& pos, int *px, int *py) = 0;


protected:
    SceneNode *mSceneRoot;
};

//----------------------------------------------------------------------------

} // namespace Ogre

#endif
