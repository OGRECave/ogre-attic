/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2005 The OGRE Team
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
#ifndef __GLPIXELBUFFER_H__
#define __GLPIXELBUFFER_H__

#include "OgreGLPrerequisites.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre {

	class GLHardwarePixelBuffer: public HardwarePixelBuffer
	{
	protected:
		/// Lock a box
		PixelBox lockImpl(const Image::Box lockBox,  LockOptions options);

		/// Unlock a box
		void unlockImpl(void);
		
		enum Type {
			TYPE_TEXTURE,		// Texture subsurface
			TYPE_FRAMEBUFFER,	// Framebuffer surface
 			TYPE_PBO			// Plain pixel buffer object
		};
		Type mType;
		
		// In case this is a texture level, define the texture type and ID
		GLenum mTarget;
		GLuint mTextureID;
		GLint mFace;
		GLint mLevel;
		bool mSoftwareMipmap;		// Use GLU for mip mapping
		// Internal buffer; either on-card or in system memory, freed/allocated on demand
		// depending on buffer usage
		PixelBox mBuffer;
		
		// Buffer allocation/freeage
		void allocateBuffer();
		void freeBuffer();
		// Upload a box of pixels to this buffer on the card
		void upload(PixelBox &data);
		// Download a box of pixels from the card
		void download(PixelBox &data);
	public:
		GLHardwarePixelBuffer(GLenum target, GLuint id, GLint face, GLint level, Usage usage, 
			bool softwareMipmap);
		
		/// @copydoc HardwarePixelBuffer::blit
		void blit(HardwarePixelBuffer *src, const Image::Box &srcBox, const Image::Box &dstBox);
		
		/// @copydoc HardwarePixelBuffer::blitFromMemory
		void blitFromMemory(const PixelBox &src, const Image::Box &dstBox);
		
		/// @copydoc HardwarePixelBuffer::blitToMemory
		void blitToMemory(const Image::Box &srcBox, const PixelBox &dst);
		
		~GLHardwarePixelBuffer();
	};

};

#endif
