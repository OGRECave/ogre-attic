#include "OgreD3D8TextureManager.h"
#include "OgreD3D8Texture.h"

#include "OgreException.h"

#include "dxutil.h"

namespace Ogre {

	D3D8TextureManager::D3D8TextureManager( LPDIRECT3DDEVICE8 pD3DDevice ) : TextureManager()
	{
		mpD3DDevice = pD3DDevice;
		if( !mpD3DDevice )
			Except( 999, "Invalid Direct3DDevice passed", "D3D8TextureManager::D3D8TextureManager" );

		mpD3DDevice->AddRef();
	}

	D3D8TextureManager::~D3D8TextureManager()
	{
		this->unloadAndDestroyAll();
		SAFE_RELEASE( mpD3DDevice );
	}

	Resource* D3D8TextureManager::create( const String& name )
	{
		D3D8Texture* t = new D3D8Texture( name, mpD3DDevice, TU_DEFAULT );
		t->enable32Bit( mIs32Bit );
		return t;
	}

    Texture * D3D8TextureManager::createAsRenderTarget( const String& name )
    {
        D3D8Texture * newTex = new D3D8Texture( name, mpD3DDevice, TU_RENDERTARGET  );
        newTex->enable32Bit( mIs32Bit );
        return newTex;
    }

	Texture * D3D8TextureManager::createManual( 
		const String & name,
		uint width,
		uint height,
		uint num_mips,
		PixelFormat format,
		TextureUsage usage )
	{
		return new D3D8Texture( name, mpD3DDevice, width, height, num_mips, format, usage );
	}

	void D3D8TextureManager::unloadAndDestroyAll()
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