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
        useSharedVertices = true;
        useTriStrips = false;
        faceVertexIndices = 0;
        numFaces = 0;
        geometry.hasColours = false;
        geometry.hasNormals = false;
        geometry.vertexStride = 0;
        geometry.normalStride =0;
        geometry.colourStride = 0;
        geometry.numTexCoords = 1;
        geometry.numTexCoordDimensions[0] = 2;
        geometry.numVertices = 0;
        geometry.pColours = 0;
        geometry.pNormals = 0;
        geometry.pBlendingWeights = 0;
        geometry.numBlendWeightsPerVertex = 0;

        for (int i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; ++i)
        {
            geometry.pTexCoords[i] = 0;
            geometry.texCoordStride[i] = 0;
        }
        geometry.pVertices = 0;

        mMatInitialised = false;
        mBoneAssignmentsOutOfDate = false;

    }
    //-----------------------------------------------------------------------
    SubMesh::~SubMesh()
    {
        if (geometry.pVertices)
        {
            delete[] geometry.pVertices;
            geometry.pVertices = 0;
        }
        // Deallocate individual components if they have their own buffers
        // NB Assuming that if some components use the same buffer, all do and vice versa
        if (geometry.vertexStride == 0)
        {
            if (geometry.hasColours && geometry.pColours)
            {
                delete[] geometry.pColours;
                geometry.pColours = 0;
            }
            if (geometry.hasNormals && geometry.pNormals)
            {
                delete[] geometry.pNormals;
                geometry.pNormals = 0;
            }
            for (int i = 0; i < geometry.numTexCoords; ++i)
            {
                if (geometry.pTexCoords[i])
                {
                    delete[] geometry.pTexCoords[i];
                    geometry.pTexCoords[i] = 0;
                }
            }
        }
        if (faceVertexIndices)
        {
            delete[] faceVertexIndices;
            faceVertexIndices = 0;
        }
        if (geometry.pBlendingWeights)
        {
            delete [] geometry.pBlendingWeights;
            geometry.pBlendingWeights = 0;
        }
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
        
		// SubMeshes always use indexes
        ro.useIndexes = true;
        ro.vertexOptions = 0;
        GeometryData* geom;

        if (useTriStrips)
            ro.operationType = RenderOperation::OT_TRIANGLE_STRIP;
        else
            ro.operationType = RenderOperation::OT_TRIANGLE_LIST;

        if (useSharedVertices)
        {
			// Use primary mesh geom
			geom = &(parent->sharedGeometry);
        }
        else
        {
            geom = &(geometry);
        }

        if (geom->numTexCoords > 0)
        {
            ro.vertexOptions |= RenderOperation::VO_TEXTURE_COORDS;
            ro.numTextureCoordSets = geom->numTexCoords;
            for (int tex = 0; tex < ro.numTextureCoordSets; ++tex)
            {
                ro.numTextureDimensions[tex] = geom->numTexCoordDimensions[tex];
                ro.pTexCoords[tex] = geom->pTexCoords[tex];
                ro.texCoordStride[tex] = geom->texCoordStride[tex];
            }

        }

        if (geom->hasNormals)
        {
            ro.vertexOptions |= RenderOperation::VO_NORMALS;
            ro.pNormals = geom->pNormals;
        }

        if (geom->hasColours)
        {
            ro.vertexOptions |= RenderOperation::VO_DIFFUSE_COLOURS;
            ro.pDiffuseColour = geom->pColours;
        }

        ro.numVertices = geom->numVertices;
        ro.pVertices = geom->pVertices;
        ro.diffuseStride = geom->colourStride;
        ro.normalStride= geom->normalStride;
        ro.vertexStride = geom->vertexStride;

		ushort currNumFaces;
		ushort *currFaces;

		if (lodIndex > 0 && lodIndex < mLodFaceList.size())
		{
			// lodIndex - 1 because we don't store full detail version in mLodFaceList
			currNumFaces = mLodFaceList[lodIndex-1].numIndexes / 3;
			currFaces = mLodFaceList[lodIndex-1].pIndexes;
		}
		else
		{
			// Full detail
			currNumFaces = numFaces;
			currFaces = faceVertexIndices;
		}
		if (useTriStrips)
            ro.numIndexes = currNumFaces + 2;
        else
            ro.numIndexes = currNumFaces * 3;

        ro.pIndexes = currFaces;

        if (geom->numBlendWeightsPerVertex > 0)
        {
            ro.vertexOptions |= RenderOperation::VO_BLEND_WEIGHTS;
            ro.numBlendWeightsPerVertex = geom->numBlendWeightsPerVertex;
            ro.pBlendingWeights = geom->pBlendingWeights;
        }
    }
    //-----------------------------------------------------------------------
    void SubMesh::addBoneAssignment(const VertexBoneAssignment& vertBoneAssign)
    {
        if (useSharedVertices)
        {
            Except(Exception::ERR_INVALIDPARAMS, "This SubMesh uses shared geometry,  you "
                "must assign bones to the Mesh, not the SubMesh", "SubMesh.addBoneAssignment");
        }
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
            new RenderOperation::VertexBlendData[geometry.numVertices * maxBones];

        // Assign data
        unsigned short v;
        i = mBoneAssignments.begin();
        RenderOperation::VertexBlendData *pBlend = geometry.pBlendingWeights;
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

        mBoneAssignmentsOutOfDate = false;


    }
    //---------------------------------------------------------------------
    SubMesh::BoneAssignmentIterator SubMesh::getBoneAssignmentIterator(void)
    {
        return BoneAssignmentIterator(mBoneAssignments.begin(),
            mBoneAssignments.end());
    }



}

