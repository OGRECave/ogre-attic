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
#include "OgreHardwareVertexBuffer.h"
#include "OgreColourValue.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------------
    HardwareVertexBuffer::HardwareVertexBuffer(size_t vertexSize,  
        size_t numVertices, HardwareBuffer::Usage usage) 
        : HardwareBuffer(usage), mVertexSize(vertexSize), mNumVertices(numVertices)
    {
        // Calculate the size of the vertices
        mSizeInBytes = mVertexSize * numVertices;

    }
    //-----------------------------------------------------------------------------
    VertexElement::VertexElement(unsigned short source, size_t offset, 
        VertexElementType theType, VertexElementSemantic semantic, unsigned short index)
        : mSource(source), mOffset(offset), mType(theType), 
        mSemantic(semantic), mIndex(index)
    {
    }
    //-----------------------------------------------------------------------------
    VertexDeclaration::VertexDeclaration()
    {
    }
    //-----------------------------------------------------------------------------
    VertexDeclaration::~VertexDeclaration()
    {
    }
    //-----------------------------------------------------------------------------
    const VertexDeclaration::VertexElementList& VertexDeclaration::getElements(void) const
    {
        return mElementList;
    }
    //-----------------------------------------------------------------------------
    void VertexDeclaration::addElement(unsigned short source, 
        size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        mElementList.push_back(
            VertexElement(source, offset, theType, semantic, index)
            );
    }
    //-----------------------------------------------------------------------------
    void VertexDeclaration::removeElement(unsigned short elem_index)
    {
        assert(elem_index >= 0 && elem_index < mElementList.size() && "Index out of bounds");
        VertexElementList::iterator i = mElementList.begin();
        i += elem_index;
        mElementList.erase(i);
    }
    //-----------------------------------------------------------------------------
    void VertexDeclaration::modifyElement(unsigned short elem_index, 
        unsigned short source, size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        assert(elem_index >= 0 && elem_index < mElementList.size() && "Index out of bounds");
        VertexElementList::iterator i = mElementList.begin();
        i += elem_index;
        (*i) = VertexElement(source, offset, theType, semantic, index);
    }
    //-----------------------------------------------------------------------------
	const VertexElement& VertexDeclaration::findElementBySemantic(
		VertexElementSemantic sem, unsigned short index)
	{
		VertexElementList::const_iterator ei, eiend;
		eiend = mElementList.end();
		for (ei = mElementList.begin(); ei != eiend; ++ei)
		{
			if (ei->getSemantic() == sem && ei->getIndex() == index)
			{
				return *ei;
			}
		}

		Except(Exception::ERR_ITEM_NOT_FOUND, "Unable to find a vertex element with "
			" the requested semantic and index.", "VertexDeclaration::findElementBySemantic");


	}
    //-----------------------------------------------------------------------------
	VertexBufferBinding::VertexBufferBinding()
	{
	}
    //-----------------------------------------------------------------------------
	VertexBufferBinding::~VertexBufferBinding()
	{
        unsetAllBindings();
	}
    //-----------------------------------------------------------------------------
	void VertexBufferBinding::setBinding(unsigned short index, HardwareVertexBufferSharedPtr buffer)
	{
		mBindingMap[index] = buffer;
	}
    //-----------------------------------------------------------------------------
	void VertexBufferBinding::unsetBinding(unsigned short index)
	{
		VertexBufferBindingMap::iterator i = mBindingMap.find(index);
		if (i == mBindingMap.end())
		{
			Except(Exception::ERR_ITEM_NOT_FOUND,
				"Cannot find buffer binding for index " + StringConverter::toString(index),
				"VertexBufferBinding::unsetBinding");
		}
		mBindingMap.erase(i);
	}
    //-----------------------------------------------------------------------------
    void VertexBufferBinding::unsetAllBindings(void)
    {
        mBindingMap.clear();
    }
    //-----------------------------------------------------------------------------
	const VertexBufferBinding::VertexBufferBindingMap& 
	VertexBufferBinding::getBindings(void) const
	{
		return mBindingMap;
	}
    //-----------------------------------------------------------------------------
	HardwareVertexBufferSharedPtr VertexBufferBinding::getBuffer(unsigned short index)
	{
		VertexBufferBindingMap::iterator i = mBindingMap.find(index);
		if (i == mBindingMap.end())
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, "No buffer is bound to that index.",
				"VertexBufferBinding::getBuffer");
		}
		return i->second;
	}
    //-----------------------------------------------------------------------------
    HardwareVertexBufferSharedPtr::HardwareVertexBufferSharedPtr(HardwareVertexBuffer* buf)
        : SharedPtr<HardwareVertexBuffer>(buf)
    {

    }
    //-----------------------------------------------------------------------------
    void HardwareVertexBufferSharedPtr::destroy(void)
    {
        HardwareBufferManager::getSingleton().destroyVertexBuffer(pRep);
		delete pUseCount;
    }




}
