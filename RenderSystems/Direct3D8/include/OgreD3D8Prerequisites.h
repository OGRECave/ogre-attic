/*
Disclaimer
*/


#ifndef __D3D8PREREQUISITES_H__
#define __D3D8PREREQUISITES_H__

#include "OgrePrerequisites.h"

// Define versions for if DirectX is in use (Win32 only)
#define DIRECT3D_VERSION 0x0810
#define DIRECTINPUT_VERSION 0x0800

namespace Ogre {

	template< typename _Interf >
    inline void __safeRelease( _Interf **interf ) throw ()
    {
        if( *interf )
        {
            (*interf)->Release();
            (*interf) = NULL;
        }
    }

#define __d3dExcept( hr, func ) Except( Exception::ERR_RENDERINGAPI_ERROR, DXGetErrorDescription8( hr ), func );

	// Predefine classes
	class D3D8Device;
	class D3D8DeviceList;
	class D3D8RenderSystem;
	class D3D8Texture;
	class D3D8TextureManager;
	class D3D8Driver;
	class D3D8DriverList;
	class D3D8VideoMode;
	class D3D8VideoModeList;

//-------------------------------------------
// Windows setttings
//-------------------------------------------
#if OGRE_PLATFORM == PLATFORM_WIN32
#	if OGRE_DYNAMIC_LINKAGE == 0
#		pragma warn( "No dynamic linkage" )
#		define _OgreD3D8Export
#	else
#		ifdef OGRED3D8ENGINEDLL_EXPORTS
#			define _OgreD3D8Export __declspec(dllexport)
#		else
#			define _OgreD3D8Export __declspec(dllimport)
#		endif
#	endif

#endif	// OGRE_WIN32

}

#endif