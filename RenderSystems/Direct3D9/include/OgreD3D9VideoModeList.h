#ifndef __D3D9VIDEOMODELIST_H__
#define __D3D9VIDEOMODELIST_H__

#include "OgreD3D9Prerequisites.h"

#include "OgreD3D9Driver.h"
#include "OgreD3D9VideoMode.h"

namespace Ogre 
{
	class D3D9VideoModeList
	{
	private:
		D3D9Driver* mpDriver;
		std::vector<D3D9VideoMode> mModeList;

	public:
		D3D9VideoModeList( D3D9Driver* pDriver );
		~D3D9VideoModeList();

		BOOL enumerate();

		D3D9VideoMode* item( int index );
		unsigned int count();
	};
}
#endif