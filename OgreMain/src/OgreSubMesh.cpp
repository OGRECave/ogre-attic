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
#include "OgreSubMesh.h"

#include "OgreMesh.h"
#include "OgreException.h"
#include "OgreMeshManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SubMesh::SubMesh()
    {
        mMatInitialised = false;
        mBoneAssignmentsOutOfDate = false;
    }
    //-----------------------------------------------------------------------
    SubMesh::~SubMesh()
    {
        // Destroy vertex and index data

        // TODO

		removeLodLevels();
    }

    //-----------------------------------------------------------------------
    void SubMesh::setMaterialName(const String& name)
    {
        mMaterialName = name;
        mMatInitialised = true;
    }
    //-----------------------------------------------------------------------
    const String& SubMesh::getMaterialName() const
    {
        return mMaterialName;
    }
    //-----------------------------------------------------------------------
    bool SubMesh::isMatInitialised(void) const
    {
        return mMatInitialised;

    }
    //-----------------------------------------------------------------------
    void SubMesh::_getRenderOperation(RenderOperation& ro, ushort lodIndex) 
    {
        
        /* TODO
        */
       
    }
    //-----------------------------------------------------------------------
    void SubMesh::addBoneAssignment(const VertexBoneAssignment& vertBoneAssign)
    {
        mBoneAssignments.insert(
            VertexBoneAssignmentList::value_type(vertBoneAssign.vertexIndex, vertBoneAssign));
        mBoneAssignmentsOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    void SubMesh::clearBoneAssignments(void)
    {
        mBoneAssignments.clear();
        mBoneAssignmentsOutOfDate = true;
    }

    //-----------------------------------------------------------------------
    void SubMesh::compileBoneAssignments(void)
    {
        /* TODO
        // Deallocate
        if (geometry.pBlendingWeights)
        {
            delete [] geometry.pBlendingWeights;
            geometry.pBlendingWeights = 0;
        }

        // Iterate through, finding the largest # bones per vertex
        unsigned short maxBones = 0;
        unsigned short currBones, lastVertIdx = std::numeric_limits< ushort>::max();
        VertexBoneAssignmentList::iterator i, iend;
        i = mBoneAssignments.begin();
        iend = mBoneAssignments.end();
        currBones = 0;
        for (; i != iend; ++i)
        {
            if (lastVertIdx != i->second.vertexIndex)
            {
                // change in vertex
                if (maxBones < currBones)
                    maxBones = currBones;
                currBones = 0;
            }

            currBones++;

            lastVertIdx = i->second.vertexIndex;

        }

        if (maxBones == 0)
        {
            // No bone assignments
            geometry.numBlendWeightsPerVertex = 0;
            return;
        }
        // Allocate a buffer for bone weights
        geometry.numBlendWeightsPerVertex = maxBones;
        geometry.pBlendingWeights = 
            new LegacyRenderOperation::VertexBlendData[geometry.numVertices * maxBones];

        // Assign data
        unsigned short v;
        i = mBoneAssignments.begin();
        LegacyRenderOperation::VertexBlendData *pBlend = geometry.pBlendingWeights;
        // Iterate by vertex
        for (v = 0; v < geometry.numVertices; ++v)
        {
            for (unsigned short bone = 0; bone < maxBones; ++bone)
            {
                // Do we still have data for this vertex?
                if (i->second.vertexIndex == v)
                {
                    // If so, assign
                    pBlend->matrixIndex = i->second.boneIndex;
                    pBlend->blendWeight = i->second.weight;
                    ++i;
                }
                else
                {
                    // Ran out of assignments for this vertex, use weight 0 to indicate empty
                    pBlend->blendWeight = 0;
                    pBlend->matrixIndex = 0;
                }
                ++pBlend;
            }
        }

        */

        mBoneAssignmentsOutOfDate = false;




    }
    //---------------------------------------------------------------------
    SubMesh::BoneAssignmentIterator SubMesh::getBoneAssignmentIterator(void)
    {
        return BoneAssignmentIterator(mBoneAssignments.begin(),
            mBoneAssignments.end());
    }
    //---------------------------------------------------------------------
    void SubMesh::removeLodLevels(void)
    {
        ProgressiveMesh::LODFaceList::iterator lodi, lodend;
		lodend = mLodFaceList.end();
		for (lodi = mLodFaceList.begin(); lodi != lodend; ++lodi)
		{
			delete *lodi;
		}

        mLodFaceList.clear();

    }
    //---------------------------------------------------------------------
    void SubMesh::clone(Mesh* newParent)
    {
        /* TODO
         - remember to check if SubMesh has a name, in which case use named createSubMesh
        */
    }
    //---------------------------------------------------------------------
    void SubMesh::calculateBounds(AxisAlignedBox* boxBounds, Real* sphereBoundSquaredRadius)
    {
        /* TODO
        */
    }


}

