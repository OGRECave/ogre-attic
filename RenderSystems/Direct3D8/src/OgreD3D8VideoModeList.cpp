#include "OgreD3D8VideoModeList.h"

#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre {

	D3D8VideoModeList::D3D8VideoModeList( D3D8Driver* pDriver )
	{
		if( NULL == pDriver )
			Except( Exception::ERR_INVALIDPARAMS, "pDriver parameter is NULL", "D3D8VideoModeList::D3D8VideoModeList" );

		mpDriver = pDriver;

		enumerate();
	}

	D3D8VideoModeList::~D3D8VideoModeList()
	{
		mpDriver = NULL;
		//std::vector<D3D8VideoMode>::iterator it = mModeList.begin();
		//while( it != mModeList.end() )
		//{
		//	delete it;
		//	it.erase();
		//}
		mModeList.clear();
	}

	BOOL D3D8VideoModeList::enumerate()
	{
		LPDIRECT3D8 pD3D = mpDriver->getD3D();
		UINT adapter = mpDriver->getAdapterNumber();

		for( UINT iMode=0; iMode < pD3D->GetAdapterModeCount( adapter ); iMode++ )
		{
			D3DDISPLAYMODE displayMode;
			pD3D->EnumAdapterModes( adapter, iMode, &displayMode );

			// Filter out low-resolutions
			if( displayMode.Width < 640 || displayMode.Height < 400 )
				continue;

			// Check to see if it is already in the list (to filter out refresh rates)
			BOOL found = FALSE;
			std::vector<D3D8VideoMode>::iterator it;
			for( it = mModeList.begin(); it != mModeList.end(); it++ )
			{
				if( it->getWidth() == displayMode.Width &&
					it->getHeight() == displayMode.Height &&
					it->getFormat() == displayMode.Format )
				{
					found = TRUE;
					break;
				}
			}

			if( !found )
				mModeList.push_back( D3D8VideoMode( displayMode ) );
		}

		return TRUE;
	}

	unsigned int D3D8VideoModeList::count()
	{
		return static_cast< unsigned int >( mModeList.size() );
	}

	D3D8VideoMode* D3D8VideoModeList::item( int index )
	{
		std::vector<D3D8VideoMode>::iterator p = mModeList.begin();

		return &p[index];
	}
}