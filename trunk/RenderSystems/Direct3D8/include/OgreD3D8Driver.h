#ifndef __D3D8DRIVER_H__
#define __D3D8DRIVER_H__

// Precomipler options
#include "OgreD3D8Prerequisites.h"

// Declaration
#include "OgreString.h"

#include "OgreNoMemoryMacros.h"
#include <d3d8.h>
#include <d3dx8.h>
#include <dxerr8.h>
#include "OgreMemoryMacros.h"

namespace Ogre {

	static int driverCount = 0;

	class D3D8VideoModeList;
	class D3D8VideoMode;

	class /*_OgreD3D8Export*/ D3D8Driver
	{
	private:
		LPDIRECT3D8 mpD3D;
		unsigned int mAdapterNumber;
		D3DADAPTER_IDENTIFIER8 mAdapterIdentifier;
		D3DDISPLAYMODE mDesktopDisplayMode;

		D3D8VideoModeList* mpVideoModeList;

		int tempNo;
	public:

		// Constructors
		D3D8Driver();						// Default
		D3D8Driver( const D3D8Driver &ob );	// Copy
		D3D8Driver( LPDIRECT3D8 pD3D, unsigned int adapterNumber, D3DADAPTER_IDENTIFIER8 adapterIdentifer, D3DDISPLAYMODE desktopDisplayMode );
		~D3D8Driver();

		// Information accessors
		String DriverName();
		String DriverDescription();

		LPDIRECT3D8 getD3D() { return mpD3D; }
		unsigned int getAdapterNumber() const { return mAdapterNumber; }
		D3DADAPTER_IDENTIFIER8 getAdapterIdentifier() const { return mAdapterIdentifier; }
		D3DDISPLAYMODE getDesktopMode() const { return mDesktopDisplayMode; }


		D3D8VideoModeList* getVideoModeList();
	};

}

#endif