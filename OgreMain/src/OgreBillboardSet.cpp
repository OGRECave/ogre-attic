/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include <algorithm>

namespace Ogre {
    #define POSITION_BINDING 0
    #define COLOUR_BINDING 1
    #define TEXCOORD_BINDING 2

    String BillboardSet::msMovableType = "BillboardSet";
    //-----------------------------------------------------------------------
    BillboardSet::BillboardSet() :
        mOriginType( BBO_CENTER ),
        mAllDefaultSize( true ),
        mAutoExtendPool( true ),
        mFixedTextureCoords(true),
        mVertexData(0),
        mIndexData(0),
        mCullIndividual( false ),
        mBillboardType(BBT_POINT)
    {
        setDefaultDimensions( 100, 100 );
        setMaterialName( "BaseWhite" );
    }

    //-----------------------------------------------------------------------
    BillboardSet::BillboardSet(
        const String& name,
        unsigned int poolSize ) :
        mName( name ),
        mOriginType( BBO_CENTER ),
        mAllDefaultSize( true ),
        mAutoExtendPool( true ),
        mFixedTextureCoords(true),
        mVertexData(0),
        mIndexData(0),
        mCullIndividual( false ),
        mBillboardType(BBT_POINT)
    {
        setDefaultDimensions( 100, 100 );
        setMaterialName( "BaseWhite" );
        setPoolSize( poolSize );
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
            delete mVertexData;
        if(mIndexData)
            delete mIndexData;
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
        mFreeBillboards.pop_front();
        mActiveBillboards.push_back(newBill);

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

        mpMaterial = static_cast<Material *>(
            MaterialManager::getSingleton().getByName(name) );

		if (!mpMaterial)
			Except( Exception::ERR_ITEM_NOT_FOUND, "Could not find material " + name,
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
    void BillboardSet::_notifyCurrentCamera( Camera* cam )
    {
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

        /*
        // Min and max bounds for AABB
        Vector3 min( Math::POS_INFINITY, Math::POS_INFINITY, Math::POS_INFINITY );
        Vector3 max( Math::NEG_INFINITY, Math::NEG_INFINITY, Math::NEG_INFINITY );
        */

        ActiveBillboardList::iterator it;
        // Parametric offsets of origin
        Real leftOff, rightOff, topOff, bottomOff;

        // Boundary offsets based on origin and camera orientation
        // Vector3 vLeftOff, vRightOff, vTopOff, vBottomOff;
        // Final vertex offsets, used where sizes all default to save calcs
        Vector3 vOffset[4];

        // Get offsets for origin type
        getParametricOffsets(leftOff, rightOff, topOff, bottomOff);
        // Get camera axes in billboard space
        Vector3 camX, camY;

        // Generate axes etc up-front if not oriented per-billboard
        if (mBillboardType != BBT_ORIENTED_SELF)
        {
            genBillboardAxes(*cam, &camX, &camY);

            /* If all billboards are the same size we can precalculate the
               offsets and just use '+' instead of '*' for each billboard,
               and it should be faster.
            */
            genVertOffsets(leftOff, rightOff, topOff, bottomOff, 
                mDefaultWidth, mDefaultHeight, camX, camY, vOffset);

        }

        // Init num visible
        mNumVisibleBillboards = 0;

        HardwareVertexBufferSharedPtr vPosBuf = 
            mVertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);

        Real* pV = static_cast<Real*>( 
            vPosBuf->lock(HardwareBuffer::HBL_DISCARD) );

        HardwareVertexBufferSharedPtr vColBuf = 
            mVertexData->vertexBufferBinding->getBuffer(COLOUR_BINDING);

        RGBA* pC = static_cast<RGBA*>( 
            vColBuf->lock(HardwareBuffer::HBL_DISCARD) );

        HardwareVertexBufferSharedPtr vTexBuf = 
            mVertexData->vertexBufferBinding->getBuffer(TEXCOORD_BINDING);

		Real* pT = 0;
        if (!mFixedTextureCoords)
        {
            pT = static_cast<Real*>( 
                vTexBuf->lock(HardwareBuffer::HBL_DISCARD) );
        }

        if( mAllDefaultSize ) // If they're all the same size
        {
            /* No per-billboard checking, just blast through.
               Saves us an if clause every billboard which may
               make a difference.
            */

			if (mBillboardType == BBT_ORIENTED_SELF)
			{
				for( it = mActiveBillboards.begin();
					it != mActiveBillboards.end();
					++it )
				{
					// Skip if not visible (NB always true if not bounds checking individual billboards)
					if (!billboardVisible(cam, it)) continue;

					// Have to generate axes & offsets per billboard
					genBillboardAxes(*cam, &camX, &camY, *it);
					genVertOffsets(leftOff, rightOff, topOff, bottomOff, 
						mDefaultWidth, mDefaultHeight, camX, camY, vOffset);

					genVertices(&pV, &pC, &pT, vOffset, *it);

					// Increment visibles
					mNumVisibleBillboards++;
				}
			} else
			{
				for( it = mActiveBillboards.begin();
					it != mActiveBillboards.end();
					++it )
				{
					// Skip if not visible (NB always true if not bounds checking individual billboards)
					if (!billboardVisible(cam, it)) continue;

					genVertices(&pV, &pC, &pT, vOffset, *it);

					// Increment visibles
					mNumVisibleBillboards++;
				}
			}
        }
        else // not all default size
        {
            Vector3 vOwnOffset[4];
            if (mBillboardType == BBT_ORIENTED_SELF)
			{
				for( it = mActiveBillboards.begin(); it != mActiveBillboards.end(); ++it )
				{
					// Skip if not visible (NB always true if not bounds checking individual billboards)
					if (!billboardVisible(cam, it)) continue;

					// Have to generate axes & offsets per billboard
					genBillboardAxes(*cam, &camX, &camY, *it);
			
					// If it has own dimensions, or self-oriented, gen offsets
					if( (*it)->mOwnDimensions) 
					{
						// Generate using own dimensions
						genVertOffsets(leftOff, rightOff, topOff, bottomOff, 
							(*it)->mWidth, (*it)->mHeight, camX, camY, vOwnOffset);
						// Create vertex data            
						genVertices(&pV, &pC, &pT, vOwnOffset, *it);
					}
					else // Use default dimension, already computed before the loop, for faster creation
					{
						genVertices(&pV, &pC, &pT, vOffset, *it);
					}

					// Increment visibles
					mNumVisibleBillboards++;

				}
			} else
			{
				for( it = mActiveBillboards.begin(); it != mActiveBillboards.end(); ++it )
				{
					// Skip if not visible (NB always true if not bounds checking individual billboards)
					if (!billboardVisible(cam, it)) continue;

					// If it has own dimensions, or self-oriented, gen offsets
					if( (*it)->mOwnDimensions) 
					{
						// Generate using own dimensions
						genVertOffsets(leftOff, rightOff, topOff, bottomOff, 
							(*it)->mWidth, (*it)->mHeight, camX, camY, vOwnOffset);
						// Create vertex data            
						genVertices(&pV, &pC, &pT, vOwnOffset, *it);
					}
					else // Use default dimension, already computed before the loop, for faster creation
					{
						genVertices(&pV, &pC, &pT, vOffset, *it);
					}

					// Increment visibles
					mNumVisibleBillboards++;

				}
			}
        }

		if (!mFixedTextureCoords)
            vTexBuf->unlock();
        vColBuf->unlock();
        vPosBuf->unlock();

        /*
        // Update bounding box limits
        unsigned int vertBufferSize = mNumVisibleBillboards * 4 * 3;

        for( j = 0; j < vertBufferSize; j += 3 )
        {
                min.makeFloor( Vector3(
                                   mpPositions[j],
                                   mpPositions[j+1],
                                   mpPositions[j+2] ) );

                max.makeCeil( Vector3(
                                   mpPositions[j],
                                   mpPositions[j+1],
                                   mpPositions[j+2] ) );
        }

        // Set AABB
        mAABB.setExtents(min, max);
        */
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
        queue->addRenderable(this, mRenderQueueID, RENDERABLE_DEFAULT_PRIORITY);
    }

    //-----------------------------------------------------------------------
    Material* BillboardSet::getMaterial(void) const
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
        *xform = _getParentNodeFullTransform(); 
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

            /* Allocate / reallocate vertex data
               Note that we allocate enough space for ALL the billboards in the pool, but only issue
               rendering operations for the sections relating to the active billboards
            */

            if (mVertexData)
                delete mVertexData;
            if (mIndexData)
                delete mIndexData;

            /* Alloc positions   ( 4 verts per billboard, 3 components )
                     colours     ( 1 x RGBA per vertex )
                     indices     ( 6 per billboard ( 2 tris ) )
                     tex. coords ( 2D coords, 4 per billboard )
            */
            mVertexData = new VertexData();
            mIndexData  = new IndexData();

            mVertexData->vertexCount = size * 4;
            mVertexData->vertexStart = 0;

            // Vertex declaration
            VertexDeclaration* decl = mVertexData->vertexDeclaration;
            VertexBufferBinding* binding = mVertexData->vertexBufferBinding;

            size_t offset = 0;
            decl->addElement(POSITION_BINDING, offset, VET_FLOAT3, VES_POSITION);
            //offset += VertexElement::getTypeSize(VET_FLOAT2);
            decl->addElement(COLOUR_BINDING, offset, VET_COLOUR, VES_DIFFUSE);
            decl->addElement(TEXCOORD_BINDING, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);

            HardwareVertexBufferSharedPtr vbuf = 
                HardwareBufferManager::getSingleton().createVertexBuffer(
                    decl->getVertexSize(POSITION_BINDING),
                    mVertexData->vertexCount, 
                    HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
            // bind position and diffuses
            binding->setBinding(POSITION_BINDING, vbuf);

            vbuf = 
                HardwareBufferManager::getSingleton().createVertexBuffer(
                    decl->getVertexSize(COLOUR_BINDING),
                    mVertexData->vertexCount, 
                    HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
            // bind position and diffuses
            binding->setBinding(COLOUR_BINDING, vbuf);

            vbuf = 
                HardwareBufferManager::getSingleton().createVertexBuffer(
                    decl->getVertexSize(TEXCOORD_BINDING),
                    mVertexData->vertexCount, 
                    HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
            // bind position
            binding->setBinding(TEXCOORD_BINDING, vbuf);

            mIndexData->indexStart = 0;
            mIndexData->indexCount = size * 6;

            mIndexData->indexBuffer = HardwareBufferManager::getSingleton().
                createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
                    mIndexData->indexCount,
                    HardwareBuffer::HBU_STATIC_WRITE_ONLY);

            /* Create indexes and tex coords (will be the same every frame)
               Using indexes because it means 1/3 less vertex transforms (4 instead of 6)

               Billboard layout relative to camera:

                2-----3
                |    /|
                |  /  |
                |/    |
                0-----1
            */

            // Create template texcoord data
            Real texData[8] = {
                0.0, 1.0,
                1.0, 1.0,
                0.0, 0.0,
                1.0, 0.0 };

            ushort* pIdx = static_cast<ushort*>(
                mIndexData->indexBuffer->lock(0,
                  mIndexData->indexBuffer->getSizeInBytes(),
                  HardwareBuffer::HBL_DISCARD) );

            vbuf = mVertexData->vertexBufferBinding->getBuffer(TEXCOORD_BINDING);

            Real* pT = static_cast<Real*>(
                vbuf->lock(HardwareBuffer::HBL_DISCARD) );

            for(
                size_t idx, idxOff, texOff, bboard = 0;
                bboard < size;
                ++bboard )
            {
                // Do indexes
                idx    = bboard * 6;
                idxOff = bboard * 4;
                texOff = bboard * 4 * 2;

                pIdx[idx] = static_cast<unsigned short>(idxOff); // + 0;, for clarity
                pIdx[idx+1] = static_cast<unsigned short>(idxOff + 1);
                pIdx[idx+2] = static_cast<unsigned short>(idxOff + 3);
                pIdx[idx+3] = static_cast<unsigned short>(idxOff + 0);
                pIdx[idx+4] = static_cast<unsigned short>(idxOff + 3);
                pIdx[idx+5] = static_cast<unsigned short>(idxOff + 2);

                // Do tex coords
                pT[texOff]   = texData[0];
                pT[texOff+1] = texData[1];
                pT[texOff+2] = texData[2];
                pT[texOff+3] = texData[3];
                pT[texOff+4] = texData[4];
                pT[texOff+5] = texData[5];
                pT[texOff+6] = texData[6];
                pT[texOff+7] = texData[7];
            }

            vbuf->unlock();
            mIndexData->indexBuffer->unlock();
        }
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
    bool BillboardSet::billboardVisible(Camera* cam, ActiveBillboardList::iterator bill)
    {
        // Return always visible if not culling individually
        if (!mCullIndividual) return true;

        // Cull based on sphere (have to transform less)
        Sphere sph;
        Matrix4 xworld;

        getWorldTransforms(&xworld);

        sph.setCenter(xworld * (*bill)->mPosition);

        if ((*bill)->mOwnDimensions)
        {
            sph.setRadius(std::max((*bill)->mWidth, (*bill)->mHeight));
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
    void BillboardSet:: genBillboardAxes(Camera& cam, Vector3* pX, Vector3 *pY, const Billboard* pBill)
    {
        // Default behaviour is that billboards are in local node space
        // so orientation of camera (in world space) must be reverse-transformed 
        // into node space to generate the axes

        Quaternion invTransform = mParentNode->_getDerivedOrientation().Inverse();
        Quaternion camQ;

        switch (mBillboardType)
        {
        case BBT_POINT:
            // Get camera world axes for X and Y (depth is irrelevant)
            camQ = cam.getDerivedOrientation();
            // Convert into billboard local space
            camQ = invTransform * camQ;
            *pX = camQ * Vector3::UNIT_X;
            *pY = camQ * Vector3::UNIT_Y;
            break;
        case BBT_ORIENTED_COMMON:
            // Y-axis is common direction
            // X-axis is cross with camera direction 
            *pY = mCommonDirection;
            // Convert into billboard local space
            *pX = invTransform * cam.getDerivedDirection().crossProduct(*pY);
            
            break;
        case BBT_ORIENTED_SELF:
            // Y-axis is direction
            // X-axis is cross with camera direction 
            *pY = pBill->mDirection;
            // Convert into billboard local space
            *pX = invTransform * cam.getDerivedDirection().crossProduct(*pY);

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
    void BillboardSet::genVertices(Real **pPos, RGBA** pCol, Real **pTex, const Vector3* offsets, const Billboard* pBillboard)
    {
		// Texcoords

       	if (!mFixedTextureCoords)
		{
			// Create template texcoord data
			Real texData[8] = {
				-0.5, 0.5,
				 0.5, 0.5,
				-0.5,-0.5,
				 0.5,-0.5 };

			const Real		rotation = pBillboard->mRotation;
			const Real		cos_rot	 = Math::Cos(rotation);
			const Real		sin_rot	 = Math::Sin(rotation);
		
			*(*pTex)++ = (cos_rot * texData[0]) + (sin_rot * texData[1]) + 0.5;
			*(*pTex)++ = (sin_rot * texData[0]) - (cos_rot * texData[1]) + 0.5;

			*(*pTex)++ = (cos_rot * texData[2]) + (sin_rot * texData[3]) + 0.5;
			*(*pTex)++ = (sin_rot * texData[2]) - (cos_rot * texData[3]) + 0.5;
	        
			*(*pTex)++ = (cos_rot * texData[4]) + (sin_rot * texData[5]) + 0.5;
			*(*pTex)++ = (sin_rot * texData[4]) - (cos_rot * texData[5]) + 0.5;
	        
			*(*pTex)++ = (cos_rot * texData[6]) + (sin_rot * texData[7]) + 0.5;
			*(*pTex)++ = (sin_rot * texData[6]) - (cos_rot * texData[7]) + 0.5;
		}

        // Positions

        // Left-top
        *(*pPos)++ = offsets[0].x + pBillboard->mPosition.x;
        *(*pPos)++ = offsets[0].y + pBillboard->mPosition.y;
        *(*pPos)++ = offsets[0].z + pBillboard->mPosition.z;
        // Right-top
        *(*pPos)++ = offsets[1].x + pBillboard->mPosition.x;
        *(*pPos)++ = offsets[1].y + pBillboard->mPosition.y;
        *(*pPos)++ = offsets[1].z + pBillboard->mPosition.z;
        // Left-bottom
        *(*pPos)++ = offsets[2].x + pBillboard->mPosition.x;
        *(*pPos)++ = offsets[2].y + pBillboard->mPosition.y;
        *(*pPos)++ = offsets[2].z + pBillboard->mPosition.z;
        // Right-bottom
        *(*pPos)++ = offsets[3].x + pBillboard->mPosition.x;
        *(*pPos)++ = offsets[3].y + pBillboard->mPosition.y;
        *(*pPos)++ = offsets[3].z + pBillboard->mPosition.z;

        // Update colours
        RGBA colour;
        Root::getSingleton().convertColourValue(pBillboard->mColour, &colour);

        *(*pCol)++ = colour;
        *(*pCol)++ = colour;
        *(*pCol)++ = colour;
        *(*pCol)++ = colour;

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
    const String& BillboardSet::getName(void) const
    {
        return mName;
    }
    //-----------------------------------------------------------------------
    const String& BillboardSet::getMovableType(void) const
    {
        return msMovableType;
    }
    //-----------------------------------------------------------------------
    Real BillboardSet::getSquaredViewDepth(const Camera* cam) const
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
        return mParentNode->getLights();
    }

}
