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
#ifndef __GpuProgram_H_
#define __GpuProgram_H_

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreResource.h"

namespace Ogre {

	/** Enumerates the types of programs which can run on the GPU. */
	enum GpuProgramType
	{
		GPT_VERTEX_PROGRAM,
		GPT_FRAGMENT_PROGRAM
	};
	/** Defines a program which runs on the GPU such as a vertex or fragment program. 
	@remarks
		This class defines the low-level program in assembler code, the sort used to
		directly assemble into machine instructions for the GPU to execute. By nature,
		this means that the assembler source is rendersystem specific, which is why this
		is an abstract class - real instances are created through the RenderSystem. 
		If you wish to use higher level shading languages like HLSL and Cg, you need to 
		use the HighLevelGpuProgram class instead.
	*/
	class _OgreExport GpuProgram : public Resource
	{
	protected:
		/// The type of the program
		GpuProgramType mType;
        /// The assembler source of the program
        String mSource;
        /// Whether we need to load source from file or not
        bool mLoadFromFile;

	public:
		GpuProgram(GpuProgramType gptype);

		/** Sets a single value constant floating-point parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @param
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
		@param index The index at which to place the parameter
		@param val The value to set
		*/
		virtual void setConstant(size_t index, Real val);
		/** Sets a single value constant integer parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @param
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
		@param index The index at which to place the parameter
		@param val The value to set
		*/
		virtual void setConstant(size_t index, int val);
		/** Sets a single value constant boolean parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @param
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
		@param index The index at which to place the parameter
		@param val The value to set
		*/
		virtual void setConstant(size_t index, bool val);
		/** Sets a Vector4 parameter to the program.
		@param index The index at which to place the parameter
			NB this index refers to the number of floats, so a Vector4 is 4. 
		@param vec The value to set
		*/
		virtual void setConstant(size_t index, const Vector4& vec);
		/** Sets a Vector3 parameter to the program.
		@param index The index at which to place the parameter
			NB this index refers to the number of floats, so a Vector3 is 3. 
		@param vec The value to set
		*/
		virtual void setConstant(size_t index, const Vector3& vec);
		/** Sets a Matrix4 parameter to the program.
		@param index The index at which to place the parameter
			NB this index refers to the number of floats, so a Matrix4 is 16. 
		@param m The value to set
		*/
		virtual void setConstant(size_t index, const Matrix4& m) = 0; // left to rendersystem since column / row vectors matter
		/** Sets a multiple value constant floating-point parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @param
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
		@param index The index at which to start placing parameters
		@param val Pointer to the values to write
		@param count The number of floats to write
		*/
		virtual void setConstant(size_t index, const Real *val, size_t count) = 0;
		
		/** Sets a multiple value constant integer parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @param
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
		@param index The index at which to start placing parameters
		@param val Pointer to the values to write
		@param count The number of integers to write
		*/
		virtual void setConstant(size_t index, const int *val, size_t count) = 0;

		/** Sets a multiple value constant boolean parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @param
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
		@param index The index at which to start placing parameters
		@param val Pointer to the values to write
		@param count The number of bools to write
		*/
		virtual void setConstant(size_t index, const bool *val, size_t count) = 0;

        /** Sets the source assembly for this program.
        @remarks
            You should not need to use this method - use the GpuProgramManager::load
            methods instead, which will call this method. Setting this will have no effect
            unless you reload the program in any case.
        */
        virtual void setSource(const String& source);

        /** Gets the assembler source for this program. */
        virtual const String& getSource(void) const { return mSource; }

        /// @copydoc Resource::load
        void load(void);

    protected:
        /// Virtual method which must be implemented by subclasses, load from mSource
        virtual void loadFromSource(void) = 0;

	};


}

#endif
