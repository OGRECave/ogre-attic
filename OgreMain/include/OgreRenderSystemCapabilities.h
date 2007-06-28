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
#include "OgreStringVector.h"
#include "OgreResource.h"

// Because there are more than 32 possible Capabilities, more than 1 int is needed to store them all.
// In fact, an array of integers is used to store capabilities. However all the capabilities are defined in the single
// enum. The only way to know which capabilities should be stored where in the array is to use some of the 32 bits
// to record the category of the capability.  These top few bits are used as an index into mCapabilities array
// The lower bits are used to identify each capability individually by setting 1 bit for each

// Identifies how many bits are reserved for categories
// NOTE: Although 4 bits (currently) are enough
#define CAPS_CATEGORY_SIZE 4
#define CAPS_BITSHIFT (32 - CAPS_CATEGORY_SIZE)
#define CAPS_CATEGORY_MASK ((2^CAPS_CATEGORY_SIZE - 1) << CAPS_BITSHIFT)
#define CAPS_VALUE(cat, val) ((cat << CAPS_BITSHIFT) | (1 << val))

namespace Ogre {

    /// Enum describing the different hardware capabilities we want to check for
		/// CAPS_VALUE(a, b) defines each capbility
		// a is the category (which can be from 0 to 15)
		// b is the value (from 0 to 27)
		enum Capabilities
    {
        /// Supporta generating mipmaps in hardware
        RSC_AUTOMIPMAP              = CAPS_VALUE(0, 0),
        RSC_BLENDING                = CAPS_VALUE(0, 1),
        /// Supports anisotropic texture filtering
        RSC_ANISOTROPY              = CAPS_VALUE(0, 2),
        /// Supports fixed-function DOT3 texture blend
        RSC_DOT3                    = CAPS_VALUE(0, 3),
        /// Supports cube mapping
        RSC_CUBEMAPPING             = CAPS_VALUE(0, 4),
        /// Supports hardware stencil buffer
        RSC_HWSTENCIL               = CAPS_VALUE(0, 5),
        /// Supports hardware vertex and index buffe6s
        RSC_VBO                     = CAPS_VALUE(0, 7),
        /// Supports vertex programs (vertex shaders8
				RSC_VERTEX_PROGRAM          = CAPS_VALUE(0, 9),
        /// Supports fragment programs (pixel shaders)
				RSC_FRAGMENT_PROGRAM        = CAPS_VALUE(0, 10),
        /// Supports performing a scissor test to exclude areas of the screen
        RSC_SCISSOR_TEST            = CAPS_VALUE(0, 11),
        /// Supports separate stencil updates for both front and back faces
        RSC_TWO_SIDED_STENCIL       = CAPS_VALUE(0, 12),
        /// Supports wrapping the stencil value at the range extremeties
        RSC_STENCIL_WRAP            = CAPS_VALUE(0, 13),
        /// Supports hardware occlusion queries
        RSC_HWOCCLUSION				= CAPS_VALUE(0, 14),
        /// Supports user clipping planes
        RSC_USER_CLIP_PLANES		= CAPS_VALUE(0, 15),
				/// Supports the VET_UBYTE4 vertex element type
				RSC_VERTEX_FORMAT_UBYTE4	= CAPS_VALUE(0, 16),
				/// Supports infinite far plane projection
				RSC_INFINITE_FAR_PLANE      = CAPS_VALUE(0, 17),
        /// Supports hardware render-to-texture (bigger than framebuffer)
        RSC_HWRENDER_TO_TEXTURE     = CAPS_VALUE(0, 18),
        /// Supports float textures and render targets
        RSC_TEXTURE_FLOAT           = CAPS_VALUE(0, 19),
        /// Supports non-power of two textures
        RSC_NON_POWER_OF_2_TEXTURES = CAPS_VALUE(0, 20),
				/// Supports 3d (volume) textures
				RSC_TEXTURE_3D				= CAPS_VALUE(0, 21),
				/// Supports basic point sprite rendering
				RSC_POINT_SPRITES		    = CAPS_VALUE(0, 22),
				/// Supports extra point parameters (minsize, maxsize, attenuation)
				RSC_POINT_EXTENDED_PARAMETERS = CAPS_VALUE(0, 23),
				/// Supports vertex texture fetch
				RSC_VERTEX_TEXTURE_FETCH = CAPS_VALUE(0, 24),
				/// Supports mipmap LOD biasing
				RSC_MIPMAP_LOD_BIAS = CAPS_VALUE(0, 25),

        /// Supports compressed textures
				RSC_TEXTURE_COMPRESSION = CAPS_VALUE(1, 0),
        /// Supports compressed textures in the DXT/ST3C formats
				RSC_TEXTURE_COMPRESSION_DXT = CAPS_VALUE(1, 1),
        /// Supports compressed textures in the VTC format
				RSC_TEXTURE_COMPRESSION_VTC = CAPS_VALUE(1, 2),
				/// Supports openGL GLEW version 1.5
				RSC_GLEW1_5_NOVBO	 = CAPS_VALUE(1, 3),
				/// Support for Frame Buffer Objects (FBOs)
				RSC_FBO						 = CAPS_VALUE(1, 4),
				/// Support for Frame Buffer Objects ARB implementation (regular FBO is higher precedence)
				RSC_FBO_ARB				 = CAPS_VALUE(1, 5),
				/// Support for Frame Buffer Objects ATI implementation (ARB FBO is higher precedence)
				RSC_FBO_ATI				 = CAPS_VALUE(1, 6),
				/// Support for PBuffer
				RSC_PBUFFER				 = CAPS_VALUE(1, 7),
				/// Support for GLEW 1.5 without HW occlusion workaround
				RSC_GLEW1_5_NOHWOCCLUSION = CAPS_VALUE(1, 8),


				/// Is DirectX feature "per stage constants" supported
				RSC_PERSTAGECONSTANT = CAPS_VALUE(1, 9)

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
            String mGLDriver;
            String mGLVendor;
            String mGLRenderer;

            String mDX9Driver;
            String mDX9Vendor;
            String mDX9Renderer;

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
            /// The best vertex program that this card / rendersystem supports
            String mMaxVertexProgramVersion;
            /// The best fragment program that this card / rendersystem supports
            String mMaxFragmentProgramVersion;
            /// The number of floating-point constants vertex programs support
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

			/// The list of supported shader profiles
			ShaderProfiles mSupportedShaderProfiles;

    	public:
            RenderSystemCapabilities ();
            virtual ~RenderSystemCapabilities ();

            virtual size_t calculateSize() const {return 0;}

            void setGLDriver(String driver)
            {
                mGLDriver = driver;
            }

            void setGLVendor(String vendor)
            {
                mGLVendor = vendor;
            }

            void setGLRenderer(String renderer)
            {
                mGLRenderer = renderer;
            }

            void setDX9Driver(String driver)
            {
                mDX9Driver = driver;
            }

            void setDX9Vendor(String vendor)
            {
                mDX9Vendor = vendor;
            }

            void setDX9Renderer(String renderer)
            {
                mDX9Renderer = renderer;
            }

            String getGLDriver()
            {
                return mGLDriver;
            }

            String gettGLVendor()
            {
                return mGLVendor;
            }

            String getGLRenderer()
            {
                return mGLRenderer;
            }

            String gettDX9Driver()
            {
                return mDX9Driver;
            }

            String getDX9Vendor()
            {
                return mDX9Vendor;
            }

            String getDX9Renderer()
            {
                return mDX9Renderer;
            }



            void setNumWorldMatricies(ushort num)
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

            ushort getNumWorldMatricies(void) const
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
            ushort numVertexBlendMatrices(void) const
            {
                return mNumVertexBlendMatrices;
            }

			/// The number of simultaneous render targets supported
			ushort numMultiRenderTargets(void) const
			{
				return mNumMultiRenderTargets;
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


            /// Gets the best low-level vertex program version supported
            const String& getMaxVertexProgramVersion(void) const
            {
                return mMaxVertexProgramVersion;
            }
            /// Gets the best fragment program that this card / rendersystem supports
            const String& getMaxFragmentProgramVersion(void) const
            {
                return mMaxFragmentProgramVersion;
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



            /// sets the best low-level vertex program version supported
            void setMaxVertexProgramVersion(const String& ver)
            {
                mMaxVertexProgramVersion = ver;
            }
            /// sets the best fragment program that this card / rendersystem supports
            void setMaxFragmentProgramVersion(const String& ver)
            {
                mMaxFragmentProgramVersion = ver;
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


            /** Write the capabilities to the pass in Log */
            void log(Log* pLog);

    };

} // namespace

#endif // __RenderSystemCapabilities__

