/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __VertexIndexData_H__
#define __VertexIndexData_H__

#include "OgrePrerequisites.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre {

	/** Summary class collecting together vertex source information. */
	class _OgreExport VertexData
	{
    private:
        /// Protected copy constructor, to prevent misuse
        VertexData(const VertexData& rhs) { /* do nothing, should not use */}
        /// Protected operator=, to prevent misuse
        VertexData& operator=(const VertexData& rhs) { return *this; /* do not use */}
    public:
        VertexData();
        ~VertexData();

		/** Declaration of the vertex to be used in this operation. 
		@remarks Note that this is created for you on construction.
		*/
		VertexDeclaration* vertexDeclaration;
		/** The vertex buffer bindings to be used. 
		@remarks Note that this is created for you on construction.
		*/
		VertexBufferBinding* vertexBufferBinding;
		/// The base vertex index to start from, if using unindexed geometry
		size_t vertexStart;
		/// The number of vertices used in this operation
		size_t vertexCount;

		/** Clones this vertex data, potentially including replicating any vertex buffers.
		@remarks The caller is expected to delete the returned pointer when ready
		*/
		VertexData* clone(bool copyData = true) const;

        /** Modifies the vertex data to be suitable for use for rendering shadow geometry.
        @remarks
            Preparing vertex data to generate a shadow volume involves firstly ensuring that the 
            vertex buffer containing the positions is a standalone vertex buffer,
            with no other components in it. This method will therefore break apart any existing
            vertex buffers if position is sharing a vertex buffer. 
            Secondly, it will double the size of this vertex buffer so that there are 2 copies of 
            the position data for the mesh. The first half is used for the original, and the second 
            half is used for the 'extruded' version. The vertex count used to render will remain 
            the same though, so as not to add any overhead to regular rendering of the object.
            Both copies of the position are required in one buffer because shadow volumes stretch 
            from the original mesh to the extruded version. 
        @par
            It's important to appreciate that this method can fundamentally change the structure of your
            vertex buffers, although in reality they will be new buffers. As it happens, if other 
            objects are using the original buffers then they will be unaffected because the reference
            counting will keep them intact. However, if you have made any assumptions about the 
            structure of the vertex data in the buffers of this object, you may have to rethink them.
        */
        void prepareForShadowVolume(void);

        /** Additional shadow volume vertex buffer storage. 
        @remarks
            This additional buffer is only used where we have prepared this VertexData for
            use in shadow volume contruction, and where the current render system supports
            vertex programs. This buffer contains the 'w' vertex position component which will
            be used by that program to differentiate between extruded and non-extruded vertices.
            This 'w' component cannot be included in the original position buffer because
            DirectX does not allow 4-component positions in the fixed-function pipeline, and the original
            position buffer must still be usable for fixed-function rendering.
        @par    
            Note that we don't store any vertex declaration or vertex buffer binding here becuase this
            can be reused in the shadow algorithm.
        */
        HardwareVertexBufferSharedPtr hardwareShadowVolWBuffer;


	};

	/** Summary class collecting together index data source information. */
	class _OgreExport IndexData 
	{
    protected:
        /// Protected copy constructor, to prevent misuse
        IndexData(const IndexData& rhs) { /* do nothing, should not use */}
        /// Protected operator=, to prevent misuse
        IndexData& operator=(const IndexData& rhs) { return *this;/* do not use */}
    public:
        IndexData();
        ~IndexData();
		/// pointer to the HardwareIndexBuffer to use, must be specified if useIndexes = true
		HardwareIndexBufferSharedPtr indexBuffer;

		/// index in the buffer to start from for this operation
		size_t indexStart;

		/// The number of indexes to use from the buffer
		size_t indexCount;

		/** Clones this index data, potentially including replicating the index buffer.
		@remarks The caller is expected to delete the returned pointer when finished
		*/
		IndexData* clone(bool copyData = true) const;
	};


}
#endif

