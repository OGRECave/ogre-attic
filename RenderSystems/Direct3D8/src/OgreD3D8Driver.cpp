#include "OgreD3D8Driver.h"
#include "OgreLogManager.h"
#include "OgreD3D8VideoModeList.h"
#include "OgreD3D8VideoMode.h"
#include "dxutil.h"

namespace Ogre {

	D3D8Driver::D3D8Driver()
	{
		tempNo = ++driverCount;
		mpD3D = NULL;
		ZeroMemory( &mAdapterIdentifier, sizeof(mAdapterIdentifier) );
		ZeroMemory( &mDesktopDisplayMode, sizeof(mDesktopDisplayMode) );
		mpVideoModeList = NULL;
	}

	D3D8Driver::D3D8Driver( const D3D8Driver &ob )
	{
		tempNo = ++driverCount;
		mpD3D = ob.mpD3D;
		mpD3D->AddRef();
		mAdapterNumber = ob.mAdapterNumber;
		mAdapterIdentifier = ob.mAdapterIdentifier;
		mDesktopDisplayMode = ob.mDesktopDisplayMode;
		mpVideoModeList = NULL;
	}

	D3D8Driver::D3D8Driver( LPDIRECT3D8 pD3D, unsigned int adapterNumber, D3DADAPTER_IDENTIFIER8 adapterIdentifier, D3DDISPLAYMODE desktopDisplayMode )
	{
		tempNo = ++driverCount;
		mpD3D = pD3D;
		mpD3D->AddRef();
		mAdapterNumber = adapterNumber;
		mAdapterIdentifier = adapterIdentifier;
		mDesktopDisplayMode = desktopDisplayMode;
		mpVideoModeList = NULL;
	}

	D3D8Driver::~D3D8Driver()
	{
		SAFE_DELETE( mpVideoModeList );
		SAFE_RELEASE( mpD3D );

		driverCount--;
		//LogManager::getSingleton().logMessage( LML_CRITICAL, "Driver %d removed from memory", tempNo );
	}

	String D3D8Driver::DriverName()
	{
		return String(mAdapterIdentifier.Driver);
	}

	String D3D8Driver::DriverDescription()
	{
		return String(mAdapterIdentifier.Description);
	}

	D3D8VideoModeList* D3D8Driver::getVideoModeList()
	{
		if( !mpVideoModeList )
			mpVideoModeList = new D3D8VideoModeList( this );

		return mpVideoModeList;
	}
}