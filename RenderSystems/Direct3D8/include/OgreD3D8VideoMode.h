#ifndef __D3D8VIDEOMODE_H__
#define __D3D8VIDEOMODE_H__

#include "OgreD3D8Prerequisites.h"

#include "OgreString.h"
#include "OgreLogManager.h"

#include "OgreNoMemoryMacros.h"
#include <d3d8.h>
#include "OgreMemoryMacros.h"

namespace Ogre {

	static int modeCount = 0;

	class D3D8VideoMode
	{
	private:
		D3DDISPLAYMODE mDisplayMode;

		int modeNumber;

	public:
		D3D8VideoMode() { modeNumber = ++modeCount; ZeroMemory( &mDisplayMode, sizeof(D3DDISPLAYMODE) ); }
		D3D8VideoMode( const D3D8VideoMode &ob ) { modeNumber = ++modeCount; mDisplayMode = ob.mDisplayMode; }
		D3D8VideoMode( D3DDISPLAYMODE d3ddm ) { modeNumber = ++modeCount; mDisplayMode = d3ddm; }
		~D3D8VideoMode()
		{
			modeCount--;
			//LogManager::getSingleton().logMessage( LML_CRITICAL, "Mode %d removed from memory", modeNumber );
		}

		unsigned int getWidth() const { return mDisplayMode.Width; }
		unsigned int getHeight() const { return mDisplayMode.Height; }
		D3DFORMAT getFormat() const { return mDisplayMode.Format; }
		unsigned int getRefreshRate() const { return mDisplayMode.RefreshRate; }
		unsigned int getColourDepth() const;
		D3DDISPLAYMODE getDisplayMode() const { return mDisplayMode; }

		void increaseRefreshRate(unsigned int rr) { mDisplayMode.RefreshRate = rr; } 

		String getDescription();
	};

}

#endif