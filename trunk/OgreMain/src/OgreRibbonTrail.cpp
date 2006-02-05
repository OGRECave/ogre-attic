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
#include "OgreStableHeaders.h"
#include "OgreRibbonTrail.h"
#include "OgreMath.h"
#include "OgreException.h"
#include "OgreSceneNode.h"

namespace Ogre
{
	/** Predefined controller value for getting / setting the frame time
	*/
	class _OgreExport TimeControllerValue : public ControllerValue<Real>
	{
	protected:
		RibbonTrail* mTrail;
	public:
		TimeControllerValue(RibbonTrail* r) { mTrail = r; }

		Real getValue(void) const { return 0; }// not a source 
		void setValue(Real value) { mTrail->_timeUpdate(value); }

	};
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	RibbonTrail::RibbonTrail(const String& name, size_t maxElements, 
		size_t numberOfChains, bool useTextureCoords, bool useColours)
		:BillboardChain(name, maxElements, numberOfChains, useTextureCoords, useColours, true)
		, mInitialColour(ColourValue::White), mFading(false), 
		mDeltaColour(ColourValue::Black), mInitialWidth(10), mDeltaWidth(0),
		mFadeController(0)
	{
		setTrailLength(100);
		mTimeControllerValue = ControllerValueRealPtr(new TimeControllerValue(this));

		// use V as varying texture coord, so we can use 1D textures to 'smear'
		setTextureCoordDirection(TCD_V);


	}
	//-----------------------------------------------------------------------
	RibbonTrail::~RibbonTrail()
	{
		// Detach listeners
		for (NodeList::iterator i = mNodeList.begin(); i != mNodeList.end(); ++i)
		{
			(*i)->setListener(0);
		}

	}
	//-----------------------------------------------------------------------
	void RibbonTrail::addNode(Node* n)
	{
		if (mNodeList.size() == mChainCount)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				mName + " cannot monitor any more nodes, chain count exceeded",
				"RibbonTrail::addNode");
		}
		if (n->getListener())
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				mName + " cannot monitor node " + n->getName() + " since it already has a listener.",
				"RibbonTrail::addNode");
		}
		size_t segIdx = mNodeList.size();
		ChainSegment& seg = mChainSegmentList[segIdx];
		// set up this segment
		seg.head = seg.tail = SEGMENT_EMPTY;
		// Create new element, v coord is always 0.0f
		Element e(n->_getDerivedPosition(), mInitialWidth, 0.0f, mInitialColour);
		// Add the start position
		addChainElement(segIdx, e);
		// Add another on the same spot, this will extend
		addChainElement(segIdx, e);

		mNodeList.push_back(n);
		n->setListener(this);

	}
	//-----------------------------------------------------------------------
	void RibbonTrail::removeNode(Node* n)
	{
		NodeList::iterator i = std::find(mNodeList.begin(), mNodeList.end(), n);
		if (i != mNodeList.end())
		{
			n->setListener(0);
			mNodeList.erase(i);
		}
	}
	//-----------------------------------------------------------------------
	RibbonTrail::NodeIterator 
	RibbonTrail::getNodeIterator(void) const
	{
		return NodeIterator(mNodeList.begin(), mNodeList.end());
	}
	//-----------------------------------------------------------------------
	void RibbonTrail::setTrailLength(Real len)
	{
		mTrailLength = len;
		mElemLength = mTrailLength / mMaxElementsPerChain;
		mSquaredElemLength = mElemLength * mElemLength;
	}
	//-----------------------------------------------------------------------
	void RibbonTrail::setMaxChainElements(size_t maxElements)
	{
		BillboardChain::setMaxChainElements(maxElements);
		mElemLength = mTrailLength / mMaxElementsPerChain;
		mSquaredElemLength = mElemLength * mElemLength;

	}
	//-----------------------------------------------------------------------
	void RibbonTrail::setInitialColour(const ColourValue& col)
	{
		setInitialColour(col.r, col.g, col.b, col.a);
	}
	//-----------------------------------------------------------------------
	void RibbonTrail::setInitialColour(Real r, Real g, Real b, Real a)
	{
		mInitialColour.r = r;
		mInitialColour.g = g;
		mInitialColour.b = b;
		mInitialColour.a = a;
	}
	//-----------------------------------------------------------------------
	void RibbonTrail::setFading(bool enabled, const ColourValue& valuePerSecond)
	{
		setFading(enabled, 
			valuePerSecond.r, valuePerSecond.g, valuePerSecond.b, valuePerSecond.a);
	}
	//-----------------------------------------------------------------------
	void RibbonTrail::setFading(bool enabled, Real r, Real g, Real b, Real a)
	{
		mFading = enabled;
		mDeltaColour.r = r;
		mDeltaColour.g = g;
		mDeltaColour.b = b;
		mDeltaColour.a = a;

		manageController();

	}
	//-----------------------------------------------------------------------
	void RibbonTrail::setWidthChange(Real widthDeltaPerSecond)
	{
		mDeltaWidth = widthDeltaPerSecond;
		manageController();
	}
	//-----------------------------------------------------------------------
	void RibbonTrail::manageController(void)
	{
		if (!mFadeController && (mFading || mDeltaWidth))
		{
			// Set up fading via frame time controller
			ControllerManager& mgr = ControllerManager::getSingleton();
			ControllerFunctionRealPtr func(new PassthroughControllerFunction());
			mFadeController = mgr.createController(
				mgr.getFrameTimeSource(), mTimeControllerValue, func);
		}
		else if (mFadeController && (!mFading && !mDeltaWidth))
		{
			// destroy controller
			ControllerManager::getSingleton().destroyController(mFadeController);
			mFadeController = 0;
		}

	}
	//-----------------------------------------------------------------------
	void RibbonTrail::nodeUpdated(const Node* node)
	{
		
		for (size_t idx = 0; idx < mNodeList.size(); ++idx)
		{
			if (mNodeList[idx] == node)
			{
				updateTrail(idx, node);
				break;
			}
		}
	}
	//-----------------------------------------------------------------------
	void RibbonTrail::nodeDestroyed(const Node* node)
	{
		removeNode(const_cast<Node*>(node));

	}
	//-----------------------------------------------------------------------
	void RibbonTrail::updateTrail(size_t index, const Node* node)
	{
		// Node has changed somehow, we're only interested in the derived position
		ChainSegment& seg = mChainSegmentList[index];
		Element& headElem = mChainElementList[seg.start + seg.head];
		size_t nextElemIdx = seg.head + 1;
		// wrap
		if (nextElemIdx == mMaxElementsPerChain)
			nextElemIdx = 0;
		Element& nextElem = mChainElementList[seg.start + nextElemIdx];

		// Vary the head elem, but bake new version if that exceeds element len
		const Vector3& newPos = node->_getDerivedPosition();
		Vector3 diff = newPos - nextElem.position;
		Real sqlen = diff.squaredLength();
		if (sqlen >= mSquaredElemLength)
		{
			// Move existing head to mElemLength
			Vector3 scaledDiff = diff * (mElemLength / Math::Sqrt(sqlen));
			headElem.position = nextElem.position + scaledDiff;
			// Add a new element to be the new head
			Element newElem(newPos, mInitialWidth, 0.0f, mInitialColour);
			addChainElement(index, newElem);

		}
		else
		{
			// Extend existing head
			headElem.position = newPos;
		}
		// TODO shorten tail as we extend head


		mBoundsDirty = true;
		// Need to dirty the parent node, but can't do it using needUpdate() here 
		// since we're in the middle of the scene graph update (node listener), 
		// so re-entrant calls don't work. Queue.
		if (mParentNode)
		{
			Node::queueNeedUpdate(getParentSceneNode());
		}




	}
	//-----------------------------------------------------------------------
	void RibbonTrail::_timeUpdate(Real time)
	{
		// TODO

	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	String RibbonTrailFactory::FACTORY_TYPE_NAME = "RibbonTrail";
	//-----------------------------------------------------------------------
	const String& RibbonTrailFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------
	MovableObject* RibbonTrailFactory::createInstanceImpl( const String& name,
		const NameValuePairList* params)
	{
		size_t maxElements = 20;
		size_t numberOfChains = 1;
		bool useTex = true;
		bool useCol = true;
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

		}

		return new RibbonTrail(name, maxElements, numberOfChains, useTex, useCol);

	}
	//-----------------------------------------------------------------------
	void RibbonTrailFactory::destroyInstance( MovableObject* obj)
	{
		delete obj;
	}




}