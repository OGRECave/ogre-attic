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
#include "OgreStableHeaders.h"

#include "OgreTexture.h"

namespace Ogre {
	
    Texture::Texture(const String& name, TextureType texType, uint width, uint height, uint depth, uint num_mips, PixelFormat format, TextureUsage usage):
            mHeight(height),
            mWidth(width),
            mDepth(1),
            mNumMipMaps(num_mips),
            mGamma(1.0f),
            mTextureType(texType),            
            mFormat(format),
            mUsage(usage),
            // mSrcBpp inited later on
            mSrcWidth(width),
            mSrcHeight(height)
            // mFinalBpp inited later on by enable32bit
            // mHasAlpha inited later on            
         {
        mName = name;
        mSrcBpp = Image::PF2BPP(mFormat);
        mHasAlpha = Image::PFHasAlpha(mFormat);

        enable32Bit(false);
        
    }
  
	void Texture::loadRawData( const DataChunk &pData, 
		ushort uWidth, ushort uHeight, PixelFormat eFormat)
	{
		Image img ;
		img.loadRawData( pData, uWidth, uHeight, eFormat);
		loadImage(img);
	}
  
}
