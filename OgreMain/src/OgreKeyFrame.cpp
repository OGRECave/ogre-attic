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

#include "OgreKeyFrame.h"
#include "OgreAnimationTrack.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    KeyFrame::KeyFrame(const AnimationTrack* parent, Real time) 
        : mTime(time), mParentTrack(parent)
    {
    }
    //---------------------------------------------------------------------
    Real KeyFrame::getTime(void) const
    {
        return mTime;
    }
	//---------------------------------------------------------------------
	NumericKeyFrame::NumericKeyFrame(const AnimationTrack* parent, Real time)
		:KeyFrame(parent, time)
	{
	}
	//---------------------------------------------------------------------
	const AnyNumeric& NumericKeyFrame::getValue(void) const
	{
		return mValue;
	}
	//---------------------------------------------------------------------
	void NumericKeyFrame::setValue(const AnyNumeric& val)
	{
		mValue = val;
	}
    //---------------------------------------------------------------------
	TransformKeyFrame::TransformKeyFrame(const AnimationTrack* parent, Real time)
		:KeyFrame(parent, time), mTranslate(Vector3::ZERO), 
		mScale(Vector3::UNIT_SCALE), mRotate(Quaternion::IDENTITY) 
	{
	}
	//---------------------------------------------------------------------
    void TransformKeyFrame::setTranslate(const Vector3& trans)
    {
        mTranslate = trans;
        if (mParentTrack)
            mParentTrack->_keyFrameDataChanged();
    }
    //---------------------------------------------------------------------
    const Vector3& TransformKeyFrame::getTranslate(void) const
    {
        return mTranslate;
    }
    //---------------------------------------------------------------------
    void TransformKeyFrame::setScale(const Vector3& scale)
    {
        mScale = scale;
        if (mParentTrack)
            mParentTrack->_keyFrameDataChanged();
    }
    //---------------------------------------------------------------------
    const Vector3& TransformKeyFrame::getScale(void) const
    {
        return mScale;
    }
    //---------------------------------------------------------------------
    void TransformKeyFrame::setRotation(const Quaternion& rot)
    {
        mRotate = rot;
        if (mParentTrack)
            mParentTrack->_keyFrameDataChanged();
    }
    //---------------------------------------------------------------------
    const Quaternion& TransformKeyFrame::getRotation(void) const
    {
        return mRotate;
    }
	//---------------------------------------------------------------------
	VertexMorphKeyFrame::VertexMorphKeyFrame(const AnimationTrack* parent, Real time)
		: KeyFrame(parent, time)
	{
	}
	//---------------------------------------------------------------------
	void VertexMorphKeyFrame::setVertexBuffer(const HardwareVertexBufferSharedPtr& buf)
	{
		mBuffer = buf;
	}
	//---------------------------------------------------------------------
	const HardwareVertexBufferSharedPtr& 
	VertexMorphKeyFrame::getVertexBuffer(void) const
	{
		return mBuffer;
	}
	//---------------------------------------------------------------------
	VertexPoseKeyFrame::VertexPoseKeyFrame(const AnimationTrack* parent)
		:KeyFrame(parent, 0.0f)
	{
	}
	//---------------------------------------------------------------------
	void VertexPoseKeyFrame::addVertex(size_t index, const Vector3& offset)
	{
		mVertexOffsetMap[index] = offset;
		mBuffer.setNull();
	}
	//---------------------------------------------------------------------
	void VertexPoseKeyFrame::removeVertex(size_t index)
	{
		VertexOffsetMap::iterator i = mVertexOffsetMap.find(index);
		if (i != mVertexOffsetMap.end())
		{
			mVertexOffsetMap.erase(i);
			mBuffer.setNull();
		}
	}
	//---------------------------------------------------------------------
	void VertexPoseKeyFrame::clearVertexOffsets(void)
	{
		mVertexOffsetMap.clear();
		mBuffer.setNull();
	}
	//---------------------------------------------------------------------
	VertexPoseKeyFrame::ConstVertexOffsetIterator 
	VertexPoseKeyFrame::getVertexOffsetIterator(void) const
	{
		return ConstVertexOffsetIterator(mVertexOffsetMap.begin(), mVertexOffsetMap.end());
	}
	//---------------------------------------------------------------------
	VertexPoseKeyFrame::VertexOffsetIterator 
	VertexPoseKeyFrame::getVertexOffsetIterator(void)
	{
		return VertexOffsetIterator(mVertexOffsetMap.begin(), mVertexOffsetMap.end());
	}
	//---------------------------------------------------------------------
	const HardwareVertexBufferSharedPtr& VertexPoseKeyFrame::_getHardwareVertexBuffer(size_t numVertices)
	{
		if (mBuffer.isNull())
		{
			// Create buffer
			mBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
				VertexElement::getTypeSize(VET_FLOAT3),
				numVertices, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

			float* pFloat = static_cast<float*>(
				mBuffer->lock(HardwareBuffer::HBL_DISCARD));
			// initialise
			memset(pFloat, 0, mBuffer->getSizeInBytes()); 
			// Set each vertex
			for (VertexOffsetMap::iterator i = mVertexOffsetMap.begin();
				i != mVertexOffsetMap.end(); ++i)
			{
				float* pDst = pFloat + (3 * i->first);
				*pDst++ = i->second.x;
				*pDst++ = i->second.y;
				*pDst++ = i->second.z;
			}
			mBuffer->unlock();
		}
		return mBuffer;
	}


}

