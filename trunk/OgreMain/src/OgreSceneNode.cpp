/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreSceneNode.h"

#include "OgreException.h"
#include "OgreEntity.h"
#include "OgreCamera.h"
#include "OgreMath.h"
#include "OgreSceneManager.h"
#include "OgreMovableObject.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SceneNode::SceneNode(SceneManager* creator)
    {
        mParent = 0;
        mOrientation = mDerivedOrientation = Quaternion::IDENTITY;
        mPosition = mDerivedPosition = Vector3::ZERO;
        mCreator = creator;
        mScale = Vector3(1.0, 1.0, 1.0);
        mInheritScale = true;
        mDerivedOutOfDate = true;

    }
    //-----------------------------------------------------------------------
    SceneNode::~SceneNode()
    {
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::getParent(void)
    {
        return mParent;
    }
    //-----------------------------------------------------------------------
    void SceneNode::setParent(SceneNode* parent)
    {
        mParent = parent;
    }

    //-----------------------------------------------------------------------
    Matrix4 SceneNode::_getFullTransform(void)
    {
        // Use derived values 
        // Note that translation is always after rotation.
        // Parent scaling is already applied to derived position
        // Own scale is applied after rotation
        Matrix3 rot_scale;
        _getDerivedOrientation().ToRotationMatrix(rot_scale);
        // Apply scale
        Vector3 scale = _getDerivedScale();
        rot_scale[0][0] *= scale.x;
        rot_scale[1][1] *= scale.y;
        rot_scale[2][2] *= scale.z;

        Matrix4 result = Matrix4::IDENTITY;
        result = rot_scale;
        result.setTrans(_getDerivedPosition());

        return result;
    }
    //-----------------------------------------------------------------------
    void SceneNode::_update(Camera* cam, bool updateChildren)
    {
        // Update transforms
        _updateFromParent();

        // Tell attached objects about camera position (incase any extra processing they want to do)
        std::vector<MovableObject*>::iterator i;
        for (i = mObjects.begin(); i != mObjects.end(); ++i)
        {
            (*i)->_notifyCurrentCamera(cam);
        }

        if (updateChildren)
        {
            std::vector<SceneNode*>::iterator i;
            for (i = mChildren.begin(); i != mChildren.end(); ++i)
            {
                (*i)->_update(cam, updateChildren);
            }
        }

        _updateBounds();

    }

    //-----------------------------------------------------------------------
    void SceneNode::_updateFromParent(void)
    {
        if (mParent)
        {
            // Combine orientation with that of parent
            Quaternion mParentQ = mParent->_getDerivedOrientation();
            mDerivedOrientation = mParentQ * mOrientation;

            // Change position vector based on parent's orientation
            mDerivedPosition = mParentQ * mPosition;

            // Update scale
            if (mInheritScale)
            {
                // Scale own position by parent scale
                Vector3 parentScale = mParent->_getDerivedScale();
                mDerivedPosition = mDerivedPosition * parentScale;

                // Set own scale, NB just combine as equivalent axes, no shearing
                mDerivedScale = mScale * parentScale;

            }
            else
            {
                // No inheritence
                mDerivedScale = mScale;
            }

            // Add altered position vector to parents
            mDerivedPosition += mParent->_getDerivedPosition();
        }
        else
        {
            // Root node, no parent
            mDerivedOrientation = mOrientation;
            mDerivedPosition = mPosition;
            mDerivedScale = mScale;
        }

        mDerivedOutOfDate = false;


    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::createChild(const Vector3& translate, const Quaternion& rotate)
    {
        SceneNode* newNode = mCreator->createSceneNode();
        newNode->translate(translate);
        newNode->rotate(rotate);
        this->addChild(newNode);


        return newNode;

    }
    //-----------------------------------------------------------------------
    void SceneNode::addChild(SceneNode* child)
    {
        mChildren.push_back(child);
        child->setParent(this);
        // Make sure child is up to date first, incase child asks for derived transform
        child->_updateFromParent();


    }
    //-----------------------------------------------------------------------
    unsigned short SceneNode::numChildren(void)
    {
        return mChildren.size();
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::getChild(unsigned short index)
    {
        if (index < mChildren.size())
        {
            std::vector<SceneNode*>::iterator i = mChildren.begin();
            return i[index];
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Child index out of bounds.", "SceneNode::getChild");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::removeChild(unsigned short index)
    {
        SceneNode* ret;
        if (index < mChildren.size())
        {
            std::vector<SceneNode*>::iterator i = mChildren.begin() + index;
            ret = *i;
            mChildren.erase(i);
            return ret;
            /* XXX temas: This is the original
            std::vector<SceneNode*>::iterator i = mChildren.begin();
            ret =  i[index];
            mChildren.erase(&i[index]);
            return ret;
            */
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Child index out of bounds.", "SceneNode::getChild");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    void SceneNode::attachObject(MovableObject* obj)
    {
        mObjects.push_back(obj);
        obj->_notifyAttached(this);
    }
    //-----------------------------------------------------------------------
    unsigned short SceneNode::numAttachedObjects(void)
    {
        return mObjects.size();
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::getAttachedObject(unsigned short index)
    {
        if (index < mObjects.size())
        {
            std::vector<MovableObject*>::iterator i = mObjects.begin();
            return i[index];
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Object index out of bounds.", "SceneNode::getAttchedEntity");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::detachObject(unsigned short index)
    {
        MovableObject* ret;
        if (index < mObjects.size())
        {

            std::vector<MovableObject*>::iterator i = mObjects.begin() + index;
            ret = *i;
            mObjects.erase(i);
            ret->_notifyAttached(0);
            return ret;

            /* XXX temas: Original code
            std::vector<MovableObject*>::iterator i = mObjects.begin();
            ret = i[index];
            mObjects.erase(&i[index]);
            ret->_notifyAttached(0);
            return ret;
            */
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Object index out of bounds.", "SceneNode::getAttchedEntity");
        }
        return 0;

    }
    //-----------------------------------------------------------------------
    void SceneNode::attachCamera(Camera* cam)
    {
        mCameras.push_back(cam);
        cam->_notifyAttached(this);
    }
    //-----------------------------------------------------------------------
    unsigned short SceneNode::numAttachedCameras(void)
    {
        return mCameras.size();
    }
    //-----------------------------------------------------------------------
    Camera* SceneNode::getAttachedCamera(unsigned short index)
    {
        if (index < mCameras.size())
        {
            std::vector<Camera*>::iterator i = mCameras.begin();
            return i[index];
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Camera index out of bounds.", "SceneNode::getAttachedCamera");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    Camera* SceneNode::detachCamera(unsigned short index)
    {
        Camera* ret;
        if (index < mCameras.size())
        {
            /* XXX temas: original code
            std::vector<Camera*>::iterator i = mCameras.begin();
            ret = i[index];
            ret->_notifyAttached(0);
            mCameras.erase(&i[index]);
            return ret;
            */
            std::vector<Camera*>::iterator i = mCameras.begin() + index;
            ret = *i;
            ret->_notifyAttached(0);
            mCameras.erase(i);
            return ret;
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Camera index out of bounds.", "SceneNode::getAttchedCamera");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    void SceneNode::attachLight(Light* ent)
    {
        mLights.push_back(ent);
    }
    //-----------------------------------------------------------------------
    unsigned short SceneNode::numAttachedLights(void)
    {
        return mLights.size();
    }
    //-----------------------------------------------------------------------
    Light* SceneNode::getAttachedLight(unsigned short index)
    {
        if (index < mLights.size())
        {
            std::vector<Light*>::iterator i = mLights.begin();
            return i[index];
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Light index out of bounds.", "SceneNode::getAttchedLight");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    Light* SceneNode::detachLight(unsigned short index)
    {
        Light* ret;
        if (index < mLights.size())
        {
            /* XXX temas: original code
            std::vector<Light*>::iterator i = mLights.begin();
            ret = i[index];
            mLights.erase(&i[index]);
            return ret;
            */
            std::vector<Light*>::iterator i = mLights.begin() + index;
            ret = *i;
            mLights.erase(i);
            return ret;
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Light index out of bounds.", "SceneNode::getAttchedLight");
        }
        return 0;

    }
    //-----------------------------------------------------------------------
    Quaternion SceneNode::getOrientation()
    {
        return mOrientation;
    }

    //-----------------------------------------------------------------------
    void SceneNode::setOrientation(Quaternion q)
    {
        mOrientation = q;
        mDerivedOutOfDate = true;
    }

    //-----------------------------------------------------------------------
    void SceneNode::resetOrientation(void)
    {
        mOrientation = Quaternion::IDENTITY;
        mDerivedOutOfDate = true;
    }

    //-----------------------------------------------------------------------
    void SceneNode::setPosition(const Vector3& pos)
    {
        mPosition = pos;
        mDerivedOutOfDate = true;
    }


    //-----------------------------------------------------------------------
    void SceneNode::setPosition(Real x, Real y, Real z)
    {
        Vector3 v(x,y,z);
        setPosition(v);
    }

    //-----------------------------------------------------------------------
    Vector3 SceneNode::getPosition(void)
    {
        return mPosition;
    }
    //-----------------------------------------------------------------------
    Matrix3 SceneNode::getLocalAxes(void)
    {
        Vector3 axisX = Vector3::UNIT_X;
        Vector3 axisY = Vector3::UNIT_Y;
        Vector3 axisZ = Vector3::UNIT_Z;

        axisX = mOrientation * axisX;
        axisY = mOrientation * axisY;
        axisZ = mOrientation * axisZ;

        return Matrix3(axisX.x, axisY.x, axisZ.x,
                       axisX.y, axisY.y, axisZ.y,
                       axisX.z, axisY.z, axisZ.z);


    }

    //-----------------------------------------------------------------------
    void SceneNode::translate(const Vector3& d)
    {
        mPosition += d;
        mDerivedOutOfDate = true;

    }
    //-----------------------------------------------------------------------
    void SceneNode::translate(Real x, Real y, Real z)
    {
        Vector3 v(x,y,z);
        translate(v);
    }
    //-----------------------------------------------------------------------
    void SceneNode::translate(const Matrix3& axes, const Vector3& move)
    {
        Vector3 derived = axes * move;
        translate(derived);
    }
    //-----------------------------------------------------------------------
    void SceneNode::translate(const Matrix3& axes, Real x, Real y, Real z)
    {
        Vector3 d(x,y,z);
        translate(axes,d);
    }
    //-----------------------------------------------------------------------
    void SceneNode::roll(Real degrees)
    {
        // Rotate around local Z axis
        Vector3 zAxis = mOrientation * Vector3::UNIT_Z;
        rotate(zAxis, degrees);
    }
    //-----------------------------------------------------------------------
    void SceneNode::pitch(Real degrees)
    {
        // Rotate around local X axis
        Vector3 xAxis = mOrientation * Vector3::UNIT_X;
        rotate(xAxis, degrees);
    }
    //-----------------------------------------------------------------------
    void SceneNode::yaw(Real degrees)
    {
        // Rotate around local Y axis
        Vector3 yAxis = mOrientation * Vector3::UNIT_Y;
        rotate(yAxis, degrees);

    }
    //-----------------------------------------------------------------------
    void SceneNode::rotate(const Vector3& axis, Real degrees)
    {
        Quaternion q;
        q.FromAngleAxis(Math::getSingleton().DegreesToRadians(degrees),axis);
        rotate(q);
    }

    //-----------------------------------------------------------------------
    void SceneNode::rotate(const Quaternion& q)
    {
        // Note the order of the mult, i.e. q comes after
        mOrientation = q * mOrientation;
        mDerivedOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    Quaternion SceneNode::_getDerivedOrientation(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedOrientation;
    }
    //-----------------------------------------------------------------------
    Vector3 SceneNode::_getDerivedPosition(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedPosition;
    }
    //-----------------------------------------------------------------------
    Vector3 SceneNode::_getDerivedScale(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedScale;
    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAllChildren(void)
    {
        mChildren.clear();
    }
    //-----------------------------------------------------------------------
    void SceneNode::detachAllObjects(void)
    {
        mObjects.clear();
    }
    //-----------------------------------------------------------------------
    void SceneNode::detachAllLights(void)
    {
        mLights.clear();
    }
    //-----------------------------------------------------------------------
    void SceneNode::detachAllCameras(void)
    {
        mCameras.clear();
    }
    //-----------------------------------------------------------------------
    void SceneNode::_updateBounds(void)
    {
        // Reset bounds first
        mWorldAABB.setNull();

        // Update bounds from own attached objects
        std::vector<MovableObject*>::iterator i;
        AxisAlignedBox bx;
        for (i = mObjects.begin(); i != mObjects.end(); ++i)
        {
            // Get local bounds of object
            bx = (*i)->getBoundingBox();
            // Transform by aggregated transform
            bx.transform(_getFullTransform());

            mWorldAABB.merge(bx);
        }

        // Merge with children
        std::vector<SceneNode*>::iterator child;
        for (child = mChildren.begin(); child != mChildren.end(); ++child)
        {
            mWorldAABB.merge((*child)->mWorldAABB);
        }

    }
    //-----------------------------------------------------------------------
    void SceneNode::_findVisibleObjects(Camera* cam, RenderQueue* queue, bool includeChildren)
    {
        // Check self visible
        if (!cam->isVisible(mWorldAABB))
            return;

        // Add all entities
        std::vector<MovableObject*>::iterator iobj;
        std::vector<MovableObject*>::iterator iobjend = mObjects.end();
        for (iobj = mObjects.begin(); iobj != iobjend; ++iobj)
        {
            (*iobj)->_updateRenderQueue(queue);
        }

        if (includeChildren)
        {
            std::vector<SceneNode*>::iterator child, childend;
            childend = mChildren.end();
            for (child = mChildren.begin(); child != childend; ++child)
            {
                (*child)->_findVisibleObjects(cam, queue, includeChildren);
            }
        }

    }
    //-----------------------------------------------------------------------
    void SceneNode::setScale(const Vector3& scale)
    {
        mScale = scale;
        mDerivedOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    void SceneNode::setScale(Real x, Real y, Real z)
    {
        mScale.x = x;
        mScale.y = y;
        mScale.z = z;
        mDerivedOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    Vector3 SceneNode::getScale(void)
    {
        return mScale;
    }
    //-----------------------------------------------------------------------
    void SceneNode::setInheritScale(bool inherit)
    {
        mInheritScale = inherit;
        mDerivedOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    bool SceneNode::getInheritScale(void)
    {
        return mInheritScale;
    }
    //-----------------------------------------------------------------------
    void SceneNode::scale(const Vector3& scale)
    {
        mScale = mScale * scale;
        mDerivedOutOfDate = true;

    }
    //-----------------------------------------------------------------------
    void SceneNode::scale(Real x, Real y, Real z)
    {
        mScale.x *= x;
        mScale.y *= y;
        mScale.z *= z;
        mDerivedOutOfDate = true;

    }
    //-----------------------------------------------------------------------




}
