#ifndef __D3D8TEXTURE_H__
#define __D3D8TEXTURE_H__

#include "OgreD3D8Prerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTarget.h"
#include "OgreRenderTexture.h"
#include "OgreRenderTargetListener.h"

#include "OgreNoMemoryMacros.h"
#include <d3d8.h>
#include <d3dx8.h>
#include <dxerr8.h>
#include "OgreMemoryMacros.h"

namespace Ogre {

	class D3D8Texture : public Texture
	{
	protected:
		LPDIRECT3DDEVICE8	mpD3DDevice;
		LPDIRECT3DTEXTURE8	mpTexture;          //< The actual texture surface
        LPDIRECT3DSURFACE8  mpRenderZBuffer;    //< The z-buffer for the render surface.
		LPDIRECT3DTEXTURE8  mpTempTexture;	    //< This is just a temporary texture to create the real one from
        bool m_bIsRenderTarget;

    protected:
		void createTexture();
		void copyMemoryToTexture( unsigned char* pBuffer );
		void getColourMasks( D3DFORMAT format, DWORD* pdwRed, DWORD* pdwGreen, DWORD* pdwBlue, DWORD* pdwAlpha, DWORD* pdwRGBBitCount );

	public:
		D3D8Texture( String name, LPDIRECT3DDEVICE8 pD3DDevice, TextureUsage usage );
		D3D8Texture( 
			String name, 
			IDirect3DDevice8 * device, 
			uint width, 
			uint height, 
			uint num_mips,
			PixelFormat format,
			TextureUsage usage );
		virtual ~D3D8Texture();

        virtual void blitToTexture( const Image &src, unsigned uStartX, unsigned uStartY );
		virtual void copyToTexture( Texture * target );

		virtual void load();
		virtual void loadImage( const Image &img );
		virtual void unload();

        virtual void getCustomAttribute( String name, void* pData );
        virtual void outputText( int x, int y, const String& text ) {}

		IDirect3DTexture8 * getD3DTexture() { return mpTexture; }
        IDirect3DSurface8 * getDepthStencil() { return mpRenderZBuffer; }
	};

    class D3D8RenderTexture : public RenderTexture
    {
    public:
        D3D8RenderTexture( const String & name, uint width, uint height )
            : RenderTexture( name, width, height )
        {
        }

        virtual void getCustomAttribute( String name, void* pData )
        {
            if( name == "DDBACKBUFFER" )
            {
                IDirect3DSurface8 ** pSurf = (IDirect3DSurface8 **)pData;

                ((D3D8Texture*)mPrivateTex)->getD3DTexture()->GetSurfaceLevel( 0, &(*pSurf) );
                (*pSurf)->Release();
                return;
            }
            else if( name == "D3DZBUFFER" )
            {
                IDirect3DSurface8 ** pSurf = (IDirect3DSurface8 **)pData;

                *pSurf = ((D3D8Texture*)mPrivateTex)->getDepthStencil();
                return;
            }
            else if( name == "DDFRONTBUFFER" )
            {
                IDirect3DSurface8 ** pSurf = (IDirect3DSurface8 **)pData;

                ((D3D8Texture*)mPrivateTex)->getD3DTexture()->GetSurfaceLevel( 0, &(*pSurf) );
                (*pSurf)->Release();
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

		bool requiresTextureFlipping() const { return true; }
        virtual void writeContentsToFile( const String & filename ) {}
        virtual void outputText(int x, int y, const String& text) {}

    protected:
        /// The texture to which rendering takes place.
        Texture * mPrivateTex;

    protected:
        virtual void _copyToTexture()
        {
            // Copy the newly-rendered data to the public texture surface.
            mPrivateTex->copyToTexture( mTexture );
        }
    };
}

#endif
