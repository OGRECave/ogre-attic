/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
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
#include "OgrePatchMesh.h"

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
			@param vertexBufferShadowed If true, the vertex buffers will be shadowed by system memory 
                copies for faster read access
			@param indexBufferShadowed If true, the index buffers will be shadowed by system memory 
                copies for faster read access
			@param priority The priority of this mesh in the resource system
        */
        Mesh* load( const String& filename, 
			HardwareBuffer::Usage vertexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
			HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
			bool vertexBufferShadowed = true, bool indexBufferShadowed = true,
			int priority = 1);

        /** Creates a Mesh resource.
            @note
                Mainly used internally.
        */
        Resource* create( const String& name);

        /** Creates a new Mesh specifically for manual definition rather
            than loading from an object file. 
		@remarks
			Note that once you've defined your mesh, you must call Mesh::_setBounds and
            Mesh::_setBoundingRadius in order to define the bounds of your mesh. In previous
            versions of OGRE you could call Mesh::_updateBounds, but OGRE's support of 
            write-only vertex buffers makes this no longer appropriate.
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
				vertexShadowBuffer If this flag is set to true, the vertex buffer will be created 
				with a system memory shadow buffer,
				allowing you to read it back more efficiently than if it is in hardware
			@param
				indexShadowBuffer If this flag is set to true, the index buffer will be 
				created with a system memory shadow buffer,
				allowing you to read it back more efficiently than if it is in hardware
        */
        Mesh* createPlane(
            const String& name, const Plane& plane,
            Real width, Real height,
            int xsegments = 1, int ysegments = 1,
            bool normals = true, int numTexCoordSets = 1,
            Real uTile = 1.0f, Real vTile = 1.0f, const Vector3& upVector = Vector3::UNIT_Y,
			HardwareBuffer::Usage vertexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
			HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY,
			bool vertexShadowBuffer = true, bool indexShadowBuffer = true);

        
        /** Creates a plane, which because of it's texture coordinates looks like a curved
			surface, useful for skies in a skybox. 
            @param
                name The name to give the resulting mesh
            @param
                plane The orientation of the plane and distance from the origin
            @param
                width The width of the plane in world coordinates
            @param
                height The height of the plane in world coordinates
            @param
				curvature The curvature of the plane. Good values are
                between 2 and 65. Higher values are more curved leading to
                a smoother effect, lower values are less curved meaning
                more distortion at the horizons but a better distance effect.
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
                orientation The orientation of the overall sphere that's used to create the illusion
			@param
				vertexBufferUsage The usage flag with which the vertex buffer for this plane will be created
			@param
				indexBufferUsage The usage flag with which the index buffer for this plane will be created
			@param
				vertexShadowBuffer If this flag is set to true, the vertex buffer will be created 
				with a system memory shadow buffer,
				allowing you to read it back more efficiently than if it is in hardware
			@param
				indexShadowBuffer If this flag is set to true, the index buffer will be 
				created with a system memory shadow buffer,
				allowing you to read it back more efficiently than if it is in hardware
            @param ySegmentsToKeep The number of segments from the top of the dome
                downwards to keep. -1 keeps all of them. This can save fillrate if
                you cannot see much of the sky lower down.
        */
		Mesh* createCurvedIllusionPlane(
            const String& name, const Plane& plane,
            Real width, Real height, Real curvature,
            int xsegments = 1, int ysegments = 1,
            bool normals = true, int numTexCoordSets = 1,
            Real uTile = 1.0f, Real vTile = 1.0f, const Vector3& upVector = Vector3::UNIT_Y,
            const Quaternion& orientation = Quaternion::IDENTITY,
			HardwareBuffer::Usage vertexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
			HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY,
			bool vertexShadowBuffer = true, bool indexShadowBuffer = true, 
            int ySegmentsToKeep = -1);

		/** Creates a genuinely curved plane, by default majoring on the x/y axes facing positive Z.
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
			@param
				vertexBufferUsage The usage flag with which the vertex buffer for this plane will be created
			@param
				indexBufferUsage The usage flag with which the index buffer for this plane will be created
			@param
				vertexShadowBuffer If this flag is set to true, the vertex buffer will be created 
				with a system memory shadow buffer,
				allowing you to read it back more efficiently than if it is in hardware
			@param
				indexShadowBuffer If this flag is set to true, the index buffer will be 
				created with a system memory shadow buffer,
				allowing you to read it back more efficiently than if it is in hardware
        */
		Mesh* createCurvedPlane( 
			const String& name, const Plane& plane, 
			Real width, Real height, Real bow = 0.5f, 
			int xsegments = 1, int ysegments = 1,
			bool normals = false, int numTexCoordSets = 1, 
			Real xTile = 1.0f, Real yTile = 1.0f, const Vector3& upVector = Vector3::UNIT_Y,
			HardwareBuffer::Usage vertexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
			HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY,
			bool vertexShadowBuffer = true, bool indexShadowBuffer = true);

        /** Creates a Bezier patch based on an array of control vertices.
            @param
                name The name to give the newly created mesh. 
            @param
                controlPointBuffer A pointer to a buffer containing the vertex data which defines control points 
                of the curves rather than actual vertices. Note that you are expected to provide not
                just position information, but potentially normals and texture coordinates too. The
                format of the buffer is defined in the VertexDeclaration parameter
            @param
                decaration VertexDeclaration describing the contents of the buffer. 
                Note this declaration must _only_ draw on buffer source 0!
            @param
                width Specifies the width of the patch in control points.
            @param
                height Specifies the height of the patch in control points. 
            @param
                uMaxSubdivisionLevel,vMaxSubdivisionLevel If you want to manually set the top level of subdivision, 
                do it here, otherwise let the system decide.
            @param
                visibleSide Determines which side of the patch (or both) triangles are generated for.
            @param
                vbUsage Vertex buffer usage flags. Recommend the default since vertex buffer should be static.
            @param
                ibUsage Index buffer usage flags. Recommend the default since index buffer should 
                be dynamic to change levels but not readable.
            @param
                vbUseShadow Flag to determine if a shadow buffer is generated for the vertex buffer. See
                    HardwareBuffer for full details.
            @param
                ibUseShadow Flag to determine if a shadow buffer is generated for the index buffer. See
                    HardwareBuffer for full details.
        */
        PatchMesh* createBezierPatch(
            const String& name, void* controlPointBuffer, 
            VertexDeclaration *declaration, size_t width, size_t height,
            size_t uMaxSubdivisionLevel = PatchSurface::AUTO_LEVEL, 
            size_t vMaxSubdivisionLevel = PatchSurface::AUTO_LEVEL,
            PatchSurface::VisibleSide visibleSide = PatchSurface::VS_FRONT,
            HardwareBuffer::Usage vbUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
            HardwareBuffer::Usage ibUsage = HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY,
            bool vbUseShadow = true, bool ibUseShadow = true);
        
        /** Tells the mesh manager that all future meshes should prepare themselves for
            shadow volumes on loading.
        */
        void setPrepareAllMeshesForShadowVolumes(bool enable);
        /** Retrieves whether all Meshes should prepare themselves for shadow volumes. */
        bool getPrepareAllMeshesForShadowVolumes(void);

        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static MeshManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static MeshManager* getSingletonPtr(void);

    protected:
        /** Utility method for tesselating 2D meshes.
        */
        void tesselate2DMesh(SubMesh* pSub, int meshWidth, int meshHeight, 
			bool doubleSided = false, 
			HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY,
			bool indexSysMem = false);

        void createPrefabPlane(void);

        bool mPrepAllMeshesForShadowVolumes;
    };


} //namespace

#endif
