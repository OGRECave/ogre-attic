#include "OgreD3D8VideoMode.h"

namespace Ogre {

	String D3D8VideoMode::getDescription()
	{
		char tmp[128];
		unsigned int colourDepth = 16;
        if( mDisplayMode.Format == D3DFMT_X8R8G8B8 ||
            mDisplayMode.Format == D3DFMT_A8R8G8B8 ||
            mDisplayMode.Format == D3DFMT_R8G8B8 )
			colourDepth = 32;

		sprintf( tmp, "%d x %d @ %d-bit colour", mDisplayMode.Width, mDisplayMode.Height, colourDepth );
		return String(tmp);
	}

	unsigned int D3D8VideoMode::getColourDepth() const
	{
		unsigned int colourDepth = 16;
        if( mDisplayMode.Format == D3DFMT_X8R8G8B8 ||
            mDisplayMode.Format == D3DFMT_A8R8G8B8 ||
            mDisplayMode.Format == D3DFMT_R8G8B8 )
			colourDepth = 32;

		return colourDepth;
	}
}