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

#include "OgreBillboardSet.h"

#include "OgreBillboard.h"
#include "OgreMaterialManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreCamera.h"
#include "OgreMath.h"
#include "OgreSphere.h"
#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreStringConverter.h"
#include <algorithm>

namespace Ogre {

    //-----------------------------------------------------------------------
    BillboardSet::BillboardSet() :
        mOriginType( BBO_CENTER ),
        mAllDefaultSize( true ),
        mAutoExtendPool( true ),
        mFixedTextureCoords(true),
        mWorldSpace(false),
        mVertexData(0),
        mIndexData(0),
        mCullIndividual( false ),
        mBillboardType(BBT_POINT),
        mBuffersCreated(false),
        mPoolSize(0),
        mExternalData(false)
    {
        setDefaultDimensions( 100, 100 );
        setMaterialName( "BaseWhite" );
        mCastShadows = false;
        setTextureStacksAndSlices( 1, 1 );
    }

    //-----------------------------------------------------------------------
    BillboardSet::BillboardSet(
        const String& name,
        unsigned int poolSize, 
        bool externalData) :
        MovableObject(name),
        mOriginType( BBO_CENTER ),
        mAllDefaultSize( true ),
        mAutoExtendPool( true ),
        mFixedTextureCoords(true),
        mWorldSpace(false),
        mVertexData(0),
        mIndexData(0),
        mCullIndividual( false ),
        mBillboardType(BBT_POINT),
        mBuffersCreated(false),
        mPoolSize(poolSize),
        mExternalData(externalData)
    {
        setDefaultDimensions( 100, 100 );
        setMaterialName( "BaseWhite" );
        setPoolSize( poolSize );
        mCastShadows = false;
        setTextureStacksAndSlices( 1, 1 );
    }
    //-----------------------------------------------------------------------
    BillboardSet::~BillboardSet()
    {
        // Free pool items
        BillboardPool::iterator i;
        for (i = mBillboardPool.begin(); i != mBillboardPool.end(); ++i)
        {
            delete *i;
        }

        // Delete shared buffers
        if(mVertexData)
        {
            delete mVertexData;
            mVertexData = 0;
        }
            
        if(mIndexData)
        {
            delete mIndexData;
            mIndexData = 0;
        }
    }
    //-----------------------------------------------------------------------
    Billboard* BillboardSet::createBillboard(
        const Vector3& position,
        const ColourValue& colour )
    {
        if( mFreeBillboards.empty() )
        {
            if( mAutoExtendPool )
            {
                setPoolSize( getPoolSize() * 2 );
            }
            else
            {
                return 0;
            }
        }

        // Get a new billboard
        Billboard* newBill = mFreeBillboards.front();
		mActiveBillboards.splice(
			mActiveBillboards.end(), mFreeBillboards, mFreeBillboards.begin()); 
        newBill->setPosition(position);
        newBill->setColour(colour);
        newBill->_notifyOwner(this);

        _updateBounds();

        return newBill;
    }

    //-----------------------------------------------------------------------
    Billboard* BillboardSet::createBillboard(
        Real x, Real y, Real z,
        const ColourValue& colour )
    {
        return createBillboard( Vector3( x, y, z ), colour );
    }

    //-----------------------------------------------------------------------
    int BillboardSet::getNumBillboards(void) const
    {
        return static_cast< int >( mActiveBillboards.size() );
    }

    //-----------------------------------------------------------------------
    void BillboardSet::clear()
    {
		// Insert actives into free list
		mFreeBillboards.insert(mFreeBillboards.end(), mActiveBillboards.begin(), mActiveBillboards.end());
      
		// Remove all active instances
      	mActiveBillboards.clear(); 
    }

    //-----------------------------------------------------------------------
    Billboard* BillboardSet::getBillboard( unsigned int index ) const
    {
        assert(
            index < mActiveBillboards.size() &&
            "Billboard index out of bounds." );

        /* We can't access it directly, so we check wether it's in the first
           or the second half, then we start either from the beginning or the
           end of the list
        */
        ActiveBillboardList::const_iterator it;
        if( index >= ( mActiveBillboards.size() >> 1 ) )
        {
			index = static_cast<unsigned int>(mActiveBillboards.size()) - index;
            for( it = mActiveBillboards.end(); index; --index, --it );
        }
        else
        {
            for( it = mActiveBillboards.begin(); index; --index, ++it );
        }

        return *it;
    }

    //-----------------------------------------------------------------------
    void BillboardSet::removeBillboard(unsigned int index)
    {
        assert(
            index < mActiveBillboards.size() &&
            "Billboard index out of bounds." );

        /* We can't access it directly, so we check wether it's in the first
           or the second half, then we start either from the beginning or the
           end of the list.
           We then remove the billboard form the 'used' list and add it to
           the 'free' list.
        */
        ActiveBillboardList::iterator it;
        if( index >= ( mActiveBillboards.size() >> 1 ) )
        {
			index = static_cast<unsigned int>(mActiveBillboards.size()) - index;
            for( it = mActiveBillboards.end(); index; --index, --it );
        }
        else
        {
            for( it = mActiveBillboards.begin(); index; --index, ++it );
        }

        mFreeBillboards.push_back( *it );
        mActiveBillboards.erase( it );
    }

    //-----------------------------------------------------------------------
    void BillboardSet::removeBillboard( Billboard* pBill )
    {
        mActiveBillboards.remove( pBill );
        mFreeBillboards.push_back( pBill );
    }

    //-----------------------------------------------------------------------
    void BillboardSet::setBillboardOrigin( BillboardOrigin origin )
    {
        mOriginType = origin;
    }

    //-----------------------------------------------------------------------
    BillboardOrigin BillboardSet::getBillboardOrigin(void) const
    {
        return mOriginType;
    }

    //-----------------------------------------------------------------------
    void BillboardSet::setDefaultDimensions( Real width, Real height )
    {
        mDefaultWidth = width;
        mDefaultHeight = height;
    }
    //-----------------------------------------------------------------------
    void BillboardSet::setDefaultWidth(Real width)
    {
        mDefaultWidth = width;
    }
    //-----------------------------------------------------------------------
    Real BillboardSet::getDefaultWidth(void) const
    {
        return mDefaultWidth;
    }
    //-----------------------------------------------------------------------
    void BillboardSet::setDefaultHeight(Real height)
    {
        mDefaultHeight = height;
    }
    //-----------------------------------------------------------------------
    Real BillboardSet::getDefaultHeight(void) const
    {
        return mDefaultHeight;
    }
    //-----------------------------------------------------------------------
    void BillboardSet::setMaterialName( const String& name )
    {
        mMaterialName = name;

        mpMaterial = MaterialManager::getSingleton().getByName(name);

		if (mpMaterial.isNull())
			OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Could not find material " + name,
				"BillboardSet::setMaterialName" );

        /* Ensure that the new material was loaded (will not load again if
           already loaded anyway)
        */
        mpMaterial->load();
    }

    //-----------------------------------------------------------------------
    const String& BillboardSet::getMaterialName(void) const
    {
        return mMaterialName;
    }

    //-----------------------------------------------------------------------
	void BillboardSet::_sortBillboards( Camera* cam)
	{
        Quaternion invTransform = mParentNode->_getDerivedOrientation().Inverse();
        Quaternion camQ = cam->getDerivedOrientation();

		// Get camera world axes for X and Y (depth is irrelevant)
		// Convert into billboard local space
		if (!mWorldSpace)
		{
			camQ = invTransform * camQ;
		}
		mSortFunctor.sortDir = camQ * Vector3::UNIT_Z;

		mRadixSorter.sort(mActiveBillboards, mSortFunctor);
	}
	float BillboardSet::SortFunctor::operator()(Billboard* bill) const
	{
		return sortDir.dotProduct(bill->getPosition());
	}
    //-----------------------------------------------------------------------
    void BillboardSet::_notifyCurrentCamera( Camera* cam )
    {
		MovableObject::_notifyCurrentCamera(cam);

        mCurrentCamera = cam;
        /* Generate the vertices for all the billboards relative to the camera
           Also take the opportunity to update the vertex colours
           May as well do it here to save on loops elsewhere
        */

        /* NOTE: most engines generate world coordinates for the billboards
           directly, taking the world axes of the camera as offsets to the 
           center points. I take a different approach, reverse-transforming 
           the camera world axes into local billboard space. 
           Why?
           Well, it's actually more efficient this way, because I only have to
           reverse-transform using the billboardset world matrix (inverse) 
           once, from then on it's simple additions (assuming identically 
           sized billboards). If I transformed every billboard center by it's 
           world transform, that's a matrix multiplication per billboard 
           instead.
           I leave the final transform to the render pipeline since that can 
           use hardware TnL if it is available.
        */

        // create vertex and index buffers if they haven't already been
        if(!mBuffersCreated)
            _createBuffers();



        // Get offsets for origin type
        getParametricOffsets(mLeftOff, mRightOff, mTopOff, mBottomOff);

        // Generate axes etc up-front if not oriented per-billboard
        if (mBillboardType != BBT_ORIENTED_SELF)
        {
            genBillboardAxes(cam, &mCamX, &mCamY);

            /* If all billboards are the same size we can precalculate the
               offsets and just use '+' instead of '*' for each billboard,
               and it should be faster.
            */
            genVertOffsets(mLeftOff, mRightOff, mTopOff, mBottomOff, 
                mDefaultWidth, mDefaultHeight, mCamX, mCamY, mVOffset);

        }

        

        // If we're driving this from our own data, go ahead
        if (!mExternalData)
        {
			if (mSortingEnabled) 
			{
				_sortBillboards(cam);
			}

            beginBillboards();
            ActiveBillboardList::iterator it;
            for(it = mActiveBillboards.begin();
                it != mActiveBillboards.end();
                ++it )
            {
                injectBillboard(*(*it));
            }
            endBillboards();
        }



    }
    //-----------------------------------------------------------------------
    void BillboardSet::beginBillboards(void)
    {
        // Init num visible
        mNumVisibleBillboards = 0;

        mLockPtr = static_cast<float*>( 
            mMainBuf->lock(HardwareBuffer::HBL_DISCARD) );

    }
    //-----------------------------------------------------------------------
    void BillboardSet::injectBillboard(const Billboard& bb)
    {
        // Skip if not visible (NB always true if not bounds checking individual billboards)
        if (!billboardVisible(mCurrentCamera, bb)) return;

        if (mBillboardType == BBT_ORIENTED_SELF)
        {
            // Have to generate axes & offsets per billboard
            genBillboardAxes(mCurrentCamera, &mCamX, &mCamY, &bb);
        }

        if( mAllDefaultSize ) // If they're all the same size
        {
            /* No per-billboard checking, just blast through.
            Saves us an if clause every billboard which may
            make a difference.
            */

            if (mBillboardType == BBT_ORIENTED_SELF)
            {
                genVertOffsets(mLeftOff, mRightOff, mTopOff, mBottomOff, 
                    mDefaultWidth, mDefaultHeight, mCamX, mCamY, mVOffset);
            }
            genVertices(mVOffset, bb);
        }
        else // not all default size
        {
            Vector3 vOwnOffset[4];
            // If it has own dimensions, or self-oriented, gen offsets
            if (mBillboardType == BBT_ORIENTED_SELF || 
                bb.mOwnDimensions)
            {
                // Generate using own dimensions
                genVertOffsets(mLeftOff, mRightOff, mTopOff, mBottomOff, 
                    bb.mWidth, bb.mHeight, mCamX, mCamY, vOwnOffset);
                // Create vertex data            
                genVertices(vOwnOffset, bb);
            } 
            else // Use default dimension, already computed before the loop, for faster creation
            {
                genVertices(mVOffset, bb);
            }
        }
        // Increment visibles
        mNumVisibleBillboards++;
    }
    //-----------------------------------------------------------------------
    void BillboardSet::endBillboards(void)
    {
        mMainBuf->unlock();
    }
	//-----------------------------------------------------------------------
	void BillboardSet::setBounds(const AxisAlignedBox& box, Real radius)
	{
		mAABB = box;
		mBoundingRadius = radius;
	}
    //-----------------------------------------------------------------------
    void BillboardSet::_updateBounds(void)
    {
        if (mActiveBillboards.empty())
        {
            // No billboards, null bbox
            mAABB.setNull();
			mBoundingRadius = 0.0f;
        }
        else
        {
			Real maxSqLen = -1.0f;
        
            Vector3 min(Math::POS_INFINITY, Math::POS_INFINITY, Math::POS_INFINITY);
            Vector3 max(Math::NEG_INFINITY, Math::NEG_INFINITY, Math::NEG_INFINITY);
            ActiveBillboardList::iterator i, iend;

            iend = mActiveBillboards.end();
            for (i = mActiveBillboards.begin(); i != iend; ++i)
            {
                const Vector3& pos = (*i)->getPosition();
                min.makeFloor(pos);
                max.makeCeil(pos);

				maxSqLen = std::max(maxSqLen, pos.squaredLength());
            }
            // Adjust for billboard size
            Real adjust = std::max(mDefaultWidth, mDefaultHeight);
            Vector3 vecAdjust(adjust, adjust, adjust);
            min -= vecAdjust;
            max += vecAdjust;

            mAABB.setExtents(min, max);
			mBoundingRadius = Math::Sqrt(maxSqLen);
			
        }

        if (mParentNode)
            mParentNode->needUpdate();
        
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& BillboardSet::getBoundingBox(void) const
    {
        return mAABB;
    }

    //-----------------------------------------------------------------------    
    void BillboardSet::_updateRenderQueue(RenderQueue* queue)
    {
        //only set the render queue group if it has been explicitly set.
        if( mRenderQueueIDSet ) 
        {
           queue->addRenderable(this, mRenderQueueID);
        } else {
           queue->addRenderable(this);
        }

    }

    //-----------------------------------------------------------------------
    const MaterialPtr& BillboardSet::getMaterial(void) const
    {
        return mpMaterial;
    }

    //-----------------------------------------------------------------------
    void BillboardSet::getRenderOperation(RenderOperation& op)
    {
        op.operationType = RenderOperation::OT_TRIANGLE_LIST;
        op.useIndexes = true;

        op.vertexData = mVertexData;
        op.vertexData->vertexCount = mNumVisibleBillboards * 4;
        op.vertexData->vertexStart = 0;

        op.indexData = mIndexData;
        op.indexData->indexCount = mNumVisibleBillboards * 6;
        op.indexData->indexStart = 0;
    }

    //-----------------------------------------------------------------------
    void BillboardSet::getWorldTransforms( Matrix4* xform ) const
    {
        if (mWorldSpace)
        {
            *xform = Matrix4::IDENTITY;
        }
        else
        {
            *xform = _getParentNodeFullTransform(); 
        }
    }

    //-----------------------------------------------------------------------
    const Quaternion& BillboardSet::getWorldOrientation(void) const
    {
        return mParentNode->_getDerivedOrientation();
    }

    //-----------------------------------------------------------------------
    const Vector3& BillboardSet::getWorldPosition(void) const
    {
        return mParentNode->_getDerivedPosition();
    }

    //-----------------------------------------------------------------------
    void BillboardSet::setAutoextend( bool autoextend )
    {
        mAutoExtendPool = autoextend;
    }

    //-----------------------------------------------------------------------
    bool BillboardSet::getAutoextend(void) const
    {
        return mAutoExtendPool;
    }

    //-----------------------------------------------------------------------
    void BillboardSet::setSortingEnabled( bool sortenable )
    {
        mSortingEnabled = sortenable;
    }

    //-----------------------------------------------------------------------
    bool BillboardSet::getSortingEnabled(void) const
    {
        return mSortingEnabled;
    }

    //-----------------------------------------------------------------------
    void BillboardSet::setPoolSize( unsigned int size )
    {
        // Never shrink below size()
        size_t currSize = mBillboardPool.size();

        if( currSize < size )
        {
            this->increasePool(size);

            for( size_t i = currSize; i < size; ++i )
            {
                // Add new items to the queue
                mFreeBillboards.push_back( mBillboardPool[i] );
            }
            
            mPoolSize = size;
            mBuffersCreated = false;

            if (mVertexData)
            {
                delete mVertexData;
                mVertexData = 0;
            }
            if (mIndexData)
            {
                delete mIndexData;
                mIndexData = 0;
            }
        }
    }

    //-----------------------------------------------------------------------
    void BillboardSet::_createBuffers(void)
    {
        /* Allocate / reallocate vertex data
           Note that we allocate enough space for ALL the billboards in the pool, but only issue
           rendering operations for the sections relating to the active billboards
        */

        /* Alloc positions   ( 4 verts per billboard, 3 components )
                 colours     ( 1 x RGBA per vertex )
                 indices     ( 6 per billboard ( 2 tris ) )
                 tex. coords ( 2D coords, 4 per billboard )
        */
        mVertexData = new VertexData();
        mIndexData  = new IndexData();

        mVertexData->vertexCount = mPoolSize * 4;
        mVertexData->vertexStart = 0;

        // Vertex declaration
        VertexDeclaration* decl = mVertexData->vertexDeclaration;
        VertexBufferBinding* binding = mVertexData->vertexBufferBinding;

        size_t offset = 0;
        decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
        decl->addElement(0, offset, VET_COLOUR, VES_DIFFUSE);
        offset += VertexElement::getTypeSize(VET_COLOUR);
        decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);

        mMainBuf = 
            HardwareBufferManager::getSingleton().createVertexBuffer(
                decl->getVertexSize(0),
                mVertexData->vertexCount, 
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
        // bind position and diffuses
        binding->setBinding(0, mMainBuf);


        mIndexData->indexStart = 0;
        mIndexData->indexCount = mPoolSize * 6;

        mIndexData->indexBuffer = HardwareBufferManager::getSingleton().
            createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
                mIndexData->indexCount,
                HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        /* Create indexes (will be the same every frame)
           Using indexes because it means 1/3 less vertex transforms (4 instead of 6)

           Billboard layout relative to camera:

            2-----3
            |    /|
            |  /  |
            |/    |
            0-----1
        */

        ushort* pIdx = static_cast<ushort*>(
            mIndexData->indexBuffer->lock(0,
              mIndexData->indexBuffer->getSizeInBytes(),
              HardwareBuffer::HBL_DISCARD) );

        for(
            size_t idx, idxOff, bboard = 0;
            bboard < mPoolSize;
            ++bboard )
        {
            // Do indexes
            idx    = bboard * 6;
            idxOff = bboard * 4;

            pIdx[idx] = static_cast<unsigned short>(idxOff); // + 0;, for clarity
            pIdx[idx+1] = static_cast<unsigned short>(idxOff + 1);
            pIdx[idx+2] = static_cast<unsigned short>(idxOff + 3);
            pIdx[idx+3] = static_cast<unsigned short>(idxOff + 0);
            pIdx[idx+4] = static_cast<unsigned short>(idxOff + 3);
            pIdx[idx+5] = static_cast<unsigned short>(idxOff + 2);

        }

        mIndexData->indexBuffer->unlock();
        mBuffersCreated = true;
    }

    //-----------------------------------------------------------------------
    unsigned int BillboardSet::getPoolSize(void) const
    {
        return static_cast< unsigned int >( mBillboardPool.size() );
    }

    //-----------------------------------------------------------------------
    void BillboardSet::_notifyBillboardResized(void)
    {
        mAllDefaultSize = false;
    }

    //-----------------------------------------------------------------------
    void BillboardSet::getParametricOffsets(
        Real& left, Real& right, Real& top, Real& bottom )
    {
        switch( mOriginType )
        {
        case BBO_TOP_LEFT:
            left = 0.0f;
            right = 1.0f;
            top = 0.0f;
            bottom = 1.0f;
            break;

        case BBO_TOP_CENTER:
            left = -0.5f;
            right = 0.5f;
            top = 0.0f;
            bottom = 1.0f;
            break;

        case BBO_TOP_RIGHT:
            left = -1.0f;
            right = 0.0f;
            top = 0.0f;
            bottom = 1.0f;
            break;

        case BBO_CENTER_LEFT:
            left = 0.0f;
            right = 1.0f;
            top = -0.5f;
            bottom = 0.5f;
            break;

        case BBO_CENTER:
            left = -0.5f;
            right = 0.5f;
            top = -0.5f;
            bottom = 0.5f;
            break;

        case BBO_CENTER_RIGHT:
            left = -1.0f;
            right = 0.0f;
            top = -0.5f;
            bottom = 0.5f;
            break;

        case BBO_BOTTOM_LEFT:
            left = 0.0f;
            right = 1.0f;
            top = -1.0f;
            bottom = 0.0f;
            break;

        case BBO_BOTTOM_CENTER:
            left = -0.5f;
            right = 0.5f;
            top = -1.0f;
            bottom = 0.0f;
            break;

        case BBO_BOTTOM_RIGHT:
            left = -1.0f;
            right = 0.0f;
            top = -1.0f;
            bottom = 0.0f;
            break;
        }
    }
    //-----------------------------------------------------------------------
    bool BillboardSet::getCullIndividually(void) const
    {
        return mCullIndividual;
    }
    //-----------------------------------------------------------------------
    void BillboardSet::setCullIndividually(bool cullIndividual)
    {
        mCullIndividual = cullIndividual;
    }
    //-----------------------------------------------------------------------
    bool BillboardSet::billboardVisible(Camera* cam, const Billboard& bill)
    {
        // Return always visible if not culling individually
        if (!mCullIndividual) return true;

        // Cull based on sphere (have to transform less)
        Sphere sph;
        Matrix4 xworld;

        getWorldTransforms(&xworld);

        sph.setCenter(xworld * bill.mPosition);

        if (bill.mOwnDimensions)
        {
            sph.setRadius(std::max(bill.mWidth, bill.mHeight));
        }
        else
        {
            sph.setRadius(std::max(mDefaultWidth, mDefaultHeight));
        }

        return cam->isVisible(sph);
        
    }
    //-----------------------------------------------------------------------
    void BillboardSet::increasePool(unsigned int size)
    {
        size_t oldSize = mBillboardPool.size();

        // Increase size
        mBillboardPool.reserve(size);
        mBillboardPool.resize(size);

        // Create new billboards
        for( size_t i = oldSize; i < size; ++i )
            mBillboardPool[i] = new Billboard();

    }
    //-----------------------------------------------------------------------
    void BillboardSet::genBillboardAxes(Camera* cam, Vector3* pX, 
        Vector3 *pY, const Billboard* bb)
    {
        // Default behaviour is that billboards are in local node space
        // so orientation of camera (in world space) must be reverse-transformed 
        // into node space to generate the axes

        Quaternion invTransform;
        if (!mWorldSpace)
        {
            invTransform = mParentNode->_getDerivedOrientation().Inverse();
        }
        Quaternion camQ;

        switch (mBillboardType)
        {
        case BBT_POINT:
            // Get camera world axes for X and Y (depth is irrelevant)
            camQ = cam->getDerivedOrientation();
            if (!mWorldSpace)
            {
                // Convert into billboard local space
                camQ = invTransform * camQ;
            }
            *pX = camQ * Vector3::UNIT_X;
            *pY = camQ * Vector3::UNIT_Y;
            break;
        case BBT_ORIENTED_COMMON:
            // Y-axis is common direction
            // X-axis is cross with camera direction 
            *pY = mCommonDirection;
            if (!mWorldSpace)
            {
                // Convert into billboard local space
                *pX = (invTransform * cam->getDerivedDirection()).crossProduct(*pY);
            }
            else
            {
                *pX = cam->getDerivedDirection().crossProduct(*pY);
            }
            pX->normalise();
            
            break;
        case BBT_ORIENTED_SELF:
            // Y-axis is direction
            // X-axis is cross with camera direction 
            // Scale direction first
            *pY = bb->mDirection;
			*pY *= 0.01f;
            if (!mWorldSpace)
            {
                // Convert into billboard local space
                *pX = (invTransform * cam->getDerivedDirection()).crossProduct(*pY);
				pX->normalise();
            }
            else
            {
                *pX = cam->getDerivedDirection().crossProduct(*pY);
            }

            break;
        }

    }
    //-----------------------------------------------------------------------
    void BillboardSet::setBillboardType(BillboardType bbt)
    {
        mBillboardType = bbt;
    }
    //-----------------------------------------------------------------------
    BillboardType BillboardSet::getBillboardType(void) const
    {
        return mBillboardType;
    }
    //-----------------------------------------------------------------------
    void BillboardSet::setCommonDirection(const Vector3& vec)
    {
        mCommonDirection = vec;
    }
    //-----------------------------------------------------------------------
    const Vector3& BillboardSet::getCommonDirection(void) const
    {
        return mCommonDirection;
    }
	//-----------------------------------------------------------------------
	uint32 BillboardSet::getTypeFlags(void) const
	{
		return SceneManager::FX_TYPE_MASK;
	}
    //-----------------------------------------------------------------------
    void BillboardSet::genVertices( 
        const Vector3* const offsets, const Billboard& bb)
    {
        RGBA colour;
        Root::getSingleton().convertColourValue(bb.mColour, &colour);
		RGBA* pCol;
        static float basicTexData[8] = {
            0.0, 1.0,
            1.0, 1.0,
            0.0, 0.0,
            1.0, 0.0 };
        static float rotTexData[8];

		float* pTexData;

        // Texcoords
        assert( bb.mTexCoords < mTextureCoords.size() );
        Ogre::FloatRect & r = mTextureCoords[bb.mTexCoords];
        if (mFixedTextureCoords)
        {
            rotTexData[0] = r.left;
            rotTexData[1] = r.bottom;
            rotTexData[2] = r.right;
            rotTexData[3] = r.bottom;
            rotTexData[4] = r.left;
            rotTexData[5] = r.top;
            rotTexData[6] = r.right;
            rotTexData[7] = r.top;
        }
        else
        {

            const Real      cos_rot  ( Math::Cos(bb.mRotation)   );
            const Real      sin_rot  ( Math::Sin(bb.mRotation)   );

            float width = (r.right-r.left)/2;
            float height = (r.top-r.bottom)/2;
            float mid_u = r.left+width;
            float mid_v = r.bottom+height;

            rotTexData[0] = mid_u + (cos_rot * -width) + (sin_rot * height);
            rotTexData[1] = mid_v + (sin_rot * -width) - (cos_rot * height);

            rotTexData[2] = mid_u + (cos_rot * width) + (sin_rot * height);
            rotTexData[3] = mid_v + (sin_rot * width) - (cos_rot * height);

            rotTexData[4] = mid_u + (cos_rot * -width) + (sin_rot * -height);
            rotTexData[5] = mid_v + (sin_rot * -width) - (cos_rot * -height);

            rotTexData[6] = mid_u + (cos_rot * width) + (sin_rot * -height);
            rotTexData[7] = mid_v + (sin_rot * width) - (cos_rot * -height);
        }
        pTexData = rotTexData;
		

        // Left-top
		// Positions
        *mLockPtr++ = offsets[0].x + bb.mPosition.x;
        *mLockPtr++ = offsets[0].y + bb.mPosition.y;
        *mLockPtr++ = offsets[0].z + bb.mPosition.z;
		// Colour
		// Convert float* to RGBA*
        pCol = static_cast<RGBA*>(static_cast<void*>(mLockPtr));
        *pCol++ = colour;
        // Update lock pointer
        mLockPtr = static_cast<float*>(static_cast<void*>(pCol));
		// Texture coords
		*mLockPtr++ = *pTexData++;
		*mLockPtr++ = *pTexData++;


		// Right-top
		// Positions
        *mLockPtr++ = offsets[1].x + bb.mPosition.x;
        *mLockPtr++ = offsets[1].y + bb.mPosition.y;
        *mLockPtr++ = offsets[1].z + bb.mPosition.z;
		// Colour
		// Convert float* to RGBA*
        pCol = static_cast<RGBA*>(static_cast<void*>(mLockPtr));
        *pCol++ = colour;
        // Update lock pointer
        mLockPtr = static_cast<float*>(static_cast<void*>(pCol));
		// Texture coords
		*mLockPtr++ = *pTexData++;
		*mLockPtr++ = *pTexData++;

		// Left-bottom
		// Positions
        *mLockPtr++ = offsets[2].x + bb.mPosition.x;
        *mLockPtr++ = offsets[2].y + bb.mPosition.y;
        *mLockPtr++ = offsets[2].z + bb.mPosition.z;
		// Colour
		// Convert float* to RGBA*
        pCol = static_cast<RGBA*>(static_cast<void*>(mLockPtr));
        *pCol++ = colour;
        // Update lock pointer
        mLockPtr = static_cast<float*>(static_cast<void*>(pCol));
		// Texture coords
		*mLockPtr++ = *pTexData++;
		*mLockPtr++ = *pTexData++;

		// Right-bottom
		// Positions
        *mLockPtr++ = offsets[3].x + bb.mPosition.x;
        *mLockPtr++ = offsets[3].y + bb.mPosition.y;
        *mLockPtr++ = offsets[3].z + bb.mPosition.z;
		// Colour
		// Convert float* to RGBA*
        pCol = static_cast<RGBA*>(static_cast<void*>(mLockPtr));
        *pCol++ = colour;
        // Update lock pointer
        mLockPtr = static_cast<float*>(static_cast<void*>(pCol));
		// Texture coords
		*mLockPtr++ = *pTexData++;
		*mLockPtr++ = *pTexData++;

    }
    //-----------------------------------------------------------------------
    void BillboardSet::genVertOffsets(Real inleft, Real inright, Real intop, Real inbottom,
        Real width, Real height, const Vector3& x, const Vector3& y, Vector3* pDestVec)
    {
        Vector3 vLeftOff, vRightOff, vTopOff, vBottomOff;
        /* Calculate default offsets. Scale the axes by
           parametric offset and dimensions, ready to be added to
           positions.
        */

        vLeftOff   = x * ( inleft   * width );
        vRightOff  = x * ( inright  * width );
        vTopOff    = y * ( intop   * height );
        vBottomOff = y * ( inbottom * height );

        // Make final offsets to vertex positions
        pDestVec[0] = vLeftOff  + vTopOff;
        pDestVec[1] = vRightOff + vTopOff;
        pDestVec[2] = vLeftOff  + vBottomOff;
        pDestVec[3] = vRightOff + vBottomOff;

    }
    //-----------------------------------------------------------------------
    const String& BillboardSet::getMovableType(void) const
    {
		return BillboardSetFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    Real BillboardSet::getSquaredViewDepth(const Camera* const cam) const
    {
        assert(mParentNode);
        return mParentNode->getSquaredViewDepth(cam);
    }
    //-----------------------------------------------------------------------
	Real BillboardSet::getBoundingRadius(void) const
	{
		return mBoundingRadius;
	}
    //-----------------------------------------------------------------------
    const LightList& BillboardSet::getLights(void) const
    {
        // It's actually quite unlikely that this will be called, 
        // because most billboards are unlit, but here we go anyway
        return getParentSceneNode()->findLights(this->getBoundingRadius());
    }

    void BillboardSet::setTextureCoords( Ogre::FloatRect const * coords, uint16 numCoords )
    {
      if( !numCoords || !coords ) {
        setTextureStacksAndSlices( 1, 1 );
      }
      //  clear out any previous allocation (as vectors may not shrink)
      TextureCoordSets().swap( mTextureCoords );
      //  make room
      mTextureCoords.resize( numCoords );
      //  copy in data
      std::copy( coords, coords+numCoords, &mTextureCoords.front() );
    }
    
    void BillboardSet::setTextureStacksAndSlices( uchar stacks, uchar slices )
    {
      if( stacks == 0 ) stacks = 1;
      if( slices == 0 ) slices = 1;
      //  clear out any previous allocation (as vectors may not shrink)
      TextureCoordSets().swap( mTextureCoords );
      //  make room
      mTextureCoords.resize( (size_t)stacks * slices );
      unsigned int coordIndex = 0;
      //  spread the U and V coordinates across the rects
      for( uint v = 0; v < stacks; ++v ) {
        //  (float)X / X is guaranteed to be == 1.0f for X up to 8 million, so 
        //  our range of 1..256 is quite enough to guarantee perfect coverage.
        float top = (float)v / (float)stacks;
        float bottom = ((float)v + 1) / (float)stacks;
        for( uint u = 0; u < slices; ++u ) {
          Ogre::FloatRect & r = mTextureCoords[coordIndex];
          r.left = (float)u / (float)slices;
          r.bottom = bottom;
          r.right = ((float)u + 1) / (float)slices;
          r.top = top;
          ++coordIndex;
        }
      }
      assert( coordIndex == (size_t)stacks * slices );
    }
    
    Ogre::FloatRect const * BillboardSet::getTextureCoords( uint16 * oNumCoords )
    {
      *oNumCoords = (uint16)mTextureCoords.size();
      //  std::vector<> is guaranteed to be contiguous
      return &mTextureCoords.front();
    }
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	String BillboardSetFactory::FACTORY_TYPE_NAME = "BillboardSet";
	//-----------------------------------------------------------------------
	const String& BillboardSetFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------
	MovableObject* BillboardSetFactory::createInstanceImpl( const String& name, 
		const NameValuePairList* params)
	{
		// may have parameters
		bool externalData = false;
		unsigned int poolSize = 0;
		
		if (params != 0)
		{
			NameValuePairList::const_iterator ni = params->find("poolSize");
			if (ni != params->end())
			{
				poolSize = StringConverter::parseUnsignedInt(ni->second);
			}
			ni = params->find("externalData");
			if (ni != params->end())
			{
				externalData = StringConverter::parseBool(ni->second);
			}

		}

		if (poolSize > 0)
		{
			return new BillboardSet(name, poolSize, externalData);
		}
		else
		{
			return new BillboardSet(name);
		}

	}
	//-----------------------------------------------------------------------
	void BillboardSetFactory::destroyInstance( MovableObject* obj)
	{
		delete obj;
	}


}
