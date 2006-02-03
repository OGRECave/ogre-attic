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

#include "OgreStableHeaders.h"
#include "OgreBillboardChain.h"

#include "OgreSimpleRenderable.h"
#include "OgreHardwareBufferManager.h"
#include "OgreNode.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "OgreMaterialManager.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

namespace Ogre {
#define NOT_VALID 0xffffffff
	//-----------------------------------------------------------------------
	BillboardChain::Element::Element()
	{
	}
	//-----------------------------------------------------------------------
	BillboardChain::Element::Element(Vector3 _position,
		Real _width,
		Real _uTexCoord,
		ColourValue _colour) :
	position(_position),
		width(_width),
		uTexCoord(_uTexCoord),
		colour(_colour)
	{
	}
	//-----------------------------------------------------------------------
	BillboardChain::BillboardChain(const String& name, size_t maxElements, 
		size_t numberOfChains, bool useTextureCoords, bool useColours, bool dynamic)
		:MovableObject(name),
		mMaxElementsPerChain(maxElements),
		mChainCount(numberOfChains),
		mUseTexCoords(useTextureCoords),
		mUseVertexColour(useColours),
		mDynamic(dynamic),
		mVertexDeclDirty(true),
		mBuffersNeedRecreating(true),
		mBoundsDirty(true),
		mIndexContentDirty(true),
		mRadius(0.0f)
	{
		mVertexData = new VertexData();
		mIndexData = new IndexData();

		setupChainContainers();

		mVertexData->vertexStart = 0;
		// index data set up later
		// set basic white material
		this->setMaterialName("BaseWhiteNoLighting");

	}
	//-----------------------------------------------------------------------
	BillboardChain::~BillboardChain()
	{
		delete mVertexData;
		delete mIndexData;
	}
	//-----------------------------------------------------------------------
	void BillboardChain::setupChainContainers(void)
	{
		// Allocate enough space for everything
		mChainElementList.resize(mChainCount * mMaxElementsPerChain);
		mVertexData->vertexCount = mChainElementList.size() * 2;

		// Configure chains
		mChainSegmentList.resize(mChainCount);
		for (size_t i = 0; i < mChainCount; ++i)
		{
			ChainSegment& seg = mChainSegmentList[i];
			seg.start = i * mMaxElementsPerChain;
			seg.tail = seg.head = NOT_VALID;

		}


	}
	//-----------------------------------------------------------------------
	void BillboardChain::setupVertexDeclaration(void)
	{
		if (mVertexDeclDirty)
		{
			VertexDeclaration* decl = mVertexData->vertexDeclaration;
			decl->removeAllElements();

			size_t offset = 0;
			// Add a description for the buffer of the positions of the vertices
			decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
			offset += VertexElement::getTypeSize(VET_FLOAT3);

			if (mUseVertexColour)
			{
				decl->addElement(0, offset, VET_COLOUR, VES_DIFFUSE);
				offset += VertexElement::getTypeSize(VET_COLOUR);
			}

			if (mUseTexCoords)
			{
				decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
				offset += VertexElement::getTypeSize(VET_FLOAT2);
			}

			mVertexDeclDirty = false;
		}
	}
	//-----------------------------------------------------------------------
	void BillboardChain::setupBuffers(void)
	{
		setupVertexDeclaration();
		if (mBuffersNeedRecreating)
		{
			// Create the vertex buffer (always dynamic due to the camera adjust)
			HardwareVertexBufferSharedPtr pBuffer = 
				HardwareBufferManager::getSingleton().createVertexBuffer(
				mVertexData->vertexDeclaration->getVertexSize(0), 
				mVertexData->vertexCount,
				HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

			// (re)Bind the buffer
			// Any existing buffer will lose its reference count and be destroyed
			mVertexData->vertexBufferBinding->setBinding(0, pBuffer);

			mIndexData->indexBuffer = 
				HardwareBufferManager::getSingleton().createIndexBuffer(
					HardwareIndexBuffer::IT_16BIT, 
					mChainCount * mMaxElementsPerChain * 6, // max we can use
					mDynamic? HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY : HardwareBuffer::HBU_STATIC_WRITE_ONLY);
			// NB we don't set the indexCount on IndexData here since we will
			// probably use less than the maximum number of indices

			mBuffersNeedRecreating = false;
		}
	}
	//-----------------------------------------------------------------------
	void BillboardChain::setMaxChainElements(size_t maxElements)
	{
		mMaxElementsPerChain = maxElements;
		setupChainContainers();
	}
	//-----------------------------------------------------------------------
	void BillboardChain::setNumberOfChains(size_t numChains)
	{
		mChainCount = numChains;
		setupChainContainers();
	}
	//-----------------------------------------------------------------------
	void BillboardChain::setUseTextureCoords(bool use)
	{
		mUseTexCoords = use;
		mVertexDeclDirty = mBuffersNeedRecreating = true;
	}
	//-----------------------------------------------------------------------
	void BillboardChain::setUseVertexColours(bool use)
	{
		mUseVertexColour = use;
		mVertexDeclDirty = mBuffersNeedRecreating = true;
	}
	//-----------------------------------------------------------------------
	void BillboardChain::setDynamic(bool dyn)
	{
		mDynamic = dyn;
		mBuffersNeedRecreating = mIndexContentDirty = true;
	}
	//-----------------------------------------------------------------------
	void BillboardChain::addChainElement(size_t chainIndex, 
		const BillboardChain::Element& dtls)
	{
		if (chainIndex >= mChainCount)
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"chainIndex out of bounds", 
				"BillboardChain::addChainElement");
		}
		ChainSegment& seg = mChainSegmentList[chainIndex];
		if (seg.head == NOT_VALID)
		{
			// Tail starts at end, head grows backwards
			seg.tail = mMaxElementsPerChain - 1;
			seg.head = seg.tail;
			mIndexContentDirty = true;
		}
		else
		{
			if (seg.head == 0)
			{
				// Wrap backwards
				seg.head = mMaxElementsPerChain - 1;
			}
			else
			{
				// Just step backward
				--seg.head; 
			}
			// Run out of elements?
			if (seg.head == seg.tail)
			{
				// Move tail backwards too, losing the end of the segment and re-using
				// it in the head
				if (seg.tail == 0)
					seg.tail = mMaxElementsPerChain - 1;
				else
					--seg.tail; 

				// Don't mark indexes as dirty since they still apply, only vertices change
			}
			else
			{
				// we added an additional entry so indexes need updating
				mIndexContentDirty = true;
			}
		}

		// Set the details
		mChainElementList[seg.start + seg.head] = dtls;

		mBoundsDirty = true;

	}
	//-----------------------------------------------------------------------
	void BillboardChain::removeChainElement(size_t chainIndex)
	{
		if (chainIndex >= mChainCount)
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"chainIndex out of bounds", 
				"BillboardChain::removeChainElement");
		}
		ChainSegment& seg = mChainSegmentList[chainIndex];
		if (seg.head == NOT_VALID)
			return; // do nothing, nothing to remove


		if (seg.tail == seg.head)
		{
			// last item
			seg.head = seg.tail = NOT_VALID;
		}
		else if (seg.tail == 0)
		{
			seg.tail = mMaxElementsPerChain - 1;
		}
		else
		{
			--seg.tail;
		}

		// we removed an entry so indexes need updating
		mIndexContentDirty = true;
		mBoundsDirty = true;

	}
	//-----------------------------------------------------------------------
	void BillboardChain::updateChainElement(size_t chainIndex, size_t elementIndex, 
		const BillboardChain::Element& dtls)
	{
		if (chainIndex >= mChainCount)
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"chainIndex out of bounds", 
				"BillboardChain::updateChainElement");
		}
		ChainSegment& seg = mChainSegmentList[chainIndex];
		if (seg.head == NOT_VALID)
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"Chain segment is empty", 
				"BillboardChain::updateChainElement");
		}

		size_t idx = seg.head + elementIndex;
		// adjust for the edge and start
		idx = (idx % mMaxElementsPerChain) + seg.start;

		mChainElementList[idx] = dtls;

		mBoundsDirty = true;


	}
	//-----------------------------------------------------------------------
	const BillboardChain::Element& 
	BillboardChain::getChainElement(size_t chainIndex, size_t elementIndex) const
	{

		if (chainIndex >= mChainCount)
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"chainIndex out of bounds", 
				"BillboardChain::updateChainElement");
		}
		const ChainSegment& seg = mChainSegmentList[chainIndex];

		size_t idx = seg.head + elementIndex;
		// adjust for the edge and start
		idx = (idx % mMaxElementsPerChain) + seg.start;

		return mChainElementList[idx];
	}
	//-----------------------------------------------------------------------
	void BillboardChain::updateBoundingBox(void) const
	{
		if (mBoundsDirty)
		{
			mAABB.setNull();
			Vector3 widthVector;
			for (ChainSegmentList::const_iterator segi = mChainSegmentList.begin();
				segi != mChainSegmentList.end(); ++segi)
			{
				const ChainSegment& seg = *segi;

				for(size_t e = seg.head; e != seg.tail; ++e)
				{
					// Wrap forwards
					if (e == mMaxElementsPerChain)
						e = 0;

					const Element& elem = mChainElementList[seg.start + e];

					widthVector.x = widthVector.y = widthVector.z = elem.width;
					mAABB.merge(elem.position - widthVector);
					mAABB.merge(elem.position + widthVector);

				}

			}
	
			// Set the current radius
			if (mAABB.isNull())
			{
				mRadius = 0.0f;
			}
			else
			{
				mRadius = Math::Sqrt(
					std::max(mAABB.getMinimum().squaredLength(), 
					mAABB.getMaximum().squaredLength()));
			}

			mBoundsDirty = false;
		}
	}
	//-----------------------------------------------------------------------
	void BillboardChain::updateVertexBuffer(Camera* cam)
	{
		setupBuffers();
		HardwareVertexBufferSharedPtr pBuffer =
			mVertexData->vertexBufferBinding->getBuffer(0);
		void* pBufferStart = pBuffer->lock(HardwareBuffer::HBL_DISCARD);

		Vector3 eyePos = mParentNode->_getDerivedOrientation().Inverse() *
			(cam->getDerivedPosition() - mParentNode->_getDerivedPosition());

		// TEST
		static bool logMe = true;

		Vector3 chainTangent;
		for (ChainSegmentList::iterator segi = mChainSegmentList.begin();
			segi != mChainSegmentList.end(); ++segi)
		{
			ChainSegment& seg = *segi;

			// Skip 0 or 1 element segment counts
			if (seg.head != NOT_VALID && seg.head != seg.tail)
			{
				size_t laste = seg.head;
				bool first = true;
				for (size_t e = seg.head; ; ++e) // until break
				{
					// Wrap forwards
					if (e == mMaxElementsPerChain)
						e = 0;

					Element& elem = mChainElementList[e + seg.start];
					uint16 baseIdx = (e + seg.start) * 2;
					uint16 lastBaseIdx = (laste + seg.start) * 2;

					// Determine base pointer to vertex #1
					void* pBase = static_cast<void*>(
						static_cast<char*>(pBufferStart) + 
							pBuffer->getVertexSize() * baseIdx);

					if (first) 
					{
						// No laste, use next item
						size_t nexte = e + 1;
						if (nexte == mMaxElementsPerChain)
							nexte = 0;
						chainTangent = mChainElementList[nexte + seg.start].position - elem.position;
						first = false;
					}
					else 
					{
						chainTangent = elem.position - mChainElementList[laste + seg.start].position;
					}

					Vector3 vP1ToEye = eyePos - elem.position;
					Vector3 vPerpendicular = chainTangent.crossProduct(vP1ToEye);
					vPerpendicular.normalise();
					vPerpendicular *= (elem.width * 0.5);

					Vector3 pos0 = elem.position - vPerpendicular;
					Vector3 pos1 = elem.position + vPerpendicular;

					// TEST
					if (logMe)
					{
						{
							StringUtil::StrStreamType s;
							s << "V" << baseIdx << " pos: " << pos0;
							LogManager::getSingleton().logMessage(s.str());
							s.str("");
							s << "V" << baseIdx+1 << " pos: " << pos1;
							LogManager::getSingleton().logMessage(s.str());
						}
					}

					float* pFloat = static_cast<float*>(pBase);
					// pos1
					*pFloat++ = pos0.x;
					*pFloat++ = pos0.y;
					*pFloat++ = pos0.z;

					pBase = static_cast<void*>(pFloat);

					if (mUseVertexColour)
					{
						RGBA* pCol = static_cast<RGBA*>(pBase);
						Root::getSingleton().convertColourValue(elem.colour, pCol);
						pCol++;
						pBase = static_cast<void*>(pCol);
					}

					if (mUseTexCoords)
					{
						pFloat = static_cast<float*>(pBase);
						*pFloat++ = elem.uTexCoord;
						*pFloat++ = 0;
						pBase = static_cast<void*>(pFloat);
					}

					// pos2
					pFloat = static_cast<float*>(pBase);
					*pFloat++ = pos1.x;
					*pFloat++ = pos1.y;
					*pFloat++ = pos1.z;
					pBase = static_cast<void*>(pFloat);

					if (mUseVertexColour)
					{
						RGBA* pCol = static_cast<RGBA*>(pBase);
						Root::getSingleton().convertColourValue(elem.colour, pCol);
						pCol++;
						pBase = static_cast<void*>(pCol);
					}

					if (mUseTexCoords)
					{
						pFloat = static_cast<float*>(pBase);
						*pFloat++ = elem.uTexCoord;
						*pFloat++ = 1;
						pBase = static_cast<void*>(pFloat);
					}

					if (e == seg.tail)
						break; // last one

					laste = e;

				} // element
			} // segment valid?

		} // each segment



		pBuffer->unlock();

		// TEST
		logMe = false;

	}
	//-----------------------------------------------------------------------
	void BillboardChain::updateIndexBuffer(void)
	{
		// TEST
		bool logMe = true;

		setupBuffers();
		if (mIndexContentDirty)
		{

			uint16* pShort = static_cast<uint16*>(
				mIndexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
			mIndexData->indexCount = 0;
			// indexes
			for (ChainSegmentList::iterator segi = mChainSegmentList.begin();
				segi != mChainSegmentList.end(); ++segi)
			{
				ChainSegment& seg = *segi;

				// Skip 0 or 1 element segment counts
				if (seg.head != NOT_VALID && seg.head != seg.tail)
				{
					// Start from head + 1 since it's only useful in pairs
					size_t laste = seg.head;
					while(1) // until break
					{
						size_t e = laste + 1;
						// Wrap forwards
						if (e == mMaxElementsPerChain)
							e = 0;
						// indexes of this element are (e * 2) and (e * 2) + 1
						// indexes of the last element are the same, -2
						uint16 baseIdx = (e + seg.start) * 2;
						uint16 lastBaseIdx = (laste + seg.start) * 2;
						*pShort++ = lastBaseIdx;
						*pShort++ = lastBaseIdx + 1;
						*pShort++ = baseIdx;
						*pShort++ = lastBaseIdx + 1;
						*pShort++ = baseIdx + 1;
						*pShort++ = baseIdx;

						if (logMe)
						{
							StringUtil::StrStreamType s;
							s << "I: " << lastBaseIdx 
								<< " " << lastBaseIdx + 1
								<< " " << baseIdx
								<< " " << lastBaseIdx + 1
								<< " " << baseIdx + 1
								<< " " << baseIdx;
							LogManager::getSingleton().logMessage(s.str());
						}
						
						mIndexData->indexCount += 6;


						if (e == seg.tail)
							break; // last one

						laste = e;

					}
				}

			}
			mIndexData->indexBuffer->unlock();

			mIndexContentDirty = false;
		}

		// TEST
		logMe = false;

	}
	//-----------------------------------------------------------------------
	void BillboardChain::_notifyCurrentCamera(Camera* cam)
	{
		updateVertexBuffer(cam);
	}
	//-----------------------------------------------------------------------
	Real BillboardChain::getSquaredViewDepth(const Camera* cam) const
	{
		Vector3 min, max, mid, dist;
		min = mAABB.getMinimum();
		max = mAABB.getMaximum();
		mid = ((max - min) * 0.5) + min;
		dist = cam->getDerivedPosition() - mid;

		return dist.squaredLength();
	}
	//-----------------------------------------------------------------------
	Real BillboardChain::getBoundingRadius(void) const
	{
		return mRadius;
	}
	//-----------------------------------------------------------------------
	const AxisAlignedBox& BillboardChain::getBoundingBox(void) const
	{
		updateBoundingBox();
		return mAABB;
	}
	//-----------------------------------------------------------------------
	const MaterialPtr& BillboardChain::getMaterial(void) const
	{
		return mMaterial;
	}
	//-----------------------------------------------------------------------
	void BillboardChain::setMaterialName(const String& name)
	{
		mMaterialName = name;
		mMaterial = MaterialManager::getSingleton().getByName(mMaterialName);

		if (mMaterial.isNull())
		{
			LogManager::getSingleton().logMessage("Can't assign material " + name + 
				" to BillboardChain " + mName + " because this "
				"Material does not exist. Have you forgotten to define it in a "
				".material script?");
			mMaterial = MaterialManager::getSingleton().getByName("BaseWhiteNoLighting");
			if (mMaterial.isNull())
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Can't assign default material "
					"to BillboardChain of " + mName + ". Did "
					"you forget to call MaterialManager::initialise()?",
					"BillboardChain.setMaterialName");
			}
		}
		// Ensure new material loaded (will not load again if already loaded)
		mMaterial->load();	
	}
	//-----------------------------------------------------------------------
	const String& BillboardChain::getMovableType(void) const
	{
		return BillboardChainFactory::FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------
	void BillboardChain::_updateRenderQueue(RenderQueue* queue)
	{
		updateIndexBuffer();

		if (mIndexData->indexCount > 0)
		{
			queue->addRenderable(this);
		}

	}
	//-----------------------------------------------------------------------
	void BillboardChain::getRenderOperation(RenderOperation& op)
	{
		op.indexData = mIndexData;
		op.operationType = RenderOperation::OT_TRIANGLE_LIST;
		op.srcRenderable = this;
		op.useIndexes = true;
		op.vertexData = mVertexData;
	}
	//-----------------------------------------------------------------------
	void BillboardChain::getWorldTransforms(Matrix4* xform) const
	{
		*xform = _getParentNodeFullTransform();
	}
	//-----------------------------------------------------------------------
	const Quaternion& BillboardChain::getWorldOrientation(void) const
	{
		return getParentNode()->_getDerivedOrientation();
	}
	//-----------------------------------------------------------------------
	const Vector3& BillboardChain::getWorldPosition(void) const
	{
		return getParentNode()->_getDerivedPosition();
	}
	//-----------------------------------------------------------------------
	const LightList& BillboardChain::getLights(void) const
	{
		return getParentSceneNode()->findLights(getBoundingRadius());
	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	String BillboardChainFactory::FACTORY_TYPE_NAME = "BillboardChain";
	//-----------------------------------------------------------------------
	const String& BillboardChainFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------
	MovableObject* BillboardChainFactory::createInstanceImpl( const String& name,
		const NameValuePairList* params)
	{
		size_t maxElements = 20;
		size_t numberOfChains = 1;
		bool useTex = true;
		bool useCol = true;
		bool dynamic = true;
		// optional params
		if (params != 0)
		{
			NameValuePairList::const_iterator ni = params->find("maxElements");
			if (ni != params->end())
			{
				maxElements = StringConverter::parseUnsignedLong(ni->second);
			}
			ni = params->find("numberOfChains");
			if (ni != params->end())
			{
				numberOfChains = StringConverter::parseUnsignedLong(ni->second);
			}
			ni = params->find("useTextureCoords");
			if (ni != params->end())
			{
				useTex = StringConverter::parseBool(ni->second);
			}
			ni = params->find("useVertexColours");
			if (ni != params->end())
			{
				useCol = StringConverter::parseBool(ni->second);
			}
			ni = params->find("dynamic");
			if (ni != params->end())
			{
				dynamic = StringConverter::parseBool(ni->second);
			}

		}

		return new BillboardChain(name, maxElements, numberOfChains, useTex, useCol, dynamic);

	}
	//-----------------------------------------------------------------------
	void BillboardChainFactory::destroyInstance( MovableObject* obj)
	{
		delete obj;
	}

}

