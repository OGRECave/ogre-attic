#ifndef __D3D9TEXTUREMANAGER_H__
#define __D3D9TEXTUREMANAGER_H__

#include "OgreD3D9Prerequisites.h"

#include "OgreTextureManager.h"

#include "OgreNoMemoryMacros.h"
#include <d3d9.h>
#include "OgreMemoryMacros.h"

namespace Ogre 
{
	class D3D9TextureManager : public TextureManager
	{
	private:
		LPDIRECT3DDEVICE9 mpD3DDevice;

	public:
		D3D9TextureManager( LPDIRECT3DDEVICE9 pD3DDevice );
		~D3D9TextureManager();

		/// Creates a D3D9Texture resource
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