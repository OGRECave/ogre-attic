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
#include "OgreVertexIndexData.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    template<> HardwareBufferManager* Singleton<HardwareBufferManager>::ms_Singleton = 0;
    HardwareBufferManager* HardwareBufferManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    HardwareBufferManager& HardwareBufferManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
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
    void HardwareBufferManager::registerVertexBufferSourceAndCopy(
			const HardwareVertexBufferSharedPtr& sourceBuffer,
			const HardwareVertexBufferSharedPtr& copy)
	{
        // Locate source buffer copy in free list
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

		// Add copy to free list
		vbmi->second->push_back(copy);
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
        TemporaryVertexBufferLicenseList::iterator i;
        i = mTempVertexBufferLicenses.begin(); 

        while (i != mTempVertexBufferLicenses.end()) 
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
            else
            {
                ++i;
            }
        }
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::_forceReleaseBufferCopies(
        const HardwareVertexBufferSharedPtr& sourceBuffer)
    {
        TemporaryVertexBufferLicenseList::iterator i;
        i = mTempVertexBufferLicenses.begin(); 
    
        // Erase the copies which are licensed out
        while (i != mTempVertexBufferLicenses.end()) 
        {
            const VertexBufferLicense& vbl = *i;
            if (vbl.originalBufferPtr == sourceBuffer.get())
            {
                // Just tell the owner that this is being released
                vbl.licensee->licenseExpired(vbl.buffer.get());
                i = mTempVertexBufferLicenses.erase(i);
            }
            else
            {
                ++i;
            }
        }
        // Erase the free copies
        FreeTemporaryVertexBufferMap::iterator fi =
            mFreeTempVertexBufferMap.begin();
        while (fi != mFreeTempVertexBufferMap.end())
        {
            if (fi->first == sourceBuffer.get())
            {
                delete fi->second;
                FreeTemporaryVertexBufferMap::iterator deli = fi++;
                mFreeTempVertexBufferMap.erase(deli);
            }
            else
            {
                ++fi;
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
    void TempBlendedBufferInfo::checkoutTempCopies(bool positions, bool normals)
    {
        bindPositions = positions;
        bindNormals = normals;
        HardwareBufferManager &mgr = HardwareBufferManager::getSingleton();
        if (bindPositions)
        {
            destPositionBuffer = mgr.allocateVertexBufferCopy(srcPositionBuffer, 
                HardwareBufferManager::BLT_AUTOMATIC_RELEASE, this);
        }
        if (bindNormals && !srcNormalBuffer.isNull() && !posNormalShareBuffer)
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
        if (bindNormals && !posNormalShareBuffer)
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
