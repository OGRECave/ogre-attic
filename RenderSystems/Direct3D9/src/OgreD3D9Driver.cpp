#include "OgreD3D9Driver.h"
#include "OgreLogManager.h"
#include "OgreD3D9VideoModeList.h"
#include "OgreD3D9VideoMode.h"
#include "dxutil.h"

namespace Ogre 
{
	D3D9Driver::D3D9Driver()
	{
		tempNo = ++driverCount;
		mpD3D = NULL;
		ZeroMemory( &mAdapterIdentifier, sizeof(mAdapterIdentifier) );
		ZeroMemory( &mDesktopDisplayMode, sizeof(mDesktopDisplayMode) );
		mpVideoModeList = NULL;
	}

	D3D9Driver::D3D9Driver( const D3D9Driver &ob )
	{
		tempNo = ++driverCount;
		mpD3D = ob.mpD3D;
		mpD3D->AddRef();
		mAdapterNumber = ob.mAdapterNumber;
		mAdapterIdentifier = ob.mAdapterIdentifier;
		mDesktopDisplayMode = ob.mDesktopDisplayMode;
		mpVideoModeList = NULL;
	}

	D3D9Driver::D3D9Driver( LPDIRECT3D9 pD3D, unsigned int adapterNumber, D3DADAPTER_IDENTIFIER9 adapterIdentifier, D3DDISPLAYMODE desktopDisplayMode )
	{
		tempNo = ++driverCount;
		mpD3D = pD3D;
		mpD3D->AddRef();
		mAdapterNumber = adapterNumber;
		mAdapterIdentifier = adapterIdentifier;
		mDesktopDisplayMode = desktopDisplayMode;
		mpVideoModeList = NULL;
	}

	D3D9Driver::~D3D9Driver()
	{
		SAFE_DELETE( mpVideoModeList );
		SAFE_RELEASE( mpD3D );

		driverCount--;
	}

	String D3D9Driver::DriverName()
	{
		return String(mAdapterIdentifier.Driver);
	}

	String D3D9Driver::DriverDescription()
	{
		return String(mAdapterIdentifier.Description);
	}

	D3D9VideoModeList* D3D9Driver::getVideoModeList()
	{
		if( !mpVideoModeList )
			mpVideoModeList = new D3D9VideoModeList( this );

		return mpVideoModeList;
	}
}