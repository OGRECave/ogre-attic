#ifndef __D3D9PREREQUISITES_H__
#define __D3D9PREREQUISITES_H__

#include "OgrePrerequisites.h"

// Define versions for if DirectX is in use (Win32 only)
#define DIRECT3D_VERSION 0x0900

namespace Ogre 
{
	// Predefine classes
	class D3D9RenderSystem;
	class D3D9Texture;
	class D3D9TextureManager;
	class D3D9Driver;
	class D3D9DriverList;
	class D3D9VideoMode;
	class D3D9VideoModeList;

	//-------------------------------------------
	// Windows setttings
	//-------------------------------------------
#if OGRE_PLATFORM == PLATFORM_WIN32
#	if OGRE_DYNAMIC_LINKAGE == 0
#		pragma warn( "No dynamic linkage" )
#		define _OgreD3D9Export
#	else
#		ifdef OGRED3DENGINEDLL_EXPORTS
#			define _OgreD3D9Export __declspec(dllexport)
#		else
#			define _OgreD3D9Export __declspec(dllimport)
#		endif
#	endif
#endif	// OGRE_WIN32
}
#endif