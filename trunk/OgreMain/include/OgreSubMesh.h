/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#ifndef __SubMesh_H_
#define __SubMesh_H_

#include "OgrePrerequisites.h"

#include "OgreGeometryData.h"
#include "OgreMaterial.h"
#include "OgreRenderOperation.h"
#include "OgreVertexBoneAssignment.h"

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


        /// Indicates if this submesh shares vertex data with other meshes or whether it has it's own vertices.
        bool useSharedVertices;

        /// Boolean indicating if the face indexes included are for a continuous triangle strip.
        bool useTriStrips;


        /** Dedicated geometry data (only valid if useSharedVertices = false).
            @remarks
                This data is completely owned by this submesh.
                When a submesh uses a very small subset of the shared geometry
                it is (processor) inefficient since the whole buffer is sent during the
                submesh rendering operation. Therefore in this case the SubMesh
                would use it's own vertex data. For other cases it is more
                memory efficient to use a shared buffer since vertices are not duplicated,
                or if hardware vertex buffers are available it can be better to
                use one set of data.
            @par
                The use of shared or non-shared buffers is determined when
                model data is converted to the OGRE .mesh format.
        */
        GeometryData geometry;

        /// Number of faces contained in this submesh.
        unsigned short numFaces;

        /// List of indices into geometry to describe faces.
        unsigned short* faceVertexIndices;

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
                rend Reference to a RenderOperation structure to populate.
        */
        void _getRenderOperation(RenderOperation& rend);

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



    protected:

        /// Name of the material this SubMesh uses.
        String mMaterialName;

        /// Is there a material yet?
        bool mMatInitialised;

        /// Multimap of verex bone assignments (orders by vertex index)
        typedef std::multimap<unsigned short, VertexBoneAssignment> VertexBoneAssignmentList;
       
        VertexBoneAssignmentList mBoneAssignments;

        /// Flag indicating that bone assignments need to be recompiled
        bool mBoneAssignmentsOutOfDate;
        /** Must be called once to compile bone assignments into geometry buffer. */
        void compileBoneAssignments(void);



    };

} // namespace

#endif



