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
#ifndef __RenderSystemCapabilities__
#define __RenderSystemCapabilities__ 1

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreStringVector.h"
#include "OgreResource.h"
#include "OgreLogManager.h"

// Because there are more than 32 possible Capabilities, more than 1 int is needed to store them all.
// In fact, an array of integers is used to store capabilities. However all the capabilities are defined in the single
// enum. The only way to know which capabilities should be stored where in the array is to use some of the 32 bits
// to record the category of the capability.  These top few bits are used as an index into mCapabilities array
// The lower bits are used to identify each capability individually by setting 1 bit for each

// Identifies how many bits are reserved for categories
// NOTE: Although 4 bits (currently) are enough
static const int CAPS_CATEGORY_SIZE = 4;
#define CAPS_BITSHIFT (32 - CAPS_CATEGORY_SIZE)
#define CAPS_CATEGORY_MASK ((2^CAPS_CATEGORY_SIZE - 1) << CAPS_BITSHIFT)
#define CAPS_VALUE(cat, val) ((cat << CAPS_BITSHIFT) | (1 << val))

static const int CAPS_CATEGORY_D3D9 = 0;
static const int CAPS_CATEGORY_GL = 1;
static const int CAPS_CATEGORY_COMMON = 2;
static const int CAPS_CATEGORY_COMMON_2 = 3;

namespace Ogre {

    /// Enum describing the different hardware capabilities we want to check for
	/// CAPS_VALUE(a, b) defines each capbility
	// a is the category (which can be from 0 to 15)
	// b is the value (from 0 to 27)
    enum Capabilities
    {
		/// Is DirectX feature "per stage constants" supported
        RSC_PERSTAGECONSTANT = CAPS_VALUE(CAPS_CATEGORY_D3D9, 0),

		/// Supports openGL GLEW version 1.5
        RSC_GLEW1_5_NOVBO    = CAPS_VALUE(CAPS_CATEGORY_GL, 1),
        /// Support for Frame Buffer Objects (FBOs)
        RSC_FBO              = CAPS_VALUE(CAPS_CATEGORY_GL, 2),
        /// Support for Frame Buffer Objects ARB implementation (regular FBO is higher precedence)
        RSC_FBO_ARB          = CAPS_VALUE(CAPS_CATEGORY_GL, 3),
        /// Support for Frame Buffer Objects ATI implementation (ARB FBO is higher precedence)
        RSC_FBO_ATI          = CAPS_VALUE(CAPS_CATEGORY_GL, 4),
        /// Support for PBuffer
        RSC_PBUFFER          = CAPS_VALUE(CAPS_CATEGORY_GL, 5),
        /// Support for GLEW 1.5 without HW occlusion workaround
        RSC_GLEW1_5_NOHWOCCLUSION = CAPS_VALUE(CAPS_CATEGORY_GL, 6),


        /// Supporta generating mipmaps in hardware
        RSC_AUTOMIPMAP              = CAPS_VALUE(CAPS_CATEGORY_COMMON, 0),
        RSC_BLENDING                = CAPS_VALUE(CAPS_CATEGORY_COMMON, 1),
        /// Supports anisotropic texture filtering
        RSC_ANISOTROPY              = CAPS_VALUE(CAPS_CATEGORY_COMMON, 2),
        /// Supports fixed-function DOT3 texture blend
        RSC_DOT3                    = CAPS_VALUE(CAPS_CATEGORY_COMMON, 3),
        /// Supports cube mapping
        RSC_CUBEMAPPING             = CAPS_VALUE(CAPS_CATEGORY_COMMON, 4),
        /// Supports hardware stencil buffer
        RSC_HWSTENCIL               = CAPS_VALUE(CAPS_CATEGORY_COMMON, 5),
        /// Supports hardware vertex and index buffers
        RSC_VBO                     = CAPS_VALUE(CAPS_CATEGORY_COMMON, 7),
        /// Supports vertex programs (vertex shaders
        RSC_VERTEX_PROGRAM          = CAPS_VALUE(CAPS_CATEGORY_COMMON, 9),
        /// Supports fragment programs (pixel shaders)
        RSC_FRAGMENT_PROGRAM        = CAPS_VALUE(CAPS_CATEGORY_COMMON, 10),
        /// Supports performing a scissor test to exclude areas of the screen
        RSC_SCISSOR_TEST            = CAPS_VALUE(CAPS_CATEGORY_COMMON, 11),
        /// Supports separate stencil updates for both front and back faces
        RSC_TWO_SIDED_STENCIL       = CAPS_VALUE(CAPS_CATEGORY_COMMON, 12),
        /// Supports wrapping the stencil value at the range extremeties
        RSC_STENCIL_WRAP            = CAPS_VALUE(CAPS_CATEGORY_COMMON, 13),
        /// Supports hardware occlusion queries
        RSC_HWOCCLUSION             = CAPS_VALUE(CAPS_CATEGORY_COMMON, 14),
        /// Supports user clipping planes
        RSC_USER_CLIP_PLANES        = CAPS_VALUE(CAPS_CATEGORY_COMMON, 15),
        /// Supports the VET_UBYTE4 vertex element type
        RSC_VERTEX_FORMAT_UBYTE4    = CAPS_VALUE(CAPS_CATEGORY_COMMON, 16),
        /// Supports infinite far plane projection
        RSC_INFINITE_FAR_PLANE      = CAPS_VALUE(CAPS_CATEGORY_COMMON, 17),
        /// Supports hardware render-to-texture (bigger than framebuffer)
        RSC_HWRENDER_TO_TEXTURE     = CAPS_VALUE(CAPS_CATEGORY_COMMON, 18),
        /// Supports float textures and render targets
        RSC_TEXTURE_FLOAT           = CAPS_VALUE(CAPS_CATEGORY_COMMON, 19),
        /// Supports non-power of two textures
        RSC_NON_POWER_OF_2_TEXTURES = CAPS_VALUE(CAPS_CATEGORY_COMMON, 20),
        /// Supports 3d (volume) textures
        RSC_TEXTURE_3D              = CAPS_VALUE(CAPS_CATEGORY_COMMON, 21),
        /// Supports basic point sprite rendering
        RSC_POINT_SPRITES           = CAPS_VALUE(CAPS_CATEGORY_COMMON, 22),
        /// Supports extra point parameters (minsize, maxsize, attenuation)
        RSC_POINT_EXTENDED_PARAMETERS = CAPS_VALUE(CAPS_CATEGORY_COMMON, 23),
        /// Supports vertex texture fetch
        RSC_VERTEX_TEXTURE_FETCH = CAPS_VALUE(CAPS_CATEGORY_COMMON, 24),
        /// Supports mipmap LOD biasing
        RSC_MIPMAP_LOD_BIAS = CAPS_VALUE(CAPS_CATEGORY_COMMON, 25),

        /// Supports compressed textures
        RSC_TEXTURE_COMPRESSION = CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 0),
        /// Supports compressed textures in the DXT/ST3C formats
        RSC_TEXTURE_COMPRESSION_DXT = CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 1),
        /// Supports compressed textures in the VTC format
        RSC_TEXTURE_COMPRESSION_VTC = CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 2),
        
    };

	/// DriverVersion is used by RenderSystemCapabilities and both GL and D3D9
	/// to store the version of the current GPU driver
	struct _OgreExport DriverVersion {
		int major;
		int minor;
		int release;
		int build;
	};


    /** singleton class for storing the capabilities of the graphics card.
        @remarks
            This class stores the capabilities of the graphics card.  This
            information is set by the individual render systems.
    */
    class _OgreExport RenderSystemCapabilities
    {

        public:

            typedef std::set<String> ShaderProfiles;
        private:
			/// This is used to build a database of RSC's
			/// if a RSC with same name, but newer version is introduced, the older one 
			/// will be removed
			DriverVersion mGLVersion;
			DriverVersion mD3D9Version;
			            
            /// The number of world matricies available
            ushort mNumWorldMatrices;
            /// The number of texture units available
            ushort mNumTextureUnits;
            /// The stencil buffer bit depth
            ushort mStencilBufferBitDepth;
            /// The number of matrices available for hardware blending
            ushort mNumVertexBlendMatrices;
            /// Stores the capabilities flags.
            int mCapabilities[4];
            /// The name of the device as reported by the render system
            String mDeviceNameD3D9;
            String mDeviceNameGL;
            
            ushort mVertexProgramConstantFloatCount;
            /// The number of integer constants vertex programs support
            ushort mVertexProgramConstantIntCount;
            /// The number of boolean constants vertex programs support
            ushort mVertexProgramConstantBoolCount;
            /// The number of floating-point constants fragment programs support
            ushort mFragmentProgramConstantFloatCount;
            /// The number of integer constants fragment programs support
            ushort mFragmentProgramConstantIntCount;
            /// The number of boolean constants fragment programs support
            ushort mFragmentProgramConstantBoolCount;
			/// The number of simultaneous render targets supported
			ushort mNumMultiRenderTargets;
			/// The maximum point size
			Real mMaxPointSize;
			/// Are non-POW2 textures feature-limited?
			bool mNonPOW2TexturesLimited;
			/// The number of vertex texture units supported
			ushort mNumVertexTextureUnits;
			/// Are vertex texture units shared with fragment processor?
			bool mVertexTextureUnitsShared;
			/// Should this RSC be used to initialize D3D9 Render System?
			bool mCapabilitiesValidForD3D9;
			bool mCapabilitiesValidForGL;


			/// The list of supported shader profiles
			ShaderProfiles mSupportedShaderProfiles;

    	public:
            RenderSystemCapabilities ();
            virtual ~RenderSystemCapabilities ();

            virtual size_t calculateSize() const {return 0;}

			void setGLVersion(const DriverVersion& version)
            {
				mGLVersion = version;
            }

			void setD3D9Version(const DriverVersion& version)
            {
				mD3D9Version = version;
            }
            
            void parseD3D9VersionFromString(const String& versionString)
            {
                DriverVersion version;
                StringVector tokens = StringUtil::split(versionString, ".");
                if(tokens.size() != 4)
                {
                    LogManager::getSingleton().logMessage("Can not parse an invalid D3D9 driver version string: " + versionString);
                }
                else
                {
                    version.major = StringConverter::parseInt(tokens[0]);
                    version.minor = StringConverter::parseInt(tokens[1]);
                    version.release = StringConverter::parseInt(tokens[2]);
                    version.build = StringConverter::parseInt(tokens[3]);
                    
                    setD3D9Version(version);
                }
            }
            
            void parseGLVersionFromString(const String& versionString)
            {
                DriverVersion version;
                StringVector tokens = StringUtil::split(versionString, ".");
                if(tokens.size() != 3)
                {
                    LogManager::getSingleton().logMessage("Can not parse an invalid GL driver version string: " + versionString);
                }
                else
                {
                    version.major = StringConverter::parseInt(tokens[0]);
                    version.minor = StringConverter::parseInt(tokens[1]);
                    version.release = StringConverter::parseInt(tokens[2]);
                    version.build = 0;
                    
                    setGLVersion(version);
                }
            }
			
			DriverVersion getGLVersion() const
            {
				return mGLVersion;
            }

			DriverVersion getD3D9Version() const
            {
				return mD3D9Version;
            }
			
			bool isGLOlderThanVersion(DriverVersion v) const
			{
				if (mGLVersion.major > v.major)
					return true;
				else if (mGLVersion.major == v.major && mGLVersion.minor > v.minor)
					return true;
				else if (mGLVersion.major == v.major && mGLVersion.minor == v.minor && mGLVersion.release > v.release)
					return true;
				return false;
			}

			bool isD3D9OlderThanVersion(DriverVersion v) const
			{
				if (mD3D9Version.major > v.major)
					return true;
				else if (mD3D9Version.major == v.major && mD3D9Version.minor > v.minor)
					return true;
				else if (mD3D9Version.major == v.major && mD3D9Version.minor == v.minor && mD3D9Version.release > v.release)
					return true;
				else if (mD3D9Version.major == v.major && mD3D9Version.minor == v.minor && mD3D9Version.release == v.release && mD3D9Version.build > v.build)
					return true;
				return false;
			}

            void setNumWorldMatrices(ushort num)
            {
                mNumWorldMatrices = num;
            }

            void setNumTextureUnits(ushort num)
            {
                mNumTextureUnits = num;
            }

            void setStencilBufferBitDepth(ushort num)
            {
                mStencilBufferBitDepth = num;
            }

            void setNumVertexBlendMatrices(ushort num)
            {
                mNumVertexBlendMatrices = num;
            }

			/// The number of simultaneous render targets supported
			void setNumMultiRenderTargets(ushort num)
			{
				mNumMultiRenderTargets = num;
			}

            ushort getNumWorldMatrices(void) const
            {
                return mNumWorldMatrices;
            }

            /** Returns the number of texture units the current output hardware
                supports.

                For use in rendering, this determines how many texture units the
                are available for multitexturing (i.e. rendering multiple
                textures in a single pass). Where a Material has multiple
                texture layers, it will try to use multitexturing where
                available, and where it is not available, will perform multipass
                rendering to achieve the same effect.
            */
            ushort getNumTextureUnits(void) const
            {
                return mNumTextureUnits;
            }

            /** Determines the bit depth of the hardware accelerated stencil
                buffer, if supported.
                @remarks
                    If hardware stencilling is not supported, the software will
                    provide an 8-bit software stencil.
            */
            ushort getStencilBufferBitDepth(void) const
            {
                return mStencilBufferBitDepth;
            }

            /** Returns the number of matrices available to hardware vertex
                blending for this rendering system. */
            ushort getNumVertexBlendMatrices(void) const
            {
                return mNumVertexBlendMatrices;
            }

			/// The number of simultaneous render targets supported
			ushort getNumMultiRenderTargets(void) const
			{
				return mNumMultiRenderTargets;
			}

			/** Returns true if capability is render system specific
			*/
			bool isCapabilityRenderSystemSpecific(const Capabilities c)
			{
				int cat = c >> CAPS_BITSHIFT;
				if(cat == CAPS_CATEGORY_GL || cat == CAPS_CATEGORY_D3D9)
					return true;
				return false;
			}

            /** Adds a capability flag to mCapabilities
            */
            void setCapability(const Capabilities c)
            {
								int index = (CAPS_CATEGORY_MASK & c) >> CAPS_BITSHIFT;
								// zero out the index from the stored capability
                mCapabilities[index] |= (c & ~CAPS_CATEGORY_MASK);
            }

            /** Checks for a capability
            */
            bool hasCapability(const Capabilities c) const
            {
								int index = (CAPS_CATEGORY_MASK & c) >> CAPS_BITSHIFT;
								// test against
                if(mCapabilities[index] & (c & ~CAPS_CATEGORY_MASK))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

						/** Adds the profile to the list of supported profiles
						 */
						void addShaderProfile(const String& profile)
						{
								mSupportedShaderProfiles.insert(profile);

						}

						/** Returns true if profile is in the list of supported profiles
						 */
						bool isShaderProfileSupported(const String& profile) const
						{
								return (mSupportedShaderProfiles.end() != mSupportedShaderProfiles.find(profile));
						}


						/** Returns a set of all supported shader profiles
						 * */
						const ShaderProfiles& getSupportedShaderProfiles() const
						{
								return mSupportedShaderProfiles;
						}


            /// The number of floating-point constants vertex programs support
            ushort getVertexProgramConstantFloatCount(void) const
            {
                return mVertexProgramConstantFloatCount;
            }
            /// The number of integer constants vertex programs support
            ushort getVertexProgramConstantIntCount(void) const
            {
                return mVertexProgramConstantIntCount;
            }
            /// The number of boolean constants vertex programs support
            ushort getVertexProgramConstantBoolCount(void) const
            {
                return mVertexProgramConstantBoolCount;
            }
            /// The number of floating-point constants fragment programs support
            ushort getFragmentProgramConstantFloatCount(void) const
            {
                return mFragmentProgramConstantFloatCount;
            }
            /// The number of integer constants fragment programs support
            ushort getFragmentProgramConstantIntCount(void) const
            {
                return mFragmentProgramConstantIntCount;
            }
            /// The number of boolean constants fragment programs support
            ushort getFragmentProgramConstantBoolCount(void) const
            {
                return mFragmentProgramConstantBoolCount;
            }

                /// sets the device name for D3D9 render system
            void setDeviceNameD3D9(const String& name)
            {
                mDeviceNameD3D9 = name;
            }
            
            /// gets the device name for D3D9 render system
            String getDeviceNameD3D9() const
            {
                return mDeviceNameD3D9;
            }
            
            /// sets the device name for OpenGL render system
            void setDeviceNameGL(const String& name)
            {
                mDeviceNameGL = name;
            }
            
            /// gets the device name for OpenGL render system
            String getDeviceNameGL() const
            {
                return mDeviceNameGL;
            }
            
            /// The number of floating-point constants vertex programs support
            void setVertexProgramConstantFloatCount(ushort c)
            {
                mVertexProgramConstantFloatCount = c;
            }
            /// The number of integer constants vertex programs support
            void setVertexProgramConstantIntCount(ushort c)
            {
                mVertexProgramConstantIntCount = c;
            }
            /// The number of boolean constants vertex programs support
            void setVertexProgramConstantBoolCount(ushort c)
            {
                mVertexProgramConstantBoolCount = c;
            }
            /// The number of floating-point constants fragment programs support
            void setFragmentProgramConstantFloatCount(ushort c)
            {
                mFragmentProgramConstantFloatCount = c;
            }
            /// The number of integer constants fragment programs support
            void setFragmentProgramConstantIntCount(ushort c)
            {
                mFragmentProgramConstantIntCount = c;
            }
            /// The number of boolean constants fragment programs support
            void setFragmentProgramConstantBoolCount(ushort c)
            {
                mFragmentProgramConstantBoolCount = c;
            }
			/// Maximum point screen size in pixels
			void setMaxPointSize(Real s)
			{
				mMaxPointSize = s;
			}
			/// Maximum point screen size in pixels
			Real getMaxPointSize(void) const
			{
				return mMaxPointSize;
			}
			/// Non-POW2 textures limited
			void setNonPOW2TexturesLimited(bool l)
			{
				mNonPOW2TexturesLimited = l;
			}
			/** Are non-power of two textures limited in features?
			@remarks
				If the RSC_NON_POWER_OF_2_TEXTURES capability is set, but this
				method returns true, you can use non power of 2 textures only if:
				<ul><li>You load them explicitly with no mip maps</li>
				<li>You don't use DXT texture compression</li>
				<li>You use clamp texture addressing</li></ul>
			*/
			bool getNonPOW2TexturesLimited(void) const
			{
				return mNonPOW2TexturesLimited;
			}

			/// Set the number of vertex texture units supported
			void setNumVertexTextureUnits(ushort n)
			{
				mNumVertexTextureUnits = n;
			}
			/// Get the number of vertex texture units supported
			ushort getNumVertexTextureUnits(void) const
			{
				return mNumVertexTextureUnits;
			}
			/// Set whether the vertex texture units are shared with the fragment processor
			void setVertexTextureUnitsShared(bool shared)
			{
				mVertexTextureUnitsShared = shared;
			}
			/// Get whether the vertex texture units are shared with the fragment processor
			bool getVertexTextureUnitsShared(void) const
			{
				return mVertexTextureUnitsShared;
			}

			/// Get whether these capabilities are valid for D3D9
			bool getCapabilitiesValidForD3D9(void) const
			{
				return mCapabilitiesValidForD3D9;
			}
			/// Set whether these capabilities are valid for D3D9
			void setCapabilitiesValidForD3D9(bool valid)
			{
				mCapabilitiesValidForD3D9 = valid;
			}

			/// Get whether these capabilities are valid for OpenGL
			bool getCapabilitiesValidForGL(void) const
			{
				return mCapabilitiesValidForGL;
			}
			/// Set whether these capabilities are valid for OpenGL
			void setCapabilitiesValidForGL(bool valid)
			{
				mCapabilitiesValidForGL = valid;
			}

            /** Write the capabilities to the pass in Log */
            void log(Log* pLog);

    };

} // namespace

#endif // __RenderSystemCapabilities__

