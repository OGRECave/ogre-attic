#include "OgreD3D8DriverList.h"

#include "OgreLogManager.h"
#include "OgreException.h"

#include "dxutil.h"

namespace Ogre {

	D3D8DriverList::D3D8DriverList( LPDIRECT3D8 pD3D ) : mpD3D(pD3D)
	{
		if( !mpD3D )
			Except( Exception::ERR_INVALIDPARAMS, "Direct3D interface pointer is NULL", "D3DDriverList::D3DDriverList" );
		mpD3D->AddRef();

		enumerate();
	}

	D3D8DriverList::~D3D8DriverList(void)
	{
		mDriverList.clear();
		SAFE_RELEASE( mpD3D );
	}

	BOOL D3D8DriverList::enumerate()
	{
		LogManager::getSingleton().logMessage( "--- Direct3D Driver Detection Starts" );
		for( UINT iAdapter=0; iAdapter < mpD3D->GetAdapterCount(); iAdapter++ )
		{
			D3DADAPTER_IDENTIFIER8 adapterIdentifier;
			D3DDISPLAYMODE d3ddm;
			mpD3D->GetAdapterIdentifier( iAdapter, D3DENUM_NO_WHQL_LEVEL, &adapterIdentifier );
			mpD3D->GetAdapterDisplayMode( iAdapter, &d3ddm );

			mDriverList.push_back( D3D8Driver( mpD3D, iAdapter, adapterIdentifier, d3ddm ) );
		}

		LogManager::getSingleton().logMessage( "--- Direct3D Driver Detection Ends" );

		return TRUE;
	}

	unsigned int D3D8DriverList::count() const 
	{
		return static_cast< unsigned int >( mDriverList.size() );
	}

	D3D8Driver* D3D8DriverList::item( int index )
	{
		return &mDriverList.at( index );
	}
}