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
#ifndef __SubMesh_H_
#define __SubMesh_H_

#include "OgrePrerequisites.h"

#include "OgreMaterial.h"
#include "OgreRenderOperation.h"
#include "OgreVertexBoneAssignment.h"
#include "OgreProgressiveMesh.h"

namespace Ogre {

    /** Defines a part of a complete mesh.
        @remarks
            Meshes which make up the definition of a discrete 3D object
            are made up of potentially multiple parts. This is because
            different parts of the mesh may use different materials or
            use different vertex formats, such that a rendering state
            change is required between them.
        @par
            Like the Mesh class, instatiations of 3D objects in the scene
            share the SubMesh instances, and have the option of overriding
            their material differences on a per-object basis if required.
            See the SubEntity class for more information.
    */
    class _OgreExport SubMesh
    {
        friend class Mesh;
        friend class MeshSerializer;
    public:
        SubMesh();
        ~SubMesh();

        /** The render operation this SubMesh will issue.
        @remarks
            Note that in previous versions, the SubMesh had an optional
            facility to share vertex data with other SubMeshes via the Mesh.
            This facility is still supported, because HardwareVertexBuffer
            objects can be shared between multiple SubMeshes (and even between 
            Meshes if you like) through the VertexData structure, therefore the
            specific shared / dedicated geometry feature of SubMesh is no longer
            required.
        @par
            Note that depending on how the mesh was created, the vertex buffers
            which are used to hold mesh data may not be modifiable - see HardwareBuffer
            and MeshManager for full details.
        */
        VertexData vertexData;
        /** The index data which describes the faces of this SubMesh. */
        IndexData indexData;

        ProgressiveMesh::LODFaceList mLodFaceList;

        /// Reference to parent Mesh.
        Mesh* parent;

        /// Sets the name of the Material which this SubMesh will use
        void setMaterialName(const String& matName);
        const String& getMaterialName(void) const;

        /** Returns true if a material has been assigned to the submesh, otherwise returns false.
        */
        bool isMatInitialised(void) const;

        /** Returns a RenderOperation structure required to render this mesh.
            @param
                lodIndex The index of the LOD to use. 
            @returns 
                rend Reference to the RenderOperation structure to use for rendering.
        */
        void _getRenderOperation(RenderOperation& op, ushort lodIndex = 0);

        /** Assigns a vertex to a bone with a given weight, for skeletal animation. 
        @remarks    
            This method is only valid after calling setSkeletonName.
            Since this is a one-off process there exists only 'addBoneAssignment' and
            'clearBoneAssignments' methods, no 'editBoneAssignment'. You should not need
            to modify bone assignments during rendering (only the positions of bones) and OGRE
            reserves the right to do some internal data reformatting of this information, depending
            on render system requirements.
        @par
            This method is for assigning weights to the dedicated geometry of the SubMesh. To assign
            weights to the shared Mesh geometry, see the equivalent methods on Mesh.
        */
        void addBoneAssignment(const VertexBoneAssignment& vertBoneAssign);

        /** Removes all bone assignments for this mesh. 
        @par
            This method is for assigning weights to the dedicated geometry of the SubMesh. To assign
            weights to the shared Mesh geometry, see the equivalent methods on Mesh.
        */
        void clearBoneAssignments(void);

        /// Multimap of verex bone assignments (orders by vertex index)
        typedef std::multimap<unsigned short, VertexBoneAssignment> VertexBoneAssignmentList;
        typedef MapIterator<VertexBoneAssignmentList> BoneAssignmentIterator;

        /** Gets an iterator for access all bone assignments. 
        @remarks
            Only valid if this SubMesh has dedicated geometry.
        */
        BoneAssignmentIterator getBoneAssignmentIterator(void);

        /** Clones this SubMesh and creates the clone as a child of the passed in Mesh. */
        void clone(Mesh* newParent);


    protected:

        /// Name of the material this SubMesh uses.
        String mMaterialName;

        /// Is there a material yet?
        bool mMatInitialised;

       
        VertexBoneAssignmentList mBoneAssignments;

        /// Flag indicating that bone assignments need to be recompiled
        bool mBoneAssignmentsOutOfDate;
        /** Must be called once to compile bone assignments into geometry buffer. */
        void compileBoneAssignments(void);

        /// Internal method for removing LOD data
        void removeLodLevels(void);

        /** Calculates the bounds for this SubMesh. */
        void calculateBounds(AxisAlignedBox* boxBounds, Real* sphereBoundSquaredRadius);


    };

} // namespace

#endif

