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

        // Destroy collision proxy


    }
    //-------------------------------------------------------------------------
    void ApplicationObject::setPosition(const Vector3& vec)
    {
        mSceneNode->setPosition(vec);
        mOdeBody->setPosition(vec.x, vec.y, vec.z);
    }
    //-------------------------------------------------------------------------
    void ApplicationObject::setOrientation(const Quaternion& orientation)
    {
        mSceneNode->setOrientation(orientation);
        dReal dquat[4] = {orientation.w, orientation.x, orientation.y, orientation.z };
        mOdeBody->setQuaternion(dquat);
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
            mOdeBody->enable();
        else
            mOdeBody->disable();
    }




}

