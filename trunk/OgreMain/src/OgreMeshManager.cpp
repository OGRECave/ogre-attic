/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreMeshManager.h"

#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMatrix4.h"
#include "OgreMatrix3.h"
#include "OgreVector3.h"
#include "OgrePlane.h"
#include "OgreHardwareBufferManager.h"
#include "OgrePatchSurface.h"
#include "OgreException.h"

namespace Ogre
{
	#define PI 3.1415926535897932384626433832795

    //-----------------------------------------------------------------------
    template<> MeshManager* Singleton<MeshManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    MeshManager::MeshManager()
    {


    }
    //-----------------------------------------------------------------------
    void MeshManager::_initialise(void)
    {
        // Create prefab objects
        createPrefabPlane();


    }
    //-----------------------------------------------------------------------
    Resource* MeshManager::create( const String& name)
    {
        return new Mesh(name);
    }
    //-----------------------------------------------------------------------
    Mesh* MeshManager::load( const String& filename, 
		HardwareBuffer::Usage vertexBufferUsage, 
		HardwareBuffer::Usage indexBufferUsage, 
		bool vertexBufferSysMem, bool indexBufferSysMem,
		int priority)
    {
        Mesh* pMesh = (Mesh*)(getByName(filename));
        if (!pMesh)
        {
            pMesh = (Mesh*)create(filename);
			pMesh->setVertexBufferPolicy(vertexBufferUsage, vertexBufferSysMem);
			pMesh->setIndexBufferPolicy(indexBufferUsage, indexBufferSysMem);
            ResourceManager::load(pMesh, priority);
            //pMesh->_registerMaterials();
        }
        return pMesh;

    }
    //-----------------------------------------------------------------------
    Mesh* MeshManager::createManual( const String& name)
    {
        Mesh* pMesh = (Mesh*)(getByName(name));
        if (!pMesh)
        {
            pMesh = (Mesh*)create(name);
            pMesh->setManuallyDefined(true);
            ResourceManager::load(pMesh,0);
        }

        return pMesh;
    }
    //-----------------------------------------------------------------------
    Mesh* MeshManager::createPlane( const String& name, const Plane& plane, Real width, Real height, int xsegments, int ysegments,
        bool normals, int numTexCoordSets, Real xTile, Real yTile, const Vector3& upVector,
		HardwareBuffer::Usage vertexBufferUsage, HardwareBuffer::Usage indexBufferUsage,
		bool vertexShadowBuffer, bool indexShadowBuffer)
    {
        int i;
        Mesh* pMesh = createManual(name);
		SubMesh *pSub = pMesh->createSubMesh();

		// Set up vertex data
		// Use a single shared buffer
		pMesh->sharedVertexData = new VertexData();
		VertexData* vertexData = pMesh->sharedVertexData;
		// Set up Vertex Declaration
		VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
		size_t currOffset = 0;
		// We always need positions
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// Optional normals
		if(normals)
		{
			vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL);
			currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		}

        for (i = 0; i < numTexCoordSets; ++i)
        {
			// Assumes 2D texture coords
            vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, i);
			currOffset += VertexElement::getTypeSize(VET_FLOAT2);
        }

		vertexData->vertexCount = (xsegments + 1) * (ysegments + 1);

        // Allocate vertex buffer
		HardwareVertexBufferSharedPtr vbuf = 
			HardwareBufferManager::getSingleton().
			createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount,
			vertexBufferUsage, vertexShadowBuffer);

		// Set up the binding (one source only)
		VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(0, vbuf);

		// Work out the transform required
        // Default orientation of plane is normal along +z, distance 0
        Matrix4 xlate, xform, rot;
        Matrix3 rot3;
        xlate = rot = Matrix4::IDENTITY;
        // Determine axes
        Vector3 zAxis, yAxis, xAxis;
        zAxis = plane.normal;
        zAxis.normalise();
        yAxis = upVector;
        yAxis.normalise();
        xAxis = yAxis.crossProduct(zAxis);
        if (xAxis.length() == 0)
        {
            //upVector must be wrong
            Except(Exception::ERR_INVALIDPARAMS, "The upVector you supplied is parallel to the plane normal, so is not valid.",
                "MeshManager::createPlane");
        }

        rot3.FromAxes(xAxis, yAxis, zAxis);
        rot = rot3;

        // Set up standard xform from origin
        xlate.setTrans(plane.normal * -plane.d);

        // concatenate
        xform = xlate * rot;

        // Generate vertex data
		// Lock the whole buffer
		Real* pReal = static_cast<Real*>(
			vbuf->lock(HardwareBuffer::HBL_DISCARD) );
        Real xSpace = width / xsegments;
        Real ySpace = height / ysegments;
        Real halfWidth = width / 2;
        Real halfHeight = height / 2;
        Real xTex = (1.0f * xTile) / xsegments;
        Real yTex = (1.0f * yTile) / ysegments;
        Vector3 vec;
        Vector3 min, max;
        Real maxSquaredLength;
        bool firstTime = true;

        for (int y = 0; y < ysegments + 1; ++y)
        {
            for (int x = 0; x < xsegments + 1; ++x)
            {
                // Work out centered on origin
                vec.x = (x * xSpace) - halfWidth;
                vec.y = (y * ySpace) - halfHeight;
                vec.z = 0.0f;
                // Transform by orientation and distance
                vec = xform * vec;
                // Assign to geometry
                *pReal++ = vec.x;
                *pReal++ = vec.y;
                *pReal++ = vec.z;

                // Build bounds as we go
                if (firstTime)
                {
                    min = vec;
                    max = vec;
                    maxSquaredLength = vec.squaredLength();
                    firstTime = false;
                }
                else
                {
                    min.makeFloor(vec);
                    max.makeCeil(vec);
                    maxSquaredLength = std::max(maxSquaredLength, vec.squaredLength());
                }

                if (normals)
                {
                    // Default normal is along unit Z
                    vec = Vector3::UNIT_Z;
                    // Rotate
                    vec = rot * vec;

                    *pReal++ = vec.x;
                    *pReal++ = vec.y;
                    *pReal++ = vec.z;
                }

                for (i = 0; i < numTexCoordSets; ++i)
                {
                    *pReal++ = x * xTex;
                    *pReal++ = 1 - (y * yTex);
                }


            } // x
        } // y

		// Unlock
		vbuf->unlock();
        // Generate face list
        pSub->useSharedVertices = true;
        tesselate2DMesh(pSub, xsegments + 1, ysegments + 1, false, indexBufferUsage, indexShadowBuffer);

        //pMesh->_updateBounds();
        pMesh->_setBounds(AxisAlignedBox(min, max));
        pMesh->_setBoundingSphereRadius(Math::Sqrt(maxSquaredLength));
        return pMesh;
    }
	
	//-----------------------------------------------------------------------
	Mesh* MeshManager::createCurvedPlane( const String& name, const Plane& plane, Real width, Real height, Real bow, int xsegments, int ysegments,
        bool normals, int numTexCoordSets, Real xTile, Real yTile, const Vector3& upVector,
			HardwareBuffer::Usage vertexBufferUsage, HardwareBuffer::Usage indexBufferUsage,
			bool vertexShadowBuffer, bool indexShadowBuffer)
    {
        int i;
        Mesh* pMesh = createManual(name);
        SubMesh *pSub = pMesh->createSubMesh();

        // Set options
		pMesh->sharedVertexData = new VertexData();
		pMesh->sharedVertexData->vertexStart = 0;
		VertexBufferBinding* bind = pMesh->sharedVertexData->vertexBufferBinding;
		VertexDeclaration* decl = pMesh->sharedVertexData->vertexDeclaration;

        pMesh->sharedVertexData->vertexCount = (xsegments + 1) * (ysegments + 1);

		size_t offset = 0;
		decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		if (normals)
		{
			decl->addElement(0, 0, VET_FLOAT3, VES_NORMAL);
			offset += VertexElement::getTypeSize(VET_FLOAT3);
		}

        for (i = 0; i < numTexCoordSets; ++i)
        {
			decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, i);
			offset += VertexElement::getTypeSize(VET_FLOAT2);
        }


        // Allocate memory
		HardwareVertexBufferSharedPtr vbuf = 
			HardwareBufferManager::getSingleton().createVertexBuffer(
				offset, 
				pMesh->sharedVertexData->vertexCount, 
				vertexBufferUsage, 
				vertexShadowBuffer);
		bind->setBinding(0, vbuf);

		// Work out the transform required
        // Default orientation of plane is normal along +z, distance 0
        Matrix4 xlate, xform, rot;
        Matrix3 rot3;
        xlate = rot = Matrix4::IDENTITY;
        // Determine axes
        Vector3 zAxis, yAxis, xAxis;
        zAxis = plane.normal;
        zAxis.normalise();
        yAxis = upVector;
        yAxis.normalise();
        xAxis = yAxis.crossProduct(zAxis);
        if (xAxis.length() == 0)
        {
            //upVector must be wrong
            Except(Exception::ERR_INVALIDPARAMS, "The upVector you supplied is parallel to the plane normal, so is not valid.",
                "MeshManager::createPlane");
        }

        rot3.FromAxes(xAxis, yAxis, zAxis);
        rot = rot3;

        // Set up standard xform from origin
        xlate.setTrans(plane.normal * -plane.d);

        // concatenate
        xform = xlate * rot;

        // Generate vertex data
		Real* pReal = static_cast<Real*>(
			vbuf->lock(HardwareBuffer::HBL_DISCARD)); 
		Real xSpace = width / xsegments;
        Real ySpace = height / ysegments;
        Real halfWidth = width / 2;
        Real halfHeight = height / 2;
        Real xTex = (1.0f * xTile) / xsegments;
        Real yTex = (1.0f * yTile) / ysegments;
        Vector3 vec;

		Vector3 min, max;
		Real maxSqLen;
		bool first = true;

		Real diff_x, diff_y, dist;

        for (int y = 0; y < ysegments + 1; ++y)
        {
            for (int x = 0; x < xsegments + 1; ++x)
            {
                // Work out centered on origin
                vec.x = (x * xSpace) - halfWidth;
                vec.y = (y * ySpace) - halfHeight;

				// Here's where curved plane is different from standard plane.  Amazing, I know.
				diff_x = (x - ((xsegments) / 2)) / static_cast<Real>((xsegments));
				diff_y = (y - ((ysegments) / 2)) / static_cast<Real>((ysegments));
				dist = sqrt(diff_x*diff_x + diff_y * diff_y );
				vec.z = (-sin((1-dist) * (PI/2)) * bow) + bow;

                // Transform by orientation and distance
                vec = xform * vec;
                // Assign to geometry
                *pReal++ = vec.x;
                *pReal++ = vec.y;
                *pReal++ = vec.z;

				// Record bounds
				if (first)
				{
					min = max = vec;
					maxSqLen = vec.squaredLength();
					first = false;
				}
				else
				{
					min.makeFloor(vec);
					max.makeCeil(vec);
					maxSqLen = std::max(maxSqLen, vec.squaredLength());
				}
				
				if (normals)
                {
					// This part is kinda 'wrong' for curved planes... but curved planes are
					//   very valuable outside sky planes, which don't typically need normals
					//   so I'm not going to mess with it for now. 

                    // Default normal is along unit Z
                    vec = Vector3::UNIT_Z;
                    // Rotate
                    vec = rot * vec;

                    *pReal++ = vec.x;
                    *pReal++ = vec.y;
                    *pReal++ = vec.z;
                }

                for (i = 0; i < numTexCoordSets; ++i)
                {
                    *pReal++ = x * xTex;
                    *pReal++ = 1 - (y * yTex);
                }

            } // x
        } // y
		vbuf->unlock();

        // Generate face list
        tesselate2DMesh(pSub, xsegments + 1, ysegments + 1, 
			false, indexBufferUsage, indexShadowBuffer);

        pMesh->_setBounds(AxisAlignedBox(min, max));
		pMesh->_setBoundingSphereRadius(Math::Sqrt(maxSqLen));

        return pMesh;
    }
    //-----------------------------------------------------------------------
	Mesh* MeshManager::createCurvedIllusionPlane(
        const String& name, const Plane& plane,
        Real width, Real height, Real curvature,
        int xsegments, int ysegments,
        bool normals, int numTexCoordSets,
        Real uTile, Real vTile, const Vector3& upVector,
		const Quaternion& orientation, 
        HardwareBuffer::Usage vertexBufferUsage, 
		HardwareBuffer::Usage indexBufferUsage,
		bool vertexShadowBuffer, bool indexShadowBuffer)
	{
        int i;
        Mesh* pMesh = createManual(name);
		SubMesh *pSub = pMesh->createSubMesh();

		// Set up vertex data
		// Use a single shared buffer
		pMesh->sharedVertexData = new VertexData();
		VertexData* vertexData = pMesh->sharedVertexData;
		// Set up Vertex Declaration
		VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
		size_t currOffset = 0;
		// We always need positions
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// Optional normals
		if(normals)
		{
			vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL);
			currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		}

        for (i = 0; i < numTexCoordSets; ++i)
        {
			// Assumes 2D texture coords
            vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, i);
			currOffset += VertexElement::getTypeSize(VET_FLOAT2);
        }

		vertexData->vertexCount = (xsegments + 1) * (ysegments + 1);

        // Allocate vertex buffer
		HardwareVertexBufferSharedPtr vbuf = 
			HardwareBufferManager::getSingleton().
			createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount,
			vertexBufferUsage, vertexShadowBuffer);

		// Set up the binding (one source only)
		VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(0, vbuf);

		// Work out the transform required
        // Default orientation of plane is normal along +z, distance 0
        Matrix4 xlate, xform, rot;
        Matrix3 rot3;
        xlate = rot = Matrix4::IDENTITY;
        // Determine axes
        Vector3 zAxis, yAxis, xAxis;
        zAxis = plane.normal;
        zAxis.normalise();
        yAxis = upVector;
        yAxis.normalise();
        xAxis = yAxis.crossProduct(zAxis);
        if (xAxis.length() == 0)
        {
            //upVector must be wrong
            Except(Exception::ERR_INVALIDPARAMS, "The upVector you supplied is parallel to the plane normal, so is not valid.",
                "MeshManager::createPlane");
        }

        rot3.FromAxes(xAxis, yAxis, zAxis);
        rot = rot3;

        // Set up standard xform from origin
        xlate.setTrans(plane.normal * -plane.d);

        // concatenate
        xform = xlate * rot;

        // Generate vertex data
        // Imagine a large sphere with the camera located near the top
        // The lower the curvature, the larger the sphere
        // Use the angle from viewer to the points on the plane
        // Credit to Aftershock for the general approach
        Real camPos;      // Camera position relative to sphere center

        // Derive sphere radius
        Vector3 vertPos;  // position relative to camera
        Real sphDist;      // Distance from camera to sphere along box vertex vector
        // Vector3 camToSph; // camera position to sphere
        Real sphereRadius;// Sphere radius
        // Actual values irrelevant, it's the relation between sphere radius and camera position that's important
        const Real SPHERE_RAD = 100.0;
        const Real CAM_DIST = 5.0;

        sphereRadius = SPHERE_RAD - curvature;
        camPos = sphereRadius - CAM_DIST;

		// Lock the whole buffer
		Real* pReal = static_cast<Real*>(
			vbuf->lock(HardwareBuffer::HBL_DISCARD) );
        Real xSpace = width / xsegments;
        Real ySpace = height / ysegments;
        Real halfWidth = width / 2;
        Real halfHeight = height / 2;
        Vector3 vec, norm;
        Vector3 min, max;
        Real maxSquaredLength;
        bool firstTime = true;

        for (int y = 0; y < ysegments + 1; ++y)
        {
            for (int x = 0; x < xsegments + 1; ++x)
            {
                // Work out centered on origin
                vec.x = (x * xSpace) - halfWidth;
                vec.y = (y * ySpace) - halfHeight;
                vec.z = 0.0f;
                // Transform by orientation and distance
                vec = xform * vec;
                // Assign to geometry
                *pReal++ = vec.x;
                *pReal++ = vec.y;
                *pReal++ = vec.z;

                // Build bounds as we go
                if (firstTime)
                {
                    min = vec;
                    max = vec;
                    maxSquaredLength = vec.squaredLength();
                    firstTime = false;
                }
                else
                {
                    min.makeFloor(vec);
                    max.makeCeil(vec);
                    maxSquaredLength = std::max(maxSquaredLength, vec.squaredLength());
                }

                if (normals)
                {
                    // Default normal is along unit Z
                    norm = Vector3::UNIT_Z;
                    // Rotate
                    norm = orientation * norm;

                    *pReal++ = norm.x;
                    *pReal++ = norm.y;
                    *pReal++ = norm.z;
                }

				// Generate texture coords
				// Normalise position
                // modify by orientation to return +y up
                vec = orientation.Inverse() * vec;
				vec.normalise();
				// Find distance to sphere
				sphDist = Math::Sqrt(camPos*camPos * (vec.y*vec.y-1.0) + sphereRadius*sphereRadius) - camPos*vec.y;

				vec.x *= sphDist;
				vec.z *= sphDist;

				// Use x and y on sphere as texture coordinates, tiled
				Real s = vec.x * (0.01 * uTile);
				Real t = 1 - (vec.z * (0.01 * vTile));
                for (i = 0; i < numTexCoordSets; ++i)
                {
                    *pReal++ = s;
                    *pReal++ = t;
                }


            } // x
        } // y

		// Unlock
		vbuf->unlock();
        // Generate face list
        pSub->useSharedVertices = true;
        tesselate2DMesh(pSub, xsegments + 1, ysegments + 1, false, indexBufferUsage, indexShadowBuffer);

        //pMesh->_updateBounds();
        pMesh->_setBounds(AxisAlignedBox(min, max));
        pMesh->_setBoundingSphereRadius(Math::Sqrt(maxSquaredLength));
        return pMesh;
	}

    //-----------------------------------------------------------------------
    void MeshManager::tesselate2DMesh(SubMesh* sm, int meshWidth, int meshHeight, 
		bool doubleSided, HardwareBuffer::Usage indexBufferUsage, bool indexShadowBuffer)
    {
        // The mesh is built, just make a list of indexes to spit out the triangles
        int vInc, uInc, v, u, iterations;
        int vCount, uCount;

        if (doubleSided)
        {
            iterations = 2;
            vInc = 1;
            v = 0; // Start with front
        }
        else
        {
            iterations = 1;
            vInc = 1;
            v = 0;
        }

        // Allocate memory for faces
        // Num faces, width*height*2 (2 tris per square), index count is * 3 on top
        sm->indexData->indexCount = (meshWidth-1) * (meshHeight-1) * 2 * iterations * 3;
		sm->indexData->indexBuffer = HardwareBufferManager::getSingleton().
			createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
			sm->indexData->indexCount, indexBufferUsage, indexShadowBuffer);

        int v1, v2, v3;
        //bool firstTri = true;
		HardwareIndexBufferSharedPtr ibuf = sm->indexData->indexBuffer;
		// Lock the whole buffer
		unsigned short* pIndexes = static_cast<unsigned short*>(
			ibuf->lock(HardwareBuffer::HBL_DISCARD) );

        while (iterations--)
        {
            // Make tris in a zigzag pattern (compatible with strips)
            u = 0;
            uInc = 1; // Start with moving +u

            vCount = meshHeight - 1;
            while (vCount--)
            {
                uCount = meshWidth - 1;
                while (uCount--)
                {
                    // First Tri in cell
                    // -----------------
                    v1 = ((v + vInc) * meshWidth) + u;
                    v2 = (v * meshWidth) + u;
                    v3 = ((v + vInc) * meshWidth) + (u + uInc);
                    // Output indexes
                    *pIndexes++ = v1;
                    *pIndexes++ = v2;
                    *pIndexes++ = v3;
                    // Second Tri in cell
                    // ------------------
                    v1 = ((v + vInc) * meshWidth) + (u + uInc);
                    v2 = (v * meshWidth) + u;
                    v3 = (v * meshWidth) + (u + uInc);
                    // Output indexes
                    *pIndexes++ = v1;
                    *pIndexes++ = v2;
                    *pIndexes++ = v3;

                    // Next column
                    u += uInc;
                }
                // Next row
                v += vInc;
                u = 0;


            }

            // Reverse vInc for double sided
            v = meshHeight - 1;
            vInc = -vInc;

        }
		// Unlock
		ibuf->unlock();

    }

    //-----------------------------------------------------------------------
    void MeshManager::createPrefabPlane(void)
    {
        Mesh* msh = (Mesh*)create("Prefab_Plane");
        SubMesh* sub = msh->createSubMesh();
        Real vertices[32] = {
			-100, -100, 0,	// pos
			0,0,1,			// normal
			0,1,			// texcoord
            100, -100, 0,
            0,0,1,
            1,1,
            100,  100, 0,
            0,0,1,
            1,0,
            -100,  100, 0 ,
			0,0,1,
            0,0 
		};
        msh->sharedVertexData = new VertexData();
        msh->sharedVertexData->vertexCount = 4;
		VertexDeclaration* decl = msh->sharedVertexData->vertexDeclaration;
		VertexBufferBinding* bind = msh->sharedVertexData->vertexBufferBinding;

		size_t offset = 0;
		decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
		offset += VertexElement::getTypeSize(VET_FLOAT2);

		HardwareVertexBufferSharedPtr vbuf = 
			HardwareBufferManager::getSingleton().createVertexBuffer(
				offset, 4, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		bind->setBinding(0, vbuf);

		vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);

		sub->useSharedVertices = true;
		HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
			createIndexBuffer(
				HardwareIndexBuffer::IT_16BIT, 
				6, 
				HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        unsigned short faces[6] = {0,1,2,
                                   0,2,3 };
        sub->indexData->indexBuffer = ibuf;
		sub->indexData->indexCount = 6;
		sub->indexData->indexStart =0;
        ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

        msh->_setBounds(AxisAlignedBox(-100,-100,0,100,100,0));
        msh->_setBoundingSphereRadius(Math::Sqrt(100*100+100*100));

        mResources[msh->getName()] = msh;
    }
    //-----------------------------------------------------------------------
    MeshManager& MeshManager::getSingleton(void)
    {
        return Singleton<MeshManager>::getSingleton();
    }
    //-----------------------------------------------------------------------
    PatchMesh* MeshManager::createBezierPatch(const String& name, 
            void* controlPointBuffer, VertexDeclaration *declaration, 
            size_t width, size_t height,
            size_t uMaxSubdivisionLevel, size_t vMaxSubdivisionLevel,
            PatchSurface::VisibleSide visibleSide, 
            HardwareBuffer::Usage vbUsage, HardwareBuffer::Usage ibUsage,
            bool vbUseShadow, bool ibUseShadow)
    {

        PatchMesh* pMesh = (PatchMesh*)(getByName(name));
        if (pMesh)
        {
            Except(Exception::ERR_DUPLICATE_ITEM, "A mesh called " + name + 
                " already exists!", "MeshManager::createBezierPatch");
        }
        pMesh = new PatchMesh(name, controlPointBuffer, declaration, width, height,
            uMaxSubdivisionLevel, vMaxSubdivisionLevel, visibleSide, vbUsage, ibUsage,
            vbUseShadow, ibUseShadow);
        pMesh->setManuallyDefined(true);
        ResourceManager::load(pMesh,0);

        return pMesh;
    }
    //-----------------------------------------------------------------------


}
