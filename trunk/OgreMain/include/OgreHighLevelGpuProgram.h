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
#ifndef __HighLevelGpuProgram_H__
#define __HighLevelGpuProgram_H__

#include "OgrePrerequisites.h"
#include "OgreResource.h"
#include "OgreGpuProgram.h"
#include "OgreStringInterface.h"

namespace Ogre {

    /** Abstract base class representing a high-level program (a vertex or
        fragment program).
    @remarks
        High-level programs are vertex and fragment programs written in a high-level
        language such as Cg or HLSL, and as such do not require you to write assembler code
        like GpuProgram does. However, the high-level program does eventually 
        get converted (compiled) into assembler and then eventually microcode which is
        what runs on the GPU. As well as the convenience, some high-level languages like Cg allow
        you to write a program which will operate under both Direct3D and OpenGL, something
        which you cannot do with just GpuProgram (which requires you to write 2 programs and
        use each in a Technique to provide cross-API compatibility). Ogre will be creating
        a GpuProgram for you based on the high-level program, which is compiled specifically 
        for the API being used at the time, but this process is transparent.
    @par
        You cannot create high-level programs direct - use HighLevelGpuProgramManager instead.
        Plugins can register new implementations of HighLevelGpuProgramFactory in order to add
        support for new languages without requiring changes to the core Ogre API. To allow 
        custom parameters to be set, this class extends StringInterface - the application
        can query on the available custom parameters and get/set them without having to 
        link specifically with it.
    */
    class _OgreExport HighLevelGpuProgram : public GpuProgram, public StringInterface
    {
    protected:
        /// Whether the high-level program (and it's parameter defs) is loaded
        bool mHighLevelLoaded;
        /// The underlying assembler program
        GpuProgram* mAssemblerProgram;

        /** Internal load implementation, loads just the high-level portion, enough to 
            get parameters.
        */
        virtual void loadHighLevelImpl(void);
        /** Internal method for creating an appropriate low-level program from this
        high-level program, must be implemented by subclasses. */
        virtual void createLowLevelImpl(void) = 0;
        /// Internal unload implementation, must be implemented by subclasses
        virtual void unloadImpl(void) = 0;
        /// Populate the passed parameters with name->index map, must be overridden
        virtual void populateParameterNames(GpuProgramParametersSharedPtr params) = 0;
    public:
        /** Constructor, should be used only by factory classes. */
        HighLevelGpuProgram(const String& name, GpuProgramType gpType, const String& language);
        ~HighLevelGpuProgram();

        /** @copydoc Resource::unload */
        void load();
        /** @copydoc Resource::unload */
        void unload();

        /** Creates a new parameters object from this program definition. 
        @remarks
            Unlike low-level assembly programs, parameters objects are specific to the
            program and therefore must be created from it rather than by the 
            HighLevelGpuProgramManager. This method creates a new instance of a parameters
            object containing the definition of the parameters this program understands.
        */
        GpuProgramParametersSharedPtr createParameters(void);
        /** @copydoc GpuProgram::getBindingDelegate */
        GpuProgram* _getBindingDelegate(void) { return mAssemblerProgram; }



    };
}
#endif
