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
            /// The current array of world matrices, as a 3x4 matrix, used for blending
            ACT_WORLD_MATRIX_ARRAY_3x4,
            /// The current array of world matrices, used for blending
            ACT_WORLD_MATRIX_ARRAY,
            /// The current view matrix
            ACT_VIEW_MATRIX,
            /// The current projection matrix
            ACT_PROJECTION_MATRIX,
            /// The current view & projection matrices concatenated
            ACT_VIEWPROJ_MATRIX,
            /// The current world & view matrices concatenated
            ACT_WORLDVIEW_MATRIX,
            /// The current world, view & projection matrices concatenated
            ACT_WORLDVIEWPROJ_MATRIX,
            /// The current world matrix, inverted
            ACT_INVERSE_WORLD_MATRIX,
            /// The current world & view matrices concatenated, then inverted
            ACT_INVERSE_WORLDVIEW_MATRIX,
            /// Light diffuse colour (index determined by setAutoConstant call)
            ACT_LIGHT_DIFFUSE_COLOUR,
            /// Light diffuse colour (index determined by setAutoConstant call)
            ACT_LIGHT_SPECULAR_COLOUR,
            /// Light attenuation parameters, Vector4(range, constant, linear, quadric)
            ACT_LIGHT_ATTENUATION,
            /// A light position in world space (index determined by setAutoConstant call)
            ACT_LIGHT_POSITION,
            /// A light direction in world space (index determined by setAutoConstant call)
            ACT_LIGHT_DIRECTION,
            /// A light position in object space (index determined by setAutoConstant call)
            ACT_LIGHT_POSITION_OBJECT_SPACE,
            /// A light direction in object space (index determined by setAutoConstant call)
            ACT_LIGHT_DIRECTION_OBJECT_SPACE,
            /// The current camera's position in object space 
            ACT_CAMERA_POSITION_OBJECT_SPACE,
			/// The ambient light colour set in the scene
			ACT_AMBIENT_LIGHT_COLOUR, 
            /// The view/projection matrix of the assigned texture projection frustum
            ACT_TEXTURE_VIEWPROJ_MATRIX
        };
        /** Structure recording the use of an automatic parameter. */
        class AutoConstantEntry
        {
        public:
            /// The type of parameter
            AutoConstantType paramType;
            /// The target constant index
            size_t index;
            /// Additional information to go with the parameter
            size_t data;

            AutoConstantEntry(AutoConstantType theType, size_t theIndex, size_t theData)
                : paramType(theType), index(theIndex), data(theData) {}

        };
        /** Real parameter entry; contains both a group of 4 values and 
        an indicator to say if it's been set or not. This allows us to 
        filter out constant entries which have not been set by the renderer
        and may actually be being used internally by the program. */
        struct RealConstantEntry
        {
            Real val[4];
            bool isSet;
            RealConstantEntry() : isSet(false) {}
        };
        /** Int parameter entry; contains both a group of 4 values and 
        an indicator to say if it's been set or not. This allows us to 
        filter out constant entries which have not been set by the renderer
        and may actually be being used internally by the program. */
        struct IntConstantEntry
        {
            int val[4];
            bool isSet;
            IntConstantEntry() : isSet(false) {}
        };
    protected:
        // Constant lists
        typedef std::vector<RealConstantEntry> RealConstantList;
        typedef std::vector<IntConstantEntry> IntConstantList;
        // Auto parameter storage
        typedef std::vector<AutoConstantEntry> AutoConstantList;
        /// Packed list of floating-point constants
        RealConstantList mRealConstants;
        /// Packed list of integer constants
        IntConstantList mIntConstants;
        /// List of automatically updated parameters
        AutoConstantList mAutoConstants;
        /// Mapping from parameter names to indexes - high-level programs are expected to populate this
        typedef std::map<String, size_t> ParamNameMap;
        ParamNameMap mParamNameMap;
        /// Do we need to transpose matrices?
        bool mTransposeMatrices;

    public:
		GpuProgramParameters();
		~GpuProgramParameters() {}

		/** Sets a 4-element floating-point parameter to the program.
		@param index The constant index at which to place the parameter (each constant is
            a 4D float)
		@param vec The value to set
		*/
		void setConstant(size_t index, const Vector4& vec);
		/** Sets a 4-element floating-point parameter to the program via Vector3.
		@param index The constant index at which to place the parameter (each constant is
            a 4D float).
            Note that since you're passing a Vector3, the last element of the 4-element
            value will be set to 1 (a homogenous vector)
		@param vec The value to set
		*/
		void setConstant(size_t index, const Vector3& vec);
		/** Sets a Matrix4 parameter to the program.
		@param index The constant index at which to place the parameter (each constant is
            a 4D float).
            NB since a Matrix4 is 16 floats long, this parameter will take up 4 indexes.
		@param m The value to set
		*/
		void setConstant(size_t index, const Matrix4& m);
        /** Sets a list of Matrix4 parameters to the program.
        @param index The constant index at which to start placing the parameter (each constant is
        a 4D float).
        NB since a Matrix4 is 16 floats long, so each entry will take up 4 indexes.
        @param m Pointer to an array of matrices to set
        @param numEntries Number of Matrix4 entries
        */
        void setConstant(size_t index, const Matrix4* m, size_t numEntries);
		/** Sets a multiple value constant floating-point parameter to the program.
		@param index The constant index at which to start placing parameters (each constant is
            a 4D float)
		@param val Pointer to the values to write, must contain 4*count floats
		@param count The number of groups of 4 floats to write
		*/
		void setConstant(size_t index, const Real *val, size_t count);
		/** Sets a ColourValue parameter to the program.
		@param index The constant index at which to place the parameter (each constant is
            a 4D float)
		@param colour The value to set
		*/
        void setConstant(size_t index, const ColourValue& colour);
		
		/** Sets a multiple value constant integer parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
		@param index The constant index at which to place the parameter (each constant is
            a 4D integer)
		@param val Pointer to the values to write, must contain 4*count ints
		@param count The number of groups of 4 ints to write
		*/
		void setConstant(size_t index, const int *val, size_t count);

        /** Deletes the contents of the Real constants registers. */
        void resetRealConstants(void) { mRealConstants.clear(); }
        /** Deletes the contents of the int constants registers. */
        void resetIntConstants(void) { mIntConstants.clear(); }

        typedef VectorIterator<RealConstantList> RealConstantIterator;
        typedef VectorIterator<IntConstantList> IntConstantIterator;
        /// Gets an iterator over the Real constant parameters
        RealConstantIterator getRealConstantIterator(void);
        /// Gets an iterator over the integer constant parameters
        IntConstantIterator getIntConstantIterator(void);
        
        /// Gets the number of Real constants that have been set
        size_t getRealConstantCount(void) const { return mRealConstants.size(); }
        /// Gets the number of int constants that have been set
        size_t getIntConstantCount(void) const { return mIntConstants.size(); }
        /// Returns true if there are any Real constants contained here
        bool hasRealConstantParams(void) const { return !(mRealConstants.empty()); }
        /// Returns true if there are any int constants contained here
        bool hasIntConstantParams(void) const { return !(mIntConstants.empty()); }

        /** Sets up a constant which will automatically be updated by the system.
        @remarks
            Vertex and fragment programs often need parameters which are to do with the
            current render state, or particular values which may very well change over time,
            and often between objects which are being rendered. This feature allows you 
            to set up a certain number of predefined parameter mappings that are kept up to 
            date for you.
        @param index The location in the constant list to place this updated constant every time
            it is changed. Note that because of the nature of the types, we know how big the 
            parameter details will be so you don't need to set that like you do for manual constants.
        @param acType The type of automatic constant to set
        @param extraInfo If the constant type needs more information (like a light index) put it here.
        */
        void setAutoConstant(size_t index, AutoConstantType acType, size_t extraInfo = 0);
        /** Sets a named parameter up to track a derivation of the current time.
        @param index The index of the parameter
        @param factor The amount by which to scale the time value
        */  
        void setConstantFromTime(size_t index, Real factor);

        /** Clears all the existing automatic constants. */
        void clearAutoConstants(void);
        typedef VectorIterator<AutoConstantList> AutoConstantIterator;
        /** Gets an iterator over the automatic constant bindings currently in place. */
        AutoConstantIterator getAutoConstantIterator(void);
        /** Returns true if this instance has any automatic constants. */
        bool hasAutoConstants(void) const { return !(mAutoConstants.empty()); }
        /** Updates the automatic parameters (except lights) based on the details provided. */
        void _updateAutoParamsNoLights(const AutoParamDataSource& source);
        /** Updates the automatic parameters for lights based on the details provided. */
        void _updateAutoParamsLightsOnly(const AutoParamDataSource& source);

		/** Sets a single value constant floating-point parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @par
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
		@param name The name of the parameter
		@param val The value to set
		*/
		void setNamedConstant(const String& name, Real val);
		/** Sets a single value constant integer parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @par
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
		@param val The value to set
		*/
		void setNamedConstant(const String& name, int val);
		/** Sets a Vector4 parameter to the program.
        @param name The name of the parameter
		@param vec The value to set
		*/
		void setNamedConstant(const String& name, const Vector4& vec);
		/** Sets a Vector3 parameter to the program.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
		@param index The index at which to place the parameter
			NB this index refers to the number of floats, so a Vector3 is 3. Note that many 
            rendersystems & programs assume that every floating point parameter is passed in
            as a vector of 4 items, so you are strongly advised to check with 
            RenderSystemCapabilities before using this version - if in doubt use Vector4
            or ColourValue instead (both are 4D).
		@param vec The value to set
		*/
		void setNamedConstant(const String& name, const Vector3& vec);
		/** Sets a Matrix4 parameter to the program.
        @param name The name of the parameter
		@param m The value to set
		*/
		void setNamedConstant(const String& name, const Matrix4& m);
        /** Sets a list of Matrix4 parameters to the program.
        @param name The name of the parameter; this must be the first index of an array,
            for examples 'matrices[0]'
        NB since a Matrix4 is 16 floats long, so each entry will take up 4 indexes.
        @param m Pointer to an array of matrices to set
        @param numEntries Number of Matrix4 entries
        */
        void setNamedConstant(const String& name, const Matrix4* m, size_t numEntries);
		/** Sets a multiple value constant floating-point parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @par
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
		@param val Pointer to the values to write
		@param count The number of floats to write
		*/
		void setNamedConstant(const String& name, const Real *val, size_t count);
		/** Sets a ColourValue parameter to the program.
        @param name The name of the parameter
		@param colour The value to set
		*/
        void setNamedConstant(const String& name, const ColourValue& colour);
		
		/** Sets a multiple value constant integer parameter to the program.
        @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
        @par
            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
		@param val Pointer to the values to write
		@param count The number of integers to write
		*/
		void setNamedConstant(const String& name, const int *val, size_t count);

        /** Sets up a constant which will automatically be updated by the system.
        @remarks
            Vertex and fragment programs often need parameters which are to do with the
            current render state, or particular values which may very well change over time,
            and often between objects which are being rendered. This feature allows you 
            to set up a certain number of predefined parameter mappings that are kept up to 
            date for you.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
        @param acType The type of automatic constant to set
        @param extraInfo If the constant type needs more information (like a light index) put it here.
        */
        void setNamedAutoConstant(const String& name, AutoConstantType acType, size_t extraInfo = 0);

        /** Sets a named parameter up to track a derivation of the current time.
        @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
        @param name The name of the parameter
        @param factor The amount by which to scale the time value
        */  
        void setNamedConstantFromTime(const String& name, Real factor);
        /// Internal method for associating a parameter name with an index
        void _mapParameterNameToIndex(const String& name, size_t index);

        /** Gets the constant index associated with a named parameter. */
        size_t getParamIndex(const String& name) const;

        /** Sets whether or not we need to transpose the matrices passed in from the rest of OGRE.
        @remarks
            D3D uses transposed matrices compared to GL and OGRE; this is not important when you
            use programs which are written to process row-major matrices, such as those generated
            by Cg, but if you use a program written to D3D's matrix layout you will need to enable
            this flag.
        */
        void setTransposeMatrices(bool val) { mTransposeMatrices = val; } 
        /// Gets whether or not matrices are to be transposed when set
        bool getTransposeMatrices(void) const { return mTransposeMatrices; } 

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
		/// The name of the file to load source from (may be blank)
		String mFilename;
        /// The assembler source of the program (may be blank until file loaded)
        String mSource;
        /// Whether we need to load source from file or not
        bool mLoadFromFile;
        /// Syntax code eg arbvp1, vs_2_0 etc
        String mSyntaxCode;
        /// Does this (vertex) program include skeletal animation?
        bool mSkeletalAnimation;
        /// Alternate (vertex) program for use when rendering shadow caster
        String mShadowCasterProgramName;
        /// Alternate (vertex) program for use when rendering shadow receiver
        String mShadowReceiverProgramName;

	public:

		GpuProgram(const String& name, GpuProgramType gptype, const String& syntaxCode);
		virtual ~GpuProgram() {}

        /** Sets the filename of the source assembly for this program.
        @remarks
            Setting this will have no effect until you (re)load the program.
        */
        virtual void setSourceFile(const String& filename);

		/** Sets the source assembly for this program from an in-memory string.
        @remarks
            Setting this will have no effect until you (re)load the program.
        */
        virtual void setSource(const String& source);

        /** Gets the syntax code for this program e.g. arbvp1, fp20, vs_1_1 etc */
        virtual const String& getSyntaxCode(void) const { return mSyntaxCode; }

		/** Gets the name of the file used as source for this program. */
		virtual const String& getSourceFile(void) const { return mFilename; }
        /** Gets the assembler source for this program. */
        virtual const String& getSource(void) const { return mSource; }
        /// Get the program type
        virtual GpuProgramType getType(void) const { return mType; }

        /// @copydoc Resource::load
        void load(void);
        /** Returns the GpuProgram which should be bound to the pipeline.
        @remarks
            This method is simply to allow some subclasses of GpuProgram to delegate
            the program which is bound to the pipeline to a delegate, if required. */
        virtual GpuProgram* _getBindingDelegate(void) { return this; }

        /** Returns whether this program can be supported on the current renderer and hardware. */
        virtual bool isSupported(void) const;

        /** Creates a new parameters object compatible with this program definition. 
        @remarks
            It is recommended that you use this method of creating parameters objects
            rather than going direct to GpuProgramManager, because this method will
            populate any implementation-specific extras (like named parameters) where
            they are appropriate.
        */
        virtual GpuProgramParametersSharedPtr createParameters(void);

        /** Sets whether a vertex program includes the required instructions
        to perform skeletal animation. 
        @remarks
        If this is set to true, OGRE will not blend the geometry according to 
        skeletal animation, it will expect the vertex program to do it.
        */
        virtual void setSkeletalAnimationIncluded(bool included) 
        { mSkeletalAnimation = included; }

        /** Returns whether a vertex program includes the required instructions
            to perform skeletal animation. 
        @remarks
            If this returns true, OGRE will not blend the geometry according to 
            skeletal animation, it will expect the vertex program to do it.
        */
        virtual bool isSkeletalAnimationIncluded(void) const { return mSkeletalAnimation; }
        /** Gets the name of the alternate program to be used for rendering
            shadow casters to a texture-based shadow.
        @remarks
            Texture-based shadows require that the caster is rendered to a texture 
            in a solid colour (the shadow colour in the case of modulative texture
            shadows). Whilst Ogre can arrange this for the fixed function 
            pipeline, passes which use vertex programs might need the vertex
            programs still to run in order to preserve any deformation etc
            that it does. However, lighting calculations must be a lot simpler, 
            with only the ambient colour being used (which the engine will ensure
            is bound to the shadow colour). 
        @par
            Therefore, it is up to implemetors of vertex programs to provide an
            alternative vertex program which can be used to render the object
            to a shadow texture. Do all the same vertex transforms, but set the
            colour of the vertex to the ambient colour, as bound using the 
            standard auto parameter binding mechanism. 
        @note
            Some vertex programs will work without doing this, because Ogre ensures
            that all lights except for ambient are set black. However, the chances
            are that your vertex program is doing a lot of unnecessary work in this
            case, since the other lights are having no effect, and it is good practice
            to supply an alternative.
        @note
            This is only applicable to vertex programs, Ogre automatically disables
            fragment programs when rendering shadows.
        @par
            The default behaviour is for Ogre to switch to fixed-function 
            rendering if an explict vertex program alternative is not set.
        */
        virtual void setShadowCasterProgramName(const String& name);
        /** Gets the alternative program to be used when rendering to a shadow
            texture. */
        virtual const String& getShadowCasterProgramName(void);
        /** Gets the name of the alternate program to be used for rendering
        shadow receivers when using texture-based shadows.
        @remarks
        Texture-based shadows require that the shadow receiver is rendered using
        a projective texture. Whilst Ogre can arrange this for the fixed function 
        pipeline, passes which use vertex programs might need the vertex
        programs still to run in order to preserve any deformation etc
        that it does. So in this case, we need a vertex program which does the 
        appropriate vertex transformation, but generates projective texture
        coordinates. 
        @par
        Therefore, it is up to implemetors of vertex programs to provide an
        alternative vertex program which can be used to render the object
        as a shadow receiver. Do all the same vertex transforms, but generate
        <strong>2 sets</strong> of texture coordinates using the auto parameter
        ACT_TEXTURE_VIEWPROJ_MATRIX. 2 sets are needed because Ogre needs to use
        2 texture units for some shadow effects.
        @note
        This is only applicable to vertex programs, Ogre automatically disables
        fragment programs when rendering shadows.
        @par
        The default behaviour is for Ogre to switch to fixed-function 
        rendering if an explict vertex program alternative is not set. 
        */
        virtual void setShadowReceiverProgramName(const String& name);
        /** Gets the alternative program to be used when rendering to a shadow
        receiver. */
        virtual const String& getShadowReceiverProgramName(void);


    protected:
        /// Virtual method which must be implemented by subclasses, load from mSource
        virtual void loadFromSource(void) = 0;

	};


}

#endif
