/*****************************************************************************

	File: HeightmapLoader
	Desc: Simple patch loader that loads data from a heightmap
	Date: 2003/03/02

	Author: Martin Persson
	Modified by Mike Prosser

*****************************************************************************/

#include <OgreConfigFile.h>
#include <OgreMaterialManager.h>
#include <OgreStringConverter.h>
#include <OgreImage.h>
#include <OgreSceneNode.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>
#include <OgreSceneManager.h>

#include "HeightmapLoader.h"

namespace Ogre
{

//----------------------------------------------------------------------------

HeightmapLoader::HeightmapLoader(SceneNode *sceneRoot)
    : NaturePatchLoader(sceneRoot)
{
    mInited = false;

    mMapSizeX  = mMapSizeY = 0;
    mHeightMap = 0;

    // get default material
    mMaterial = reinterpret_cast<Material *>
	(MaterialManager::getSingleton().getByName("BaseWhite"));
}

//----------------------------------------------------------------------------

HeightmapLoader::~HeightmapLoader()
{
    if (mHeightMap != 0) delete mHeightMap;
}

//----------------------------------------------------------------------------

unsigned char* HeightmapLoader::loadRAWHeightmap(const char *filename,
			   			 unsigned int realSize,
						 unsigned int requestSize,
						 unsigned int xoff,
						 unsigned int yoff)
{
    unsigned char *heightMap;
    unsigned int readBytes = 0;
    unsigned int off = 0;

    FILE *fp;

    // allocate memory for the heightmap 
    if ((heightMap = new unsigned char[requestSize*requestSize]) == 0)
	return 0;

    // open the mapfile in read-only mode
    if ((fp = fopen(filename, "rb")) == 0)
	return 0;

    // seek to start offset
    fseek(fp, (yoff*realSize) + xoff, SEEK_SET);

    while (readBytes < (requestSize * requestSize))
    {
		// read 1 row of data
		fread(&heightMap[off], requestSize, 1, fp);
		off += requestSize;
		readBytes += requestSize;

		// seek to the next row
		fseek(fp, (realSize - requestSize), SEEK_CUR);
    }

    fclose(fp);
    return heightMap;
}

//----------------------------------------------------------------------------

bool HeightmapLoader::initialise(const String& filename)
{
    ConfigFile config;
    config.load(filename);

    String heightmap = config.getSetting("HeightMap");

    mHeightMap = new Image();
    mHeightMap->load(heightmap);

    mScale.x = atof(config.getSetting("Scale.x").c_str());
    mScale.y = atof(config.getSetting("Scale.y").c_str());
    mScale.z = atof(config.getSetting("Scale.z").c_str());
    if (mScale.x == 0) mScale.x = 1.0;
    if (mScale.y == 0) mScale.y = 0.25;
    if (mScale.z == 0) mScale.z = 1.0;

    int sx = mHeightMap->getWidth() / PATCH_SIZE;
    int sy = mHeightMap->getHeight() / PATCH_SIZE;
    
    // check that heightmap is of correct size
    if ((sx * PATCH_SIZE) + 1 != mHeightMap->getWidth() || 
		(sy * PATCH_SIZE) + 1 != mHeightMap->getHeight())
    {
		mMapSizeX = mMapSizeY = 0;

			String err = "Error: Invalid heightmap size : "
			+ StringConverter::toString(mHeightMap->getWidth())
			+ "," + StringConverter::toString(mHeightMap->getHeight()) 
			+ ". Should be 64*n+1, 64*n+1";
		
		std::cout << err << std::endl;
		return false;
    }

    if (mHeightMap->getFormat() != PF_L8)
    {
		std::cout << "Error: Heightmap is not a grayscale image." << std::endl;
		return false;
    }

    mMapSizeX = sx;
    mMapSizeY = sy;

#if USE_TEXTURES
    // create the material
    String texture = config.getSetting("WorldTexture");
    if (texture != "")
    {
		// get material
		mMaterial = mSceneRoot->getCreator()->getMaterial("NaturePatchMaterial");
		if (mMaterial != 0) MaterialManager::getSingleton().unload(mMaterial);

		mMaterial = mSceneRoot->getCreator()->createMaterial("NaturePatchMaterial");
	    
		TextureUnitState *layer;
		layer = mMaterial->getTechnique(0)->getPass(0)->createTextureUnitState(texture, 1);

	#if USE_NORMALS
		mMaterial->setLightingEnabled(true);
	#endif

		String detail_texture = config.getSetting("DetailTexture");
		if (detail_texture != "")
		{
			layer = mMaterial->getTechnique(0)->getPass(0)->createTextureUnitState(detail_texture, 0);
			layer->setTextureScale(0.2, 0.2);
		}
    }
#endif

    mMaterial->load();

    mInited = true;
    return true;
}

//----------------------------------------------------------------------------

NaturePatch::NaturePatchData *HeightmapLoader::requestData(int x, int y,
    Vector3 *worldPos, Vector3 *zonePos, Vector3 *scale)
{
    NaturePatch::NaturePatchData *data = 0;

    // return null if patch requested is out of bounds
    if (!mInited || x < 0 || x >= mMapSizeX || y < 0 || y >= mMapSizeY)
	return 0;

    // create and fill the patch data structure
    data = new NaturePatch::NaturePatchData;
    data->type = NaturePatch::TYPE_TERRAIN;

    data->terrain.pHeightMap     = mHeightMap->getData()
				 + (y * PATCH_SIZE * mHeightMap->getWidth())
				 + (x * PATCH_SIZE);
    data->terrain.heightMapWidth = mHeightMap->getWidth();
    data->terrain.pMaterial	 = mMaterial;
    data->terrain.numBlendMaps   = 0;

    // set world/zone position and scaling info
    zonePos->x = (x % 16) * PATCH_SIZE;
    zonePos->y = 0;
    zonePos->z = (y % 16) * PATCH_SIZE;

    worldPos->x = x * PATCH_SIZE;
    worldPos->y = 0;
    worldPos->z = y * PATCH_SIZE;

    scale->x = mScale.x;
    scale->y = mScale.y;
    scale->z = mScale.z;

    return data;
}

//----------------------------------------------------------------------------

int HeightmapLoader::getZoneSize()
{
    if (!mInited) return 0;

    return  (mHeightMap->getWidth()<(16*PATCH_SIZE))
	   ? mHeightMap->getWidth():(16*PATCH_SIZE);
}

//----------------------------------------------------------------------------

int HeightmapLoader::getMapSize()
{
    if (!mInited) return 0;

    return  mHeightMap->getWidth();
}

//----------------------------------------------------------------------------

void HeightmapLoader::releaseData(NaturePatch::NaturePatchData *data)
{
    if (!mInited) return;

    // for now just delete the data
    delete data;
}

//----------------------------------------------------------------------------

void HeightmapLoader::getPatchAtPosition(const Vector3& pos, int *px, int *py)
{
    // get patch x/y for position
    *px = pos.x / (EDGE_LENGTH * mScale.x);
    *py = pos.z / (EDGE_LENGTH * mScale.z);

}
} // namespace Ogre
