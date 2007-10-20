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
#ifndef __D3D10VERTEXDECLARATION_H__
#define __D3D10VERTEXDECLARATION_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreHardwareVertexBuffer.h"

namespace Ogre { 

    /** Specialisation of VertexDeclaration for D3D10 */
    class D3D10VertexDeclaration : public VertexDeclaration
    {
    protected:
        ID3D10Device * mlpD3DDevice;
        ID3D10InputLayout* mlpD3DDecl;
        bool mNeedsRebuild;
    public:
        D3D10VertexDeclaration(ID3D10Device * device);
        ~D3D10VertexDeclaration();
        
        /** See VertexDeclaration */
        const VertexElement& addElement(unsigned short source, size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);

        /** See VertexDeclaration */
        const VertexElement& insertElement(unsigned short atPosition,
            unsigned short source, size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);

        /** See VertexDeclaration */
        void removeElement(unsigned short elem_index);
		
		/** See VertexDeclaration */
		void removeElement(VertexElementSemantic semantic, unsigned short index = 0);

		/** See VertexDeclaration */
		void removeAllElements(void);


        /** See VertexDeclaration */
        void modifyElement(unsigned short elem_index, unsigned short source, size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);

        /** Gets the D3D10-specific vertex declaration. */
        ID3D10InputLayout* getD3DVertexDeclaration(void);


    };

}

#endif
