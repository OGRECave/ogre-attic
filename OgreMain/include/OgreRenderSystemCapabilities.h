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
#ifndef __RenderSystemCapabilities__
#define __RenderSystemCapabilities__ 1

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreString.h"

namespace Ogre {

    /// Enum describing the different hardware capabilities we want to check for
    enum Capabilities
    {
        //RSC_MULTITEXTURE   = 0x00000001,
        RSC_AUTOMIPMAP       = 0x00000002,
        RSC_BLENDING         = 0x00000004,
        RSC_ANISOTROPY       = 0x00000008,
        RSC_DOT3             = 0x00000010,
        RSC_CUBEMAPPING      = 0x00000020,
        RSC_HWSTENCIL        = 0x00000040,
        RSC_VBO              = 0x00000080,
        RSC_VERTEXBLENDING   = 0x00000100,
		RSC_VERTEX_PROGRAM   = 0x00000200,
		RSC_FRAGMENT_PROGRAM = 0x00000400,
		RSC_COMPRESSED_TEX   = 0x00000800
    };

    /** singleton class for storing the capabilities of the graphics card. 
        @remarks
            This class stores the capabilities of the graphics card.  This
            information is set by the individual render systems.
    */
    class _OgreExport RenderSystemCapabilities 
    {
        private:
            /// The number of world matricies available
            ushort mNumWorldMatrices;
            /// The number of texture units available
            ushort mNumTextureUnits;
            /// The stencil buffer bit depth
            ushort mStencilBufferBitDepth;
            /// The number of matrices available for hardware blending
            ushort mNumVertexBlendMatrices;
            /// Stores the capabilities flags.
            int mCapabilities;
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

    	public:	
            RenderSystemCapabilities ();
            ~RenderSystemCapabilities ();

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

            /** Adds a capability flag to mCapabilities
            */
            void setCapability(const Capabilities c) 
            { 
                mCapabilities |= c;
            }

            /** Checks for a capability
            */
            bool hasCapability(const Capabilities c) const
            {
                if(mCapabilities & c)
                {
                    return true;
                }
                else
                {
                    return false;
                }
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

            /** Write the capabilities to the pass in Log */
            void log(Log* pLog);




    };
};

#endif // __RenderSystemCapabilities__

