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
#include "OgreGLHardwarePixelBuffer.h"
#include "OgreGLTexture.h"
#include "OgreGLSupport.h"
#include "OgreGLPixelFormat.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

namespace Ogre {
//----------------------------------------------------------------------------- 
GLHardwarePixelBuffer::GLHardwarePixelBuffer(GLenum target, GLuint id, GLint face, GLint level, Usage usage, bool crappyCard):
	HardwarePixelBuffer(0, 0, 0, PF_UNKNOWN, usage, false, false),
	mType(TYPE_TEXTURE),
	mTarget(target), mTextureID(id), mFace(face), mLevel(level), mSoftwareMipmap(crappyCard)
{
	// devise mWidth, mHeight and mDepth and mFormat
	GLint value;
	
	glBindTexture( mTarget, mTextureID );
	
	// Get face identifier
	mFaceTarget = mTarget;
	if(mTarget == GL_TEXTURE_CUBE_MAP)
		mFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
	
	// Get width
	glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_WIDTH, &value);
	mWidth = value;
	
	// Get height
	if(target == GL_TEXTURE_1D)
		value = 1;	// Height always 1 for 1D textures
	else
		glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_HEIGHT, &value);
	mHeight = value;
	
	// Get depth
	if(target != GL_TEXTURE_3D)
		value = 1; // Depth always 1 for non-3D textures
	else
		glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_DEPTH, &value);
	mDepth = value;
	
	// Get format
	glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_INTERNAL_FORMAT, &value);
	int mGLInternalFormat = value;
	mFormat = GLPixelUtil::getClosestOGREFormat(value);
	
	// Default
	mRowPitch = mWidth;
	mSlicePitch = mHeight*mWidth;
	mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
	
	// Log a message
	/*
	std::stringstream str;
	str << "GLHardwarePixelBuffer constructed for texture " << mTextureID 
		<< " face " << mFace << " level " << mLevel << ": "
		<< "width=" << mWidth << " height="<< mHeight << " depth=" << mDepth
		<< "format=" << PixelUtil::getFormatName(mFormat) << "(internal 0x"
		<< std::hex << value << ")";
	LogManager::getSingleton().logMessage( 
                LML_NORMAL, str.str());
	*/
	// Set up pixel box
	mBuffer = PixelBox(mWidth, mHeight, mDepth, mFormat);
				
	// Allocate buffer
	if(mUsage & HBU_STATIC)
		allocateBuffer();
}

//-----------------------------------------------------------------------------  
GLHardwarePixelBuffer::~GLHardwarePixelBuffer()
{
	// Force free buffer
	delete [] (uint8*)mBuffer.data;
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::allocateBuffer()
{
	if(mBuffer.data)
		// Already allocated
		return;
	mBuffer.data = new uint8[mSizeInBytes];
	// TODO: use PBO if we're HBU_DYNAMIC
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::freeBuffer()
{
	// Free buffer if we're STATIC to save memory
	if(mUsage & HBU_STATIC)
	{
		delete [] (uint8*)mBuffer.data;
		mBuffer.data = 0;
	}
}
//-----------------------------------------------------------------------------  
PixelBox GLHardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
{
	allocateBuffer();
	//if(!(mUsage & HBU_WRITEONLY) && options!=HBU_DISCARD)
	if(options == HBL_READ_ONLY)
		// Download the old contents of the texture
		download(mBuffer);
	return mBuffer.getSubVolume(lockBox);
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::unlockImpl(void)
{
	// From buffer to card
	upload(mCurrentLock);
	
	freeBuffer();
}
//-----------------------------------------------------------------------------
void GLHardwarePixelBuffer::upload(const PixelBox &data)
{
	glBindTexture( mTarget, mTextureID );
	if(PixelUtil::isCompressed(data.format))
	{
		if(data.format != mFormat || !data.isConsecutive())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"Compressed images must be consecutive, in the source format",
		 	"GLHardwarePixelBuffer::upload");
		GLenum format = GLPixelUtil::getClosestGLInternalFormat(mFormat);
		// Data must be consecutive and at beginning of buffer as PixelStorei not allowed
		// for compressed formats
		switch(mTarget) {
			case GL_TEXTURE_1D:
				glCompressedTexSubImage1DARB_ptr(GL_TEXTURE_1D, mLevel, 
					data.left,
					data.getWidth(),
					format, data.getConsecutiveSize(),
					data.data);
				break;
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP:
				glCompressedTexSubImage2DARB_ptr(mFaceTarget, mLevel, 
					data.left, data.top, 
					data.getWidth(), data.getHeight(),
					format, data.getConsecutiveSize(),
					data.data);
				break;
			case GL_TEXTURE_3D:
				glCompressedTexSubImage3DARB_ptr(GL_TEXTURE_3D, mLevel, 
					data.left, data.top, data.front,
					data.getWidth(), data.getHeight(), data.getDepth(),
					format, data.getConsecutiveSize(),
					data.data);
				break;
		}
		
	} 
	else if(mSoftwareMipmap)
	{
		GLint internalFormat;
		glGetTexLevelParameteriv(mTarget, mLevel, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
		if(data.getWidth() != data.rowPitch)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, data.rowPitch);
		if(data.getHeight()*data.getWidth() != data.slicePitch)
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		switch(mTarget)
		{
		case GL_TEXTURE_1D:
			gluBuild1DMipmaps(
				GL_TEXTURE_1D, internalFormat,
				data.getWidth(),
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
				data.data);
			break;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_CUBE_MAP:
			gluBuild2DMipmaps(
				mFaceTarget,
				internalFormat, data.getWidth(), data.getHeight(), 
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format), 
				data.data);
			break;		
		case GL_TEXTURE_3D:
			/* Requires GLU 1.3 which is harder to come by than cards doing hardware mipmapping
				Most 3D textures don't need mipmaps?
			gluBuild3DMipmaps(
				GL_TEXTURE_3D, internalFormat, 
				data.getWidth(), data.getHeight(), data.getDepth(),
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
				data.data);
			*/
			glTexImage3D(
				GL_TEXTURE_3D, 0, internalFormat, 
				data.getWidth(), data.getHeight(), data.getDepth(), 0, 
				GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
				data.data );
			break;
		}
	} 
	else
	{
		if(data.getWidth() != data.rowPitch)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, data.rowPitch);
		if(data.getHeight()*data.getWidth() != data.slicePitch)
			glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
		if((data.getWidth()*PixelUtil::getNumElemBytes(data.format)) & 3) {
			// Standard alignment of 4 is not right
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
		switch(mTarget) {
			case GL_TEXTURE_1D:
				glTexSubImage1D(GL_TEXTURE_1D, mLevel, 
					data.left,
					data.getWidth(),
					GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
					data.data);
				break;
			case GL_TEXTURE_2D:
			case GL_TEXTURE_CUBE_MAP:
				glTexSubImage2D(mFaceTarget, mLevel, 
					data.left, data.top, 
					data.getWidth(), data.getHeight(),
					GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
					data.data);
				break;
			case GL_TEXTURE_3D:
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
				// GLX doesn't define it
				glTexSubImage3DEXT(
#else
				glTexSubImage3D(
#endif
					GL_TEXTURE_3D, mLevel, 
					data.left, data.top, data.front,
					data.getWidth(), data.getHeight(), data.getDepth(),
					GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
					data.data);
				break;
		}	
	}
	// Restore defaults
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::download(const PixelBox &data)
{
	if(data.getWidth() != getWidth() ||
		data.getHeight() != getHeight() ||
		data.getDepth() != getDepth())
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "only download of entire buffer is supported by GL",
		 	"GLHardwarePixelBuffer::download");
	glBindTexture( mTarget, mTextureID );
	if(PixelUtil::isCompressed(data.format))
	{
		if(data.format != mFormat || !data.isConsecutive())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
			"Compressed images must be consecutive, in the source format",
		 	"GLHardwarePixelBuffer::upload");
		// Data must be consecutive and at beginning of buffer as PixelStorei not allowed
		// for compressed formate
		glGetCompressedTexImageARB_ptr(mFaceTarget, mLevel, data.data);
	} 
	else
	{
		if(data.getWidth() != data.rowPitch)
			glPixelStorei(GL_PACK_ROW_LENGTH, data.rowPitch);
		if(data.getHeight()*data.getWidth() != data.slicePitch)
			glPixelStorei(GL_PACK_IMAGE_HEIGHT, (data.slicePitch/data.getWidth()));
		if((data.getWidth()*PixelUtil::getNumElemBytes(data.format)) & 3) {
			// Standard alignment of 4 is not right
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
		}
		// We can only get the entire texture
		glGetTexImage(mFaceTarget, mLevel, 
			GLPixelUtil::getGLOriginFormat(data.format), GLPixelUtil::getGLOriginDataType(data.format),
			data.data);
		// Restore defaults
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::blit(HardwarePixelBuffer *src, const Image::Box &srcBox, const Image::Box &dstBox)
{
	// this can be sped up with some copy pixels primitive, sometimes, maybe
	HardwarePixelBuffer::blit(src, srcBox, dstBox);
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
{
	if(!mBuffer.contains(dstBox))
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "destination box out of range",
		 "GLHardwarePixelBuffer::blitFromMemory");
	PixelBox scaled;
	
	if(src.getWidth() != dstBox.getWidth() ||
		src.getHeight() != dstBox.getHeight() ||
		src.getDepth() != dstBox.getDepth())
	{
		// Scale to destination size. Use DevIL and not iluScale because ILU screws up for 
		// floating point textures and cannot cope with 3D images.
		// This also does pixel format conversion if needed
		allocateBuffer();
		scaled = mBuffer.getSubVolume(dstBox);
		Image::scale(src, scaled, Image::FILTER_BILINEAR);
	}
	else if(GLPixelUtil::getGLOriginFormat(src.format) == 0)
	{
		// Extents match, but format is not accepted as valid source format for GL
		// do conversion in temporary buffer
		allocateBuffer();
		scaled = mBuffer.getSubVolume(dstBox);
		PixelUtil::bulkPixelConversion(src, scaled);
	}
	else
	{
		// No scaling or conversion needed
		scaled = src;
		// Set extents for upload
		scaled.left = dstBox.left;
		scaled.right = dstBox.right;
		scaled.top = dstBox.top;
		scaled.bottom = dstBox.bottom;
		scaled.front = dstBox.front;
		scaled.back = dstBox.back;
	}
	
	upload(scaled);
	freeBuffer();
}
//-----------------------------------------------------------------------------  
void GLHardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
{
	if(!mBuffer.contains(srcBox))
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "source box out of range",
		 "GLHardwarePixelBuffer::blitToMemory");
	if(srcBox.left == 0 && srcBox.right == getWidth() &&
	   srcBox.top == 0 && srcBox.bottom == getHeight() &&
	   srcBox.front == 0 && srcBox.back == getDepth() &&
	   dst.getWidth() == getWidth() &&
	   dst.getHeight() == getHeight() &&
	   dst.getDepth() == getDepth() &&
	   GLPixelUtil::getGLOriginFormat(dst.format) != 0)
	{
		// The direct case: the user wants the entire texture in a format supported by GL
		// so we don't need an intermediate buffer
		download(dst);
	}
	else
	{
		// Use buffer for intermediate copy
		allocateBuffer();
		// Download entire buffer
		download(mBuffer);
		if(srcBox.getWidth() != dst.getWidth() ||
			srcBox.getHeight() != dst.getHeight() ||
			srcBox.getDepth() != dst.getDepth())
		{
			// We need scaling
			Image::scale(mBuffer.getSubVolume(srcBox), dst, Image::FILTER_BILINEAR);
		}
		else
		{
			// Just copy the bit that we need
			PixelUtil::bulkPixelConversion(mBuffer.getSubVolume(srcBox), dst);
		}
		freeBuffer();
	}
}


};
