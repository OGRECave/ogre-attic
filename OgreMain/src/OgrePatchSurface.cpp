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

#include "OgrePatchSurface.h"

#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreException.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"

#define LEVEL_WIDTH(lvl) ((1 << (lvl+1)) + 1)

namespace Ogre {

    // TODO: make this deal with specular colours and more than 2 texture coords

    //-----------------------------------------------------------------------
    PatchSurface::PatchSurface()
    {
        mMesh = 0;
        mType = PST_BEZIER;
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
    void PatchSurface::defineSurface(String meshName, void* controlPointBuffer, 
        VertexDeclaration *declaration, size_t width, size_t height,
        PatchSurface::PatchSurfaceType pType, size_t subdivisionLevel, size_t maxSubdivisionLevel,
        VisibleSide visibleSide)
    {
        if (height == 0 || width == 0)
            return; // Do nothing - garbage

        mMeshName = meshName;
        mType = pType;
        mCtlWidth = width;
        mCtlHeight = height;
        mCtlCount = width * height;
        mControlPointBuffer = controlPointBuffer;
        mDeclaration = declaration;

        // Copy positions into Vector3 vector
        mVecCtlPoints.clear();
        const VertexElement* elem = declaration->findElementBySemantic(VES_POSITION);
        size_t vertSize = declaration->getVertexSize(0);
        const unsigned char *pVert = static_cast<const unsigned char*>(controlPointBuffer);
        Real* pReal;
        for (size_t i = 0; i < mCtlCount; ++i)
        {
            elem->baseVertexPointerToElement((void*)pVert, &pReal);
            mVecCtlPoints.push_back(Vector3(pReal));
            pVert += vertSize;
        }

        mVSide = visibleSide;

        setSubdivisionLevel(subdivisionLevel);
        setMaxSubdivisionLevel(maxSubdivisionLevel);


        mNeedsBuild = true;

    }
    //-----------------------------------------------------------------------
    void PatchSurface::build(void)
    {

        if (mVecCtlPoints.empty())
            return;

        // Set current vertex count
        mMesh->sharedVertexData->vertexCount = mMeshWidth * mMeshHeight;

        HardwareVertexBufferSharedPtr vbuf = mMesh->sharedVertexData->vertexBufferBinding->getBuffer(0);
        void* lockedBuffer = vbuf->lock(HardwareBuffer::HBL_DISCARD);

        distributeControlPoints(lockedBuffer);

        // DEBUG
        //mMesh->_dumpContents(mMesh->getName() + "_preSubdivision.log");


        
        // Subdivide the curve
        // Do u direction first, so need to step over v levels not done yet
        size_t vStep = 1 << mVLevel;
        size_t uStep = 1 << mULevel;

        size_t v, u;
        for (v = 0; v < mMeshHeight; v += vStep)
        {
            // subdivide this row in u
            subdivideCurve(lockedBuffer, v*mMeshWidth, uStep, mMeshWidth / uStep, mULevel);
        }

        // DEBUG
        //mMesh->_dumpContents(mMesh->getName() + "_postSubdivisionU.log");

        // Now subdivide in v direction, this time all the u direction points are there so no step
        for (u = 0; u < mMeshWidth; ++u)
        {
            subdivideCurve(lockedBuffer, u, vStep*mMeshWidth, mMeshHeight / vStep, mVLevel);
        }
        

        vbuf->unlock();

        // DEBUG
        //mMesh->_dumpContents(mMesh->getName() + "_postSubdivisionV.log");

        // Make triangles from mesh
        makeTriangles();

        // Set bounds based on control points
        std::vector<Vector3>::const_iterator ctli;
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
    void PatchSurface::setSubdivisionLevel(size_t level)
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
            size_t u,v;
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
        if (mVLevel > mMaxVLevel)
        {
            mVLevel = mMaxVLevel;
        }
        if (mULevel > mMaxULevel)
        {
            mULevel = mMaxULevel;
        }
        // Derive mesh width / height
        mMeshWidth  = (LEVEL_WIDTH(mULevel)-1) * ((mCtlWidth-1)/2) + 1;
        mMeshHeight = (LEVEL_WIDTH(mVLevel)-1) * ((mCtlHeight-1)/2) + 1;



        mNeedsBuild = true;
    }
    //-----------------------------------------------------------------------
    void PatchSurface::setMaxSubdivisionLevel(size_t level)
    {
        size_t maxULevel, maxVLevel;

        if (level != AUTO_LEVEL)
        {
            maxULevel = level;
            maxVLevel = level;
        }
        else
        {
            maxULevel = mULevel;
            maxVLevel = mVLevel;
        }

        mMaxULevel = maxULevel;
        mMaxVLevel = maxVLevel;
        // Derive mesh width / height
        mMaxMeshWidth  = (LEVEL_WIDTH(mMaxULevel)-1) * ((mCtlWidth-1)/2) + 1;
        mMaxMeshHeight = (LEVEL_WIDTH(mMaxVLevel)-1) * ((mCtlHeight-1)/2) + 1;
        allocateMemory();
    }
    //-----------------------------------------------------------------------
    Mesh* PatchSurface::getMesh(void)
    {
        if (mNeedsBuild)
            build();

        return mMesh;
    }
    //-----------------------------------------------------------------------
    size_t PatchSurface::findLevel(Vector3& a, Vector3& b, Vector3& c)
    {
        // Derived from work by Bart Sekura in rogl
        // Apart from I think I fixed a bug - see below
        // I also commented the code, the only thing wrong with rogl is almost no comments!!

        const size_t max_levels = 5;
        const float subdiv = 10;
        size_t level;

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

        // Allocate to the size of max level

        // Create mesh
        mMesh = MeshManager::getSingleton().createManual(mMeshName);
        mMesh->sharedVertexData = new VertexData();
        // Copy all vertex parameters
        mMesh->sharedVertexData->vertexStart = 0;
        // Vertex count will be set on build() because it depends on current level
        // NB clone the declaration because Mesh's VertexData will destroy it
        mMesh->sharedVertexData->vertexDeclaration = mDeclaration->clone();
        // Create buffer (only a single buffer)
        // Allocate enough buffer memory for maximum subdivision, not current subdivision
        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().
            createVertexBuffer(
                mDeclaration->getVertexSize(0), 
                mMaxMeshHeight * mMaxMeshWidth, // maximum size 
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY); // dynamic for changing level

        // Set binding
        mMesh->sharedVertexData->vertexBufferBinding->setBinding(0, vbuf);

        SubMesh* sm = mMesh->createSubMesh();
        // Allocate enough index data for max subdivision
        sm->indexData->indexStart = 0;
        // Index count will be set on build()
        unsigned short iterations = (mVSide == VS_BOTH ? 2 : 1);
        sm->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 
            (mMaxMeshWidth-1) * (mMaxMeshHeight-1) * 2 * iterations * 3,  
            HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

        mMesh->load();

        // Derive bounds from control points, cannot stray outside that
        Vector3 min, max;
        Real maxSquaredRadius;
        bool first = true;
        std::vector<Vector3>::iterator i, iend;
        iend = mVecCtlPoints.end();
        for (i = mVecCtlPoints.begin(); i != iend; ++i)
        {
            if (first)
            {
                min = max = *i;
                maxSquaredRadius = i->squaredLength();
            }
            else
            {
                min.makeFloor(*i);
                max.makeCeil(*i);
                maxSquaredRadius = std::max(maxSquaredRadius, i->squaredLength());
            }

        }
        mMesh->_setBounds(AxisAlignedBox(min, max));
        mMesh->_setBoundingSphereRadius(Math::Sqrt(maxSquaredRadius));



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
    void PatchSurface::distributeControlPoints(void* lockedBuffer)
    {
        // Insert original control points into expanded mesh
        size_t uStep = 1 << mULevel;
        size_t vStep = 1 << mVLevel;


        void* pSrc = mControlPointBuffer;
        size_t vertexSize = mDeclaration->getVertexSize(0);
        Real *pSrcReal, *pDestReal;
        RGBA *pSrcRGBA, *pDestRGBA;
        void* pDest;
        const VertexElement* elemPos = mDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* elemNorm = mDeclaration->findElementBySemantic(VES_NORMAL);
        const VertexElement* elemTex0 = mDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES, 0);
        const VertexElement* elemTex1 = mDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES, 1);
        const VertexElement* elemDiffuse = mDeclaration->findElementBySemantic(VES_DIFFUSE);
        for (size_t v = 0; v < mMeshHeight; v += vStep)
        {
            // set dest by v from base
            pDest = static_cast<void*>(
                static_cast<unsigned char*>(lockedBuffer) + (vertexSize * mMeshWidth * v));
            for (size_t u = 0; u < mMeshWidth; u += uStep)
            {

                // Copy Position
                elemPos->baseVertexPointerToElement(pSrc, &pSrcReal);
                elemPos->baseVertexPointerToElement(pDest, &pDestReal);
                *pDestReal++ = *pSrcReal++;
                *pDestReal++ = *pSrcReal++;
                *pDestReal++ = *pSrcReal++;

                // Copy Normals
                if (elemNorm)
                {
                    elemNorm->baseVertexPointerToElement(pSrc, &pSrcReal);
                    elemNorm->baseVertexPointerToElement(pDest, &pDestReal);
                    *pDestReal++ = *pSrcReal++;
                    *pDestReal++ = *pSrcReal++;
                    *pDestReal++ = *pSrcReal++;
                }

                // Copy Diffuse
                if (elemDiffuse)
                {
                    elemDiffuse->baseVertexPointerToElement(pSrc, &pSrcRGBA);
                    elemDiffuse->baseVertexPointerToElement(pDest, &pDestRGBA);
                    *pDestRGBA++ = *pSrcRGBA++;
                }

                // Copy texture coords
                if (elemTex0)
                {
                    elemTex0->baseVertexPointerToElement(pSrc, &pSrcReal);
                    elemTex0->baseVertexPointerToElement(pDest, &pDestReal);
                    for (size_t dim = 0; dim < VertexElement::getTypeCount(elemTex0->getType()); ++dim)
                        *pDestReal++ = *pSrcReal++;
                }
                if (elemTex1)
                {
                    elemTex1->baseVertexPointerToElement(pSrc, &pSrcReal);
                    elemTex1->baseVertexPointerToElement(pDest, &pDestReal);
                    for (size_t dim = 0; dim < VertexElement::getTypeCount(elemTex1->getType()); ++dim)
                        *pDestReal++ = *pSrcReal++;
                }

                // Increment source by one vertex
                pSrc = static_cast<void*>(
                    static_cast<unsigned char*>(pSrc) + vertexSize);
                // Increment dest by 1 vertex * uStep
                pDest = static_cast<void*>(
                    static_cast<unsigned char*>(pDest) + (vertexSize * uStep));
            } // u
        } // v

       
    }
    //-----------------------------------------------------------------------
    void PatchSurface::subdivideCurve(void* lockedBuffer, size_t startIdx, size_t stepSize, size_t numSteps, size_t iterations)
    {
        // Subdivides a curve within a sparsely populated buffer (gaps are already there to be interpolated into)
        size_t leftIdx, rightIdx, destIdx, halfStep, maxIdx;
        bool firstSegment;

        maxIdx = startIdx + (numSteps * stepSize);
        size_t step = stepSize;

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
                interpolateVertexData(lockedBuffer, leftIdx, rightIdx, destIdx);

                // If 2nd or more segment, interpolate current left between current and last mid points
                if (!firstSegment)
                {
                    interpolateVertexData(lockedBuffer, leftIdx - halfStep, leftIdx + halfStep, leftIdx);
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
        int vInc, uInc;
        
        int vCount, uCount, v, u, iterations;

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

        SubMesh* sm = mMesh->getSubMesh(0);
        // Num faces, width*height*2 (2 tris per square)
        sm->indexData->indexCount = (mMeshWidth-1) * (mMeshHeight-1) * 2 * iterations * 3;

        size_t v1, v2, v3;
        unsigned short* pIndexes = static_cast<unsigned short*>(
            sm->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

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

        sm->indexData->indexBuffer->unlock();


    }
    //-----------------------------------------------------------------------
    void PatchSurface::interpolateVertexData(void* lockedBuffer, size_t leftIdx, size_t rightIdx, size_t destIdx)
    {
        size_t vertexSize = mDeclaration->getVertexSize(0);
        const VertexElement* elemPos = mDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* elemNorm = mDeclaration->findElementBySemantic(VES_NORMAL);
        const VertexElement* elemDiffuse = mDeclaration->findElementBySemantic(VES_DIFFUSE);
        const VertexElement* elemTex0 = mDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES, 0);
        const VertexElement* elemTex1 = mDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES, 1);

        Real *pDestReal, *pLeftReal, *pRightReal;
        unsigned char *pDestChar, *pLeftChar, *pRightChar;
        unsigned char *pDest, *pLeft, *pRight;

        // Set up pointers & interpolate
        pDest = static_cast<unsigned char*>(lockedBuffer) + (vertexSize * destIdx);
        pLeft = static_cast<unsigned char*>(lockedBuffer) + (vertexSize * leftIdx);
        pRight = static_cast<unsigned char*>(lockedBuffer) + (vertexSize * rightIdx);

        // Position
        elemPos->baseVertexPointerToElement(pDest, &pDestReal);
        elemPos->baseVertexPointerToElement(pLeft, &pLeftReal);
        elemPos->baseVertexPointerToElement(pRight, &pRightReal);

        *pDestReal++ = (*pLeftReal++ + *pRightReal++) * 0.5;
        *pDestReal++ = (*pLeftReal++ + *pRightReal++) * 0.5;
        *pDestReal++ = (*pLeftReal++ + *pRightReal++) * 0.5;

        if (elemNorm)
        {
            elemNorm->baseVertexPointerToElement(pDest, &pDestReal);
            elemNorm->baseVertexPointerToElement(pLeft, &pLeftReal);
            elemNorm->baseVertexPointerToElement(pRight, &pRightReal);
            Vector3 norm;
            norm.x = (*pLeftReal++ + *pRightReal++) * 0.5;
            norm.y = (*pLeftReal++ + *pRightReal++) * 0.5;
            norm.z = (*pLeftReal++ + *pRightReal++) * 0.5;
            norm.normalise();

            *pDestReal++ = norm.x;
            *pDestReal++ = norm.y;
            *pDestReal++ = norm.z;
        }
        if (elemDiffuse)
        {
            // Blend each byte individually
            elemDiffuse->baseVertexPointerToElement(pDest, &pDestChar);
            elemDiffuse->baseVertexPointerToElement(pLeft, &pLeftChar);
            elemDiffuse->baseVertexPointerToElement(pRight, &pRightChar);
            // 4 bytes to RGBA
            *pDestChar++ = ((*pLeftChar++) + (*pRightChar++)) * 0.5;
            *pDestChar++ = ((*pLeftChar++) + (*pRightChar++)) * 0.5;
            *pDestChar++ = ((*pLeftChar++) + (*pRightChar++)) * 0.5;
            *pDestChar++ = ((*pLeftChar++) + (*pRightChar++)) * 0.5;
        }
        if (elemTex0)
        {
            elemTex0->baseVertexPointerToElement(pDest, &pDestReal);
            elemTex0->baseVertexPointerToElement(pLeft, &pLeftReal);
            elemTex0->baseVertexPointerToElement(pRight, &pRightReal);

            for (size_t dim = 0; dim < VertexElement::getTypeCount(elemTex0->getType()); ++dim)
                *pDestReal++ = ((*pLeftReal++) + (*pRightReal++)) * 0.5;
        }
        if (elemTex1)
        {
            elemTex1->baseVertexPointerToElement(pDest, &pDestReal);
            elemTex1->baseVertexPointerToElement(pLeft, &pLeftReal);
            elemTex1->baseVertexPointerToElement(pRight, &pRightReal);

            for (size_t dim = 0; dim < VertexElement::getTypeCount(elemTex1->getType()); ++dim)
                *pDestReal++ = ((*pLeftReal++) + (*pRightReal++)) * 0.5;
        }
    }

}

