/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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

#include "OgreGLPixelFormat.h"

namespace Ogre  {
	//-----------------------------------------------------------------------------
    GLenum GLPixelUtil::getGLOriginFormat(PixelFormat mFormat)
    {
        switch(mFormat)
        {
            case PF_L8:
                return GL_LUMINANCE;
            case PF_L16:
                return GL_LUMINANCE;
#if OGRE_ENDIAN == ENDIAN_BIG
            // Formats are in native endian, so R8G8B8 on little endian is
            // BGR, on big endian it is RGB.
            case PF_R8G8B8:
                return GL_RGB;
            case PF_B8G8R8:
                return GL_BGR;
#else
            case PF_R8G8B8:
                return GL_BGR;
            case PF_B8G8R8:
                return GL_RGB;
#endif
			case PF_X8R8G8B8:
			case PF_A8R8G8B8:
				return GL_BGRA;
			case PF_X8B8G8R8:
            case PF_A8B8G8R8:
                return GL_RGBA;
            case PF_B8G8R8A8:
                return GL_BGRA;
			case PF_R8G8B8A8:
				return GL_RGBA;
            case PF_A2R10G10B10:
                return GL_BGRA;
            case PF_A2B10G10R10:
                return GL_RGBA;
            case PF_FP_R16G16B16:
                return GL_RGB;
            case PF_FP_R16G16B16A16:
                return GL_RGBA;
            case PF_FP_R32G32B32:
                return GL_RGB;
            case PF_FP_R32G32B32A32:
                return GL_RGBA;
            case PF_DXT1:
                return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            case PF_DXT3:
                 return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            case PF_DXT5:
                 return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            default:
                return 0;
        }
    }
	//----------------------------------------------------------------------------- 
    GLenum GLPixelUtil::getGLOriginDataType(PixelFormat mFormat)
    {
        switch(mFormat)
        {
            case PF_L8:
            case PF_R8G8B8:
            case PF_B8G8R8:
                return GL_UNSIGNED_BYTE;
            case PF_L16:
                return GL_UNSIGNED_SHORT;
            
#if OGRE_ENDIAN == ENDIAN_BIG
			case PF_X8B8G8R8:
			case PF_A8B8G8R8:
                return GL_UNSIGNED_INT_8_8_8_8_REV;
			case PF_X8R8G8B8:
            case PF_A8R8G8B8:
				return GL_UNSIGNED_INT_8_8_8_8_REV;
            case PF_B8G8R8A8:
                return GL_UNSIGNED_BYTE;
			case PF_R8G8B8A8:
				return GL_UNSIGNED_BYTE;
#else
			case PF_X8B8G8R8:
			case PF_A8B8G8R8:
                return GL_UNSIGNED_BYTE;
			case PF_X8R8G8B8:
            case PF_A8R8G8B8:
				return GL_UNSIGNED_BYTE;
            case PF_B8G8R8A8:
                return GL_UNSIGNED_INT_8_8_8_8;
			case PF_R8G8B8A8:
				return GL_UNSIGNED_INT_8_8_8_8;
#endif
            case PF_A2R10G10B10:
                return GL_UNSIGNED_INT_2_10_10_10_REV;
            case PF_A2B10G10R10:
                return GL_UNSIGNED_INT_2_10_10_10_REV;
            case PF_FP_R16G16B16:
            case PF_FP_R16G16B16A16:
                return 0; // GL_HALF_FLOAT_ARB -- nyi
            case PF_FP_R32G32B32:
            case PF_FP_R32G32B32A32:
                return GL_FLOAT;
            default:
                return 0;
        }
    }

    GLenum GLPixelUtil::getClosestGLInternalFormat(PixelFormat mFormat)
    {
        switch(mFormat) {
            case PF_L8:
                return GL_LUMINANCE8;
            case PF_L16:
                return GL_LUMINANCE16;
            case PF_A8:
                return GL_ALPHA8;
            case PF_A4L4:
            case PF_L4A4:
                return GL_LUMINANCE4_ALPHA4;
            case PF_R5G6B5:
            case PF_B5G6R5:
                return GL_RGB5;
            case PF_A4R4G4B4:
            case PF_B4G4R4A4:
                return GL_RGBA4;
            case PF_R8G8B8:
            case PF_B8G8R8:
			case PF_X8B8G8R8:
			case PF_X8R8G8B8:
                return GL_RGB8;
            case PF_A8R8G8B8:
            case PF_B8G8R8A8:
                return GL_RGBA8;
            case PF_A2R10G10B10:
            case PF_A2B10G10R10:
                return GL_RGB10_A2;
            case PF_FP_R16G16B16:
                return GL_RGB_FLOAT16_ATI;
                //    return GL_RGB16F_ARB;
            case PF_FP_R16G16B16A16:
                return GL_RGBA_FLOAT16_ATI;
                //    return GL_RGBA16F_ARB;
            case PF_FP_R32G32B32:
                return GL_RGB_FLOAT32_ATI;
                //    return GL_RGB32F_ARB;
            case PF_FP_R32G32B32A32:
                return GL_RGBA_FLOAT32_ATI;
                //    return GL_RGBA32F_ARB;
            case PF_DXT1:
                return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            case PF_DXT3:
                return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            case PF_DXT5:
                return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            default:
                return GL_RGBA8;
        }
    }
	
	//----------------------------------------------------------------------------- 	
	PixelFormat GLPixelUtil::getClosestOGREFormat(GLenum fmt)
	{
		switch(fmt) 
		{
		case GL_LUMINANCE8:
			return PF_L8;
		case GL_LUMINANCE16:
			return PF_L16;
		case GL_ALPHA8:
			return PF_A8;
		//case GL_LUMINANCE4_ALPHA4:
		//	return PF_A8L8; // TODO luminance alpha formats for GL
		case GL_RGB5:
			return PF_R5G6B5;
		case GL_RGBA4:
			return PF_A4R4G4B4;
		case GL_RGB8:
			return PF_X8R8G8B8;
		case GL_RGBA8:
			return PF_A8R8G8B8;
		case GL_RGB10_A2:
			return PF_A2R10G10B10;
		case GL_RGB_FLOAT16_ATI: // GL_RGB16F_ARB
		case GL_RGBA_FLOAT16_ATI:
		case GL_RGB_FLOAT32_ATI:
		case GL_RGBA_FLOAT32_ATI:
			return PF_FP_R32G32B32A32;
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			return PF_DXT1;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			return PF_DXT3;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			return PF_DXT5;
		default:
			return PF_A8R8G8B8;
		};
	}
	//----------------------------------------------------------------------------- 

	size_t GLPixelUtil::getMaxMipmaps(size_t width, size_t height, size_t depth, PixelFormat format)
	{
		size_t count = 0;
		do {
			width = (width+1)/2;
			height = (height+1)/2;
			depth = (depth+1)/2;
			if(PixelUtil::isValidExtent(width, height, depth, format))
				count ++;
			else
				break;
		} while(!(width == 1 && height == 1 && depth == 1));
		
		return count;
	}
	
};
