/*****************************************************************************

	File: NatureCustomPatch.cpp
	Desc: This class will eventually be able to render custom meshes
	      as a terrain patch (think cave openings and other things
	      heightmap data can't represent)
	Date: 2003/02/22

	Author: Martin Persson

*****************************************************************************/

#ifndef __NATURECUSTOMPATCH_H
#define __NATURECUSTOMPATCH_H

#include <OgrePrerequisites.h>
#include <OgreSimpleRenderable.h>
#include <OgreMaterialManager.h>

#include "NatureConfig.h"
#include "NaturePatch.h"

namespace Ogre
{

class NatureCustomPatch : public NaturePatch
{
public:
    NatureCustomPatch();

    ~NatureCustomPatch();

    void prepareMesh();

    void generateMesh();

private:
};

} // namespace Ogre

#endif
