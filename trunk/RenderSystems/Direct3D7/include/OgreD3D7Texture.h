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
#ifndef _D3DTexture_H__
#define _D3DTexture_H__

#include "OgreD3D7Prerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTarget.h"
#include "OgreRenderTexture.h"
#include "OgreRenderTargetListener.h"
#include <ddraw.h>
#include <d3d.h>

namespace Ogre {

    /** Direct3D7-specific texture resource implementation. 
    */
    class D3DTexture : public Texture
    {
    public:
        // Constructor, called from D3DTextureManager
        D3DTexture(String name, LPDIRECT3DDEVICE7 lpDirect3dDevice, TextureUsage usage );
        virtual ~D3DTexture();

        virtual void load(void);
        virtual void loadImage( const Image &img );
        virtual void blitToTexture( const Image &src, unsigned uStartX, unsigned uStartY );        
        virtual void blitImage( const Image& src, 
            const Image::Rect imgRect, const Image::Rect texRect );
        virtual void unload(void);
		virtual void copyToTexture( Texture * target );

        /// D3D-specific member that returns the underlying surface.
        LPDIRECTDRAWSURFACE7 getDDSurface(void);

    protected:
        IDirect3DDevice7 * mD3DDevice;       ///< A pointer to the Direct3D device.
        IDirectDrawSurface7 * mSurface;      ///< Surface of the (first) device-specific texture.

    protected:
        void createSurface();
    };

    class D3D7RenderTexture : public RenderTexture
    {
    public:
        D3D7RenderTexture( const String & name, uint width, uint height )
        {
			mName = name;
			mWidth = width;
			mHeight = height;
			mPriority = 2;
			mTexture = TextureManager::getSingleton().createAsRenderTarget( mName );
			TextureManager::getSingleton().load( static_cast< Resource * >( mTexture ) );

			mPrivateTex = TextureManager::getSingleton().createAsRenderTarget( mName + "_PRIVATE##" );
        }

		bool requiresTextureFlipping() const { return true; }

        virtual void getCustomAttribute( String name, void* pData )
        {
            if( name == "DDBACKBUFFER" )
            {
                LPDIRECTDRAWSURFACE7 *pSurf = (LPDIRECTDRAWSURFACE7*)pData;

                *pSurf = ((D3DTexture*)mPrivateTex)->getDDSurface();
                return;
            }
            else if( name == "DDFRONTBUFFER" )
            {
                LPDIRECTDRAWSURFACE7 *pSurf = (LPDIRECTDRAWSURFACE7*)pData;

                *pSurf = ((D3DTexture*)mPrivateTex)->getDDSurface();
                return;
            }
            else if( name == "HWND" )
            {
                HWND *pHwnd = (HWND*)pData;

                *pHwnd = NULL;
                return;
            }
            else if( name == "isTexture" )
            {
                bool *b = reinterpret_cast< bool * >( pData );
                *b = true;

                return;
            }
        }
        virtual void writeContentsToFile( const String & filename ) {}
        virtual void outputText(int x, int y, const String& text) {}
    };
}

#endif
