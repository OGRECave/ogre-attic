/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreLogManager.h"


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
    // Free temporary vertex buffers every 5 minutes on 100fps
    const size_t HardwareBufferManager::UNDER_USED_FRAME_THRESHOLD = 30000;
    //-----------------------------------------------------------------------
    HardwareBufferManager::HardwareBufferManager()
        : mUnderUsedFrameCount(0)
    {
    }
    //-----------------------------------------------------------------------
    HardwareBufferManager::~HardwareBufferManager()
    {
        // Destroy everything
        destroyAllDeclarations();
        destroyAllBindings();
        // No need to destroy main buffers - they will be destroyed by removal of bindings

        // No need to destroy temp buffers - they will be destroyed automatically.
    }
    //-----------------------------------------------------------------------
    VertexDeclaration* HardwareBufferManager::createVertexDeclaration(void)
    {
        VertexDeclaration* decl = createVertexDeclarationImpl();
        mVertexDeclarations.insert(decl);
        return decl;
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::destroyVertexDeclaration(VertexDeclaration* decl)
    {
        mVertexDeclarations.erase(decl);
        destroyVertexDeclarationImpl(decl);
    }
    //-----------------------------------------------------------------------
	VertexBufferBinding* HardwareBufferManager::createVertexBufferBinding(void)
	{
		VertexBufferBinding* ret = createVertexBufferBindingImpl();
		mVertexBufferBindings.insert(ret);
		return ret;
	}
    //-----------------------------------------------------------------------
	void HardwareBufferManager::destroyVertexBufferBinding(VertexBufferBinding* binding)
	{
		mVertexBufferBindings.erase(binding);
		destroyVertexBufferBindingImpl(binding);
	}
    //-----------------------------------------------------------------------
    VertexDeclaration* HardwareBufferManager::createVertexDeclarationImpl(void)
    {
        return new VertexDeclaration();
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::destroyVertexDeclarationImpl(VertexDeclaration* decl)
    {
        delete decl;
    }
    //-----------------------------------------------------------------------
	VertexBufferBinding* HardwareBufferManager::createVertexBufferBindingImpl(void)
	{
		return new VertexBufferBinding();
	}
    //-----------------------------------------------------------------------
	void HardwareBufferManager::destroyVertexBufferBindingImpl(VertexBufferBinding* binding)
	{
		delete binding;
	}
    //-----------------------------------------------------------------------
    void HardwareBufferManager::destroyAllDeclarations(void)
    {
        VertexDeclarationList::iterator decl;
        for (decl = mVertexDeclarations.begin(); decl != mVertexDeclarations.end(); ++decl)
        {
            destroyVertexDeclarationImpl(*decl);
        }
        mVertexDeclarations.clear();
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::destroyAllBindings(void)
    {
        VertexBufferBindingList::iterator bind;
        for (bind = mVertexBufferBindings.begin(); bind != mVertexBufferBindings.end(); ++bind)
        {
            destroyVertexBufferBindingImpl(*bind);
        }
        mVertexBufferBindings.clear();
    }
	//-----------------------------------------------------------------------
    void HardwareBufferManager::registerVertexBufferSourceAndCopy(
			const HardwareVertexBufferSharedPtr& sourceBuffer,
			const HardwareVertexBufferSharedPtr& copy)
	{
		// Add copy to free temporary vertex buffers
        mFreeTempVertexBufferMap.insert(
            FreeTemporaryVertexBufferMap::value_type(sourceBuffer.get(), copy));
	}
	//-----------------------------------------------------------------------
    HardwareVertexBufferSharedPtr 
    HardwareBufferManager::allocateVertexBufferCopy(
        const HardwareVertexBufferSharedPtr& sourceBuffer, 
        BufferLicenseType licenseType, HardwareBufferLicensee* licensee,
        bool copyData)
    {
        HardwareVertexBufferSharedPtr vbuf;

        // Locate existing buffer copy in temporary vertex buffers
        FreeTemporaryVertexBufferMap::iterator i = 
            mFreeTempVertexBufferMap.find(sourceBuffer.get());
        if (i == mFreeTempVertexBufferMap.end())
        {
            // copy buffer, use shadow buffer and make dynamic
            vbuf = makeBufferCopy(
                sourceBuffer, 
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, 
                true);
        }
        else
        {
            // Allocate existing copy
            vbuf = i->second;
            mFreeTempVertexBufferMap.erase(i);
        }

        // Copy data?
        if (copyData)
        {
            vbuf->copyData(*(sourceBuffer.get()), 0, 0, sourceBuffer->getSizeInBytes(), true);
        }

        // Insert copy into licensee list
        mTempVertexBufferLicenses.insert(
            TemporaryVertexBufferLicenseMap::value_type(
                vbuf.get(),
                VertexBufferLicense(sourceBuffer.get(), licenseType, vbuf, licensee)));

        return vbuf;
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::releaseVertexBufferCopy(
        const HardwareVertexBufferSharedPtr& bufferCopy)
    {
        TemporaryVertexBufferLicenseMap::iterator i =
            mTempVertexBufferLicenses.find(bufferCopy.get());
        if (i != mTempVertexBufferLicenses.end())
        {
            const VertexBufferLicense& vbl = i->second;

            vbl.licensee->licenseExpired(vbl.buffer.get());

            mFreeTempVertexBufferMap.insert(
                FreeTemporaryVertexBufferMap::value_type(vbl.originalBufferPtr, vbl.buffer));
            mTempVertexBufferLicenses.erase(i);
        }
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::_freeUnusedBufferCopies(void)
    {
        size_t numFreed = 0;

        // Free unused temporary buffers
        FreeTemporaryVertexBufferMap::iterator i;
        i = mFreeTempVertexBufferMap.begin();
        while (i != mFreeTempVertexBufferMap.end())
        {
            FreeTemporaryVertexBufferMap::iterator icur = i++;
            // Free the temporary buffer that referenced by ourself only.
            // TODO: Some temporary buffers are bound to vertex buffer bindings
            // but not checked out, need to sort out method to unbind them.
            if (icur->second.useCount() <= 1)
            {
                ++numFreed;
                mFreeTempVertexBufferMap.erase(icur);
            }
        }

        StringUtil::StrStreamType str;
        if (numFreed)
        {
            str << "HardwareBufferManager: Freed " << numFreed << " unused temporary vertex buffers.";
        }
        else
        {
            str << "HardwareBufferManager: No unused temporary vertex buffers found.";
        }
        LogManager::getSingleton().logMessage(str.str());
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::_releaseBufferCopies(bool forceFreeUnused)
    {
        size_t numUnused = mFreeTempVertexBufferMap.size();
        size_t numUsed = mTempVertexBufferLicenses.size();

        // Erase the copies which are automatic licensed out
        TemporaryVertexBufferLicenseMap::iterator i;
        i = mTempVertexBufferLicenses.begin(); 
        while (i != mTempVertexBufferLicenses.end()) 
        {
            TemporaryVertexBufferLicenseMap::iterator icur = i++;
            const VertexBufferLicense& vbl = icur->second;
            if (vbl.licenseType == BLT_AUTOMATIC_RELEASE)
            {
				vbl.licensee->licenseExpired(vbl.buffer.get());

                mFreeTempVertexBufferMap.insert(
                    FreeTemporaryVertexBufferMap::value_type(vbl.originalBufferPtr, vbl.buffer));
                mTempVertexBufferLicenses.erase(icur);
            }
        }

        // Check whether or not free unused temporary vertex buffers.
        if (forceFreeUnused)
        {
            _freeUnusedBufferCopies();
            mUnderUsedFrameCount = 0;
        }
        else
        {
            if (numUsed < numUnused)
            {
                // Free temporary vertex buffers if too many unused for a long time.
                // Do overall temporary vertex buffers instead of per source buffer
                // to avoid overhead.
                ++mUnderUsedFrameCount;
                if (mUnderUsedFrameCount >= UNDER_USED_FRAME_THRESHOLD)
                {
                    _freeUnusedBufferCopies();
                    mUnderUsedFrameCount = 0;
                }
            }
            else
            {
                mUnderUsedFrameCount = 0;
            }
        }
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::_forceReleaseBufferCopies(
        const HardwareVertexBufferSharedPtr& sourceBuffer)
    {
        _forceReleaseBufferCopies(sourceBuffer.get());
    }
    //-----------------------------------------------------------------------
    void HardwareBufferManager::_forceReleaseBufferCopies(
        HardwareVertexBuffer* sourceBuffer)
    {
        // Erase the copies which are licensed out
        TemporaryVertexBufferLicenseMap::iterator i;
        i = mTempVertexBufferLicenses.begin();
        while (i != mTempVertexBufferLicenses.end()) 
        {
            TemporaryVertexBufferLicenseMap::iterator icur = i++;
            const VertexBufferLicense& vbl = icur->second;
            if (vbl.originalBufferPtr == sourceBuffer)
            {
                // Just tell the owner that this is being released
                vbl.licensee->licenseExpired(vbl.buffer.get());

                mTempVertexBufferLicenses.erase(icur);
            }
        }

        // Erase the free copies
        mFreeTempVertexBufferMap.erase(sourceBuffer);
    }
	//-----------------------------------------------------------------------
	void HardwareBufferManager::_notifyVertexBufferDestroyed(HardwareVertexBuffer* buf)
	{
		VertexBufferList::iterator i = mVertexBuffers.find(buf);
		if (i != mVertexBuffers.end())
		{
            // release vertex buffer copies
			mVertexBuffers.erase(i);
            _forceReleaseBufferCopies(buf);
		}
	}
	//-----------------------------------------------------------------------
	void HardwareBufferManager::_notifyIndexBufferDestroyed(HardwareIndexBuffer* buf)
	{
		IndexBufferList::iterator i = mIndexBuffers.find(buf);
		if (i != mIndexBuffers.end())
		{
			mIndexBuffers.erase(i);
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
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    TempBlendedBufferInfo::~TempBlendedBufferInfo(void)
    {
        // check that temp buffers have been released
        HardwareBufferManager &mgr = HardwareBufferManager::getSingleton();
        if (!destPositionBuffer.isNull())
            mgr.releaseVertexBufferCopy(destPositionBuffer);
        if (!destNormalBuffer.isNull())
            mgr.releaseVertexBufferCopy(destNormalBuffer);

    }
    //-----------------------------------------------------------------------------
    void TempBlendedBufferInfo::checkoutTempCopies(bool positions, bool normals)
    {
        bindPositions = positions;
        bindNormals = normals;

        HardwareBufferManager &mgr = HardwareBufferManager::getSingleton();

        if (!destPositionBuffer.isNull())
        {
            mgr.releaseVertexBufferCopy(destPositionBuffer);
            destPositionBuffer.setNull();
        }
        if (!destNormalBuffer.isNull())
        {
            mgr.releaseVertexBufferCopy(destNormalBuffer);
            destNormalBuffer.setNull();
        }

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
	bool TempBlendedBufferInfo::buffersCheckedOut(bool positions, bool normals) const
	{
		return (!positions || !destPositionBuffer.isNull()) && 
            (!normals || !(posNormalShareBuffer ? destPositionBuffer.isNull() : destNormalBuffer.isNull()));
	}
    //-----------------------------------------------------------------------------
    void TempBlendedBufferInfo::bindTempCopies(VertexData* targetData, bool suppressHardwareUpload)
    {
        this->destPositionBuffer->suppressHardwareUpdate(suppressHardwareUpload);
        targetData->vertexBufferBinding->setBinding(
            this->posBindIndex, this->destPositionBuffer);
        if (bindNormals && !posNormalShareBuffer && !destNormalBuffer.isNull())
        {
            this->destNormalBuffer->suppressHardwareUpdate(suppressHardwareUpload);
            targetData->vertexBufferBinding->setBinding(
                this->normBindIndex, this->destNormalBuffer);
        }
    }
    //-----------------------------------------------------------------------------
    void TempBlendedBufferInfo::licenseExpired(HardwareBuffer* buffer)
    {
        assert(buffer == destPositionBuffer.get()
            || buffer == destNormalBuffer.get());

        if (buffer == destPositionBuffer.get())
            destPositionBuffer.setNull();
        if (buffer == destNormalBuffer.get())
            destNormalBuffer.setNull();

    }

}
