/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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
#ifndef _RenderOperation_H__
#define _RenderOperation_H__

#include "OgrePrerequisites.h"
#include "OgreColourValue.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre {

	/** Summary class collecting together vertex source information. */
	class VertexData
	{
    public:
        VertexData() {}
        ~VertexData() {}

		/** Declaration of the vertex to be used in this operation. */
		VertexDeclaration vertexDeclaration;
		/** The vertex buffer bindings to be used. */
		VertexBufferBinding vertexBufferBinding;
		/// The base vertex index to start from, either as a vertex list, or as a base for indexes
		size_t vertexStart;
		/// The number of vertices used in this operation
		size_t vertexCount;

	};

	/** Summary class collecting together index data source information. */
	class IndexData 
	{
    public:
        IndexData() {}
        ~IndexData() {}
		/// pointer to the HardwareIndexBuffer to use, must be specified if useIndexes = true
		HardwareIndexBufferSharedPtr indexBuffer;

		/// index in the buffer to start from for this operation
		size_t indexStart;

		/// The number of indexes to use from the buffer
		size_t indexCount;

	};


	/** 'New' rendering operation using vertex buffers. */
	class RenderOperation {
	public:
		/// The rendering operation type to perform
		enum OperationType {
			/// A list of points, 1 vertex per point
            OT_POINT_LIST,
			/// A list of lines, 2 vertices per line
            OT_LINE_LIST,
			/// A strip of connected lines, 1 vertex per line plus 1 start vertex
            OT_LINE_STRIP,
			/// A list of triangles, 3 vertices per triangle
            OT_TRIANGLE_LIST,
			/// A strip of triangles, 3 vertices for the first triangle, and 1 per triangle after that 
            OT_TRIANGLE_STRIP,
			/// A fan of triangles, 3 vertices for the first triangle, and 1 per triangle after that
            OT_TRIANGLE_FAN
        };

		/// Vertex source data
		VertexData *vertexData;

		/// The type of operation to perform
		OperationType operationType;

		/** Specifies whether to use indexes to determine the vertices to use as input. If false, the vertices are
		 simply read in sequence to define the primitives. If true, indexes are used instead to identify vertices
		 anywhere in the buffer, and allowing vertices to be used more than once.
	   	 If true, then the indexBuffer, indexStart and numIndexes properties must be valid. */
		bool useIndexes;

		/// Index data - only valid if useIndexes is true
		IndexData *indexData;

	};
}



#endif
