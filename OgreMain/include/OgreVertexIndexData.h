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
#ifndef __VertexIndexData_H__
#define __VertexIndexData_H__

#include "OgrePrerequisites.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre {

	/** Summary class collecting together vertex source information. */
	class _OgreExport VertexData
	{
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
		/// The base vertex index to start from, either as a vertex list, or as a base for indexes
		size_t vertexStart;
		/// The number of vertices used in this operation
		size_t vertexCount;

		/** Clones this vertex data, including replicating any vertex buffers.
		@remarks The caller is expected to delete the returned pointer when ready
		*/
		VertexData* clone(void);

		
        /** Gets the positional bounds of this vertex data.
        REMOVED BECAUSE NOT COMPATIBLE WITH WRITE-ONLY BUFFERS
		@remarks The results of this vertex data will be combined with any
		existing values in the input parameters, so ensure they are initialised.
		@param box Pointer to an AxisAlignedBox to update
		@param maxSquaredRadius Pointer to a Real to update with the squared radius
		void getBounds(AxisAlignedBox *box, Real *maxSquaredRadius);
		*/

	};

	/** Summary class collecting together index data source information. */
	class _OgreExport IndexData 
	{
    public:
        IndexData();
        ~IndexData();
		/// pointer to the HardwareIndexBuffer to use, must be specified if useIndexes = true
		HardwareIndexBufferSharedPtr indexBuffer;

		/// index in the buffer to start from for this operation
		size_t indexStart;

		/// The number of indexes to use from the buffer
		size_t indexCount;

		/** Clones this index data, including replicating the index buffer.
		@remarks The caller is expected to delete the returned pointer when finished
		*/
		IndexData* clone(void);
	};


}
#endif

