/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __D3D9PREREQUISITES_H__
#define __D3D9PREREQUISITES_H__

#include "OgrePrerequisites.h"

// Define versions for if DirectX is in use (Win32 only)
#define DIRECT3D_VERSION 0x0900

// some D3D commonly used macros
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


#undef NOMINMAX
#define NOMINMAX // required to stop windows.h screwing up std::min definition
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>


namespace Ogre
{
	// Predefine classes
	class D3D9RenderSystem;
	class D3D9RenderWindow;
	class D3D9Texture;
	class D3D9TextureManager;
	class D3D9Driver;
	class D3D9DriverList;
	class D3D9VideoMode;
	class D3D9VideoModeList;
	class D3D9GpuProgram;
	class D3D9GpuProgramManager;
    class D3D9HardwareBufferManager;
    class D3D9HardwareIndexBuffer;
    class D3D9HLSLProgramFactory;
    class D3D9HLSLProgram;
    class D3D9VertexDeclaration;

// Should we ask D3D to manage vertex/index buffers automatically?
// Doing so avoids lost devices, but also has a performance impact
// which is unacceptably bad when using very large buffers
#define OGRE_D3D_MANAGE_BUFFERS 1

    //-------------------------------------------
	// Windows setttings
	//-------------------------------------------
#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(OGRE_STATIC_LIB)
#	ifdef OGRED3DENGINEDLL_EXPORTS
#		define _OgreD3D9Export __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define _OgreD3D9Export
#       else
#    		define _OgreD3D9Export __declspec(dllimport)
#       endif
#	endif
#else
#	define _OgreD3D9Export
#endif	// OGRE_WIN32
}
#endif
