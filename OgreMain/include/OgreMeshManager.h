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
#ifndef __MeshManager_H__
#define __MeshManager_H__

#include "OgrePrerequisites.h"

#include "OgreResourceManager.h"
#include "OgreSingleton.h"
#include "OgreVector3.h"
#include "OgreHardwareBuffer.h"

namespace Ogre {

    /** Handles the management of mesh resources.
        @remarks
            This class deals with the runtime management of
            mesh data; like other resource managers it handles
            the creation of resources (in this case mesh data),
            working within a fixed memory budget.
    */
    class _OgreExport MeshManager: public ResourceManager, public Singleton<MeshManager>
    {
    public:
        MeshManager();

        /** Initialises the manager, only to be called by OGRE internally. */
        void _initialise(void);

        /** Loads a mesh from a file, making it available for use.
            @note
                If the model has already been loaded, the existing instance
                will be returned.
            @remarks
                Ogre loads model files from it's own proprietary
                format called .mesh. This is because having a single file
                format is better for runtime performance, and we also have
                control over pre-processed data (such as
                collision boxes, LOD reductions etc).
			@param filename The name of the .mesh file
			@param vertexBufferUsage The usage flags with which the vertex buffer(s)
				will be created
			@param indexBufferUsage The usage flags with which the index buffer(s) created for 
				this mesh will be created with.
			@param vertexBufferSysMem If true, the vertex buffers will be created in system memory
			@param indexBufferSysMem If true, the index buffers will be created in system memory
			@param priority The priority of this mesh in the resource system
        */
        Mesh* load( const String& filename, 
			HardwareBuffer::Usage vertexBufferUsage = HardwareBuffer::HBU_STATIC, 
			HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC, 
			bool vertexBufferSysMem = false, bool indexBufferSysMem = false,
			int priority = 1);

        /** Creates a Mesh resource.
            @note
                Mainly used internally.
        */
        Resource* create( const String& name);

        /** Creates a new Mesh specifically for manual definition rather
            than loading from an object file. 
		@remarks
			Note that once you've defined your mesh, you must call Mesh::_updateBounds
			in order to define the bounding box of your mesh.
        */
        Mesh* createManual( const String& name);

        /** Creates a basic plane, by default majoring on the x/y axes facing positive Z.
            @param
                name The name to give the resulting mesh
            @param
                plane The orientation of the plane and distance from the origin
            @param
                width The width of the plane in world coordinates
            @param
                height The height of the plane in world coordinates
            @param
                xsegments The number of segements to the plane in the x direction
            @param
                ysegments The number of segements to the plane in the y direction
            @param
                normals If true, normals are created perpendicular to the plane
            @param
                numTexCoordSets The number of 2D texture coordinate sets created - by default the corners
                are created to be the corner of the texture.
            @param
                uTile The number of times the texture should be repeated in the u direction
            @param
                vTile The number of times the texture should be repeated in the v direction
            @param
                upVector The 'Up' direction of the plane.
			@param
				vertexBufferUsage The usage flag with which the vertex buffer for this plane will be created
			@param
				indexBufferUsage The usage flag with which the index buffer for this plane will be created
			@param
				vertexSystemMemory If this flag is set to true, the vertex buffer will be created in system memory,
				allowing you to read it back more efficiently than if it is in hardware
			@param
				indexSystemMemory If this flag is set to true, the vertex buffer will be created in system memory,
				allowing you to read it back more efficiently than if it is in hardware
        */
        Mesh* createPlane(
            const String& name, const Plane& plane,
            Real width, Real height,
            int xsegments = 1, int ysegments = 1,
            bool normals = true, int numTexCoordSets = 1,
            Real uTile = 1.0f, Real vTile = 1.0f, const Vector3& upVector = Vector3::UNIT_Y,
			HardwareBuffer::Usage vertexBufferUsage = HardwareBuffer::HBU_STATIC, 
			HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC,
			bool vertexSystemMemory = false, bool indexSystemMemory = false);

		/** Creates a curved plane, by default majoring on the x/y axes facing positive Z.
            @param
                name The name to give the resulting mesh
            @param
                plane The orientation of the plane and distance from the origin
            @param
                width The width of the plane in world coordinates
            @param
                height The height of the plane in world coordinates
			@param
				bow The amount of 'bow' in the curved plane.  (Could also be concidered the depth.)
            @param
                xsegments The number of segements to the plane in the x direction
            @param
                ysegments The number of segements to the plane in the y direction
            @param
                normals If true, normals are created perpendicular to the plane
            @param
                numTexCoordSets The number of 2D texture coordinate sets created - by default the corners
                are created to be the corner of the texture.
            @param
                uTile The number of times the texture should be repeated in the u direction
            @param
                vTile The number of times the texture should be repeated in the v direction
            @param
                upVector The 'Up' direction of the plane.
        */
		Mesh* createCurvedPlane( 
			const String& name, const Plane& plane, 
			Real width, Real height, Real bow = 0.5f, 
			int xsegments = 1, int ysegments = 1,
			bool normals = false, int numTexCoordSets = 1, 
			Real xTile = 1.0f, Real yTile = 1.0f, const Vector3& upVector = Vector3::UNIT_Y);

        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton implementation is in a .h file,
                which means it gets compiled into anybody who includes it. This is needed for the Singleton
                template to work, but we actually only want it compiled into the implementation of the
                class based on the Singleton, not all of them. If we don't change this, we get link errors
                when trying to use the Singleton-based class from an outside dll.
            @par
                This method just delegates to the template version anyway, but the implementation stays in this
                single compilation unit, preventing link errors.
        */
        static MeshManager& getSingleton(void);

    protected:
        /** Utility method for tesselating 2D meshes.
        */
        void tesselate2DMesh(SubMesh* pSub, int meshWidth, int meshHeight, 
			bool doubleSided = false, 
			HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC,
			bool indexSysMem = false);

        void createPrefabPlane(void);
    };


} //namespace

#endif
