/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef __D3D9HLSLProgram_H__
#define __D3D9HLSLProgram_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreHighLevelGpuProgram.h"

namespace Ogre {
    /** Specialisation of HighLevelGpuProgram to provide support for D3D9 
        High-Level Shader Language (HLSL).
    @remarks
        Note that the syntax of D3D9 HLSL is identical to nVidia's Cg language, therefore
        unless you know you will only ever be deploying on Direct3D, or you have some specific
        reason for not wanting to use the Cg plugin, I suggest you use Cg instead since that
        can produce programs for OpenGL too.
    */
    class D3D9HLSLProgram : public HighLevelGpuProgram
    {
    public:
        /// Command object for setting entry point
        class CmdEntryPoint : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /// Command object for setting target assembler
        class CmdTarget : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

    protected:

        static CmdEntryPoint msCmdEntryPoint;
        static CmdTarget msCmdTarget;

        /** Internal load implementation, must be implemented by subclasses.
        */
        void loadFromSource(void);
        /** Internal method for creating an appropriate low-level program from this
        high-level program, must be implemented by subclasses. */
        void createLowLevelImpl(void);
        /// Internal unload implementation, must be implemented by subclasses
        void unloadImpl(void);
        /// Populate the passed parameters with name->index map, must be overridden
        void populateParameterNames(GpuProgramParametersSharedPtr params);

        // Recursive utility method for populateParameterNames
        void processParamElement(D3DXHANDLE parent, String prefix, unsigned int index, GpuProgramParametersSharedPtr params);

        String mTarget;
        String mEntryPoint;

        LPD3DXBUFFER mpMicroCode;
        LPD3DXCONSTANTTABLE mpConstTable;

    public:
        D3D9HLSLProgram(const String& name, GpuProgramType gpType, const String& language);
        ~D3D9HLSLProgram();

        /** Sets the entry point for this program ie the first method called. */
        void setEntryPoint(const String& entryPoint) { mEntryPoint = entryPoint; }
        /** Gets the entry point defined for this program. */
        const String& getEntryPoint(void) { return mEntryPoint; }
        /** Sets the shader target to compile down to, e.g. 'vs_1_1'. */
        void setTarget(const String& target);
        /** Gets the shader target to compile down to, e.g. 'vs_1_1'. */
        const String& getTarget(void) { return mTarget; }
        /// Overridden from GpuProgram
        bool isSupported(void);
        /// Overridden from GpuProgram
        GpuProgramParametersSharedPtr createParameters(void);
    };
}

#endif
