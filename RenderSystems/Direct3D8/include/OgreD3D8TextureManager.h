#ifndef __D3D8TEXTUREMANAGER_H__
#define __D3D8TEXTUREMANAGER_H__

#include "OgreD3D8Prerequisites.h"

#include "OgreTextureManager.h"

#include "OgreNoMemoryMacros.h"
#include <d3d8.h>
#include "OgreMemoryMacros.h"

namespace Ogre {

	class D3D8TextureManager : public TextureManager
	{
	private:
		LPDIRECT3DDEVICE8 mpD3DDevice;

	public:
		D3D8TextureManager( LPDIRECT3DDEVICE8 pD3DDevice );
		~D3D8TextureManager();

		/// Creates a D3D8Texture resource
		virtual Resource * create( const String& name );
        virtual Texture * createAsRenderTarget( const String& name );

		virtual Texture * createManual( 
			const String & name,
			uint width,
			uint height,
			uint num_mips,
			PixelFormat format,
			TextureUsage usage );

		void unloadAndDestroyAll();
	};

}

#endif