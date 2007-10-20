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
#include "OgreD3D10VertexDeclaration.h"
#include "OgreD3D10Mappings.h"
#include "OgreException.h"
#include "OgreRoot.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    D3D10VertexDeclaration::D3D10VertexDeclaration(ID3D10Device * device) 
        : mlpD3DDevice(device), mlpD3DDecl(NULL), mNeedsRebuild(true)
    {
    }
    //-----------------------------------------------------------------------
    D3D10VertexDeclaration::~D3D10VertexDeclaration()
    {
        SAFE_RELEASE(mlpD3DDecl);
    }
    //-----------------------------------------------------------------------
    const VertexElement& D3D10VertexDeclaration::addElement(unsigned short source, 
        size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        mNeedsRebuild = true;
        return VertexDeclaration::addElement(source, offset, theType, semantic, index);
    }
    //-----------------------------------------------------------------------------
    const VertexElement& D3D10VertexDeclaration::insertElement(unsigned short atPosition,
        unsigned short source, size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        mNeedsRebuild = true;
        return VertexDeclaration::insertElement(atPosition, source, offset, theType, semantic, index);
    }
    //-----------------------------------------------------------------------
    void D3D10VertexDeclaration::removeElement(unsigned short elem_index)
    {
        VertexDeclaration::removeElement(elem_index);
        mNeedsRebuild = true;
    }
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::removeElement(VertexElementSemantic semantic, unsigned short index)
	{
		VertexDeclaration::removeElement(semantic, index);
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::removeAllElements(void)
	{
		VertexDeclaration::removeAllElements();
		mNeedsRebuild = true;
	}
    //-----------------------------------------------------------------------
    void D3D10VertexDeclaration::modifyElement(unsigned short elem_index, 
        unsigned short source, size_t offset, VertexElementType theType,
        VertexElementSemantic semantic, unsigned short index)
    {
        VertexDeclaration::modifyElement(elem_index, source, offset, theType, semantic, index);
        mNeedsRebuild = true;
    }
    //-----------------------------------------------------------------------
    ID3D10InputLayout* D3D10VertexDeclaration::getD3DVertexDeclaration(void)
    {
        if (mNeedsRebuild)
        {
            SAFE_RELEASE(mlpD3DDecl);
            // Create D3D elements
            D3D10_INPUT_ELEMENT_DESC* d3delems = new D3D10_INPUT_ELEMENT_DESC[mElementList.size() + 1];

            VertexElementList::const_iterator i, iend;
            unsigned int idx;
            iend = mElementList.end();
            for (idx = 0, i = mElementList.begin(); i != iend; ++i, ++idx)
            {
             //   d3delems[idx].Method = D3DDECLMETHOD_DEFAULT;
                d3delems[idx].AlignedByteOffset = static_cast<WORD>(i->getOffset());
                d3delems[idx].InputSlot = i->getSource();
				d3delems[idx].Format = D3D10Mappings::get(i->getType());
				d3delems[idx].SemanticName = D3D10Mappings::get(i->getSemantic()).c_str();
				/*// NB force index if colours since D3D uses the same usage for 
				// diffuse & specular
				if (i->getSemantic() == VES_SPECULAR)
				{
					d3delems[idx].SemanticIndex = 1;
				}
				else if (i->getSemantic() == VES_DIFFUSE)
				{
					d3delems[idx].SemanticIndex = 0;
				}
				else*/
				{
					d3delems[idx].SemanticIndex = i->getIndex();
				}
            }
            /*// Add terminator
		    d3delems[idx].Stream = 0xff;
		    d3delems[idx].Offset = 0;
		    d3delems[idx].Type = D3DDECLTYPE_UNUSED;
		    d3delems[idx].Method = 0;
		    d3delems[idx].Usage = 0;
		    d3delems[idx].SemanticIndex = 0;
            */
           // HRESULT hr = mlpD3DDevice->CreateVertexDeclaration(d3delems, &mlpD3DDecl);
			HRESULT hr = mlpD3DDevice->CreateInputLayout(d3delems,idx-1,0,0, &mlpD3DDecl);
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                    "Cannot create D3D10 vertex declaration: " + 
                    Root::getSingleton().getErrorDescription(hr), 
                    "Direct3D9VertexDeclaration::getD3DVertexDeclaration");
            }

            delete [] d3delems;

			mNeedsRebuild = false;

        }
        return mlpD3DDecl;
    }
    //-----------------------------------------------------------------------


}

