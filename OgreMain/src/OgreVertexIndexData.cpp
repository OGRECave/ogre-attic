/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2003 The OGRE Team
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
#include "OgreStableHeaders.h"
#include "OgreVertexIndexData.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreVector3.h"
#include "OgreAxisAlignedBox.h"

namespace Ogre {

    //-----------------------------------------------------------------------
	VertexData::VertexData()
	{
		vertexBufferBinding = HardwareBufferManager::getSingleton().
			createVertexBufferBinding();
		vertexDeclaration = HardwareBufferManager::getSingleton().
			createVertexDeclaration();
		vertexCount = 0;
		vertexStart = 0;
        softwareBlendInfo = NULL;

	}
    //-----------------------------------------------------------------------
	VertexData::~VertexData()
	{
		HardwareBufferManager::getSingleton().
			destroyVertexBufferBinding(vertexBufferBinding);
		HardwareBufferManager::getSingleton().destroyVertexDeclaration(vertexDeclaration);
        if(softwareBlendInfo)
        {
            delete softwareBlendInfo;
            softwareBlendInfo = NULL;
        }

	}
    //-----------------------------------------------------------------------
	VertexData* VertexData::clone(void)
	{
		VertexData* dest = new VertexData();

		// Copy vertex buffers in turn
		const VertexBufferBinding::VertexBufferBindingMap bindings = 
			this->vertexBufferBinding->getBindings();
		VertexBufferBinding::VertexBufferBindingMap::const_iterator vbi, vbend;
		vbend = bindings.end();
		for (vbi = bindings.begin(); vbi != vbend; ++vbi)
		{
			HardwareVertexBufferSharedPtr srcbuf = vbi->second;
			// create new buffer with the same settings
			HardwareVertexBufferSharedPtr dstBuf = 
				HardwareBufferManager::getSingleton().createVertexBuffer(
					srcbuf->getVertexSize(), srcbuf->getNumVertices(), srcbuf->getUsage(),
					srcbuf->isSystemMemory());

			// copy data
			dstBuf->copyData(*srcbuf, 0, 0, srcbuf->getSizeInBytes(), true);

			// Copy binding
			dest->vertexBufferBinding->setBinding(vbi->first, dstBuf);
		}

        // Basic vertex info
        dest->vertexStart = this->vertexStart;
		dest->vertexCount = this->vertexCount;
        // Copy elements
        const VertexDeclaration::VertexElementList elems = 
            this->vertexDeclaration->getElements();
        VertexDeclaration::VertexElementList::const_iterator ei, eiend;
        eiend = elems.end();
        for (ei = elems.begin(); ei != eiend; ++ei)
        {
            dest->vertexDeclaration->addElement(
                ei->getSource(),
                ei->getOffset(),
                ei->getType(),
                ei->getSemantic(),
                ei->getIndex() );
        }

        // Copy software blend info
        if(softwareBlendInfo)
        {
            dest->softwareBlendInfo = new SoftwareBlendInfo();
            dest->softwareBlendInfo->automaticBlend = softwareBlendInfo->automaticBlend;
            dest->softwareBlendInfo->numWeightsPerVertex = softwareBlendInfo->numWeightsPerVertex;
            dest->softwareBlendInfo->pBlendIndexes = 
                new unsigned char[vertexCount * softwareBlendInfo->numWeightsPerVertex];
            dest->softwareBlendInfo->pBlendWeights = 
                new Real[vertexCount * softwareBlendInfo->numWeightsPerVertex];
            // copy data
            memcpy(dest->softwareBlendInfo->pBlendIndexes, softwareBlendInfo->pBlendIndexes, 
                sizeof(unsigned char) * vertexCount * softwareBlendInfo->numWeightsPerVertex);
            memcpy(dest->softwareBlendInfo->pBlendWeights, softwareBlendInfo->pBlendWeights, 
                sizeof(Real) * vertexCount * softwareBlendInfo->numWeightsPerVertex);

            dest->softwareBlendInfo->pSrcPositions = new Real[vertexCount * 3];
            dest->softwareBlendInfo->pSrcNormals = new Real[vertexCount * 3];
            memcpy(dest->softwareBlendInfo->pSrcPositions, 
                softwareBlendInfo->pSrcPositions, sizeof(Real) * vertexCount * 3);
            memcpy(dest->softwareBlendInfo->pSrcNormals, 
                softwareBlendInfo->pSrcNormals, sizeof(Real) * vertexCount * 3);

        }
        
        return dest;
	}
    //-----------------------------------------------------------------------
    VertexData::SoftwareBlendInfo::~SoftwareBlendInfo()
    {
        if (pSrcPositions)
            delete [] pSrcPositions;
        if (pSrcNormals)
            delete [] pSrcNormals;
        if (pBlendIndexes)
            delete [] pBlendIndexes;
        if (pBlendWeights)
            delete [] pBlendWeights;
    }
    //-----------------------------------------------------------------------
    void VertexData::prepareForShadowVolume(void)
    {

        // Look for a position element
        const VertexElement* posElem = vertexDeclaration->findElementBySemantic(VES_POSITION);
        if (posElem)
        {
            HardwareVertexBufferSharedPtr vbuf = vertexBufferBinding->getBuffer(posElem->getSource());
            bool wasSharedBuffer = false;
            // Are there other elements in the buffer except for the position?
            if (vbuf->getVertexSize() > posElem->getSize())
            {
                // We need to create another buffer to contain the remaining elements
                // Most drivers don't like gaps in the declaration, and in any case it's waste
                wasSharedBuffer = true;
            }
            HardwareVertexBufferSharedPtr newPosBuffer, newRemainderBuffer;
            if (wasSharedBuffer)
            {
                newRemainderBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
                    vbuf->getVertexSize() - posElem->getSize(), vbuf->getNumVertices(), vbuf->getUsage(),
                    vbuf->hasShadowBuffer());
            }
            // Allocate new position buffer, will be FLOAT4 and 2x the size
            size_t oldVertexCount = vbuf->getNumVertices();
            size_t newVertexCount = oldVertexCount * 2;
            newPosBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
                VertexElement::getTypeSize(VET_FLOAT4), newVertexCount, vbuf->getUsage(), 
                vbuf->hasShadowBuffer());

            // Iterate over the old buffer, copying the appropriate elements and initialising the rest
            Real* pSrc;
            unsigned char *pBaseSrc = static_cast<unsigned char*>(
                vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
            // Point first destination pointer at the start of the new position buffer,
            // the other one half way along
            Real *pDest = static_cast<Real*>(newPosBuffer->lock(HardwareBuffer::HBL_DISCARD));
            Real* pDest2 = pDest + oldVertexCount * 4; // *4 because this is VET_FLOAT4

            // Figure out the previous dimensions (probably 3, but no need to assume)
            unsigned short oldPosDimensions = VertexElement::getTypeCount(posElem->getType());
            // Also, if it had 4, override the 4th
            if (oldPosDimensions > 3) oldPosDimensions = 3;

            // Precalculate any dimensions of vertex areas outside the position
            size_t prePosVertexSize, postPosVertexSize, postPosVertexOffset;
            unsigned char *pBaseDestRem = 0;
            if (wasSharedBuffer)
            {
                pBaseDestRem = static_cast<unsigned char*>(
                    newRemainderBuffer->lock(HardwareBuffer::HBL_DISCARD));
                prePosVertexSize = posElem->getOffset();
                postPosVertexOffset = prePosVertexSize + posElem->getSize();
                postPosVertexSize = vbuf->getVertexSize() - postPosVertexOffset;
                // the 2 separate bits together should be the same size as the remainder buffer vertex
                assert (newRemainderBuffer->getVertexSize() == prePosVertexSize + postPosVertexSize);
            }
            // Iterate over the vertices
            for (size_t v = 0; v < oldVertexCount; ++v)
            {
                unsigned short e;
                // Copy position, twice
                posElem->baseVertexPointerToElement(pBaseSrc, &pSrc);
                for (e = 0; e < oldPosDimensions; ++e)
                    *pDest++ = *pDest2++ = *pSrc++;
                // Fill in any missing y or z (e < 3)
                for (; e < 3; ++e)
                    *pDest++ = *pDest2++ = 0;
                // Complete w, 1.0 for the first half, 0.0 for the second (ie at infinity)
                *pDest++  = 1.0f;
                *pDest2++ = 0.0f;

                // now deal with any other elements 
                // Basically we just memcpy the vertex excluding the position
                if (wasSharedBuffer)
                {
                    if (prePosVertexSize > 0)
                        memcpy(pBaseDestRem, pBaseSrc, prePosVertexSize);
                    if (postPosVertexSize > 0)
                        memcpy(pBaseDestRem + prePosVertexSize, 
                            pBaseSrc + postPosVertexOffset, postPosVertexSize);
                    pBaseDestRem += newRemainderBuffer->getVertexSize();
                }


                pBaseSrc += vbuf->getVertexSize();

            } // next vertex

            vbuf->unlock();
            newPosBuffer->unlock();
            if (wasSharedBuffer)
                newRemainderBuffer->unlock();

            unsigned short newPosBufferSource; 
            if (wasSharedBuffer)
            {
                // Get the a new buffer binding index
                newPosBufferSource= vertexBufferBinding->getNextIndex();
                // Re-bind the old index to the remainder buffer
                vertexBufferBinding->setBinding(posElem->getSource(), newRemainderBuffer);
            }
            else
            {
                // We can just re-use the same source idex for the new position buffer
                newPosBufferSource = posElem->getSource();
            }
            // Bind the new position buffer
            vertexBufferBinding->setBinding(newPosBufferSource, newPosBuffer);

            // Now, alter the vertex declaration to change the position source
            // and the type
            // Find index of position element first
            VertexDeclaration::VertexElementList::const_iterator elemi = 
                vertexDeclaration->getElements().begin();
            unsigned short idx;
            for(idx = 0; &(*elemi) != posElem; ++elemi) 
                ++idx;

            vertexDeclaration->modifyElement(
                idx, 
                newPosBufferSource, // new source buffer
                0, // no offset now
                VET_FLOAT4, // now homogenous
                VES_POSITION);

            // Note that we don't change vertexCount, because the other buffer(s) are still the same
            // size after all

        }
    }
    //-----------------------------------------------------------------------
	IndexData::IndexData()
	{
		indexCount = 0;
		indexStart = 0;
		
	}
    //-----------------------------------------------------------------------
	IndexData::~IndexData()
	{
	}
    //-----------------------------------------------------------------------
	IndexData* IndexData::clone(void)
	{
		IndexData* dest = new IndexData();
		if (indexBuffer.get())
		{
			dest->indexBuffer = HardwareBufferManager::getSingleton().
				createIndexBuffer(indexBuffer->getType(), indexBuffer->getNumIndexes(),
				indexBuffer->getUsage(), indexBuffer->isSystemMemory());
			dest->indexBuffer->copyData(*indexBuffer, 0, 0, indexBuffer->getSizeInBytes(), true);
		}
		dest->indexCount = indexCount;
		dest->indexStart = indexStart;
		return dest;
	}

}
