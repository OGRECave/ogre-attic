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
#ifndef __HardwareVertexBuffer__
#define __HardwareVertexBuffer__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreHardwareBuffer.h"
#include "OgreSharedPtr.h"

namespace Ogre {
    /** Specialisation of HardwareBuffer for a vertex buffer. */
    class _OgreExport HardwareVertexBuffer : public HardwareBuffer
    {
	    protected:
		    
		    size_t mNumVertices;
            size_t mVertexSize;

	    public:
		    /// Should be called by HardwareBufferManager
		    HardwareVertexBuffer(size_t vertexSize, size_t numVertices, 
                HardwareBuffer::Usage usage);
            /// Gets the size in bytes of a single vertex in this buffer
            size_t getVertexSize(void) const { return mVertexSize; }
            /// Get the number of vertices in this buffer
            size_t getNumVertices(void) const { return mNumVertices; }
    		


		    // NB subclasses should override lock, unlock, readData, writeData
    	
    };

    /** Shared pointer implementation used to share index buffers. */
    class _OgreExport HardwareVertexBufferSharedPtr : public SharedPtr<HardwareVertexBuffer>
    {
    public:
        HardwareVertexBufferSharedPtr() : SharedPtr<HardwareVertexBuffer>() {}
        HardwareVertexBufferSharedPtr(HardwareVertexBuffer* buf);
        /// Just override the destroy method to desotry through factory
        void destroy(void);


    };

    /// Vertex element semantics, used to identify the meaning of vertex buffer contents
	enum VertexElementSemantic {
		/// Position, 3 reals per vertex
		VES_POSITION,
		/// Normal, 3 reals per vertex
		VES_NORMAL,
		/// Blending weights
		VES_BLEND_WEIGHTS,
        /// Blending indices
        VES_BLEND_INDICES,
		/// Diffuse colours
		VES_DIFFUSE,
		/// Specular colours
		VES_SPECULAR,
		/// Texture coordinates
		VES_TEXTURE_COORDINATES
	};

    /// Vertex element type, used to identify the base types of the vertex contents
    enum VertexElementType
    {
        VET_FLOAT1,
        VET_FLOAT2,
        VET_FLOAT3,
        VET_FLOAT4,
        VET_COLOUR
    };

    /** This class declares the usage of a single vertex buffer as a component
        of a complete VertexDeclaration. 
        @remarks
        Several vertex buffers can be used to supply the input geometry for a
        rendering operation, and in each case a vertex buffer can be used in
        different ways for different operations; the buffer itself does not
        define the semantics (position, normal etc), the VertexElement
        class does.
    */
    class _OgreExport VertexElement
    {
    protected:
        /// The source vertex buffer, as bound to an index using VertexBufferBinding
        unsigned short mSource;
        /// The offset in the buffer that this element starts at
        size_t mOffset;
        /// The type of element
        VertexElementType mType;
        /// The meaning of the element
        VertexElementSemantic mSemantic;
        /// Index of the item, only applicable for some elements like texture coords
        unsigned short mIndex;
    public:
        /// Constructor, should not be called directly, call VertexDeclaration::addElement
        VertexElement(unsigned short source, size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);
        /// Gets the vertex buffer index from where this element draws it's values
        unsigned short getSource(void) const { return mSource; }
        /// Gets the offset into the buffer where this element starts
        size_t getOffset(void) const { return mOffset; }
        /// Gets the data format of this element
        VertexElementType getType(void) const { return mType; }
        /// Gets the meaning of this element
        VertexElementSemantic getSemantic(void) const { return mSemantic; }
        /// Gets the index of this element, only applicable for repeating elements
        unsigned short getIndex(void) const { return mIndex; }

    };
    /** This class declares the format of a set of vertex inputs, which
        can be issued to the rendering API through a RenderOperation. 
	@remarks
		Like the other classes in this functional area, these declarations should be created and
		destroyed using the HardwareBufferManager.
    */
    class _OgreExport VertexDeclaration
    {
    public:
		/// Defines the list of vertex elements that makes up this declaration
        typedef std::vector<VertexElement> VertexElementList;
    protected:
        VertexElementList mElementList;
    public:
        /// Standard constructor, not you should use HardwareBufferManager::createVertexDeclaration
        VertexDeclaration();
        virtual ~VertexDeclaration();
        
        /** Gets read-only access to the list of vertex elements. */
        const VertexElementList& getElements(void) const;

        /** Adds a new VertexElement to this declaration. 
        @remarks
            This method adds a single element (positions, normals etc) to the
            vertex declaration. <b>Note that on some APIs such as D3D there are 
            restrictions on the ordering of the vertex elements</b> on older drivers, 
            so for maximum compatibility you should order your elements like this:
            position, blending weights, normals, diffuse colours, specular colours, 
            texture coordinates (in order, with no gaps).
        @param source The binding index of HardwareVertexBuffer which will provide the source for this element.
			See VertexBufferBindingState for full information.
        @param offset The offset in bytes where this element is located in the buffer
        @param theType The data format of the element (3 floats, a colour etc)
        @param semantic The meaning of the data (position, normal, diffuse colour etc)
        @param index Optional index for multi-input elements like texture coordinates
        */
        virtual void addElement(unsigned short source, size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);

        /** Remove the element at the given index from this declaration. */
        virtual void removeElement(unsigned short elem_index);

        /** Modify an element in-place, params as addElement. */
        virtual void modifyElement(unsigned short elem_index, unsigned short source, size_t offset, VertexElementType theType,
            VertexElementSemantic semantic, unsigned short index = 0);


    };

	/** Records the state of all the vertex buffer bindings required to provide a vertex declaration
		with the input data it needs for the vertex elements.
	@remarks
		Why do we have this binding list rather than just have VertexElement referring to the
		vertex buffers direct? Well, in the underlying APIs, binding the vertex buffers to an
		index (or 'stream') is the way that vertex data is linked, so this structure better
		reflects the realities of that. In addition, by separating the vertex declaration from
		the list of vertex buffer bindings, it becomes possible to reuse bindings between declarations
		and vice versa, giving opportunities to reduce the state changes required to perform rendering.
	@par
		Like the other classes in this functional area, these binding maps should be created and
		destroyed using the HardwareBufferManager.
	*/
	class _OgreExport VertexBufferBinding
	{
	public:
		/// Defines the vertex buffer bindings used as source for vertex declarations
		typedef std::map<unsigned short, HardwareVertexBufferSharedPtr> VertexBufferBindingMap;
	protected:
		VertexBufferBindingMap mBindingMap;
	public:
		/// Constructor, should not be called direct, use HardwareBufferManager::createVertexBufferBinding
		VertexBufferBinding();
		virtual ~VertexBufferBinding();
		/** Set a binding, associating a vertex buffer with a given index. 
		@remarks
			If the index is already associated with a vertex buffer, the association will be replaced.
		*/
		virtual void setBinding(unsigned short index, HardwareVertexBufferSharedPtr buffer);
		/** Removes an existing binding. */
		virtual void unsetBinding(unsigned short index);

        /** Removes all the bindings. */
        virtual void unsetAllBindings(void);

		/// Gets a read-only version of the buffer bindings
		virtual const VertexBufferBindingMap& getBindings(void) const;

		/// Gets the buffer bound to the given source index
		virtual HardwareVertexBufferSharedPtr getBuffer(unsigned short index);



	};



}
#endif

