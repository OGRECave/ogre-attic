#ifndef __D3D8TEXTURE_H__
#define __D3D8TEXTURE_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTarget.h"
#include "OgreRenderTexture.h"
#include "OgreRenderTargetListener.h"

#include "OgreNoMemoryMacros.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

namespace Ogre {

	class D3D9Texture : public Texture
	{
	protected:
		LPDIRECT3DDEVICE9	mpD3DDevice;
		LPDIRECT3DTEXTURE9	mpTexture;          //< The actual texture surface
        LPDIRECT3DSURFACE9  mpRenderZBuffer;    //< The z-buffer for the render surface.
		LPDIRECT3DTEXTURE9  mpTempTexture;	    //< This is just a temporary texture to create the real one from
		D3DCAPS9			mCaps;

    protected:
		void createTexture();
		void copyMemoryToTexture( unsigned char* pBuffer );
		void getColourMasks( D3DFORMAT format, DWORD* pdwRed, DWORD* pdwGreen, DWORD* pdwBlue, DWORD* pdwAlpha, DWORD* pdwRGBBitCount );

	public:
		D3D9Texture( String name, LPDIRECT3DDEVICE9 pD3DDevice, TextureUsage usage );
		D3D9Texture( 
			String name, 
			IDirect3DDevice9 *device, 
			uint width, 
			uint height, 
			uint num_mips,
			PixelFormat format,
			TextureUsage usage );
		virtual ~D3D9Texture();

		virtual void blitImage(const Image& src, const Image::Rect imgRect, const Image::Rect texRect );
        virtual void blitToTexture( const Image &src, unsigned uStartX, unsigned uStartY );
		virtual void copyToTexture( Texture * target );

		virtual void load();
		virtual void loadImage( const Image &img );
		virtual void unload();

        virtual void getCustomAttribute( String name, void* pData );
        virtual void outputText( int x, int y, const String& text ) {}

		IDirect3DTexture9 * getD3DTexture() { return mpTexture; }
        IDirect3DSurface9 * getDepthStencil() { return mpRenderZBuffer; }
	};

    class D3D9RenderTexture : public RenderTexture
    {
    public:
        D3D9RenderTexture( const String & name, uint width, uint height )
            : RenderTexture( name, width, height )
        {
        }
		
		virtual void getCustomAttribute( String name, void* pData )
        {
			if( name == "DDBACKBUFFER" )
            {
                IDirect3DSurface9 ** pSurf = (IDirect3DSurface9 **)pData;
                ((D3D9Texture*)mPrivateTex)->getD3DTexture()->GetSurfaceLevel( 0, &(*pSurf) );
                (*pSurf)->Release();
                return;
            }
            else if( name == "D3DZBUFFER" )
            {
                IDirect3DSurface9 ** pSurf = (IDirect3DSurface9 **)pData;
                *pSurf = ((D3D9Texture*)mPrivateTex)->getDepthStencil();
                return;
            }
            else if( name == "DDFRONTBUFFER" )
            {
                IDirect3DSurface9 ** pSurf = (IDirect3DSurface9 **)pData;
                ((D3D9Texture*)mPrivateTex)->getD3DTexture()->GetSurfaceLevel( 0, &(*pSurf) );
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
    };
}

#endif