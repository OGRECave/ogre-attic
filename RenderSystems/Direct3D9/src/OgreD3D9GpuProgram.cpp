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
#include "OgreD3D9GpuProgram.h"

namespace Ogre {

    //-----------------------------------------------------------------------------
	D3D9GpuProgram::D3D9GpuProgram(GpuProgramType gptype) : GpuProgram(gptype)
    {
    }
    //-----------------------------------------------------------------------------
    void D3D9GpuProgram::setConstant(size_t index, const Matrix3& m)
    {
    }
	//-----------------------------------------------------------------------------
	void D3D9GpuProgram::setConstant(size_t index, const Matrix4& m)
    {
    }
	//-----------------------------------------------------------------------------
    D3D9GpuVertexProgram::D3D9GpuVertexProgram(GpuProgramType gptype) : D3D9GpuProgram(gptype)
    {
        // do nothing here, all is done in load()
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuVertexProgram::load(void)
    {
        // Create the shader

    }
	//-----------------------------------------------------------------------------
    void D3D9GpuVertexProgram::unload(void)
    {
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuVertexProgram::setConstant(size_t index, const Real *val, size_t count)
    {
    }
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
    D3D9GpuFragmentProgram::D3D9GpuFragmentProgram(GpuProgramType gptype) : D3D9GpuProgram(gptype)
    {
        // do nothing here, all is done in load()
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuFragmentProgram::load(void)
    {
        // Create the shader

    }
	//-----------------------------------------------------------------------------
    void D3D9GpuFragmentProgram::unload(void)
    {
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuFragmentProgram::setConstant(size_t index, const Real *val, size_t count)
    {
    }
	//-----------------------------------------------------------------------------

}

