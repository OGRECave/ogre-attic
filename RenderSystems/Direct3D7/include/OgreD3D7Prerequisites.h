/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/

#ifndef _D3DPrerequisites_H__
#define _D3DPrerequisites_H__

#include "OgrePrerequisites.h"

// Define versions for if DirectX is in use (Win32 only)
#define DIRECT3D_VERSION  0x0700
#define DIRECTINPUT_VERSION 0x0700
#define DIRECTDRAW_VERSION 0x0700

namespace Ogre {

    // Predefine classes
    class D3DDevice;
    class D3DDeviceList;
    class D3DRenderSystem;
    class D3DTexture;
    class D3DTextureManager;
    class DDDriver;
    class DDDriverList;
    class DDVideoMode;
    class DDVideoModeList;

//-----------------------------------------------------------------------
// Windows Settings
//-----------------------------------------------------------------------
#if OGRE_PLATFORM == PLATFORM_WIN32
#   if OGRE_DYNAMIC_LINKAGE == 0
#       pragma warn( "No dynamic linkage" )
#       define _OgreD3DExport
#   else
#       ifdef OGRED3DENGINEDLL_EXPORTS
#           define _OgreD3DExport __declspec(dllexport)
#       else
#           define _OgreD3DExport __declspec(dllimport)
#       endif
#   endif
#endif // OGRE_WIN32


}


#endif
