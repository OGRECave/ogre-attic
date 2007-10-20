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
#ifndef __D3D10HARDWAREVERTEXBUFFER_H__
#define __D3D10HARDWAREVERTEXBUFFER_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreHardwareVertexBuffer.h"
#include <d3d10.h>
#include <d3dx10.h>
#include <dxerr.h>


namespace Ogre {

    /// Specialisation of HardwareVertexBuffer for D3D10
    class D3D10HardwareVertexBuffer : public HardwareVertexBuffer 
    {
    protected:
        ID3D10Buffer * mlpD3DBuffer;
//		D3DPOOL mD3DPool;
        /** See HardwareBuffer. */
        void* lockImpl(size_t offset, size_t length, LockOptions options);
        /** See HardwareBuffer. */
		void unlockImpl(void);
    public:
		D3D10HardwareVertexBuffer(size_t vertexSize, size_t numVertices, 
            HardwareBuffer::Usage usage, ID3D10Device * pDev, bool useSystemMem, bool useShadowBuffer);
        ~D3D10HardwareVertexBuffer();
        /** See HardwareBuffer. */
        void readData(size_t offset, size_t length, void* pDest);
        /** See HardwareBuffer. */
        void writeData(size_t offset, size_t length, const void* pSource,
				bool discardWholeBuffer = false);
		/// For dealing with lost devices - release the resource if in the default pool
		bool releaseIfDefaultPool(void);
		/// For dealing with lost devices - recreate the resource if in the default pool
		bool recreateIfDefaultPool(ID3D10Device * pDev);

        /// Get D3D10-specific vertex buffer
        ID3D10Buffer * getD3D10VertexBuffer(void) const { return mlpD3DBuffer; }


    };

}
#endif

