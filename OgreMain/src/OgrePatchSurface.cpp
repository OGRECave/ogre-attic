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

#include "OgrePatchSurface.h"

#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreException.h"

#define LEVEL_WIDTH(lvl) ((1 << (lvl+1)) + 1)

namespace Ogre {


    //-----------------------------------------------------------------------
    PatchSurface::PatchSurface()
    {
        mMesh = 0;
        mType = PST_BEZIER;
        mCtlPointData.numVertices = 0;
        mMemoryAllocated = false;
    }
    //-----------------------------------------------------------------------
    PatchSurface::~PatchSurface()
    {
        /*
        if (mMesh)
        {
            delete mMesh;
            mMesh = 0;
        }
        */

    }
    //-----------------------------------------------------------------------
    void PatchSurface::defineSurface(String meshName, const GeometryData& controlPoints, int width,
        PatchSurface::PatchSurfaceType pType, int subdivisionLevel, VisibleSide visibleSide)
    {
        if (controlPoints.numVertices == 0 || width == 0)
            return; // Do nothing - garbage

        mMeshName = meshName;
        mCtlPointData = controlPoints;
        mType = pType;
        mCtlWidth = width;
        if (mCtlPointData.numVertices % mCtlWidth != 0)
        {
            Except(Exception::ERR_INVALIDPARAMS, "Invalid patch width - does not divide equally into number of control points.",
                "PatchSurface::defineSurface");
        }
        mCtlHeight = mCtlPointData.numVertices / mCtlWidth;

        // Make a copy of the control point positions as Vector3 objects
        int vertCount = mCtlPointData.numVertices;
        Real* pVerts = mCtlPointData.pVertices;
        mVecCtlPoints.clear();
        while(vertCount--)
        {
            mVecCtlPoints.push_back(Vector3(pVerts));
            pVerts = (Real*)((char*)pVerts + (sizeof(Real) * 3) + mCtlPointData.vertexStride);
        }

        setSubdivisionLevel(subdivisionLevel);

        mVSide = visibleSide;

        mNeedsBuild = true;

    }
    //-----------------------------------------------------------------------
    void PatchSurface::build(void)
    {

        if (mCtlPointData.numVertices == 0)
            return;

        allocateMemory();

        distributeControlPoints();

        // DEBUG
        //mMesh->_dumpContents(mMesh->getName() + "_preSubdivision.log");


        // Subdivide the curve
        // Do u direction first, so need to step over v levels not done yet
        int vStep = 1 << mVLevel;
        int uStep = 1 << mULevel;

        int v, u;
        for (v = 0; v < mMeshHeight; v += vStep)
        {
            // subdivide this row in u
            subdivideCurve(v*mMeshWidth, uStep, mMeshWidth / uStep, mULevel);
        }

        // DEBUG
        //mMesh->_dumpContents(mMesh->getName() + "_postSubdivisionU.log");

        // Now subdivide in v direction, this time all the u direction points are there so no step
        for (u = 0; u < mMeshWidth; ++u)
        {
            subdivideCurve(u, vStep*mMeshWidth, mMeshHeight / vStep, mVLevel);
        }

        // DEBUG
        //mMesh->_dumpContents(mMesh->getName() + "_postSubdivisionV.log");

        // Make triangles from mesh
        makeTriangles();

        // Set bounds based on control points
        std::vector<Vector3>::iterator ctli;
        Vector3 min, max;
        bool first = true;
        for (ctli = mVecCtlPoints.begin(); ctli != mVecCtlPoints.end(); ++ctli)
        {
            if (first || ctli->x < min.x)
                min.x = ctli->x;
            if (first || ctli->y < min.y)
                min.y = ctli->y;
            if (first || ctli->z < min.z)
                min.z = ctli->z;
            if (first || ctli->x > max.x)
                max.x = ctli->x;
            if (first || ctli->y > max.y)
                max.y = ctli->y;
            if (first || ctli->z > max.z)
                max.z = ctli->z;
            first = false;
        }
        mMesh->_setBounds(AxisAlignedBox(min,max));

        // Done!
        mNeedsBuild = false;

    }
    //-----------------------------------------------------------------------
    void PatchSurface::setSubdivisionLevel(int level)
    {
        if (level != AUTO_LEVEL)
        {
            mULevel = level;
            mVLevel = level;
        }
        else
        {
            // determine levels
            // Derived from work by Bart Sekura in Rogl
            Vector3 a,b,c;
            int u,v;
            bool found=false;
            // Find u level
            for(v = 0; v < mCtlHeight; v++) {
                for(u = 0; u < mCtlWidth-1; u += 2) {
                    a = mVecCtlPoints[v * mCtlWidth + u];
                    b = mVecCtlPoints[v * mCtlWidth + u+1];
                    c = mVecCtlPoints[v * mCtlWidth + u+2];
                    if(a!=c) {
                        found=true;
                        break;
                    }
                }
                if(found) break;
            }
            if(!found) {
                Except(Exception::ERR_INTERNAL_ERROR, "Can't find suitable control points for determining U subdivision level",
                    "PatchSurface::setSubdivisionLevel");
            }

            mULevel = findLevel(a,b,c);


            found=false;
            for(u = 0; u < mCtlWidth; u++) {
                for(v = 0; v < mCtlHeight-1; v += 2) {
                    a = mVecCtlPoints[v * mCtlWidth + u];
                    b = mVecCtlPoints[(v+1) * mCtlWidth + u];
                    c = mVecCtlPoints[(v+2) * mCtlWidth + u];
                    if(a!=c) {
                        found=true;
                        break;
                    }
                }
                if(found) break;
            }
            if(!found) {
                Except(Exception::ERR_INTERNAL_ERROR, "Can't find suitable control points for determining V subdivision level",
                    "PatchSurface::setSubdivisionLevel");
            }

            mVLevel = findLevel(a,b,c);


        }
        // Derive mesh width / height
        mMeshWidth  = (LEVEL_WIDTH(mULevel)-1) * ((mCtlWidth-1)/2) + 1;
        mMeshHeight = (LEVEL_WIDTH(mVLevel)-1) * ((mCtlHeight-1)/2) + 1;

        mNeedsBuild = true;
    }
    //-----------------------------------------------------------------------
    Mesh* PatchSurface::getMesh(void)
    {
        if (mNeedsBuild)
            build();

        return mMesh;
    }
    //-----------------------------------------------------------------------
    int PatchSurface::findLevel(Vector3& a, Vector3& b, Vector3& c)
    {
        // Derived from work by Bart Sekura in rogl
        // Apart from I think I fixed a bug - see below
        // I also commented the code, the only thing wrong with rogl is almost no comments!!

        const int max_levels = 5;
        const float subdiv = 10;
        int level;

        float test=subdiv*subdiv;
        Vector3 s,t,d;
        for(level=0; level<max_levels-1; level++)
        {
            // Subdivide the 2 lines
            s = a.midPoint(b);
            t = b.midPoint(c);
            // Find the midpoint between the 2 midpoints
            c = s.midPoint(t);
            // Get the vector between this subdivided midpoint and the middle point of the original line
            d = c - b;
            // Find the squared length, and break when small enough
            if(d.dotProduct(d) < test) {
                break;
            }
            b=a; // BUG?
            //b = s; // SJS fix
        }

        return level;

    }

    //-----------------------------------------------------------------------
    void PatchSurface::allocateMemory(void)
    {
        if (mMemoryAllocated)
            deallocateMemory();

        int tex;

        // Is the input geometry sharing a buffer for all components?
        if (mCtlPointData.vertexStride == 0)
            // Not shared, separate allocation buffers per vertex component
            mSharedVertexData = false;
        else
            mSharedVertexData = true;

        // Create mesh
        mMesh = MeshManager::getSingleton().createManual(mMeshName);
        // Copy all vertex parameters
        mMesh->sharedGeometry.numVertices = mMeshWidth * mMeshHeight;
        mMesh->sharedGeometry.hasColours =     mCtlPointData.hasColours;
        mMesh->sharedGeometry.hasNormals = mCtlPointData.hasNormals;
        mMesh->sharedGeometry.numTexCoords = mCtlPointData.numTexCoords;
        for ( tex = 0; tex < mCtlPointData.numTexCoords; ++tex)
        {
            mMesh->sharedGeometry.numTexCoordDimensions[tex] = mCtlPointData.numTexCoordDimensions[tex];
            mMesh->sharedGeometry.texCoordStride[tex] = mCtlPointData.texCoordStride[tex];
        }
        mMesh->sharedGeometry.normalStride = mCtlPointData.normalStride;
        mMesh->sharedGeometry.vertexStride = mCtlPointData.vertexStride;
        mMesh->sharedGeometry.colourStride = mCtlPointData.colourStride;

        /* SubMesh* sm = */ mMesh->createSubMesh();

        // Allocate geometry data
        if (mSharedVertexData)
        {
            // 1 buffer for all data
            // Assume size is vertexStride + vertex pos size
            int vertSize = sizeof(Real) * 3; // position data mandatory
            vertSize += mCtlPointData.vertexStride;
            /*
            if (mCtlPointData.hasNormals)
                vertSize += sizeof(Real) * 3;
            if (mCtlPointData.hasColours)
                vertSize += sizeof(int);
            for (tex = 0; tex < mCtlPointData.numTexCoords; ++tex)
            {
                vertSize += sizeof(Real) * mCtlPointData.numTexCoordDimensions[tex];
            }
            */

            // Allocate whole at once
            mMesh->sharedGeometry.pVertices = new Real[mMeshWidth * mMeshHeight];

            // Set other pointers relative to this
            if (mCtlPointData.hasNormals)
                mMesh->sharedGeometry.pNormals = mMesh->sharedGeometry.pVertices +
                    (mCtlPointData.pNormals - mCtlPointData.pVertices);
            if (mCtlPointData.hasColours)
                mMesh->sharedGeometry.pColours = (unsigned long*)((char*)mMesh->sharedGeometry.pVertices +
                    ((char*)mCtlPointData.pColours - (char*)mCtlPointData.pVertices));
            for (tex = 0; tex < mCtlPointData.numTexCoords; ++tex)
            {
                mMesh->sharedGeometry.pTexCoords[tex] = mMesh->sharedGeometry.pVertices +
                    (mCtlPointData.pTexCoords[tex] - mCtlPointData.pVertices);
            }


        }
        else
        {
            // Separate buffers
            mMesh->sharedGeometry.pVertices = new Real[3 * mMeshWidth * mMeshHeight];
            if (mCtlPointData.hasNormals)
                mMesh->sharedGeometry.pNormals = new Real[3 * mMeshWidth * mMeshHeight];
            if (mCtlPointData.hasColours)
                mMesh->sharedGeometry.pColours = new unsigned long[mMeshWidth * mMeshHeight];
            for (tex = 0; tex < mCtlPointData.numTexCoords; ++tex)
            {
                mMesh->sharedGeometry.pTexCoords[tex] = new Real[mCtlPointData.numTexCoordDimensions[tex] * mMeshWidth * mMeshHeight];
            }

        }

        mMesh->load();

    }
    //-----------------------------------------------------------------------
    void PatchSurface::deallocateMemory(void)
    {
        // All detailed deallocation will be done by Mesh
        MeshManager::getSingleton().unload((Resource*)mMesh);

        delete mMesh;
        mMemoryAllocated = false;

    }
    //-----------------------------------------------------------------------
    void PatchSurface::distributeControlPoints(void)
    {
        // Insert original control points into expanded mesh
        int uStep = 1 << mULevel;
        int vStep = 1 << mVLevel;
        int tex;

        // Set up the 'step' variables for indexing into the source and destination buffers
        // These are required in order to support both vertex data which all components share a
        // single buffer, AND vertex data in which each component has its own buffer
        mBufPosStep = (sizeof(Real) * 3) + mCtlPointData.vertexStride;
        mBufNormStep = (sizeof(Real) * 3) + mCtlPointData.normalStride;
        mBufColourStep = (sizeof(Real) * 3) + mCtlPointData.colourStride;
        for ( tex = 0; tex < mCtlPointData.numTexCoords; ++tex)
            mBufTexCoordStep[tex] = (sizeof(Real) * mCtlPointData.numTexCoordDimensions[tex]) + mCtlPointData.texCoordStride[tex];

        int u, v;
        int destIdx, srcIdx;
        GeomVertexPosition *pDestVert, *pSrcVert;
        GeomVertexNormal *pDestNorm, *pSrcNorm;
        Real *pDestReal, *pSrcReal;

        srcIdx = 0;
        for (v = 0; v < mMeshHeight; v += vStep)
        {
            for (u = 0; u < mMeshWidth; u += uStep)
            {
                destIdx = (v * mMeshWidth) + u;
                // Copy Position
                pDestVert = (GeomVertexPosition*)((char*)mMesh->sharedGeometry.pVertices + (destIdx * mBufPosStep));
                pSrcVert = (GeomVertexPosition*)((char*)mCtlPointData.pVertices + (srcIdx * mBufPosStep));
                *pDestVert = *pSrcVert;

                // Copy Normals
                if (mCtlPointData.hasNormals)
                {
                    pDestNorm = (GeomVertexNormal*)((char*)mMesh->sharedGeometry.pNormals + (destIdx * mBufNormStep));
                    pSrcNorm = (GeomVertexNormal*)((char*)mCtlPointData.pNormals + (srcIdx * mBufNormStep));
                    *pDestNorm = *pSrcNorm;
                }

                // Copy texture coords
                for (tex = 0; tex < mCtlPointData.numTexCoords; ++tex)
                {
                    pDestReal = (Real*)((char*)mMesh->sharedGeometry.pTexCoords[tex] + (destIdx * mBufTexCoordStep[tex]));
                    pSrcReal = (Real*)((char*)mCtlPointData.pTexCoords[tex] + (srcIdx * mBufTexCoordStep[tex]));
                    for (int dim = 0; dim < mCtlPointData.numTexCoordDimensions[tex]; ++dim)
                        *pDestReal++ = *pSrcReal++;
                }
                ++srcIdx;
            } // u
        } // v
    }
    //-----------------------------------------------------------------------
    void PatchSurface::subdivideCurve(int startIdx, int stepSize, int numSteps, int iterations)
    {
        // Subdivides a curve within a sparsely populated buffer (gaps are already there to be interpolated into)
        int leftIdx, rightIdx, destIdx, halfStep, maxIdx;
        bool firstSegment;

        maxIdx = startIdx + (numSteps * stepSize);
        int step = stepSize;

        while(iterations--)
        {
            halfStep = step / 2;
            leftIdx = startIdx;
            destIdx = leftIdx + halfStep;
            rightIdx = leftIdx + step;
            firstSegment = true;
            while (leftIdx < maxIdx)
            {
                // Interpolate
                interpolateVertexData(leftIdx, rightIdx, destIdx);

                // If 2nd or more segment, interpolate current left between current and last mid points
                if (!firstSegment)
                {
                    interpolateVertexData(leftIdx - halfStep, leftIdx + halfStep, leftIdx);
                }
                // Next segment
                leftIdx = rightIdx;
                destIdx = leftIdx + halfStep;
                rightIdx = leftIdx + step;
                firstSegment = false;
            }

            step = halfStep;
        }
    }
    //-----------------------------------------------------------------------
    void PatchSurface::makeTriangles(void)
    {
        // The mesh is built, just make a list of indexes to spit out the triangles
        int vInc, uInc, v, u, iterations;
        int vCount, uCount;

        if (mVSide == VS_BOTH)
        {
            iterations = 2;
            vInc = 1;
            v = 0; // Start with front
        }
        else
        {
            iterations = 1;
            if (mVSide == VS_FRONT)
            {
                vInc = 1;
                v = 0;
            }
            else
            {
                vInc = -1;
                v = mMeshHeight - 1;
            }
        }

        // Allocate memory for faces
        SubMesh* sm = mMesh->getSubMesh(0);
        // Num faces, width*height*2 (2 tris per square)
        sm->numFaces = (mMeshWidth-1) * (mMeshHeight-1) * 2 * iterations;
        sm->faceVertexIndices = new unsigned short[sm->numFaces * 3];

        int v1, v2, v3;
        // bool firstTri = true;
        unsigned short* pIndexes = sm->faceVertexIndices;

        while (iterations--)
        {
            // Make tris in a zigzag pattern (compatible with strips)
            u = 0;
            uInc = 1; // Start with moving +u

            vCount = mMeshHeight - 1;
            while (vCount--)
            {
                uCount = mMeshWidth - 1;
                while (uCount--)
                {
                    // First Tri in cell
                    // -----------------
                    v1 = ((v + vInc) * mMeshWidth) + u;
                    v2 = (v * mMeshWidth) + u;
                    v3 = ((v + vInc) * mMeshWidth) + (u + uInc);
                    // Output indexes
                    *pIndexes++ = v1;
                    *pIndexes++ = v2;
                    *pIndexes++ = v3;
                    // Second Tri in cell
                    // ------------------
                    v1 = ((v + vInc) * mMeshWidth) + (u + uInc);
                    v2 = (v * mMeshWidth) + u;
                    v3 = (v * mMeshWidth) + (u + uInc);
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
            v = mMeshHeight - 1;
            vInc = -vInc;

        }



    }
    //-----------------------------------------------------------------------
    void PatchSurface::interpolateVertexData(int leftIdx, int rightIdx, int destIdx)
    {
        GeomVertexPosition *pDestVert, *pLeftVert, *pRightVert;
        GeomVertexNormal *pDestNorm, *pLeftNorm, *pRightNorm;
        Real *pDestReal, *pLeftReal, *pRightReal;

        // Set up pointers & interpolate
        pDestVert = (GeomVertexPosition*)((char*)mMesh->sharedGeometry.pVertices + (destIdx * mBufPosStep));
        pLeftVert = (GeomVertexPosition*)((char*)mMesh->sharedGeometry.pVertices + (leftIdx * mBufPosStep));
        pRightVert = (GeomVertexPosition*)((char*)mMesh->sharedGeometry.pVertices + (rightIdx * mBufPosStep));
        pDestVert->x = (pLeftVert->x + pRightVert->x) * 0.5;
        pDestVert->y = (pLeftVert->y + pRightVert->y) * 0.5;
        pDestVert->z = (pLeftVert->z + pRightVert->z) * 0.5;

        if (mMesh->sharedGeometry.hasNormals)
        {
            pDestNorm = (GeomVertexNormal*)((char*)mMesh->sharedGeometry.pNormals + (destIdx * mBufNormStep));
            pLeftNorm = (GeomVertexNormal*)((char*)mMesh->sharedGeometry.pNormals + (leftIdx * mBufNormStep));
            pRightNorm = (GeomVertexNormal*)((char*)mMesh->sharedGeometry.pNormals + (rightIdx * mBufNormStep));
            pDestNorm->x = (pLeftNorm->x + pRightNorm->x) * 0.5;
            pDestNorm->y = (pLeftNorm->y + pRightNorm->y) * 0.5;
            pDestNorm->z = (pLeftNorm->z + pRightNorm->z) * 0.5;
        }
        // Skip colours for now
        for (int tex = 0; tex < mCtlPointData.numTexCoords; ++tex)
        {
            pDestReal = (Real*)((char*)mMesh->sharedGeometry.pTexCoords[tex] + (destIdx * mBufTexCoordStep[tex]));
            pLeftReal = (Real*)((char*)mMesh->sharedGeometry.pTexCoords[tex] + (leftIdx * mBufTexCoordStep[tex]));
            pRightReal = (Real*)((char*)mMesh->sharedGeometry.pTexCoords[tex] + (rightIdx * mBufTexCoordStep[tex]));
            for (int dim = 0; dim < mMesh->sharedGeometry.numTexCoordDimensions[tex]; ++dim)
                *pDestReal++ = ((*pLeftReal++) + (*pRightReal++)) * 0.5;
        }
    }

}

