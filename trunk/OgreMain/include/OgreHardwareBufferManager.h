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
#ifndef __HardwareBufferManager__
#define __HardwareBufferManager__

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreSingleton.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"


namespace Ogre {

    /** Abstract singleton class for managing hardware buffers, a concrete instance
    of this will be created by the RenderSystem. */
    class _OgreExport HardwareBufferManager : public Singleton<HardwareBufferManager>
    {
    protected:
        typedef std::list<HardwareVertexBuffer*> VertexBufferList;
        typedef std::list<HardwareIndexBuffer*> IndexBufferList;
        typedef std::list<VertexDeclaration*> VertexDeclarationList;
		typedef std::list<VertexBufferBinding*> VertexBufferBindingList;
        VertexBufferList mVertexBuffers;
        IndexBufferList mIndexBuffers;
        VertexDeclarationList mVertexDeclarations;
		VertexBufferBindingList mVertexBufferBindings;

    public:
        HardwareBufferManager();
        virtual ~HardwareBufferManager();
		/** Create a hardware vertex buffer.
        @remarks
            This method creates a new vertex buffer; this will act as a source of geometry
            data for rendering objects. Note that because the meaning of the contents of
            the vertex buffer depends on the usage, this method does not specify a
            vertex format; the user of this buffer can actually insert whatever data 
            they wish, in any format. However, in order to use this with a RenderOperation,
            the data in this vertex buffer will have to be associated with a semantic element
            of the rendering pipeline, e.g. a position, or texture coordinates. This is done 
            using the VertexDeclaration class, which itself contains VertexElement structures
            referring to the source data.
        @param vertexSize The size in bytes of each vertex in this buffer; you must calculate
            this based on the kind of data you expect to populate this buffer with.
        @param numVerts The number of vertices in this buffer.
        @param usage One or more members of the HardwareBuffer::Usage enumeration.
        */
		virtual HardwareVertexBuffer* 
            createVertexBuffer(size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage) = 0;
		/// Destroy a hardware index buffer
		virtual void destroyVertexBuffer(HardwareVertexBuffer* buf) = 0;
		/// Create a hardware vertex buffer
		virtual HardwareIndexBuffer* 
            createIndexBuffer(HardwareIndexBuffer::IndexType itype, size_t numIndexes, HardwareBuffer::Usage usage) = 0;
		/// Destroy a hardware vertex buffer
		virtual void destroyIndexBuffer(HardwareIndexBuffer* buf) = 0;
        /// Creates a vertex declaration, may be overridden by certain rendering APIs
        virtual VertexDeclaration* createVertexDeclaration(void);
        /// Destroys a vertex declaration, may be overridden by certain rendering APIs
        virtual void destroyVertexDeclaration(VertexDeclaration* decl);

		/** Creates a new VertexBufferBinding. */
		virtual VertexBufferBinding* createVertexBufferBinding(void);
		/** Destroys a VertexBufferBinding. */
		virtual void destroyVertexBufferBinding(VertexBufferBinding* binding);

        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton
                implementation is in a .h file, which means it gets compiled
                into anybody who includes it. This is needed for the
                Singleton template to work, but we actually only want it
                compiled into the implementation of the class based on the
                Singleton, not all of them. If we don't change this, we get
                link errors when trying to use the Singleton-based class from
                an outside dll.
            @par
                This method just delegates to the template version anyway,
                but the implementation stays in this single compilation unit,
                preventing link errors.
        */
        static HardwareBufferManager& getSingleton(void);
    };

}

#endif

