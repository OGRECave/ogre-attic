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

		/** Clones this vertex data, including replicating any vertex buffers.
		@remarks The caller is expected to delete the returned pointer when ready
		*/
		VertexData* clone(void);

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
            from the original mesh to the extruded version. Note that <strong>the position type
            will be changed to VET_FLOAT4</strong>, so do not assume VET_FLOAT3; if in doubt, check.
        @par
            It's important to appreciate that this method can fundamentally change the structure of your
            vertex buffers, although in reality they will be new buffers. As it happens, if other 
            objects are using the original buffers then they will be unaffected because the reference
            counting will keep them intact. However, if you have made any assumptions about the 
            structure of the vertex data in the buffers of this object, you may have to rethink them.
        */
        void prepareForShadowVolume(void);


		
        /** Software vertex blend information.
        @remarks
            This data is here in order to allow the creator of the VertexData 
            to request a software vertex blend, ie a blend using information which you
            do not want to be passed to the GPU.
        @par
            The assumption here is that you have a VES_POSITION and VES_NORMAL elements in 
            your declaration which you wish to update with a blended version of 
            positions / normals from a system-memory location. We advise that 
            if you're blending a lot, you set the hardware vertex buffer 
            to HBU_DYNAMIC_WRITE_ONLY, with no shadow buffer. 
        @par    
            Note that future versions of the engine are likely to support vertex shader
            based animation so there will be a hardware alternative; however, note that sometimes
            you may still want to perform blending in software, for example when you need to read
            back the blended positions in applications such as shadow volume construction.
        @par
            In order to apply this blending, the world matrices must be set and 
            RenderSystem::softwareVertexBlend called. This is done automatically for skeletally
            animated entities, but this can be done manually if required. After calling this
            method, the vertex buffers are updated with the blended positions and the blend does
            not need to be called again unless it's basis changes.
        */
        class SoftwareBlendInfo
        {
        public:
            /** If true, the RenderSystem will automatically apply the blend when rendering 
            with this vertexData, otherwise the user of the vertex data must call 
            RenderSystem::sofwareVertexBlend manually as required. */
            bool automaticBlend;
            /// System-memory pointer to source positions, note this will be deleted when this class is destroyed
            Real* pSrcPositions;
            /** System-memory pointer to source normals, can be null if vertexData does not include normals
                , note this will be deleted when this class is destroyed
            */
            Real* pSrcNormals;
            /// The number of blending weights per vertex, will be deleted on destruction
            unsigned short numWeightsPerVertex;
            /// Pointer to blending weights, will be deleted on destruction
            Real* pBlendWeights;
            /// Pointer to blending indexes (index into world matrices)
            unsigned char* pBlendIndexes;
            
            SoftwareBlendInfo() : automaticBlend(true), pSrcPositions(0), pSrcNormals(0),
                numWeightsPerVertex(1), pBlendWeights(0), pBlendIndexes(0) {}
            ~SoftwareBlendInfo();
        };
        /// Software vertex blend information
        SoftwareBlendInfo* softwareBlendInfo;

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

