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

#include "OgreRenderTexture.h"
#include "OgreException.h"

namespace Ogre
{
	RenderTexture::RenderTexture( const String & name, uint width, uint height, TextureType texType )
	{
        if (texType != TEX_TYPE_2D &&
			texType != TEX_TYPE_CUBE_MAP)
		{
			Except( Exception::UNIMPLEMENTED_FEATURE, 
					"Render to texture is implemented only for 2D and cube textures !!!", 
					"RenderTexture::RenderTexture" );
		}

        mName = name;
        mWidth = width;
        mHeight = height;
        mPriority = OGRE_REND_TO_TEX_RT_GROUP;

		mTexture = TextureManager::getSingleton().createManual( mName, texType, mWidth, mHeight, 0, PF_R8G8B8, TU_RENDERTARGET );
		TextureManager::getSingleton().load( static_cast< Resource * >( mTexture ) );
    }

    RenderTexture::~RenderTexture()
    {
        TextureManager::getSingleton().unload(mName);
        delete mTexture;
    }

	void RenderTexture::firePostUpdate()
	{
		RenderTarget::firePostUpdate();

		_copyToTexture();
	}
}
