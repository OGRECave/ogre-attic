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
#ifndef __HardwareBuffer__
#define __HardwareBuffer__

// Precompiler options
#include "OgrePrerequisites.h"

namespace Ogre {

    /** Abstract class defining common features of hardware buffers.
    @remarks
 	    A 'hardware buffer' is any area of memory held outside of core system ram,
	    and in our case refers mostly to video ram, although in theory this class
	    could be used with other memory areas such as sound card memory, custom
	    coprocessor memory etc.
    @par
 	    This reflects the fact that memory held outside of main system RAM must 
	    be interacted with in a more formal fashion in order to promote
	    cooperative and optimal usage of the buffers between the various 
	    processing units which manipulate them.
    @par
 	    This abstract class defines the core interface which is common to all
	    buffers, whether it be vertex buffers, index buffers, texture memory
	    or framebuffer memory etc.
    */
    class _OgreExport HardwareBuffer 
    {

	    public:
		    /// Enums describing buffer usage; not mutually exclusive
		    enum Usage 
		    {
                /** Static buffer which the application rarely modifies once created. Modifying 
                the contents of this buffer will involve a performance hit.
                */
                HBU_STATIC,
			    /** Indicates the application would like to modify this buffer with the CPU
			    fairly often. 
			    Buffers created with this flag will typically end up in AGP memory rather 
			    than video memory.
			    */
			    HBU_DYNAMIC,
			    /** Indicates the application will never read the contents of the buffer back, 
			    it will only ever write data. Locking a buffer with this flag will ALWAYS 
			    return a pointer to new, blank memory rather than the memory associated 
			    with the contents of the buffer; this avoids DMA stalls because you can 
			    write to a new memory area while the previous one is being used
			    */
			    HBU_WRITE_ONLY
		    };
		    /// Locking options
		    enum LockOptions
		    {
                /** Normal mode, ie allows read/write and contents are preserved. */
                HBL_NORMAL,
			    /** Discards the <em>entire</em> buffer while locking; this allows optimisation to be 
				performed because synchronisation issues are relaxed. Only allowed on buffers 
			    created with the HBU_DYNAMIC flag. 
			    */
			    HBL_DISCARD,
			    /** Lock the buffer for reading only. Not allowed in buffers which are created with HBU_WRITE_ONLY. 
				Mandatory on statuc buffers, ie those created without the HBU_DYNAMIC flag. 
				*/ 
			    HBL_READ_ONLY,
    			
		    };
	    protected:
		    bool mIsLocked;
		    size_t mSizeInBytes;
		    Usage mUsage;
    		
	    public:
		    /// Constructor, to be called by HardwareBufferManager only
            HardwareBuffer(Usage usage) : mUsage(usage) {}
            virtual ~HardwareBuffer() {}
		    /** Lock the buffer for (potentially) reading / writing.
		    @param offset The byte offset from the start of the buffer to lock
		    @param length The size of the area to lock, in bytes
		    @param options Locking options
		    @returns Pointer to the locked memory
		    */
		    virtual void* lock(size_t offset, size_t length, LockOptions options) = 0;
		    /** Releases the lock on this buffer. 
            @remarks 
                Locking and unlocking a buffer can, in some rare circumstances such as 
                switching video modes whilst the buffer is locked, corrupt the 
                contents of a buffer. This is pretty rare, but if it occurs, 
                this method will throw an exception, meaning you
                must re-upload the data.
            @par
                Note that using the 'read' and 'write' forms of updating the buffer does not
                suffer from this problem, so if you want to be 100% sure your
                data will not be lost, use the 'read' and 'write' forms instead.
            */
		    virtual void unlock(void) = 0;

            /** Reads data from the buffer and places it in the memory pointed to by pDest.
		    @param offset The byte offset from the start of the buffer to read
		    @param length The size of the area to read, in bytes
            @param pDest The area of memory in which to place the data, must be large enough to 
                accommodate the data!
            */
            virtual void readData(size_t offset, size_t length, void* pDest) = 0;
            /** Writes data to the buffer from an area of system memory; note that you must
                ensure that your buffer is big enough.
		    @param offset The byte offset from the start of the buffer to start writing
		    @param length The size of the data to write to, in bytes
            @param pSource The source of the data to be written
			@param discardWholeBuffer If true, this allows the driver to discard the entire buffer when writing,
				such that DMA stalls can be avoided; use if you can.
            */
            virtual void writeData(size_t offset, size_t length, const void* pSource,
					bool discardWholeBuffer = false) = 0;

			/** Copy data from another buffer into this one.
			@remarks
				Note that this buffer must be able to be written to so it must
				not static. 
			@param srcBuffer The buffer from which to read the copied data
			@param srcOffset Offset in the source buffer at which to start reading
			@param dstOffset Offset in the destination buffer to start writing
			@param length Length of the data to copy, in bytes.
			@param discardWholeBuffer If true, will discard the entire contents of this buffer before copying
			*/
			virtual void copyData(HardwareBuffer& srcBuffer, size_t srcOffset, 
				size_t dstOffset, size_t length, bool discardWholeBuffer = false)
			{
				const void *srcData = srcBuffer.lock(
					srcOffset, length, HBL_READ_ONLY);
				this->writeData(dstOffset, length, srcData, discardWholeBuffer);
				srcBuffer.unlock();
			}

            /// Returns the size of this buffer in bytes
            size_t getSizeInBytes(void) { return mSizeInBytes; }
            /// Returns the Usage flags with which this buffer was created
            Usage getUsage(void) { return mUsage; }
            /// Returns whether or not this buffer is currently locked.
            bool isLocked(void) { return mIsLocked; };




    		
    };
}
#endif


