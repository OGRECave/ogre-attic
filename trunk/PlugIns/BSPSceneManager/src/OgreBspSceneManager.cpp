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
#include "OgreBspSceneManager.h"
#include "OgreBspResourceManager.h"
#include "OgreBspLevel.h"
#include "OgreBspNode.h"
#include "OgreException.h"
#include "OgreRenderSystem.h"
#include "OgreCamera.h"
#include "OgreMaterial.h"
#include "OgrePatchSurface.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMath.h"
#include "OgreControllerManager.h"
#include "OgreLogManager.h"
#include "OgreBspSceneNode.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"


#include <fstream>

namespace Ogre {

    //-----------------------------------------------------------------------
    BspSceneManager::BspSceneManager()
    {
        // Set features for debugging render
        mShowNodeAABs = false;

        // Instantiate BspResourceManager
        // Will be managed by singleton
        mBspResMgr = new BspResourceManager();

        // No sky by default
        mSkyPlaneEnabled = false;
        mSkyBoxEnabled = false;
        mSkyDomeEnabled = false;

        mLevel = 0;

    }

    //-----------------------------------------------------------------------
    BspSceneManager::~BspSceneManager()
    {
        freeMemory();
        delete mBspResMgr;
    }

    //-----------------------------------------------------------------------
    void BspSceneManager::setWorldGeometry(const String& filename)
    {
        // Check extension is .bsp
        char extension[6];
        size_t pos = filename.find_last_of(".");
		if( pos == String::npos )
            Except(
				Exception::ERR_INVALIDPARAMS,
                "Unable to load world geometry. Invalid extension (must be .bsp).",
                "BspSceneManager::setWorldGeometry");

        strcpy(extension, filename.substr(pos + 1, filename.length() - pos).c_str());

        if (stricmp(extension, "bsp"))
            Except(Exception::ERR_INVALIDPARAMS,
			"Unable to load world geometry. Invalid extension (must be .bsp).",
            "BspSceneManager::setWorldGeometry");

        // Load using resource manager
        mLevel = BspResourceManager::getSingleton().load(filename);

        // Init static render operation
        mRenderOp.vertexData = mLevel->mVertexData;
        // index data is per-frame
        mRenderOp.indexData = new IndexData();
        mRenderOp.indexData->indexStart = 0;
        mRenderOp.indexData->indexCount = 0;
        // Create enough index space to render whole level
        mRenderOp.indexData->indexBuffer = HardwareBufferManager::getSingleton()
            .createIndexBuffer(
                HardwareIndexBuffer::IT_32BIT, // always 32-bit
                mLevel->mNumIndexes, 
                HardwareBuffer::HBU_DYNAMIC, false);

        mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
        mRenderOp.useIndexes = true;


    }
    //-----------------------------------------------------------------------
    void BspSceneManager::_findVisibleObjects(Camera* cam)
    {
        // Clear unique list of movables for this frame
        mMovablesForRendering.clear();
        // Walk the tree, tag static geometry, return camera's node (for info only)
        // Movables are now added to the render queue in processVisibleLeaf
        BspNode* cameraNode = walkTree(cam);


    }
    //-----------------------------------------------------------------------
    void BspSceneManager::renderStaticGeometry(void)
    {
        // Cache vertex/face data first
        std::vector<StaticFaceGroup*>::const_iterator faceGrpi;
        static RenderOperation patchOp;
        
        // no world transform required
        mDestRenderSystem->_setWorldMatrix(Matrix4::IDENTITY);
        // Set view / proj
        mDestRenderSystem->_setViewMatrix(mCameraInProgress->getViewMatrix());
        mDestRenderSystem->_setProjectionMatrix(mCameraInProgress->getProjectionMatrix());

        // For each material in turn, cache rendering data & render
        MaterialFaceGroupMap::const_iterator mati;

        for (mati = mMatFaceGroupMap.begin(); mati != mMatFaceGroupMap.end(); ++mati)
        {
            // Get Material
            Material* thisMaterial = mati->first;

            // Empty existing cache
            mRenderOp.indexData->indexCount = 0;
            // lock index buffer ready to receive data
            unsigned int* pIdx = static_cast<unsigned int*>(
                mRenderOp.indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

            for (faceGrpi = mati->second.begin(); faceGrpi != mati->second.end(); ++faceGrpi)
            {
                // Cache each
                unsigned int numelems = cacheGeometry(pIdx, *faceGrpi);
                mRenderOp.indexData->indexCount += numelems;
                pIdx += numelems;
            }
            // Unlock the buffer
            mRenderOp.indexData->indexBuffer->unlock();

            // Skip if no faces to process (we're not doing flare types yet)
            if (mRenderOp.indexData->indexCount == 0)
                continue;

            Technique::PassIterator pit = thisMaterial->getTechnique(0)->getPassIterator();

            while (pit.hasMoreElements())
            {
                setPass(pit.getNext());

                mDestRenderSystem->_render(mRenderOp);


            } 


        } // for each material

        /*
        if (mShowNodeAABs)
        {
            mDestRenderSystem->_render(mAABGeometry);
        }
        */
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::_renderVisibleObjects(void)
    {
        // Render static level geometry first
        renderStaticGeometry();

        // Call superclass to render the rest
        SceneManager::_renderVisibleObjects();

    }

    //-----------------------------------------------------------------------
    // REMOVE THIS CRAP
    //-----------------------------------------------------------------------
    // Temp debug lines
    bool firstTime = true;
    std::ofstream of;
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    BspNode* BspSceneManager::walkTree(Camera* camera)
    {
        // Locate the leaf node where the camera is located
        BspNode* cameraNode = mLevel->findLeaf(camera->getDerivedPosition());

        mMatFaceGroupMap.clear();
        mFaceGroupSet.clear();

        // Scan through all the other leaf nodes looking for visibles
        int i = mLevel->mNumNodes - mLevel->mLeafStart;
        BspNode* nd = mLevel->mRootNode + mLevel->mLeafStart;

        /*
        if (firstTime)
        {
            camera->getViewMatrix(); // Force update view before report
            of.open("BspSceneManager.log");
            of << *camera << std::endl;
            of << "Camera Node: " << *cameraNode << std::endl;
            of << "Vertex Data: " << std::endl;
            for (int testi = 0; testi < mLevel->mNumVertices; ++testi)
            {
                of << " " << testi << ": pos(" <<
                  mLevel->mVertices[testi].position[0] << ", " <<
                    mLevel->mVertices[testi].position[1] << ", " << mLevel->mVertices[testi].position[2] << ")" <<
                    " uv(" << mLevel->mVertices[testi].texcoords[0] << ", " << mLevel->mVertices[testi].texcoords[1] << ")" <<
                    " lm(" << mLevel->mVertices[testi].lightmap[0] << ", " << mLevel->mVertices[testi].lightmap[1] << ")" << std::endl;
            }
            of << "Element data:" << std::endl;
            for (testi = 0; testi < mLevel->mNumElements; ++testi)
            {
                of << " " << testi << ": " << mLevel->mElements[testi] << std::endl;

            }
        }
        */

        while (i--)
        {
            if (mLevel->isLeafVisible(cameraNode, nd))
            {

                // Visible according to PVS, check bounding box against frustum
                FrustumPlane plane;
                if (camera->isVisible(nd->getBoundingBox(), &plane))
                {
                    //if (firstTime)
                    //{
                    //    of << "Visible Node: " << *nd << std::endl;
                    //}
                    processVisibleLeaf(nd, camera);
                    if (mShowNodeAABs)
                        addBoundingBox(nd->getBoundingBox(), true);
                }
            }
            nd++;
        }


        // TEST
        //if (firstTime)
        //{
        //    of.close();
        //    firstTime = false;
        //}
        return cameraNode;

    }
    //-----------------------------------------------------------------------
    void BspSceneManager::processVisibleLeaf(BspNode* leaf, Camera* cam)
    {
        Material* pMat;
        // Parse the leaf node's faces, add face groups to material map
        int numGroups = leaf->getNumFaceGroups();
        int idx = leaf->getFaceGroupStart();

        while (numGroups--)
        {
            int realIndex = mLevel->mLeafFaceGroups[idx++];
            // Check not already included
            if (mFaceGroupSet.find(realIndex) != mFaceGroupSet.end())
                continue;
            StaticFaceGroup* faceGroup = mLevel->mFaceGroups + realIndex;
            // Get Material pointer by handle
            pMat = getMaterial(faceGroup->materialHandle);
            // Check normal (manual culling)
            ManualCullingMode cullMode = pMat->getTechnique(0)->getPass(0)->getManualCullingMode();
            if (cullMode != MANUAL_CULL_NONE)
            {
                Real dist = faceGroup->plane.getDistance(cam->getDerivedPosition());
                if ( (dist < 0 && cullMode == MANUAL_CULL_BACK) ||
                     (dist > 0 && cullMode == MANUAL_CULL_FRONT) )
                    continue; // skip
            }
            mFaceGroupSet.insert(realIndex);
            // Try to insert, will find existing if already there
            std::pair<MaterialFaceGroupMap::iterator, bool> matgrpi;
            matgrpi = mMatFaceGroupMap.insert(
                MaterialFaceGroupMap::value_type(pMat, std::vector<StaticFaceGroup*>())
                );
            // Whatever happened, matgrpi.first is map iterator
            // Need to get second part of that to get vector
            matgrpi.first->second.push_back(faceGroup);

            //if (firstTime)
            //{
            //    of << "  Emitting faceGroup: index=" << realIndex << ", " << *faceGroup << std::endl;
            //}
        }

        // Add movables to render queue, provided it hasn't been seen already
        const BspNode::IntersectingObjectSet& objects = leaf->getObjects();
        BspNode::IntersectingObjectSet::const_iterator oi, oiend;
        oiend = objects.end();
        for (oi = objects.begin(); oi != oiend; ++oi)
        {
            if (mMovablesForRendering.find(*oi) == mMovablesForRendering.end())
            {
                // It hasn't been seen yet
                MovableObject *mov = const_cast<MovableObject*>(*oi); // hacky
                if (mov->isVisible() && 
					cam->isVisible(mov->getWorldBoundingBox()))
                {
                    mov->_notifyCurrentCamera(cam);
                    mov->_updateRenderQueue(&mRenderQueue);
                    // Check if the bounding box should be shown.
                    SceneNode* sn = static_cast<SceneNode*>(mov->getParentNode());
                    if (sn->getShowBoundingBox() || mShowBoundingBoxes)
                    {
                        sn->_addBoundingBoxToQueue(&mRenderQueue);
                    }
                    mMovablesForRendering.insert(*oi);
                }

            }
        }


    }
    //-----------------------------------------------------------------------
    unsigned int BspSceneManager::cacheGeometry(unsigned int* pIndexes, 
        const StaticFaceGroup* faceGroup)
    {
        // Skip sky always
        if (faceGroup->isSky)
            return 0;

        size_t idxStart, numIdx, vertexStart;

        if (faceGroup->fType == FGT_FACE_LIST)
        {
            idxStart = faceGroup->elementStart;
            numIdx = faceGroup->numElements;
            vertexStart = faceGroup->vertexStart;
        }
        else if (faceGroup->fType == FGT_PATCH)
        {

            idxStart = faceGroup->patchSurf->getIndexOffset();
            numIdx = faceGroup->patchSurf->getCurrentIndexCount();
            vertexStart = faceGroup->patchSurf->getVertexOffset();
        }


        // Copy index data
        unsigned int* pSrc = static_cast<unsigned int*>(
            mLevel->mIndexes->lock(
                idxStart * sizeof(unsigned int),
                numIdx * sizeof(unsigned int), 
                HardwareBuffer::HBL_READ_ONLY));
        // Offset the indexes here
        // we have to do this now rather than up-front because the 
        // indexes are sometimes reused to address different vertex chunks
        for (size_t elem = 0; elem < numIdx; ++elem)
        {
            *pIndexes++ = *pSrc++ + vertexStart;
        }
        mLevel->mIndexes->unlock();

        // return number of elements
        return static_cast<unsigned int>(numIdx);


    }

    //-----------------------------------------------------------------------
    void BspSceneManager::freeMemory(void)
    {
        // no need to delete index buffer, will be handled by shared pointer
        //delete mRenderOp.indexData; // causing an error right now?
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::showNodeBoxes(bool show)
    {
        mShowNodeAABs = show;
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::addBoundingBox(AxisAlignedBox& aab, bool visible)
    {
        /*
        unsigned long visibleColour;
        unsigned long nonVisibleColour;
        Root& r = Root::getSingleton();

        r.convertColourValue(ColourValue::White, &visibleColour);
        r.convertColourValue(ColourValue::Blue, &nonVisibleColour);
        if (mShowNodeAABs)
        {
            // Add set of lines
            Real* pVertices = (Real*)mAABGeometry.pVertices + (mAABGeometry.numVertices*3);
            unsigned short* pIndexes = mAABGeometry.pIndexes + mAABGeometry.numIndexes;
            unsigned long* pColours = (unsigned long*)mAABGeometry.pDiffuseColour + mAABGeometry.numVertices;

            const Vector3* pCorner = aab.getAllCorners();

            int i;
            for (i = 0; i < 8; ++i)
            {
                *pVertices++ = pCorner->x;
                *pVertices++ = pCorner->y;
                *pVertices++ = pCorner->z;
                pCorner++;

                if (visible)
                {
                    *pColours++ = visibleColour;
                }
                else
                {
                    *pColours++ = nonVisibleColour;
                }

            }

            *pIndexes++ = 0 + mAABGeometry.numVertices;
            *pIndexes++ = 1 + mAABGeometry.numVertices;
            *pIndexes++ = 1 + mAABGeometry.numVertices;
            *pIndexes++ = 2 + mAABGeometry.numVertices;
            *pIndexes++ = 2 + mAABGeometry.numVertices;
            *pIndexes++ = 3 + mAABGeometry.numVertices;
            *pIndexes++ = 3 + mAABGeometry.numVertices;
            *pIndexes++ = 1 + mAABGeometry.numVertices;
            *pIndexes++ = 4 + mAABGeometry.numVertices;
            *pIndexes++ = 5 + mAABGeometry.numVertices;
            *pIndexes++ = 5 + mAABGeometry.numVertices;
            *pIndexes++ = 6 + mAABGeometry.numVertices;
            *pIndexes++ = 6 + mAABGeometry.numVertices;
            *pIndexes++ = 7 + mAABGeometry.numVertices;
            *pIndexes++ = 7 + mAABGeometry.numVertices;
            *pIndexes++ = 4 + mAABGeometry.numVertices;
            *pIndexes++ = 1 + mAABGeometry.numVertices;
            *pIndexes++ = 5 + mAABGeometry.numVertices;
            *pIndexes++ = 2 + mAABGeometry.numVertices;
            *pIndexes++ = 4 + mAABGeometry.numVertices;
            *pIndexes++ = 0 + mAABGeometry.numVertices;
            *pIndexes++ = 6 + mAABGeometry.numVertices;
            *pIndexes++ = 3 + mAABGeometry.numVertices;
            *pIndexes++ = 7 + mAABGeometry.numVertices;


            mAABGeometry.numVertices += 8;
            mAABGeometry.numIndexes += 24;


        }
        */

    }
    //-----------------------------------------------------------------------
    ViewPoint BspSceneManager::getSuggestedViewpoint(bool random)
    {
        if (!mLevel || mLevel->mPlayerStarts.size() == 0)
        {
            // No level, use default
            return SceneManager::getSuggestedViewpoint(random);
        }
        else
        {
            if (random)
            {
                size_t idx = (size_t)( Math::getSingleton().UnitRandom() * mLevel->mPlayerStarts.size() );
                return mLevel->mPlayerStarts[idx];
            }
            else
            {
                return mLevel->mPlayerStarts[0];
            }


        }

    }
    //-----------------------------------------------------------------------
    SceneNode * BspSceneManager::createSceneNode( void )
    {
        BspSceneNode * sn = new BspSceneNode( this );
        mSceneNodes[ sn->getName() ] = sn;
        return sn;
    }
    //-----------------------------------------------------------------------
    SceneNode * BspSceneManager::createSceneNode( const String &name )
    {
        BspSceneNode * sn = new BspSceneNode( this, name );
        mSceneNodes[ sn->getName() ] = sn;
        return sn;
    }
    //-----------------------------------------------------------------------
    void BspSceneManager::_notifyObjectMoved(const MovableObject* mov, 
        const Vector3& pos)
    {
        mLevel->_notifyObjectMoved(mov, pos);
    }
    //-----------------------------------------------------------------------
	void BspSceneManager::_notifyObjectDetached(const MovableObject* mov)
	{
		mLevel->_notifyObjectDetached(mov);
	}
    //-----------------------------------------------------------------------
    AxisAlignedBoxSceneQuery* BspSceneManager::
    createAABBQuery(const AxisAlignedBox& box, unsigned long mask)
    {
        // TODO
        return NULL;
    }
    //-----------------------------------------------------------------------
    SphereSceneQuery* BspSceneManager::
    createSphereQuery(const Sphere& sphere, unsigned long mask)
    {
        // TODO
        return NULL;
    }
    //-----------------------------------------------------------------------
    RaySceneQuery* BspSceneManager::
    createRayQuery(const Ray& ray, unsigned long mask)
    {
        // TODO
        return NULL;
    }
    //-----------------------------------------------------------------------
    IntersectionSceneQuery* BspSceneManager::
    createIntersectionQuery(unsigned long mask)
    {
        BspIntersectionSceneQuery* q = new BspIntersectionSceneQuery(this);
        q->setQueryMask(mask);
        return q;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    BspIntersectionSceneQuery::BspIntersectionSceneQuery(SceneManager* creator) 
        : DefaultIntersectionSceneQuery(creator)
    {
        // Add bounds fragment type
        mSupportedWorldFragments.insert(SceneQuery::WFT_PLANE_BOUNDED_REGION);
        
    }
    void BspIntersectionSceneQuery::execute(IntersectionSceneQueryListener* listener)
    {
        /*
        Go through each leaf node in BspLevel and check movables against each other and world
        Issue: some movable-movable intersections could be reported twice if 2 movables
        overlap 2 leaves?
        */
        BspLevel* lvl = ((BspSceneManager*)mParentSceneMgr)->getLevel();
        BspNode* leaf = lvl->getLeafStart();
        int numLeaves = lvl->getNumLeaves();
        
        while (numLeaves--)
        {
            const BspNode::IntersectingObjectSet& objects = leaf->getObjects();
            int numObjects = (int)objects.size();

            BspNode::IntersectingObjectSet::const_iterator a, b, theEnd;
            theEnd = objects.end();
            a = objects.begin();
            for (int oi = 0; oi < numObjects; ++oi, ++a)
            {
                const MovableObject* aObj = *a;
                // Skip this object if collision not enabled
                if (!(aObj->getQueryFlags() & mQueryMask))
                    continue;

                if (oi < (numObjects-1))
                {
                    // Check object against others in this node
                    b = a;
                    for (++b; b != theEnd; ++b)
                    {
                        const MovableObject* bObj = *b;
                        // Apply mask to b (both must pass)
                        if ( bObj->getQueryFlags() & mQueryMask)
                        {
                            const AxisAlignedBox& box1 = aObj->getWorldBoundingBox();
                            const AxisAlignedBox& box2 = bObj->getWorldBoundingBox();

                            if (box1.intersects(box2))
                            {
                                listener->queryResult(const_cast<MovableObject*>(aObj), 
                                    const_cast<MovableObject*>(bObj)); // hacky
                            }
                        }
                    }
                }
                // Check object against brushes
                const BspNode::NodeBrushList& brushes = leaf->getSolidBrushes();
                BspNode::NodeBrushList::const_iterator bi, biend;
                biend = brushes.end();
                Real radius = aObj->getBoundingRadius();
                const Vector3& pos = aObj->getParentNode()->_getDerivedPosition();

                for (bi = brushes.begin(); bi != biend; ++bi)
                {
                    std::list<Plane>::const_iterator planeit, planeitend;
                    planeitend = (*bi)->planes.end();
                    bool brushIntersect = true; // Assume intersecting for now

                    for (planeit = (*bi)->planes.begin(); planeit != planeitend; ++planeit)
                    {
                        Real dist = planeit->getDistance(pos);
                        if (dist > radius)
                        {
                            // Definitely excluded
                            brushIntersect = false;
                            break;
                        }
                    }
                    if (brushIntersect)
                    {
                        // report this brush as it's WorldFragment
                        assert((*bi)->fragment.fragmentType == SceneQuery::WFT_PLANE_BOUNDED_REGION);
                        listener->queryResult(const_cast<MovableObject*>(aObj), // hacky
                                const_cast<WorldFragment*>(&((*bi)->fragment))); 
                    }

                }


            }

            ++leaf;
        }



    }
}

