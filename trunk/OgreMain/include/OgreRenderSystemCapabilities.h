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

#include "OgreSingleton.h"

namespace Ogre {

    /// Enum describing the different hardware capabilities we want to check for
    enum Capabilities
    {
        RSC_MULTITEXTURE = 0x00000001,
        RSC_AUTOMIPMAP   = 0x00000002,
        RSC_BLENDING     = 0x00000004,
        RSC_ANISOTROPY   = 0x00000008,
        RSC_DOT3         = 0x00000010,
        RSC_CUBEMAPPING  = 0x00000020,
        RSC_HWSTENCIL    = 0x00000040,
        RSC_VBO          = 0x00000080,
    };

    /** singleton class for storing the capabilities of the graphics card. 
        @remarks
            This class stores the capabilities of the graphics card.  This
            information is set by the individual render systems.
    */
    class _OgreExport RenderSystemCapabilities : public Singleton<RenderSystemCapabilities>
    {
        private:
            /// The number of world matricies available
            int mNumWorldMatrices;
            /// The number of texture units available
            int mNumTextureUnits;
            /// Stores the capabilities flags.
            int mCapabilities;
           
    	public:	
            RenderSystemCapabilities ();
            ~RenderSystemCapabilities ();

            void setNumWorldMatricies(int num)
            {
                mNumWorldMatrices = num;
            }

            void setNumTextureUnits(int num)
            {
                mNumTextureUnits = num;
            }

            ushort numWorldMatricies() const
            { 
                return mNumWorldMatrices;
            }

            ushort numTextureUnits() const
            {
                return mNumTextureUnits;
            }

            /** Adds a capability flag to mCapabilities
            */
            void setCapability(const Capabilities c) 
            { 
                mCapabilities |= c;
            }

            /** Checks for a capability
            */
            bool hasCapability(const Capabilities c)
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

            /** Override standard Singleton retrieval.
                @remarks
                    Why do we do this? Well, it's because the Singleton 
                    implementation is in a .h file, which means it gets 
                    compiled into anybody who includes it. This is needed for 
                    the Singleton template to work, but we actually only want 
                    it compiled into the implementation of the class based on 
                    the Singleton, not all of them. If we don't change this, we 
                    get link errors when trying to use the Singleton-based class
                    from an outside dll.
                @par
                    This method just delegates to the template version anyway, 
                    but the implementation stays in this single compilation 
                    unit, preventing link errors.
            */

            static RenderSystemCapabilities& getSingleton(void);

    };
};

#endif // __RenderSystemCapabilities__

