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
#include "OgreD3D9TextureManager.h"
#include "OgreD3D9Texture.h"
#include "OgreException.h"

namespace Ogre 
{
	D3D9TextureManager::D3D9TextureManager( LPDIRECT3DDEVICE9 pD3DDevice ) : TextureManager()
	{
		mpD3DDevice = pD3DDevice;
		if( !mpD3DDevice )
			Except( Exception::ERR_INVALIDPARAMS, "Invalid Direct3DDevice passed", "D3D9TextureManager::D3D9TextureManager" );
	}

	D3D9TextureManager::~D3D9TextureManager()
	{
		this->unloadAndDestroyAll();
	}

	Texture *D3D9TextureManager::create( const String& name, TextureType texType )
	{
		D3D9Texture *t = new D3D9Texture( name, texType, mpD3DDevice, TU_DEFAULT );
		t->enable32Bit( mIs32Bit );
		return t;
	}

	Texture *D3D9TextureManager::createAsRenderTarget( const String& name )
	{
		D3D9Texture *newTex = new D3D9Texture( name, TEX_TYPE_2D, mpD3DDevice, TU_RENDERTARGET  );
		newTex->enable32Bit( mIs32Bit );
		newTex->load();
		return newTex;
	}

	Texture *D3D9TextureManager::createManual( 
		const String & name,
		TextureType texType,
		uint width,
		uint height,
		uint num_mips,
		PixelFormat format,
		TextureUsage usage )
	{
		return new D3D9Texture( name, texType, mpD3DDevice, width, height, num_mips, format, usage );
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