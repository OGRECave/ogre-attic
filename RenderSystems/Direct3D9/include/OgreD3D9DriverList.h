#ifndef __D3D9DRIVERLIST_H__
#define __D3D9DRIVERLIST_H__

#include "OgreD3D9Prerequisites.h"

#include "OgreD3D9Driver.h"

#include "OgreNoMemoryMacros.h"
#include <d3d9.h>
#include "OgreMemoryMacros.h"

namespace Ogre 
{
	class D3D9DriverList
	{
	private:
		std::vector<D3D9Driver> mDriverList;
		LPDIRECT3D9 mpD3D;

	public:
		D3D9DriverList( LPDIRECT3D9 pD3D );
		~D3D9DriverList();

		BOOL enumerate();
		unsigned int count() const;
		D3D9Driver* item( int index );
	};
}
#endif