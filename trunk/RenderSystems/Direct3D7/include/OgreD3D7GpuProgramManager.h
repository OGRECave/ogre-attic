/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __D3D7GpuProgramManager_H__
#define __D3D7GpuProgramManager_H__

#include "OgreD3D7Prerequisites.h"
#include "OgreGpuProgramManager.h"

namespace Ogre {

    /** Dummy implementation of GpuProgram - cannot do anything
        since D3D7 did not support vertex or fragment programs. */
    class D3D7GpuProgram : public GpuProgram
    {
    public:
        D3D7GpuProgram(const String& name, GpuProgramType gptype, const String& syntaxCode)
            : GpuProgram(name, gptype, syntaxCode) {}

    protected:
        /** Overridden from GpuProgram, do nothing */
        void loadFromSource(void) {}
    };
    /** Dummy implementation of GpuProgramManager - cannot do anything
        since D3D7 did not support vertex or fragment programs. */
    class D3D7GpuProgramManager : public GpuProgramManager
    {
    public:
        D3D7GpuProgramManager() {}
		~D3D7GpuProgramManager() {}
        /// @copydoc GpuProgramManager::createParameters
        GpuProgramParametersSharedPtr createParameters(void) 
        { return GpuProgramParametersSharedPtr(new GpuProgramParameters()); }
        /// @copydoc GpuProgramManager::create
        GpuProgram* create(const String& name, GpuProgramType gptype, const String& syntaxCode)
        { return new D3D7GpuProgram(name, gptype, syntaxCode); }
    };

}

#endif
