#include "OgreD3D9DriverList.h"

#include "OgreLogManager.h"
#include "OgreException.h"

#include "dxutil.h"

namespace Ogre 
{
	D3D9DriverList::D3D9DriverList( LPDIRECT3D9 pD3D ) : mpD3D(pD3D)
	{
		if( !mpD3D )
			Except( Exception::ERR_INVALIDPARAMS, "Direct3D9 interface pointer is NULL", "D3D9DriverList::D3D9DriverList" );
		mpD3D->AddRef();

		enumerate();
	}

	D3D9DriverList::~D3D9DriverList(void)
	{
		mDriverList.clear();
		SAFE_RELEASE( mpD3D );
	}

	BOOL D3D9DriverList::enumerate()
	{
		LogManager::getSingleton().logMessage( "--- Direct3D9 Driver Detection Starts" );
		for( UINT iAdapter=0; iAdapter < mpD3D->GetAdapterCount(); iAdapter++ )
		{
			D3DADAPTER_IDENTIFIER9 adapterIdentifier;
			D3DDISPLAYMODE d3ddm;
			mpD3D->GetAdapterIdentifier( iAdapter, 0, &adapterIdentifier );
			mpD3D->GetAdapterDisplayMode( iAdapter, &d3ddm );

			mDriverList.push_back( D3D9Driver( mpD3D, iAdapter, adapterIdentifier, d3ddm ) );
		}

		LogManager::getSingleton().logMessage( "--- Direct3D9 Driver Detection Ends" );

		return TRUE;
	}

	unsigned int D3D9DriverList::count() const 
	{
		return static_cast< unsigned int >( mDriverList.size() );
	}

	D3D9Driver* D3D9DriverList::item( int index )
	{
		return &mDriverList.at( index );
	}
}