/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
    /*
	void VertexData::getBounds(AxisAlignedBox *box, Real *maxSquaredRadius)
	{
		// Find the position semantic element(s)
		const VertexElement* elem = 
			vertexDeclaration->findElementBySemantic(VES_POSITION);
		// Now find the buffer
		HardwareVertexBufferSharedPtr pBuf = 
			vertexBufferBinding->getBuffer(elem->getSource());
		// Get the data
		unsigned char* pRaw = (unsigned char*)pBuf->lock(0, pBuf->getSizeInBytes(), 
			HardwareVertexBuffer::HBL_READ_ONLY);
		pRaw += elem->getOffset();

		AxisAlignedBox localBox;
		Vector3 min, max;
		bool first = true;
		for (size_t vert = 0; vert < pBuf->getNumVertices(); ++vert)
		{
			Real* pVert = (Real*)pRaw;

			Vector3 vec(pVert[0], pVert[1], pVert[2]);

			// Update sphere bounds
			*maxSquaredRadius = std::max(vec.squaredLength(), *maxSquaredRadius);

			// Update box
			if (first)
			{
				min = vec;
				max = vec;
				first = false;
			}
			else
			{
				min.makeFloor(vec);
				max.makeCeil(vec);
			}

			pRaw += pBuf->getVertexSize();
		}
		localBox.setExtents(min, max);
		box->merge(localBox);
	}
    */
    //-----------------------------------------------------------------------
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
