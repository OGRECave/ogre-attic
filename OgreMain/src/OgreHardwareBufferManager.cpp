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
#include "OgreHardwareBufferManager.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    template<> HardwareBufferManager* Singleton<HardwareBufferManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    HardwareBufferManager::HardwareBufferManager()
    {
    }
    //-----------------------------------------------------------------------
    HardwareBufferManager::~HardwareBufferManager()
    {
        // Destroy everything
        destroyAllDeclarations();
        destroyAllBindings();
        // No need to destroy main buffers - they will be destroyed by removal of bindings

        // Destroy temp buffers
        FreeTemporaryVertexBufferMap::iterator i, iend;
        iend = mFreeTempVertexBufferMap.end();
        for (i = mFreeTempVertexBufferMap.begin(); i != iend; ++i)
        {
            delete i->second;
        }
    }
    //-----------------------------------------------------------------------
    HardwareBufferManager& HardwareBufferManager::getSingleton(void)
    {
        return Singleton<HardwareBufferManager>::getSingleton();
    }
    //-----------------------------------------------------------------------
    VertexDeclaration* HardwareBufferManager::createVertexDeclaration(void)
    {
        VertexDeclaration* decl = new VertexDeclaration();
        mVertexDeclarations.push_back(decl);
        return decl;
        
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::destroyVertexDeclaration(VertexDeclaration* decl)
    {
        mVertexDeclarations.remove(decl);
        delete decl;
    }
    //-----------------------------------------------------------------------
	VertexBufferBinding* HardwareBufferManager::createVertexBufferBinding(void)
	{
		VertexBufferBinding* ret = new VertexBufferBinding();
		mVertexBufferBindings.push_back(ret);
		return ret;
	}
    //-----------------------------------------------------------------------
	void HardwareBufferManager::destroyVertexBufferBinding(VertexBufferBinding* binding)
	{
		mVertexBufferBindings.remove(binding);
		delete binding;
	}
    //-----------------------------------------------------------------------
    void HardwareBufferManager::destroyAllDeclarations(void)
    {
        VertexDeclarationList::iterator decl;
        for (decl = mVertexDeclarations.begin(); decl != mVertexDeclarations.end(); ++decl)
        {
            delete *decl;
        }
        mVertexDeclarations.clear();

    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::destroyAllBindings(void)
    {
        VertexBufferBindingList::iterator bind;
        for (bind = mVertexBufferBindings.begin(); bind != mVertexBufferBindings.end(); ++bind)
        {
            delete *bind;
        }
        mVertexBufferBindings.clear();
    }
    //-----------------------------------------------------------------------
    HardwareVertexBufferSharedPtr 
    HardwareBufferManager::allocateVertexBufferCopy(
        const HardwareVertexBufferSharedPtr& sourceBuffer, 
        BufferLicenseType licenseType, HardwareBufferLicensee* licensee,
        bool copyData)
    {
        // Locate existing buffer copy in free list
        FreeTemporaryVertexBufferMap::iterator vbmi = 
            mFreeTempVertexBufferMap.find(sourceBuffer.getPointer());

        if (vbmi == mFreeTempVertexBufferMap.end())
        {
            // Add new entry
            FreeTemporaryVertexBufferList *newList = new FreeTemporaryVertexBufferList();
            std::pair<FreeTemporaryVertexBufferMap::iterator, bool> retPair = 
                mFreeTempVertexBufferMap.insert(
                    FreeTemporaryVertexBufferMap::value_type(
                        sourceBuffer.getPointer(), newList));
            assert(retPair.second && "Error inserting buffer list");
            vbmi = retPair.first;
        }

        HardwareVertexBufferSharedPtr vbuf;
        // Are there any free buffers?
        if (vbmi->second->empty())
        {
            // copy buffer, use shadow buffer and make dynamic
            vbuf = makeBufferCopy(sourceBuffer, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
                true);
        }
        else
        {
            // Allocate existing copy
            vbuf = vbmi->second->back();
            vbmi->second->pop_back();
        }

        // Copy data?
        if (copyData)
        {
            vbuf->copyData(*(sourceBuffer.get()), 0, 0, sourceBuffer->getSizeInBytes(), true);
        }
        // Insert copy into licensee list
        mTempVertexBufferLicenses.push_back(
            VertexBufferLicense(sourceBuffer.getPointer(), licenseType, vbuf, licensee));

        return vbuf;
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::releaseVertexBufferCopy(
        const HardwareVertexBufferSharedPtr& bufferCopy)
    {
        TemporaryVertexBufferLicenseList::iterator i, iend;
        iend = mTempVertexBufferLicenses.end();
        for (i = mTempVertexBufferLicenses.begin(); i != iend; ++i)
        {
            const VertexBufferLicense& vbl = *i;
            if (vbl.buffer.getPointer() == bufferCopy.getPointer())
            {

                FreeTemporaryVertexBufferMap::iterator vbi =
                    mFreeTempVertexBufferMap.find(vbl.originalBufferPtr);
                assert (vbi != mFreeTempVertexBufferMap.end());

                vbi->second->push_back(vbl.buffer);
                mTempVertexBufferLicenses.erase(i);
                break;

            }
        }

    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::_releaseBufferCopies(void)
    {
        TemporaryVertexBufferLicenseList::iterator i, iend;
        iend = mTempVertexBufferLicenses.end();
        for (i = mTempVertexBufferLicenses.begin(); i != iend; ++i)
        {

            const VertexBufferLicense& vbl = *i;
            if (vbl.licenseType == BLT_AUTOMATIC_RELEASE)
            {

                FreeTemporaryVertexBufferMap::iterator vbi =
                    mFreeTempVertexBufferMap.find(vbl.originalBufferPtr);
                assert (vbi != mFreeTempVertexBufferMap.end());

                vbi->second->push_back(vbl.buffer);
                i = mTempVertexBufferLicenses.erase(i);

            }
        }
    }
    //-----------------------------------------------------------------------
    HardwareVertexBufferSharedPtr 
    HardwareBufferManager::makeBufferCopy(
        const HardwareVertexBufferSharedPtr& source,
        HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
        return this->createVertexBuffer(
            source->getVertexSize(), 
            source->getNumVertices(),
            usage, useShadowBuffer);
    }
    //-----------------------------------------------------------------------------
    void TempBlendedBufferInfo::checkoutTempCopies(void)
    {
        HardwareBufferManager &mgr = HardwareBufferManager::getSingleton();
        destPositionBuffer = mgr.allocateVertexBufferCopy(srcPositionBuffer, 
            HardwareBufferManager::BLT_AUTOMATIC_RELEASE, this);
        if (!posNormalShareBuffer)
        {
            destNormalBuffer = mgr.allocateVertexBufferCopy(srcNormalBuffer, 
                HardwareBufferManager::BLT_AUTOMATIC_RELEASE, this);
        }
    }
    //-----------------------------------------------------------------------------
    void TempBlendedBufferInfo::bindTempCopies(VertexData* targetData, bool suppressHardwareUpload)
    {
        this->destPositionBuffer->suppressHardwareUpdate(suppressHardwareUpload);
        targetData->vertexBufferBinding->setBinding(
            this->posBindIndex, this->destPositionBuffer);
        if (!posNormalShareBuffer)
        {
            this->destNormalBuffer->suppressHardwareUpdate(suppressHardwareUpload);
            targetData->vertexBufferBinding->setBinding(
                this->normBindIndex, this->destNormalBuffer);
        }
    }
    //-----------------------------------------------------------------------------
    void TempBlendedBufferInfo::licenseExpired(HardwareBuffer* buffer)
    {
        if (buffer == destPositionBuffer.get())
            destPositionBuffer.release();
        if (buffer == destNormalBuffer.get())
            destNormalBuffer.release();

    }

}
