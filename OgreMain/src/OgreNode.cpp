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
#include "OgreNode.h"

#include "OgreException.h"
#include "OgreEntity.h"
#include "OgreCamera.h"
#include "OgreMath.h"
#include "OgreSceneManager.h"
#include "OgreMovableObject.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    Node::Node()
    {
        mParent = 0;
        mOrientation = mDerivedOrientation = Quaternion::IDENTITY;
        mPosition = mDerivedPosition = Vector3::ZERO;
        mScale = Vector3(1.0, 1.0, 1.0);
        mInheritScale = true;
        mDerivedOutOfDate = true;

    }
    //-----------------------------------------------------------------------
    Node::~Node()
    {
    }
    //-----------------------------------------------------------------------
    Node* Node::getParent(void)
    {
        return mParent;
    }
    //-----------------------------------------------------------------------
    void Node::setParent(Node* parent)
    {
        mParent = parent;
    }

    //-----------------------------------------------------------------------
    Matrix4 Node::_getFullTransform(void)
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
    void Node::_update(bool updateChildren)
    {
        // Update transforms
        _updateFromParent();

        if (updateChildren)
        {
            std::vector<Node*>::iterator i;
            for (i = mChildren.begin(); i != mChildren.end(); ++i)
            {
                (*i)->_update(updateChildren);
            }
        }

    }

    //-----------------------------------------------------------------------
    void Node::_updateFromParent(void)
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
    Node* Node::createChild(const Vector3& translate, const Quaternion& rotate)
    {
        Node* newNode = createChildImpl();
        newNode->translate(translate);
        newNode->rotate(rotate);
        this->addChild(newNode);


        return newNode;

    }
    //-----------------------------------------------------------------------
    void Node::addChild(Node* child)
    {
        mChildren.push_back(child);
        child->setParent(this);
        // Make sure child is up to date first, incase child asks for derived transform
        child->_updateFromParent();


    }
    //-----------------------------------------------------------------------
    unsigned short Node::numChildren(void)
    {
        return mChildren.size();
    }
    //-----------------------------------------------------------------------
    Node* Node::getChild(unsigned short index)
    {
        if (index < mChildren.size())
        {
            std::vector<Node*>::iterator i = mChildren.begin();
            return i[index];
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Child index out of bounds.", "Node::getChild");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    Node* Node::removeChild(unsigned short index)
    {
        Node* ret;
        if (index < mChildren.size())
        {
            std::vector<Node*>::iterator i = mChildren.begin() + index;
            ret = *i;
            mChildren.erase(i);
            return ret;
            /* XXX temas: This is the original
            std::vector<Node*>::iterator i = mChildren.begin();
            ret =  i[index];
            mChildren.erase(&i[index]);
            return ret;
            */
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Child index out of bounds.", "Node::getChild");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    Quaternion Node::getOrientation()
    {
        return mOrientation;
    }

    //-----------------------------------------------------------------------
    void Node::setOrientation(Quaternion q)
    {
        mOrientation = q;
        mDerivedOutOfDate = true;
    }

    //-----------------------------------------------------------------------
    void Node::resetOrientation(void)
    {
        mOrientation = Quaternion::IDENTITY;
        mDerivedOutOfDate = true;
    }

    //-----------------------------------------------------------------------
    void Node::setPosition(const Vector3& pos)
    {
        mPosition = pos;
        mDerivedOutOfDate = true;
    }


    //-----------------------------------------------------------------------
    void Node::setPosition(Real x, Real y, Real z)
    {
        Vector3 v(x,y,z);
        setPosition(v);
    }

    //-----------------------------------------------------------------------
    Vector3 Node::getPosition(void)
    {
        return mPosition;
    }
    //-----------------------------------------------------------------------
    Matrix3 Node::getLocalAxes(void)
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
    void Node::translate(const Vector3& d)
    {
        mPosition += d;
        mDerivedOutOfDate = true;

    }
    //-----------------------------------------------------------------------
    void Node::translate(Real x, Real y, Real z)
    {
        Vector3 v(x,y,z);
        translate(v);
    }
    //-----------------------------------------------------------------------
    void Node::translate(const Matrix3& axes, const Vector3& move)
    {
        Vector3 derived = axes * move;
        translate(derived);
    }
    //-----------------------------------------------------------------------
    void Node::translate(const Matrix3& axes, Real x, Real y, Real z)
    {
        Vector3 d(x,y,z);
        translate(axes,d);
    }
    //-----------------------------------------------------------------------
    void Node::roll(Real degrees)
    {
        // Rotate around local Z axis
        Vector3 zAxis = mOrientation * Vector3::UNIT_Z;
        rotate(zAxis, degrees);
    }
    //-----------------------------------------------------------------------
    void Node::pitch(Real degrees)
    {
        // Rotate around local X axis
        Vector3 xAxis = mOrientation * Vector3::UNIT_X;
        rotate(xAxis, degrees);
    }
    //-----------------------------------------------------------------------
    void Node::yaw(Real degrees)
    {
        // Rotate around local Y axis
        Vector3 yAxis = mOrientation * Vector3::UNIT_Y;
        rotate(yAxis, degrees);

    }
    //-----------------------------------------------------------------------
    void Node::rotate(const Vector3& axis, Real degrees)
    {
        Quaternion q;
        q.FromAngleAxis(Math::getSingleton().DegreesToRadians(degrees),axis);
        rotate(q);
    }

    //-----------------------------------------------------------------------
    void Node::rotate(const Quaternion& q)
    {
        // Note the order of the mult, i.e. q comes after
        mOrientation = q * mOrientation;
        mDerivedOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    Quaternion Node::_getDerivedOrientation(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedOrientation;
    }
    //-----------------------------------------------------------------------
    Vector3 Node::_getDerivedPosition(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedPosition;
    }
    //-----------------------------------------------------------------------
    Vector3 Node::_getDerivedScale(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedScale;
    }
    //-----------------------------------------------------------------------
    void Node::removeAllChildren(void)
    {
        mChildren.clear();
    }
    //-----------------------------------------------------------------------
    void Node::setScale(const Vector3& scale)
    {
        mScale = scale;
        mDerivedOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    void Node::setScale(Real x, Real y, Real z)
    {
        mScale.x = x;
        mScale.y = y;
        mScale.z = z;
        mDerivedOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    Vector3 Node::getScale(void)
    {
        return mScale;
    }
    //-----------------------------------------------------------------------
    void Node::setInheritScale(bool inherit)
    {
        mInheritScale = inherit;
        mDerivedOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    bool Node::getInheritScale(void)
    {
        return mInheritScale;
    }
    //-----------------------------------------------------------------------
    void Node::scale(const Vector3& scale)
    {
        mScale = mScale * scale;
        mDerivedOutOfDate = true;

    }
    //-----------------------------------------------------------------------
    void Node::scale(Real x, Real y, Real z)
    {
        mScale.x *= x;
        mScale.y *= y;
        mScale.z *= z;
        mDerivedOutOfDate = true;

    }
    //-----------------------------------------------------------------------




}
