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
		vertexData = NULL;
		indexData = new IndexData();
        mMatInitialised = false;
        mBoneAssignmentsOutOfDate = false;
        operationType = RenderOperation::OT_TRIANGLE_LIST;

    }
    //-----------------------------------------------------------------------
    SubMesh::~SubMesh()
    {
        if (vertexData)
        {
            delete vertexData;
        }
		delete indexData;

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
        
		// SubMeshes always use indexes
        ro.useIndexes = true;
		if (lodIndex > 0 && static_cast< size_t >( lodIndex - 1 ) < mLodFaceList.size())
		{
			// lodIndex - 1 because we don't store full detail version in mLodFaceList
			ro.indexData = mLodFaceList[lodIndex-1];
        }
        else
        {
    		ro.indexData = indexData;
        }
		ro.operationType = operationType;
		ro.vertexData = useSharedVertices? parent->sharedVertexData : vertexData;

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
    void SubMesh::_compileBoneAssignments(void)
    {
        unsigned short maxBones = 
            parent->_rationaliseBoneAssignments(vertexData->vertexCount, mBoneAssignments);

        if (maxBones == 0)
        {
            // No bone assignments
            return;
        }

        if (parent->mUseSoftwareBlending)
        {
            parent->compileBoneAssignmentsSoftware(mBoneAssignments, maxBones, vertexData);
        }
        else
        {
            parent->compileBoneAssignmentsHardware(mBoneAssignments, maxBones, vertexData);
        }

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



}

