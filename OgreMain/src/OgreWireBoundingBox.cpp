/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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
-----------------------------------------------------------------------------
*/
#include "OgreWireBoundingBox.h"

#include "OgreSimpleRenderable.h"
#include "OgreCamera.h"

namespace Ogre {

	WireBoundingBox::WireBoundingBox() {	
	}
	
	WireBoundingBox::~WireBoundingBox() {
	}

	void WireBoundingBox::setupBoundingBox(AxisAlignedBox aabb) {

		// init the vertices to the aabb
		setupBoundingBoxVertices(aabb);

		memset(mDiffuses,0xFF,sizeof(mDiffuses));

		mRendOp.numVertices = 24; 
		mRendOp.useIndexes = false; 
		mRendOp.pNormals = NULL; 
		mRendOp.pVertices = mVertexData; 
		mRendOp.pDiffuseColour = (unsigned long*)mDiffuses; 
		mRendOp.pSpecularColour = NULL; 
		mRendOp.operationType = RenderOperation::OT_LINE_LIST; 

		// only need diffuse colors, other options not needed
		mRendOp.vertexOptions = RenderOperation::VO_DIFFUSE_COLOURS; 

		// setup the bounding box of this SimpleRenderable
		setBoundingBox(aabb);

	}

	// Override this method to prevent parent transforms (rotation,translation,scale)
    void WireBoundingBox::getWorldTransforms( Matrix4* xform )
    {
		// return identity matrix to prevent parent transforms
        *xform = Matrix4::IDENTITY;
    }

	void WireBoundingBox::setupBoundingBoxVertices(AxisAlignedBox &aab) {

		Vector3 vmax = aab.getMaximum();
		Vector3 vmin = aab.getMinimum();
		
		// inflate the wire bounding box just a bit so that it will set apart from
		//      a solid object
		float maxx = vmax.x + 1.0;
		float maxy = vmax.y + 1.0;
		float maxz = vmax.z + 1.0;
		
		float minx = vmin.x - 1.0;
		float miny = vmin.y - 1.0;
		float minz = vmin.z - 1.0;
		
		int i = 0;

		// fill in the Vertex array: 12 lines with 2 endpoints each make up a box
		// line 0
		mVertexData[i++] = minx;
		mVertexData[i++] = miny;
		mVertexData[i++] = minz;
		mVertexData[i++] = maxx;
		mVertexData[i++] = miny;
		mVertexData[i++] = minz;
		// line 1
		mVertexData[i++] = minx;
		mVertexData[i++] = miny;
		mVertexData[i++] = minz;
		mVertexData[i++] = minx;
		mVertexData[i++] = miny;
		mVertexData[i++] = maxz;
		// line 2
		mVertexData[i++] = minx;
		mVertexData[i++] = miny;
		mVertexData[i++] = minz;
		mVertexData[i++] = minx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = minz;
		// line 3
		mVertexData[i++] = minx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = minz;
		mVertexData[i++] = minx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = maxz;
		// line 4
		mVertexData[i++] = minx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = minz;
		mVertexData[i++] = maxx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = minz;
		// line 5
		mVertexData[i++] = maxx;
		mVertexData[i++] = miny;
		mVertexData[i++] = minz;
		mVertexData[i++] = maxx;
		mVertexData[i++] = miny;
		mVertexData[i++] = maxz;
		// line 6
		mVertexData[i++] = maxx;
		mVertexData[i++] = miny;
		mVertexData[i++] = minz;
		mVertexData[i++] = maxx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = minz;
		// line 7
		mVertexData[i++] = minx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = maxz;
		mVertexData[i++] = maxx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = maxz;
		// line 8
		mVertexData[i++] = minx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = maxz;
		mVertexData[i++] = minx;
		mVertexData[i++] = miny;
		mVertexData[i++] = maxz;
		// line 9
		mVertexData[i++] = maxx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = minz;
		mVertexData[i++] = maxx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = maxz;
		// line 10
		mVertexData[i++] = maxx;
		mVertexData[i++] = miny;
		mVertexData[i++] = maxz;
		mVertexData[i++] = maxx;
		mVertexData[i++] = maxy;
		mVertexData[i++] = maxz;
		// line 11
		mVertexData[i++] = minx;
		mVertexData[i++] = miny;
		mVertexData[i++] = maxz;
		mVertexData[i++] = maxx;
		mVertexData[i++] = miny;
		mVertexData[i++] = maxz;
	}


	Real WireBoundingBox::getViewDepth(const Camera* cam) const
	{
		Vector3 min, max, mid, dist;
		min = mBox.getMinimum();
		max = mBox.getMaximum();
		mid = ((min - max) * 0.5) + min;
		dist = cam->getDerivedPosition() - mid;


		return dist.squaredLength();
	}



}

