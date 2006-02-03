/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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

// Thanks to Vincent Cantin (karmaGfa) for the original implementation of this
// class, although it has not been mostly rewritten

#ifndef _BillboardChain_H__
#define _BillboardChain_H__

#include "OgrePrerequisites.h"

#include "OgreMovableObject.h"
#include "OgreRenderable.h"

namespace Ogre {


	/** Allows the rendering of a chain of connected billboards.
	*/
	class _OgreExport BillboardChain : public MovableObject, public Renderable
	{

	public:

		/** Contains the data of an element of the BillboardChain.
		*/
		class _OgreExport Element
		{

		public:

			Element();

			Element(Vector3 position,
				Real width,
				Real uTexCoord,
				ColourValue colour);

			Vector3 position;
			Real width;
			Real uTexCoord;
			ColourValue colour;

		};
		typedef std::vector<Element> ElementList;

		/** Constructor (don't use directly, use factory) 
		@param name The name to give this object
		@param maxElements The maximum number of elements per chain
		@param numberOfChains The number of separate chain segments contained in this object
		@param useTextureCoords If true, use texture coordinates from the chain elements
		@param useVertexColours If true, use vertex colours from the chain elements
		@param dynamic If true, buffers are created with the intention of being updated
		*/
		BillboardChain(const String& name, size_t maxElements = 20, size_t numberOfChains = 1, 
			bool useTextureCoords = true, bool useColours = true, bool dynamic = true);
		/// destructor
		virtual ~BillboardChain();

		/** Set the maximum number of chain elements per chain 
		*/
		void setMaxChainElements(size_t maxElements);
		/** Get the maximum number of chain elements per chain 
		*/
		size_t getMaxChainElements(void) const { return mMaxElementsPerChain; }
		/** Set the number of chain segments (this class can render multiple chains
			at once using the same material). 
		*/
		void setNumberOfChains(size_t numChains);
		/** Get the number of chain segments (this class can render multiple chains
		at once using the same material). 
		*/
		size_t getNumberOfChains(void) const { return mChainCount; }

		/** Sets whether texture coordinate information should be included in the
			final buffers generated.
		@note You must use either texture coordinates or vertex colour since the
			vertices have no normals and without one of these there is no source of
			colour for the vertices.
		*/
		void setUseTextureCoords(bool use);
		/** Gets whether texture coordinate information should be included in the
			final buffers generated.
		*/
		bool getUseTextureCoords(void) const { return mUseTexCoords; }
		/** Sets whether vertex colour information should be included in the
			final buffers generated.
		@note You must use either texture coordinates or vertex colour since the
			vertices have no normals and without one of these there is no source of
			colour for the vertices.
		*/
		void setUseVertexColours(bool use);
		/** Gets whether vertex colour information should be included in the
			final buffers generated.
		*/
		bool getUseVertexColours(void) const { return mUseVertexColour; }

		/** Sets whether or not the buffers created for this object are suitable
			for dynamic alteration.
		*/
		void setDynamic(bool dyn);

		/** Gets whether or not the buffers created for this object are suitable
			for dynamic alteration.
		*/
		bool getDynamic(void) const { return mDynamic; }
		
		/** Add an element to the 'head' of a chain.
		@remarks
			If this causes the number of elements to exceed the maximum elements
			per chain, the last element in the chain (the 'tail') will be removed
			to allow the additional element to be added.
		@param chainIndex The index of the chain
		@param billboardChainElement The details to add
		*/
		void addChainElement(size_t chainIndex, 
			const Element& billboardChainElement);
		/** Remove an element from the 'tail' of a chain.
		@param chainIndex The index of the chain
		*/
		void removeChainElement(size_t chainIndex);
		/** Update the details of an existing chain element.
		@param chainIndex The index of the chain
		@param elementIndex The element index within the chain, measured from 
			the 'head' of the chain
		@param billboardChainElement The details to set
		*/
		void updateChainElement(size_t chainIndex, size_t elementIndex, 
			const Element& billboardChainElement);
		/** Get the detail of a chain element.
		@param chainIndex The index of the chain
		@param elementIndex The element index within the chain, measured from
			the 'head' of the chain
		*/
		const Element& getChainElement(size_t chainIndex, size_t elementIndex) const;
		/// Get the material name in use
		const String& getMaterialName(void) const { return mMaterialName; }
		/// Set the material name to use for rendering
		void setMaterialName(const String& name);

		void _notifyCurrentCamera(Camera* cam);
		Real getSquaredViewDepth(const Camera* cam) const;
		Real getBoundingRadius(void) const;
		const AxisAlignedBox& getBoundingBox(void) const;
		const MaterialPtr& getMaterial(void) const;
		const String& getMovableType(void) const;
		void _updateRenderQueue(RenderQueue *);
		void getRenderOperation(RenderOperation &);
		void getWorldTransforms(Matrix4 *) const;
		const Quaternion& getWorldOrientation(void) const;
		const Vector3& getWorldPosition(void) const;
		const LightList& getLights(void) const;



	protected:

		/// Maximum length of each chain
		size_t mMaxElementsPerChain;
		/// Number of chains
		size_t mChainCount;
		/// Use texture coords?
		bool mUseTexCoords;
		/// Use vertex colour?
		bool mUseVertexColour;
		/// Dynamic use?
		bool mDynamic;
		/// Vertex data
		VertexData* mVertexData;
		/// Index data (to allow multiple unconnected chains)
		IndexData* mIndexData;
		/// Is the vertex declaration dirty?
		bool mVertexDeclDirty;
		/// Do the buffers need recreating?
		bool mBuffersNeedRecreating;
		/// Do the bounds need redefining?
		mutable bool mBoundsDirty;
		/// Is the index buffer dirty?
		bool mIndexContentDirty;
		/// AABB
		mutable AxisAlignedBox mAABB;
		/// Bounding radius
		mutable Real mRadius;
		/// Material 
		String mMaterialName;
		MaterialPtr mMaterial;


		/// The list holding the chain elements
		ElementList mChainElementList;

		/** Simple struct defining a chain segment by referencing a subset of
			the preallocated buffer (which will be mMaxElementsPerChain * mChainCount
			long), by it's chain index, and a head and tail value which describe
			the current chain. The buffer subset wraps at mMaxElementsPerChain
			so that head and tail can move freely. head and tail are inclusive,
			when the chain is empty head and tail are filled with high-values.
		*/
		struct ChainSegment
		{
			/// The start of this chains subset of the buffer
			size_t start;
			/// The 'head' of the chain, relative to start
			size_t head;
			/// The 'tail' of the chain, relative to start
			size_t tail;
		};
		typedef std::vector<ChainSegment> ChainSegmentList;
		ChainSegmentList mChainSegmentList;

		/// Setup the STL collections
		void setupChainContainers(void);
		/// Setup vertex declaration
		void setupVertexDeclaration(void);
		// Setup buffers
		void setupBuffers(void);
		/// Update the contents of the vertex buffer
		void updateVertexBuffer(Camera* cam);
		/// Update the contents of the index buffer
		void updateIndexBuffer(void);
		void updateBoundingBox(void) const;
	};


	/** Factory object for creating BillboardChain instances */
	class _OgreExport BillboardChainFactory : public MovableObjectFactory
	{
	protected:
		MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params);
	public:
		BillboardChainFactory() {}
		~BillboardChainFactory() {}

		static String FACTORY_TYPE_NAME;

		const String& getType(void) const;
		void destroyInstance( MovableObject* obj);  

	};


} // namespace

#endif

