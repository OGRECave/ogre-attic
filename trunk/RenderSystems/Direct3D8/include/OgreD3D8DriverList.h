#ifndef __D3DDRIVERLIST_H__
#define __D3DDRIVERLIST_H__

#include "OgreD3D8Prerequisites.h"

#include "OgreD3D8Driver.h"

#include "OgreNoMemoryMacros.h"
#include <d3d8.h>
#include "OgreMemoryMacros.h"

namespace Ogre {

	class D3D8DriverList
	{
	private:
		std::vector<D3D8Driver> mDriverList;
		LPDIRECT3D8 mpD3D;

	public:
		D3D8DriverList( LPDIRECT3D8 pD3D );
		~D3D8DriverList();

		BOOL enumerate();
		unsigned int count() const;
		D3D8Driver* item( int index );
	};

}

#endif