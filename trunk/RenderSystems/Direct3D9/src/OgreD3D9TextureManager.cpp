#include "OgreD3D9TextureManager.h"
#include "OgreD3D9Texture.h"

#include "OgreException.h"

#include "dxutil.h"

namespace Ogre 
{
	D3D9TextureManager::D3D9TextureManager( LPDIRECT3DDEVICE9 pD3DDevice ) : TextureManager()
	{
		mpD3DDevice = pD3DDevice;
		if( !mpD3DDevice )
			Except( 999, "Invalid Direct3DDevice passed", "D3D9TextureManager::D3D9TextureManager" );

		mpD3DDevice->AddRef();
	}

	D3D9TextureManager::~D3D9TextureManager()
	{
		this->unloadAndDestroyAll();
		SAFE_RELEASE( mpD3DDevice );
	}

	Resource* D3D9TextureManager::create( const String& name )
	{
		D3D9Texture* t = new D3D9Texture( name, mpD3DDevice, TU_DEFAULT );
		t->enable32Bit( mIs32Bit );
		return t;
	}

	Texture * D3D9TextureManager::createAsRenderTarget( const String& name )
	{
		D3D9Texture * newTex = new D3D9Texture( name, mpD3DDevice, TU_RENDERTARGET  );
		newTex->enable32Bit( mIs32Bit );
		newTex->load();
		return newTex;
	}

	Texture * D3D9TextureManager::createManual( 
		const String & name,
		uint width,
		uint height,
		uint num_mips,
		PixelFormat format,
		TextureUsage usage )
	{
		return new D3D9Texture( name, mpD3DDevice, width, height, num_mips, format, usage );
	}

	void D3D9TextureManager::unloadAndDestroyAll()
	{
		// Unload & delete resources in turn
		for( ResourceMap::iterator i = mResources.begin(); i != mResources.end(); i++ )
		{
			i->second->unload();
			delete i->second;
		}
		mResources.clear();
	}
}