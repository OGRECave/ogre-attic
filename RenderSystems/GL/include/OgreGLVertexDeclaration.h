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
#ifndef __GLVERTEXDECLARATION_H__
#define __GLVERTEXDECLARATION_H__

#include "OgreGLPrerequisites.h"
#include "OgreHardwareVertexBuffer.h"

namespace Ogre { 

    /** Specialisation of VertexDeclaration for GL */
    class GLVertexDeclaration : public VertexDeclaration
    {
    protected:
        bool mNeedsRebuild;

    public:
        GLVertexDeclaration();
        ~GLVertexDeclaration();
        
        /** See VertexDeclaration */
        void addElement(unsigned short source, size_t offset, 
            VertexElementType theType, VertexElementSemantic semantic, 
            unsigned short index = 0);

        /** See VertexDeclaration */
        void removeElement(unsigned short elem_index);

        /** See VertexDeclaration */
        void modifyElement(unsigned short elem_index, unsigned short source, 
            size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);

        /** Gets the D3D9-specific vertex declaration. */
        //LPDIRECT3DVERTEXDECLARATION9 getD3DVertexDeclaration(void);


    };

}

#endif // __GLVERTEXDECLARATION_H__
