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
			/** Indicates the application would like to modify this buffer with the CPU
			  sometimes. Absence of this flag means the application will never modify. 
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
			/** Discards the <em>entire</em> buffer while locking, this is implied whenever 
			  you lock a buffer created with HBU_WRITE_ONLY. Not allowed on buffers 
			  created without the HBU_DYNAMIC flag.
			*/
			HBL_DISCARD,
			/// Lock the buffer for reading only. Not allowed in buffers which are created with HBU_WRITE_ONLY. 
			HBL_READ_ONLY,
			
		};
	protected:
		bool mIsLocked;
		size_t mSizeInBytes;
		Usage mUsage;
		
	public:
		/// Constructor, to be called by HardwareBufferManager only
		HardwareBuffer(size_t sizeInBytes, Usage usage);
		/** Lock the buffer for (potentially) reading / writing.
		  @param offset The byte offset from the start of the buffer to lock
		  @param length The size of the area to lock, in bytes
		  @param options Locking options
		  @returns Pointer to the locked memory
		*/
		unsigned char* lock(size_t offset, size_t length, LockOptions options) = 0;
		/** Releases the lock on this buffer. */
		void unlock(void) = 0;
		
};

#endif


