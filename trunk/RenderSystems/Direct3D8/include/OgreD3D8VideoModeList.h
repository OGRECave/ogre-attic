#ifndef __D3D8VIDEOMODELIST_H__
#define __D3D8VIDEOMODELIST_H__

#include "OgreD3D8Prerequisites.h"

#include "OgreD3D8Driver.h"
#include "OgreD3D8VideoMode.h"

namespace Ogre {

	class D3D8VideoModeList
	{
	private:
		D3D8Driver* mpDriver;

		std::vector<D3D8VideoMode> mModeList;

	public:
		D3D8VideoModeList( D3D8Driver* pDriver );
		~D3D8VideoModeList();

		BOOL enumerate();

		D3D8VideoMode* item( int index );
		unsigned int count();
	};

}

#endif