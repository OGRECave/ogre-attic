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
#ifndef __HardwarePixelBuffer__
#define __HardwarePixelBuffer__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreHardwareBuffer.h"
#include "OgreSharedPtr.h"
#include "OgrePixelFormat.h"
#include "OgreImage.h"

namespace Ogre {
    /** Specialisation of HardwareBuffer for a pixel buffer. */
    class _OgreExport HardwarePixelBuffer : public HardwareBuffer
    {
    protected: 
        // Extents
        size_t mWidth, mHeight, mDepth;
        // Pitches (offsets between rows and slices)
        int mRowPitch, mSlicePitch;
        // Internal format
        PixelFormat mFormat;
        // Size in bytes
        size_t mSize; 
        // Currently locked region
        PixelBox mCurrentLock;
        
        /// Internal implementation of lock(), must be overridden in subclasses
        virtual const PixelBox &lockImpl(const Image::Box lockBox,  LockOptions options) = 0;

        /// Internal implementation of lock(), do not OVERRIDE or CALL this
        /// for HardwarePixelBuffer implementations, but override the previous method
        virtual void* lockImpl(size_t offset, size_t length, LockOptions options);

        /// Internal implementation of unlock(), must be overridden in subclasses
        /// virtual void unlockImpl(void) = 0;
    public:
        /// Should be called by HardwareBufferManager
        HardwarePixelBuffer(size_t mWidth, size_t mHeight, size_t mDepth,
                PixelFormat mFormat,
                HardwareBuffer::Usage usage, bool useSystemMemory, bool useShadowBuffer);
        ~HardwarePixelBuffer();

        virtual const PixelBox &lock(const Image::Box lockBox, LockOptions options);        
        virtual void* lock(size_t offset, size_t length, LockOptions options);
        
        const PixelBox &getCurrentLock();
    };

}
#endif

