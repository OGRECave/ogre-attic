/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
#include "OgreStableHeaders.h"
#include "OgreSceneQuery.h"
#include "OgreException.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    SceneQuery::SceneQuery(SceneManager* mgr)
        : mParentSceneMgr(mgr), mQueryMask(0xFFFFFFFF), mWorldFragmentType(SceneQuery::WFT_NONE)
    {
    }
    //-----------------------------------------------------------------------
    SceneQuery::~SceneQuery()
    {
    }
    //-----------------------------------------------------------------------
    void SceneQuery::setQueryMask(unsigned long mask)
    {
        mQueryMask = mask;
    }
    //-----------------------------------------------------------------------
    unsigned long SceneQuery::getQueryMask(void)
    {
        return mQueryMask;
    }
    //-----------------------------------------------------------------------
    void SceneQuery::setWorldFragmentType(enum SceneQuery::WorldFragmentType wft)
    {
        // Check supported
        if (mSupportedWorldFragments.find(wft) == mSupportedWorldFragments.end())
        {
            Except(Exception::ERR_INVALIDPARAMS, "This world fragment type is not supported.",
                "SceneQuery::setWorldFragmentType");
        }
        mWorldFragmentType = wft;
    }
    //-----------------------------------------------------------------------
    SceneQuery::WorldFragmentType 
    SceneQuery::getWorldFragmentType(void)
    {
        return mWorldFragmentType;
    }
    //-----------------------------------------------------------------------
    RegionSceneQuery::RegionSceneQuery(SceneManager* mgr)
        :SceneQuery(mgr), mLastResult(NULL)
    {
    }
    //-----------------------------------------------------------------------
    RegionSceneQuery::~RegionSceneQuery()
    {
        clearResults();
    }
    //-----------------------------------------------------------------------
    SceneQueryResult& RegionSceneQuery::getLastResults(void)
    {
        assert(mLastResult);
        return *mLastResult;
    }
    //-----------------------------------------------------------------------
    void RegionSceneQuery::clearResults(void)
    {
        if (mLastResult)
        {
            delete mLastResult;
        }
        mLastResult = NULL;
    }
	//---------------------------------------------------------------------
    SceneQueryResult&
    RegionSceneQuery::execute(void)
    {
        clearResults();
        mLastResult = new SceneQueryResult();
        // Call callback version with self as listener
        execute(this);
        return *mLastResult;
    }
	//---------------------------------------------------------------------
    bool RegionSceneQuery::
        queryResult(MovableObject* obj)
    {
        // Add to internal list
        mLastResult->movables.push_back(obj);
        // Continue
        return true;
    }
	//---------------------------------------------------------------------
    bool RegionSceneQuery::queryResult(SceneQuery::WorldFragment* fragment)
    {
        // Add to internal list
        mLastResult->worldFragments.push_back(fragment);
        // Continue
        return true;
    }
    //-----------------------------------------------------------------------
    AxisAlignedBoxSceneQuery::AxisAlignedBoxSceneQuery(SceneManager* mgr)
        : RegionSceneQuery(mgr)
    {
    }
    //-----------------------------------------------------------------------
    AxisAlignedBoxSceneQuery::~AxisAlignedBoxSceneQuery()
    {
    }
    //-----------------------------------------------------------------------
    void AxisAlignedBoxSceneQuery::setBox(const AxisAlignedBox& box)
    {
        mAABB = box;
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& AxisAlignedBoxSceneQuery::getBox(void)
    {
        return mAABB;
    }
    //-----------------------------------------------------------------------
    SphereSceneQuery::SphereSceneQuery(SceneManager* mgr)
        : RegionSceneQuery(mgr)
    {
    }
    //-----------------------------------------------------------------------
    SphereSceneQuery::~SphereSceneQuery()
    {
    }
    //-----------------------------------------------------------------------
    void SphereSceneQuery::setSphere(const Sphere& sphere)
    {
        mSphere = sphere;
    }
    //-----------------------------------------------------------------------
    const Sphere& SphereSceneQuery::getSphere()
    {
        return mSphere;
    }
    //-----------------------------------------------------------------------
    RaySceneQuery::RaySceneQuery(SceneManager* mgr) : SceneQuery(mgr)
    {
        mSortByDistance = false;
        mMaxResults = 0;
        mLastResult = NULL;
    }
    //-----------------------------------------------------------------------
    RaySceneQuery::~RaySceneQuery()
    {
    }
    //-----------------------------------------------------------------------
    void RaySceneQuery::setRay(const Ray& ray)
    {
        mRay = ray;
    }
    //-----------------------------------------------------------------------
    const Ray& RaySceneQuery::getRay(void)
    {
        return mRay;
    }
    //-----------------------------------------------------------------------
    void RaySceneQuery::setSortByDistance(bool sort, ushort maxresults)
    {
        mSortByDistance = sort;
        mMaxResults = maxresults;
    }
    //-----------------------------------------------------------------------
    bool RaySceneQuery::getSortByDistance(void)
    {
        return mSortByDistance;
    }
    //-----------------------------------------------------------------------
    ushort RaySceneQuery::getMaxResults(void)
    {
        return mMaxResults;
    }
    //-----------------------------------------------------------------------
    RaySceneQueryResult& RaySceneQuery::execute(void)
    {
        clearResults();
        mLastResult = new RaySceneQueryResult();
        // Call callback version with self as listener
        this->execute(this);

        if (mSortByDistance)
        {
            // Perform sort
            mLastResult->sort();

            if (mMaxResults && mLastResult->size() > mMaxResults)
            {
                // Constrain to maxresults
                RaySceneQueryResult::iterator start;
                int x = 0;
                for(start = mLastResult->begin(); x < mMaxResults; ++x)
                {
                    // Increment deletion start point
                    ++start;
                }
                // erase
                mLastResult->erase(start, mLastResult->end());
            }
        }

        return *mLastResult;

    }
    //-----------------------------------------------------------------------
    RaySceneQueryResult& RaySceneQuery::getLastResults(void)
    {
        assert (mLastResult);
        return *mLastResult;
    }
    //-----------------------------------------------------------------------
    void RaySceneQuery::clearResults(void)
    {
        if (mLastResult)
        {
            delete mLastResult;
        }
        mLastResult = NULL;
    }
    //-----------------------------------------------------------------------
    bool RaySceneQuery::queryResult(MovableObject* obj, Real distance)
    {
        // Add to internal list
        RaySceneQueryResultEntry dets;
        dets.distance = distance;
        dets.movable = obj;
        dets.worldFragment = NULL;
        mLastResult->push_back(dets);
        // Continue
        return true;
    }
    //-----------------------------------------------------------------------
    bool RaySceneQuery::queryResult(SceneQuery::WorldFragment* fragment, Real distance)
    {
        // Add to internal list
        RaySceneQueryResultEntry dets;
        dets.distance = distance;
        dets.movable = NULL;
        dets.worldFragment = fragment;
        mLastResult->push_back(dets);
        // Continue
        return true;
    }
    //-----------------------------------------------------------------------
    /*
    PyramidSceneQuery::PyramidSceneQuery(SceneManager* mgr) : RegionSceneQuery(mgr)
    {
    }
    //-----------------------------------------------------------------------
    PyramidSceneQuery::~PyramidSceneQuery()
    {
    }
    */
    //-----------------------------------------------------------------------
    IntersectionSceneQuery::IntersectionSceneQuery(SceneManager* mgr)
    : SceneQuery(mgr), mLastResult(NULL)
    {
    }
    //-----------------------------------------------------------------------
    IntersectionSceneQuery::~IntersectionSceneQuery()
    {
        clearResults();
    }
    //-----------------------------------------------------------------------
    IntersectionSceneQueryResult& IntersectionSceneQuery::getLastResults(void)
    {
        assert(mLastResult);
        return *mLastResult;
    }
    //-----------------------------------------------------------------------
    void IntersectionSceneQuery::clearResults(void)
    {
        if (mLastResult)
        {
            delete mLastResult;
        }
        mLastResult = NULL;
    }
	//---------------------------------------------------------------------
    IntersectionSceneQueryResult&
    IntersectionSceneQuery::execute(void)
    {
        clearResults();
        mLastResult = new IntersectionSceneQueryResult();
        // Call callback version with self as listener
        execute(this);
        return *mLastResult;
    }
	//---------------------------------------------------------------------
    bool IntersectionSceneQuery::
        queryResult(MovableObject* first, MovableObject* second)
    {
        // Add to internal list
        mLastResult->movables2movables.push_back(
            SceneQueryMovableObjectPair(first, second)
            );
        // Continue
        return true;
    }
	//---------------------------------------------------------------------
    bool IntersectionSceneQuery::
        queryResult(MovableObject* movable, SceneQuery::WorldFragment* fragment)
    {
        // Add to internal list
        mLastResult->movables2world.push_back(
            SceneQueryMovableObjectWorldFragmentPair(movable, fragment)
            );
        // Continue
        return true;
    }




}
    



