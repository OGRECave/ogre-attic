/*****************************************************************************

	File: HeightmapLoader.h
	Desc: Simple patch loader that loads data from a heightmap
	Date: 2003/03/02

	Author: Martin Persson

*****************************************************************************/

#ifndef __HEIGHTMAPLOADER_H
#define __HEIGHTMAPLOADER_H

#include <OgreString.h>
#include <OgreImage.h>

#include "NaturePatchLoader.h"

namespace Ogre
{

//----------------------------------------------------------------------------

class HeightmapLoader : public NaturePatchLoader
{
public:
    HeightmapLoader(SceneNode *sceneMgr);

    ~HeightmapLoader();

    bool initialise(const String& filename);

    /// loads patch data from the heightmap and returns it, 0 on error
    NaturePatch::NaturePatchData *requestData(int x, int y,
	Vector3 *worldPos, Vector3 *zonePos, Vector3 *scale);

    /// deletes the patch data structure
    void releaseData(NaturePatch::NaturePatchData *data);

    /// returns the size of each zone (in unscaled units)
    int getZoneSize();

    /// returns the size of the map (in unscaled units)
    int getMapSize();

    /// Gets the index of the patch at a set of world coords
    void getPatchAtPosition(const Vector3& pos, int *px, int *py);
private:
    /// loads data from a raw heightmap file
    unsigned char* loadRAWHeightmap(const char *filename,
			   	    unsigned int realSize,
				    unsigned int requestSize,
				    unsigned int xoff,
				    unsigned int yoff);

    /// holds number of patches in x direction
    int mMapSizeX;
    /// holds number of patches in y direction
    int mMapSizeY;

    Vector3 mScale;

    /// holds filename of the heightmap
    Image *mHeightMap;

    /// common material for the terrain
    Material *mMaterial;

    bool mInited;
};

//----------------------------------------------------------------------------

} // namespace Ogre

#endif
