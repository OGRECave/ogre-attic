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
#include "OgreSharedPtr.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

	/** Enumerates the types of programs which can run on the GPU. */
	enum GpuProgramType
	{
		GPT_VERTEX_PROGRAM,
		GPT_FRAGMENT_PROGRAM
	};

    
    /** Collects together the program parameters used for a GpuProgram.
    @remarks
        Gpu program state includes constant parameters used by the program, and
        bindings to render system state which is propagated into the constants 
        by the engine automatically if requested.
    @par
        GpuProgramParameters objects should be created through the GpuProgramManager and
        may be shared between multiple GpuProgram instances. For this reason they
        are managed using a shared pointer, which will ensure they are automatically
        deleted when no program is using them anymore.
    */
    class _OgreExport GpuProgramParameters
    {
    public:
        /** Defines the types of automatically updated values that may be bound to GpuProgram
        parameters, or used to modify parameters on a per-object basis.
        */
        enum AutoConstantType
        {
            /// The current world matrix
            ACT_WORLD_MATRIX,
            /// The current world & view matrices concatenated
            ACT_WORLDVIEW_MATRIX,
            /// The current world matrix, inverted
            ACT_INVERSE_WORLD_MATRIX,
            /// The current world & view matrices concatenated, then inverted
            ACT_INVERSE_WORLDVIEW_MATRIX,
            /// A light position in object space (index determined by setAutoConstant call)
            ACT_LIGHT_POSITION_OBJECT_SPACE,
            /// A light direction in object space (index determined by setAutoConstant call)
            ACT_LIGHT_DIRECTION_OBJECT_SPACE,
            /// The current camera's position in object space 
            ACT_CAMERA_POSITION_OBJECT_SPACE
        };
        /** Structure recording the use of an automatic parameter. */
        class AutoConstantEntry
        {
        public:
            /// The type of parameter
            AutoConstantType paramType;
            /// The target index
            size_t index;
            /// Additional information to go with the parameter
            size_t data;

            AutoConstantEntry(AutoConstantType theType, size_t theIndex, size_t theData)
                : paramType(theType), index(theIndex), data(theData) {}

        };
    protected:
        // Constant lists
        typedef std::vector<Real> RealConstantList;
        typedef std::vector<int> IntConstantList;
        // Auto parameter storage
        typedef std::vector<AutoConstantEntry> AutoConstantList;
        /// Packed list of floating-point constants
        RealConstantList mRealConstants;
        /// Packed list of integer constants
        IntConstantList mIntConstants;
        /// List of automatically updated parameters
        AutoConstantList mAutoConstants;
    public:
		GpuProgramParameters() {}
		virtual ~GpuProgramParameters() {}

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
		virtual void setConstant(size_t index, const Real *val, size_t count);
		
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
		virtual void setConstant(size_t index, const int *val, size_t count);

        /** Deletes the contents of the Real constants registers. */
        virtual void resetRealConstants(void) { mRealConstants.clear(); }
        /** Deletes the contents of the int constants registers. */
        virtual void resetIntConstants(void) { mIntConstants.clear(); }

        /// Gets a pointer to the array of Real constants
        virtual const Real* getRealConstantPointer(void) const { return &mRealConstants[0]; }
        /// Gets a pointer to the array of int constants
        virtual const int* getIntConstantPointer(void) const { return &mIntConstants[0]; }
        /// Gets the number of Real constants that have been set
        virtual size_t getRealConstantCount(void) const { return mRealConstants.size(); }
        /// Gets the number of int constants that have been set
        virtual size_t getIntConstantCount(void) const { return mIntConstants.size(); }
        /// Returns true if there are any Real constants contained here
        virtual bool hasRealConstantParams(void) const { return !(mRealConstants.empty()); }
        /// Returns true if there are any int constants contained here
        virtual bool hasIntConstantParams(void) const { return !(mIntConstants.empty()); }

        /** Sets up a constant which will automatically be updated by the system.
        @remarks
            Vertex and fragment programs often need parameters which are to do with the
            current render state, or particular values which may very well change over time,
            and often between objects which are being rendered. This feature allows you 
            to set up a certain number of predefined parameter mappings that are kept up to 
            date for you.
        @param acType The type of automatic constant to set
        @param index The location in the constant list to place this updated constant every time
            it is changed. Note that because of the nature of the types, we know how big the 
            parameter details will be so you don't need to set that like you do for manual constants.
        @param extraInfo If the constant type needs more information (like a light index) put it here.
        */
        virtual void setAutoConstant(AutoConstantType acType, size_t index, size_t extraInfo = 0);
        /** Clears all the existing automatic constants. */
        virtual void clearAutoConstants(void);
        typedef VectorIterator<AutoConstantList> AutoConstantIterator;
        /** Gets an iterator over the automatic constant bindings currently in place. */
        virtual AutoConstantIterator getAutoConstantIterator(void);
        /** Returns true if this instance has any automatic constants. */
        virtual bool hasAutoConstants(void){ return !(mAutoConstants.empty()); }
    };

    /// Shared pointer used to hold references to GpuProgramParameters instances
    typedef SharedPtr<GpuProgramParameters> GpuProgramParametersSharedPtr;

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
        /// Syntax code eg arbvp1, vs_2_0 etc
        String mSyntaxCode;

	public:

		GpuProgram(const String& name, GpuProgramType gptype, const String& syntaxCode);
		virtual ~GpuProgram() {}

        /** Sets the source assembly for this program.
        @remarks
            You should not need to use this method - use the GpuProgramManager::load or
            GpuProgramManager::create methods instead, which will call this method. 
            Setting this will have no effect unless you reload the program in any case.
        */
        virtual void setSource(const String& source);

        /** Gets the syntax code for this program e.g. arbvp1, fp20, vs_1_1 etc */
        virtual const String& getSyntaxCode(void) const { return mSyntaxCode; }

        /** Gets the assembler source for this program. */
        virtual const String& getSource(void) const { return mSource; }
        /// Get the program type
        virtual GpuProgramType getType(void) const { return mType; }

        /// @copydoc Resource::load
        void load(void);

    protected:
        /// Virtual method which must be implemented by subclasses, load from mSource
        virtual void loadFromSource(void) = 0;

	};


}

#endif
