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

#include "OgreRefAppApplicationObject.h"
#include "OgreRefAppWorld.h"
#include "ode/collision.h"

namespace OgreRefApp
{
    //-------------------------------------------------------------------------
    ApplicationObject::ApplicationObject(const String& name)
    {
        mSceneNode = 0;
        mEntity = 0;
        mOdeBody = 0;
        mDynamicsEnabled = false;
        mCollisionEnabled = false;
		mSoftness = 0.0;
		mBounceCoeffRestitution = 0;
		mBounceVelocityThreshold = 0.1;
        mFriction = Math::POS_INFINITY;

    }
    //-------------------------------------------------------------------------
    ApplicationObject::~ApplicationObject()
    {
        SceneManager* sm = World::getSingleton().getSceneManager();
        if (mSceneNode)
        {
            sm->destroySceneNode(mSceneNode->getName());
            mSceneNode = 0;
        }

        // TODO destroy entity

        // Destroy mass
        if (mOdeBody)
        {
            delete mOdeBody;
            mOdeBody = 0;
        }

        // Destroy collision proxies
        CollisionProxyList::iterator i, iend;
        iend = mCollisionProxies.end();
        for (i = mCollisionProxies.begin(); i != iend; ++i)
        {
            delete (*i);
        }



    }
    //-------------------------------------------------------------------------
    void ApplicationObject::setPosition(const Vector3& vec)
    {
        mSceneNode->setPosition(vec);
        if (mDynamicsEnabled && mOdeBody)
            mOdeBody->setPosition(vec.x, vec.y, vec.z);
        updateCollisionProxies();
    }
    //-------------------------------------------------------------------------
    void ApplicationObject::setOrientation(const Quaternion& orientation)
    {
        mSceneNode->setOrientation(orientation);
        if (mDynamicsEnabled && mOdeBody)
        {
            dReal dquat[4] = {orientation.w, orientation.x, orientation.y, orientation.z };
            mOdeBody->setQuaternion(dquat);
        }
        updateCollisionProxies();
    }
    //-------------------------------------------------------------------------
    const Vector3& ApplicationObject::getPosition(void)
    {
        return mSceneNode->getPosition();
    }
    //-------------------------------------------------------------------------
    const Quaternion& ApplicationObject::getOrientation(void)
    {
        return mSceneNode->getOrientation();
    }
    //-------------------------------------------------------------------------
    void ApplicationObject::_updateFromDynamics()
    {
        if (mDynamicsEnabled && mOdeBody)
        {
            // Get position & rotation from ODE
            const dReal* pos = mOdeBody->getPosition();
            const dReal* quat = mOdeBody->getQuaternion();

            mSceneNode->setPosition((Real)pos[0], (Real)pos[1], (Real)pos[2]);
            mSceneNode->setOrientation((Real)quat[0], (Real)quat[1], 
                (Real)quat[2], (Real)quat[3]);

            updateCollisionProxies();
        }
    }
    //-------------------------------------------------------------------------
    bool ApplicationObject::isCollisionEnabled(void)
    {
        return mCollisionEnabled;
    }
    //-------------------------------------------------------------------------
    bool ApplicationObject::isDynamicsEnabled(void)
    {
        return mDynamicsEnabled;
    }
    //-------------------------------------------------------------------------
    void ApplicationObject::setCollisionEnabled(bool enabled)
    {
        mCollisionEnabled = enabled;
    }
    //-------------------------------------------------------------------------
    void ApplicationObject::setDynamicsEnabled(bool enabled)
    {
        if (mDynamicsEnabled != enabled)
        {
            World::getSingleton()._notifyDynamicsStateForObject(this, enabled);
        }
        mDynamicsEnabled = enabled;
        if (enabled)
        {
            // Ensure body is synced
            mOdeBody->enable();
            const Vector3& pos = getPosition();
            mOdeBody->setPosition(pos.x, pos.y, pos.z);
            const Quaternion& q = getOrientation();
            dReal dquat[4] = {q.w, q.x, q.y, q.z };
            mOdeBody->setQuaternion(dquat);
        }
        else
        {
            mOdeBody->disable();
        }
    }
    //-------------------------------------------------------------------------
    void ApplicationObject::addForce(const Vector3& direction, const Vector3& atPosition)
    {
        assert (mOdeBody && "No dynamics body set up for this object");
        mOdeBody->addRelForceAtRelPos(direction.x, direction.y, direction.z, 
            atPosition.x, atPosition.y, atPosition.z);

        // Test
        const dReal* force = mOdeBody->getForce();
        Real val = (Real)force[1];
    }
    //-------------------------------------------------------------------------
    void ApplicationObject::addForceWorldSpace(const Vector3& direction, const Vector3& atPosition)
    {
        assert (mOdeBody && "No dynamics body set up for this object");
        mOdeBody->addForceAtPos(direction.x, direction.y, direction.z, 
            atPosition.x, atPosition.y, atPosition.z);
    }
    //-------------------------------------------------------------------------
    void ApplicationObject::addTorque(const Vector3& direction)
    {
        assert (mOdeBody && "No dynamics body set up for this object");
        mOdeBody->addRelTorque(direction.x, direction.y, direction.z);
    }
    //-------------------------------------------------------------------------
    void ApplicationObject::addTorqueWorldSpace(const Vector3& direction)
    {
        assert (mOdeBody && "No dynamics body set up for this object");
        mOdeBody->addTorque(direction.x, direction.y, direction.z);
    }
    //-------------------------------------------------------------------------
    SceneNode* ApplicationObject::getSceneNode(void)
    {
        return mSceneNode;
    }
    //-------------------------------------------------------------------------
    Entity* ApplicationObject::getEntity(void)
    {
        return mEntity;
    }
    //-------------------------------------------------------------------------
    dBody* ApplicationObject::getOdeBody(void)
    {
        return mOdeBody;
    }
    //-------------------------------------------------------------------------
    void ApplicationObject::updateCollisionProxies(void)
    {
        CollisionProxyList::iterator i, iend;
        iend = mCollisionProxies.end();
        for (i = mCollisionProxies.begin(); i != iend; ++i)
        {
            // set from node
            const Vector3& pos = mSceneNode->getPosition();
            dGeom* pProxy = *i;
            pProxy->setPosition(pos.x, pos.y, pos.z);
            const Quaternion& orientation = mSceneNode->getOrientation();
            // Hmm, no setQuaternion on proxy
            // Do a really awkward conversion
            // For some bizarre reason, using ODE's dQtoR does not work
            // If you do this:
            /*
            dReal dquat[4] = {orientation.w, orientation.x, orientation.y, orientation.z };
            dMatrix3 dm3;
            memset(dm3, 0, sizeof(dMatrix3));
            dQtoR(dquat, dm3);
            pProxy->setRotation(dm3); 
            */
            // ... none of the objects are visible!! Even though dm3 comes out the same
            // as dRSetIdentity for an identity quaternion. The weird thing is that ODE
            // uses a 4x3 matrix rather than a 3x3 matrix, so the code below should not work! 
            // Plus, the same quaternion conversion works fine for bodies. Very, very odd.
            Matrix3 m3;
            dMatrix3 dm3;
            orientation.ToRotationMatrix(m3);
            dRSetIdentity(dm3);
            dm3[0] = m3[0][0];
            dm3[1] = m3[0][1];
            dm3[2] = m3[0][2];
            dm3[3] = m3[1][0];
            dm3[4] = m3[1][1];
            dm3[5] = m3[1][2];
            dm3[6] = m3[2][0];
            dm3[7] = m3[2][1];
            dm3[8] = m3[2][2];
            pProxy->setRotation(dm3); 
        }

    }
    //-------------------------------------------------------------------------
    bool ApplicationObject::testCollide(ApplicationObject* otherObj)
    {
        bool collided = false;
        dContactGeom contactGeom;
        dGeom *o1, *o2;
        CollisionProxyList::const_iterator proxy1, proxy2, proxy1end, proxy2end;
        proxy1end = mCollisionProxies.end();
        proxy2end = otherObj->mCollisionProxies.end();

        CollisionInfo collInfo;

        for (proxy1 = mCollisionProxies.begin(); proxy1 != proxy1end; ++proxy1)
        {
            for (proxy2 = otherObj->mCollisionProxies.begin(); proxy2 != proxy2end; ++proxy2)
            {
                o1 = *proxy1;
                o2 = *proxy2;
                int numc = dCollide(o1->id(), o2->id(), 0, &contactGeom, sizeof(dContactGeom));
                if (numc)
                {
                    // Create contact joints if either object is dynamics simulated
                    // If one is not, then sim will not affect it anyway, it will be fixed
                    // However if one is enabled, we need the contact joint
                    if (this->isDynamicsEnabled() || otherObj->isDynamicsEnabled())
                    {
						// We use the most agressive parameters from both objects for the contact
                        dContact contact;
						Real bounce, velThresh, softness;
						// Use the highest coeff of restitution from both objects
						bounce = std::max(this->getBounceRestitutionValue(), 
								otherObj->getBounceRestitutionValue());
						// Use the lowest velocity threshold from both objects
						velThresh = std::min(this->getBounceVelocityThreshold(),
								otherObj->getBounceVelocityThreshold());
						// Set flags
						contact.surface.mode = dContactBounce | dContactApprox1;
						contact.surface.bounce = bounce;
						contact.surface.bounce_vel = velThresh;

						softness = this->getSoftness() + otherObj->getSoftness();
						if (softness > 0)
						{
                        	contact.surface.mode |= dContactSoftCFM;
							contact.surface.soft_cfm = softness;
						}
						
                        // Set friction to min of 2 objects
                        // Note that ODE dInfinity == Math::POS_INFINITY
                        contact.surface.mu = std::min(this->getFriction(), otherObj->getFriction());
                        contact.surface.mu2 = 0;
                        contact.geom = contactGeom;
                        dContactJoint contactJoint(
                            World::getSingleton().getOdeWorld()->id(), 
                            World::getSingleton().getOdeContactJointGroup()->id(), 
                            &contact);

                        // Get ODE bodies
                        // May be null, if so use 0 (immovable) body ids
                        dBody *b1, *b2;
                        dBodyID bid1, bid2;
                        bid1 = bid2 = 0;
                        b1 = this->getOdeBody();
                        b2 = otherObj->getOdeBody();
                        if (b1) bid1 = b1->id();
                        if (b2) bid2 = b2->id();
                        contactJoint.attach(bid1, bid2);
                    }

                    // Tell both objects about the collision
                    collInfo.position.x = contactGeom.pos[0];
                    collInfo.position.y = contactGeom.pos[1];
                    collInfo.position.z = contactGeom.pos[2];
                    collInfo.normal.x = contactGeom.normal[0];
                    collInfo.normal.y = contactGeom.normal[1];
                    collInfo.normal.z = contactGeom.normal[2];
                    collInfo.penetrationDepth = contactGeom.depth;
                    this->_notifyCollided(otherObj, collInfo);
                    otherObj->_notifyCollided(this, collInfo);


                    // set return 
                    collided = true;
                }
            }
        }
        return collided;

    }
    //-------------------------------------------------------------------------
    void ApplicationObject::_notifyCollided(ApplicationObject* otherObj, 
        const ApplicationObject::CollisionInfo& info)
    {
        // NB contacts for physics are not created here but in testCollide
    }
    //-------------------------------------------------------------------------
	void ApplicationObject::setBounceParameters(Real restitutionValue, 
			Real velocityThreshold)
	{
		mBounceCoeffRestitution = restitutionValue;
		mBounceVelocityThreshold = velocityThreshold;
	}
    //-------------------------------------------------------------------------
	Real ApplicationObject::getBounceRestitutionValue(void)
	{
		return mBounceCoeffRestitution;
	}
    //-------------------------------------------------------------------------
	Real ApplicationObject::getBounceVelocityThreshold(void)
	{
		return mBounceVelocityThreshold;
	}
    //-------------------------------------------------------------------------
	void ApplicationObject::setSoftness(Real softness)
	{
		mSoftness = softness;
	}
    //-------------------------------------------------------------------------
	Real ApplicationObject::getSoftness(void)
	{
		return mSoftness;
	}
    //-------------------------------------------------------------------------
    void ApplicationObject::setFriction(Real friction)
    {
        mFriction = friction;
    }
    //-------------------------------------------------------------------------
    Real ApplicationObject::getFriction(void)
    {
        return mFriction;
    }



}

