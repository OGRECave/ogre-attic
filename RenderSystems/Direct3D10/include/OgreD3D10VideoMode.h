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
#ifndef __D3D10VIDEOMODE_H__
#define __D3D10VIDEOMODE_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreString.h"

#include "OgreNoMemoryMacros.h"
#include <d3d10.h>
#include "OgreMemoryMacros.h"

namespace Ogre 
{
	static unsigned int modeCount = 0;

	class D3D10VideoMode
	{
	private:
		DXGI_OUTPUT_DESC mDisplayMode;
		DXGI_MODE_DESC mModeDesc;
		unsigned int modeNumber;

	public:
		D3D10VideoMode() { modeNumber = ++modeCount; ZeroMemory( &mDisplayMode, sizeof(DXGI_OUTPUT_DESC) );ZeroMemory( &mModeDesc, sizeof(DXGI_MODE_DESC) ); }
		D3D10VideoMode( const D3D10VideoMode &ob ) { modeNumber = ++modeCount; mDisplayMode = ob.mDisplayMode;mModeDesc=ob.mModeDesc; }
		D3D10VideoMode( DXGI_OUTPUT_DESC d3ddm,DXGI_MODE_DESC ModeDesc ) { modeNumber = ++modeCount; mDisplayMode = d3ddm;mModeDesc=ModeDesc; }
		~D3D10VideoMode()
		{
			modeCount--;
		}

		unsigned int getWidth() const { return mModeDesc.Width; }
		unsigned int getHeight() const { return mModeDesc.Height; }
		DXGI_FORMAT getFormat() const { return mModeDesc.Format; }
		DXGI_RATIONAL getRefreshRate() const { return mModeDesc.RefreshRate; }
		unsigned int getColourDepth() const;
		DXGI_OUTPUT_DESC getDisplayMode() const { return mDisplayMode; }
		DXGI_MODE_DESC getModeDesc() const { return mModeDesc; }
		void increaseRefreshRate(DXGI_RATIONAL rr) { mModeDesc.RefreshRate = rr; } 
		String getDescription() const;
	};
}
#endif
