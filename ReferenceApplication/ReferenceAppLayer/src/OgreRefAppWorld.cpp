/*
-----------------------------------------------------------------------------
This source file is part of the OGRE Reference Application, a layer built
on top of OGRE(Object-oriented Graphics Rendering Engine)
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
#include "OgreRefAppWorld.h"
#include "OgreRefAppOgreHead.h"
#include "OgreRefAppPlane.h"
#include "OgreRefAppBall.h"
#include "OgreRefAppJointSubtypes.h"

//-------------------------------------------------------------------------
template<> OgreRefApp::World* Ogre::Singleton<OgreRefApp::World>::ms_Singleton = 0;
//-------------------------------------------------------------------------
namespace OgreRefApp
{
    //-------------------------------------------------------------------------
    World::World(SceneManager* sceneMgr)
    {
        mSceneMgr = sceneMgr;
        mSimulationStepSize = 0.01f;

        // Create the dynamics world
        mOdeWorld = new dWorld();
        mOdeContactGroup = new dJointGroup();

        mIntersectionQuery = mSceneMgr->createIntersectionQuery();

    }
    //-------------------------------------------------------------------------
    World::~World()
    {
		clear();

		delete mIntersectionQuery;

        // Destroy dynamix world
		delete mOdeContactGroup;
        delete mOdeWorld;

    }
    //-------------------------------------------------------------------------
    SceneManager* World::getSceneManager(void)
    {
        return mSceneMgr;
    }
    //-------------------------------------------------------------------------
    OgreHead* World::createOgreHead(const String& name, 
        const Vector3& pos, const Quaternion& orientation)
    {
        OgreHead* head = new OgreHead(name);
        head->setPosition(pos);
        head->setOrientation(orientation);

        mObjects[name] = head;

        return head;
    }
    //-------------------------------------------------------------------------
    OgreRefApp::Plane* World::createPlane(const String& name, Real width, Real height, const Vector3& pos, 
        const Quaternion& orientation)
    {
        OgreRefApp::Plane* plane = new OgreRefApp::Plane(name, width, height);
        plane->setPosition(pos);
        plane->setOrientation(orientation);

        mObjects[name] = plane;

        return plane;
    }
    //-------------------------------------------------------------------------
    OgreRefApp::Ball* World::createBall(const String& name, Real radius, const Vector3& pos, 
        const Quaternion& orientation)
    {
        OgreRefApp::Ball* ball = new OgreRefApp::Ball(name, radius);
        ball->setPosition(pos);
        ball->setOrientation(orientation);

        mObjects[name] = ball;

        return ball;
    }
    //-------------------------------------------------------------------------
    void World::clear(void)
    {
        ObjectMap::iterator i;
        for (i = mObjects.begin(); i != mObjects.end(); ++i)
        {
            delete i->second;
        }
        mObjects.clear();

        JointMap::iterator ji;
        for (ji = mJoints.begin(); ji != mJoints.end(); ++ji)
        {
            delete ji->second;
        }
        mJoints.clear();
    }
    //-------------------------------------------------------------------------
    World& World::getSingleton(void)
    {
        return Singleton<World>::getSingleton();
    }
    //-------------------------------------------------------------------------
    dWorld* World::getOdeWorld(void)
    {
        return mOdeWorld;
    }
    //-------------------------------------------------------------------------
    void World::_applyDynamics(Real timeElapsed)
    {
        if (timeElapsed != 0.0f)
        {
            // ODE will throw an error if timestep = 0

            mOdeWorld->step(dReal(timeElapsed));
            // Now update the objects in the world
            ObjectSet::iterator i, iend;
            iend = mDynamicsObjects.end();
            for (i = mDynamicsObjects.begin(); i != iend; ++i)
            {
                (*i)->_updateFromDynamics();
            }
            // Clear contacts
            mOdeContactGroup->empty();
        }

    }
    //-------------------------------------------------------------------------
    void World::_notifyDynamicsStateForObject(ApplicationObject* obj, bool dynamicsEnabled)
    {
        // NB std::set prevents duplicates & errors on erasing non-existent objects
        if (dynamicsEnabled)
        {
            mDynamicsObjects.insert(obj);
        }
        else
        {
            mDynamicsObjects.erase(obj);
        }
    }
    //-------------------------------------------------------------------------
    void World::setGravity(const Vector3& vec)
    {
        mGravity = vec;
        mOdeWorld->setGravity(vec.x, vec.y, vec.x);
    }
    //-------------------------------------------------------------------------
    const Vector3& World::getGravity(void)
    {
        return mGravity;
    }
    //-------------------------------------------------------------------------
    dJointGroup* World::getOdeContactJointGroup(void)
    {
        return mOdeContactGroup;
    }
    //-------------------------------------------------------------------------
    void World::_applyCollision(void)
    {
        // Collision detection
        IntersectionSceneQueryResult& results = mIntersectionQuery->execute();

        SceneQueryMovableIntersectionList::iterator it, itend;
        itend = results.movables2movables.end();
        for (it = results.movables2movables.begin(); it != itend; ++it)
        {
            /* debugging
            MovableObject *mo1, *mo2;
            mo1 = it->first;
            mo2 = it->second;
            */

            // Get user defined objects (generic in OGRE)
            UserDefinedObject *uo1, *uo2;
            uo1 = it->first->getUserObject();
            uo2 = it->second->getUserObject();

            // Only perform collision if we have UserDefinedObject links
            if (uo1 && uo2)
            {
                // Cast to ApplicationObject
                ApplicationObject *ao1, *ao2;
                ao1 = static_cast<ApplicationObject*>(uo1);
                ao2 = static_cast<ApplicationObject*>(uo2);
                // Do detailed collision test
                ao1->testCollide(ao2);
            }
        }
    }
    //-------------------------------------------------------------------------
    Joint* World::createJoint(const String& name, Joint::JointType jtype,
        ApplicationObject* obj1, ApplicationObject* obj2)
    {
        Joint* ret;
        switch (jtype)
        {
        case Joint::JT_BALL:
            ret = new BallJoint(jtype, obj1, obj2);
            break;
        case Joint::JT_HINGE:
            ret = new HingeJoint(jtype, obj1, obj2);
            break;
        case Joint::JT_HINGE2:
            ret = new Hinge2Joint(jtype, obj1, obj2);
            break;
        case Joint::JT_SLIDER:
            ret = new SliderJoint(jtype, obj1, obj2);
            break;
        case Joint::JT_UNIVERSAL:
            ret = new UniversalJoint(jtype, obj1, obj2);
            break;

        }

        mJoints[name] = ret;
        return ret;
    }
    //-------------------------------------------------------------------------
    void World::setSimulationStepSize(Real step)
    {
        mSimulationStepSize = step;
    }
    //-------------------------------------------------------------------------
    Real World::getSimulationStepSize(void)
    {
        return mSimulationStepSize;
    }
    //-------------------------------------------------------------------------
    void World::simulationStep(Real timeElapsed)
    {
        /* Hmm, gives somewhat jerky results
        static Real leftOverTime = 0.0f;

		Real time = timeElapsed + leftOverTime;	
		uint steps = (uint)(time / mSimulationStepSize);
		for(uint i=0; i < steps; ++i)
        {
			_applyCollision();
            _applyDynamics(mSimulationStepSize);
        }
		leftOverTime = time - (steps * mSimulationStepSize);
        */
		_applyCollision();
        _applyDynamics(timeElapsed);



    }
    //-------------------------------------------------------------------------

}

