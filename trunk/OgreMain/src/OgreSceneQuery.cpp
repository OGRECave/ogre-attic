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
#include "OgreSceneQuery.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    SceneQuery::SceneQuery(SceneManager* mgr)
    : mLastResult(NULL), mParentSceneMgr(mgr), mQueryMask(0xFFFFFFFF)
    {
    }
    //-----------------------------------------------------------------------
    SceneQuery::~SceneQuery()
    {
        clearResults();
    }
    //-----------------------------------------------------------------------
    SceneQueryResult& SceneQuery::getLastResults(void)
    {
        assert(mLastResult);
        return *mLastResult;
    }
    //-----------------------------------------------------------------------
    void SceneQuery::clearResults(void)
    {
        if (mLastResult)
        {
            delete mLastResult;
        }
        mLastResult = NULL;
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
    AxisAlignedBoxSceneQuery::AxisAlignedBoxSceneQuery(SceneManager* mgr)
        : SceneQuery(mgr)
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
        : SceneQuery(mgr)
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
    PyramidSceneQuery::PyramidSceneQuery(SceneManager* mgr) : SceneQuery(mgr)
    {
    }
    //-----------------------------------------------------------------------
    PyramidSceneQuery::~PyramidSceneQuery()
    {
    }
    //-----------------------------------------------------------------------
    IntersectionSceneQuery::IntersectionSceneQuery(SceneManager* mgr)
    : mLastResult(NULL), mParentSceneMgr(mgr), mQueryMask(0xFFFFFFFF)
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
    //-----------------------------------------------------------------------
    void IntersectionSceneQuery::setQueryMask(unsigned long mask)
    {
        mQueryMask = mask;
    }
    //-----------------------------------------------------------------------
    unsigned long IntersectionSceneQuery::getQueryMask(void)
    {
        return mQueryMask;
    }





}
    



