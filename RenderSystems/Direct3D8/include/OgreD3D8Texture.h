#ifndef __D3D8TEXTURE_H__
#define __D3D8TEXTURE_H__

#include "OgreD3D8Prerequisites.h"
#include "OgreTexture.h"

#include "OgreNoMemoryMacros.h"
#include <d3d8.h>
#include "OgreMemoryMacros.h"

namespace Ogre {

	class D3D8Texture : public Texture
	{
	protected:
		LPDIRECT3DDEVICE8	mpD3DDevice;
		LPDIRECT3DTEXTURE8	mpTexture;
		LPDIRECT3DTEXTURE8  mpTempTexture;	// This is just a temporary texture to create the real one from

		void createTexture();
		void copyMemoryToTexture( unsigned char* pBuffer );
		void getColourMasks( D3DFORMAT format, DWORD* pdwRed, DWORD* pdwGreen, DWORD* pdwBlue, DWORD* pdwAlpha, DWORD* pdwRGBBitCount );

	public:
		D3D8Texture( String name, LPDIRECT3DDEVICE8 pD3DDevice );
		virtual ~D3D8Texture();

        void blitToTexture( const Image &src, unsigned uStartX, unsigned uStartY );

		void load();
		void loadImage( const Image &img );
		void unload();

		LPDIRECT3DTEXTURE8 getD3DTexture() { return mpTexture; }
	};

}

#endif