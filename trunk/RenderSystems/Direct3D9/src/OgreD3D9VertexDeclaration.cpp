/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2003 The OGRE Team
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
#include "OgreD3D9VertexDeclaration.h"
#include "OgreD3D9Mappings.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    D3D9VertexDeclaration::D3D9VertexDeclaration(LPDIRECT3DDEVICE9 device) 
        : mlpD3DDevice(device), mlpD3DDecl(NULL), mNeedsRebuild(true)
    {
    }
    //-----------------------------------------------------------------------
    D3D9VertexDeclaration::~D3D9VertexDeclaration()
    {
        SAFE_RELEASE(mlpD3DDecl);
    }
    //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::addElement(unsigned short source, 
        size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        VertexDeclaration::addElement(source, offset, theType, semantic, index);
        mNeedsRebuild = true;
    }
    //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::removeElement(unsigned short elem_index)
    {
        VertexDeclaration::removeElement(elem_index);
        mNeedsRebuild = true;
    }
    //-----------------------------------------------------------------------
    void D3D9VertexDeclaration::modifyElement(unsigned short elem_index, 
        unsigned short source, size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        VertexDeclaration::modifyElement(elem_index, source, offset, theType, semantic, index);
        mNeedsRebuild = true;
    }
    //-----------------------------------------------------------------------
    LPDIRECT3DVERTEXDECLARATION9 D3D9VertexDeclaration::getD3DVertexDeclaration(void)
    {
        if (mNeedsRebuild)
        {
            SAFE_RELEASE(mlpD3DDecl);
            // Create D3D elements
            D3DVERTEXELEMENT9* d3delems = new D3DVERTEXELEMENT9[mElementList.size()];

            VertexElementList::const_iterator i, iend;
            unsigned int idx;
            iend = mElementList.end();
            for (idx = 0, i = mElementList.begin(); i != iend; ++i, ++idx)
            {
                d3delems[idx].Method = D3DDECLMETHOD_DEFAULT;
                d3delems[idx].Offset = i->getOffset();
                d3delems[idx].Stream = i->getSource();
				d3delems[idx].Type = D3D9Mappings::get(i->getType());
				d3delems[idx].Usage = D3D9Mappings::get(i->getSemantic());
				// NB force index if colours since D3D uses the same usage for 
				// diffuse & specular
				if (i->getSemantic() == VES_SPECULAR)
				{
					d3delems[idx].UsageIndex = 1;
				}
				else if (i->getSemantic() == VES_DIFFUSE)
				{
					d3delems[idx].UsageIndex = 0;
				}
				else
				{
					d3delems[idx].UsageIndex = i->getIndex();
				}
            }
            
            HRESULT hr = mlpD3DDevice->CreateVertexDeclaration(d3delems, &mlpD3DDecl);

            delete [] d3delems;

			mNeedsRebuild = false;

        }
        return mlpD3DDecl;
    }
    //-----------------------------------------------------------------------


}

