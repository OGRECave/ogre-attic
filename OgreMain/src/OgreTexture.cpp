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
	//--------------------------------------------------------------------------
    Texture::Texture(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader)
        : Resource(creator, name, handle, group, isManual, loader),
            // init defaults; can be overridden before load()
            mHeight(512),
            mWidth(512),
            mDepth(1),
            mNumMipMaps(0),
            mGamma(1.0f),
            mTextureType(TEX_TYPE_2D),            
            mFormat(PF_A8R8G8B8),
            mUsage(TU_DEFAULT),
            // mSrcBpp inited later on
            mSrcWidth(0),
            mSrcHeight(0)
            // mFinalBpp inited later on by enable32bit
            // mHasAlpha inited later on            
    {

        enable32Bit(false);

        if (createParamDictionary("Texture"))
        {
            // Define the parameters that have to be present to load
            // from a generic source; actually there are none, since when
            // predeclaring, you use a texture file which includes all the
            // information required.
        }

        
    }
    //--------------------------------------------------------------------------
	void Texture::loadRawData( DataStreamPtr& stream, 
		ushort uWidth, ushort uHeight, PixelFormat eFormat)
	{
		Image img;
		img.loadRawData(stream, uWidth, uHeight, eFormat);
		loadImage(img);
	}
    //--------------------------------------------------------------------------
    void Texture::setFormat(PixelFormat pf)
    {
        mFormat = pf;
        // This should probably change with new texture access methods, but
        // no changes made for now
        mSrcBpp = Image::PF2BPP(mFormat);
        mHasAlpha = Image::formatHasAlpha(mFormat);
    }
    //--------------------------------------------------------------------------
	size_t Texture::calculateSize(void) const
	{
		// TODO - how do we calculate real DDS texture size?
		return mWidth * mHeight * mDepth * mFinalBpp;
	}
  
}
