/*
*/

#ifndef __D3D8DEVICELIST_H__
#define __D3D8DEVICELIST_H__

// Precompiler options
#include "OgreD3D8Prerequisites.h"

namespace Ogre {

	class /*_OgreD3D8Export*/ D3D8Device;

	class /*_OgreD3D8Export*/ D3D8DeviceList
	{
	private:

		// STL list of drivers (Vector used to all random access)
		std::vector<D3D8Device> mDeviceList;

	public:
		D3D8DeviceList(void);
		~D3D8DeviceList(void);
	};

}

#endif