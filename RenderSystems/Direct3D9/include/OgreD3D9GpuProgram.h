/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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
-----------------------------------------------------------------------------
*/
#ifndef __D3D9GpuProgram_H_
#define __D3D9GpuProgram_H_

// Precompiler options
#include "OgreD3D9Prerequisites.h"
#include "OgreGpuProgram.h"

namespace Ogre {

    /** Direct3D implementation of a few things common to low-level vertex & fragment programs. */
    class D3D9GpuProgram : public GpuProgram
    {
    protected:
        LPDIRECT3DDEVICE9 mpDevice;
    public:
        D3D9GpuProgram(const String& name, GpuProgramType gptype, const String& syntaxCode, LPDIRECT3DDEVICE9 pDev);

    };

    /** Direct3D implementation of low-level vertex programs. */
    class D3D9GpuVertexProgram : public D3D9GpuProgram
    {
    protected:
        LPDIRECT3DVERTEXSHADER9 mpVertexShader;
    public:
        D3D9GpuVertexProgram(const String& name, const String& syntaxCode, LPDIRECT3DDEVICE9 pDev);
        /// @copydoc Resource::unload
        void unload(void);

        /// Gets the vertex shader
        LPDIRECT3DVERTEXSHADER9 getVertexShader(void) const { return mpVertexShader; }
    protected:
        void loadFromSource(void);
    };

    /** Direct3D implementation of low-level fragment programs. */
    class D3D9GpuFragmentProgram : public D3D9GpuProgram
    {
    protected:
        LPDIRECT3DPIXELSHADER9 mpPixelShader;
    public:
        D3D9GpuFragmentProgram(const String& name, const String& syntaxCode, LPDIRECT3DDEVICE9 pDev);
        /// @copydoc Resource::unload
        void unload(void);
        /// Gets the pixel shader
        LPDIRECT3DPIXELSHADER9 getPixelShader(void) const { return mpPixelShader; }
    protected:
        void loadFromSource(void);
    };

}


#endif
