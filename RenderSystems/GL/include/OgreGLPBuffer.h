/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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

#ifndef __GLPBUFFER_H__
#define __GLPBUFFER_H__

#include "OgreGLPrerequisites.h"

namespace Ogre {
    /** An off-screen rendering context. These contexts are always RGBA for simplicity, speed and
        convience, but the component format is configurable.
    */
    class GLPBuffer
    {
    public:
        /** PBuffer component format */
        enum ComponentType
        {
            PT_BYTE = 0,    /// Byte per component (8 bit fixed 0.0..1.0)
            PT_SHORT = 1,   /// Short per component (16 bit fixed 0.0..1.0))
            PT_FLOAT16 = 2, /// 16 bit float per component
            PT_FLOAT32 = 3, /// 32 bit float per component
            PT_COUNT = 4    /// Number of pixel types
        };
        GLPBuffer(ComponentType format, size_t width, size_t height);
        virtual ~GLPBuffer();
        
        /** Get the GL context that needs to be active to render to this PBuffer.
        */
        virtual GLContext *getContext() = 0;
        
        ComponentType getFormat() { return mFormat; }
        size_t getWidth() { return mWidth; }
        size_t getHeight() { return mHeight; }
        
        /** Get PBuffer component format for an OGRE pixel format.
         */
        static ComponentType getComponentType(PixelFormat fmt);
    protected:
        ComponentType mFormat;
        size_t mWidth, mHeight;
    };
    
}

#endif // __GLPBRENDERTEXTURE_H__
