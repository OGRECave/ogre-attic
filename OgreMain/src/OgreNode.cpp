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

// Dependencies on render-related types due to ability to render node
#include "OgreMaterialManager.h"
#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"

namespace Ogre {
    
    unsigned long Node::msNextGeneratedNameExt = 1;
    //-----------------------------------------------------------------------
    Node::Node()
    {
        mParent = 0;
        mOrientation = mInitialOrientation = mDerivedOrientation = Quaternion::IDENTITY;
        mPosition = mInitialPosition = mDerivedPosition = Vector3::ZERO;
        mScale = mInitialScale = mDerivedScale = Vector3::UNIT_SCALE;
        mInheritScale = true;
        mDerivedOutOfDate = true;

        // Generate a name
        static char temp[64];
        sprintf(temp, "Unnamed_%lu", msNextGeneratedNameExt++);
        mName = temp;
        mAccumAnimWeight = 0.0f;


    }
    //-----------------------------------------------------------------------
    Node::Node(const String& name)
    {
        mName = name;
        mParent = 0;
        mOrientation = mInitialOrientation = mDerivedOrientation = Quaternion::IDENTITY;
        mPosition = mInitialPosition = mDerivedPosition = Vector3::ZERO;
        mScale = mInitialScale = mDerivedScale = Vector3::UNIT_SCALE;
        mInheritScale = true;
        mDerivedOutOfDate = true;
        mAccumAnimWeight = 0.0f;
    }

    //-----------------------------------------------------------------------
    Node::~Node()
    {
    }    

    //-----------------------------------------------------------------------
    Node* Node::getParent(void) const
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
        Matrix4 result;
        makeTransform( 
            _getDerivedPosition(), _getDerivedScale(), 
            _getDerivedOrientation(), result);
        return result;
    }
    //-----------------------------------------------------------------------
    void Node::_update(bool updateChildren)
    {
        // Update transforms
        _updateFromParent();

        if (updateChildren)
        {
            ChildNodeMap::iterator i;
            for (i = mChildren.begin(); i != mChildren.end(); ++i)
            {
                i->second->_update(updateChildren);
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
    Node* Node::createChild(const String& name, const Vector3& translate, const Quaternion& rotate)
    {
        Node* newNode = createChildImpl(name);
        newNode->translate(translate);
        newNode->rotate(rotate);
        this->addChild(newNode);

        return newNode;
    }
    //-----------------------------------------------------------------------
    void Node::addChild(Node* child)
    {
        mChildren.insert(ChildNodeMap::value_type(child->getName(), child));
        child->setParent(this);
        // Make sure child is up to date first, incase child asks for derived transform
        child->_updateFromParent();
    }
    //-----------------------------------------------------------------------
    unsigned short Node::numChildren(void) const
    {
        return static_cast< unsigned short >( mChildren.size() );
    }
    //-----------------------------------------------------------------------
    Node* Node::getChild(unsigned short index) const
    {
        if( index < mChildren.size() )
        {
            ChildNodeMap::const_iterator i = mChildren.begin();
            while (index--) ++i;
            return i->second;
        }
        else
            return NULL;
    }
    //-----------------------------------------------------------------------
    Node* Node::removeChild(unsigned short index)
    {
        Node* ret;
        if (index < mChildren.size())
        {
            ChildNodeMap::iterator i = mChildren.begin();
            while (index--) ++i;
            ret = i->second;
            mChildren.erase(i);
            return ret;            
        }
        else
        {
            Except(
                Exception::ERR_INVALIDPARAMS, 
                "Child index out of bounds.", 
                "Node::getChild" );
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    const Quaternion& Node::getOrientation() const
    {
        return mOrientation;
    }

    //-----------------------------------------------------------------------
    void Node::setOrientation( const Quaternion & q )
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
    const Vector3 & Node::getPosition(void) const
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
        rotate(Vector3::UNIT_Z, degrees);
    }
    //-----------------------------------------------------------------------
    void Node::pitch(Real degrees)
    {
        rotate(Vector3::UNIT_X, degrees);
    }
    //-----------------------------------------------------------------------
    void Node::yaw(Real degrees)
    {
        rotate(Vector3::UNIT_Y, degrees);

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
        mOrientation = mOrientation * q;
        mDerivedOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    const Quaternion & Node::_getDerivedOrientation(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedOrientation;
    }
    //-----------------------------------------------------------------------
    const Vector3 & Node::_getDerivedPosition(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedPosition;
    }
    //-----------------------------------------------------------------------
    const Vector3 & Node::_getDerivedScale(void)
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
    const Vector3 & Node::getScale(void) const
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
    bool Node::getInheritScale(void) const
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
    void Node::makeTransform(const Vector3& position, const Vector3& scale, const Quaternion& orientation, 
        Matrix4& destMatrix)
    {
        destMatrix = Matrix4::IDENTITY;
        // Ordering:
        //    1. Scale
        //    2. Rotate
        //    3. Translate

        // Parent scaling is already applied to derived position
        // Own scale is applied before rotation
        Matrix3 rot3x3, scale3x3;
        orientation.ToRotationMatrix(rot3x3);
        scale3x3 = Matrix3::ZERO;
        scale3x3[0][0] = scale.x;
        scale3x3[1][1] = scale.y;
        scale3x3[2][2] = scale.z;

        destMatrix = rot3x3 * scale3x3;
        destMatrix.setTrans(position);
    }
    //-----------------------------------------------------------------------
    void Node::makeInverseTransform(const Vector3& position, const Vector3& scale, const Quaternion& orientation, 
        Matrix4& destMatrix)
    {
        destMatrix = Matrix4::IDENTITY;

        // Invert the parameters
        Vector3 invTranslate = -position;
        Vector3 invScale;
        invScale.x = 1 / scale.x;
        invScale.y = 1 / scale.x;
        invScale.z = 1 / scale.x;

        Quaternion invRot = orientation.Inverse();
        
        // Because we're inverting, order is translation, rotation, scale
        // So make translation relative to scale & rotation
        invTranslate.x *= invScale.x; // scale
        invTranslate.y *= invScale.y; // scale
        invTranslate.z *= invScale.z; // scale
        invTranslate = invRot * invTranslate; // rotate

        // Next, make a 3x3 rotation matrix and apply inverse scale
        Matrix3 rot3x3, scale3x3;
        invRot.ToRotationMatrix(rot3x3);
        scale3x3 = Matrix3::ZERO;
        scale3x3[0][0] = invScale.x;
        scale3x3[1][1] = invScale.y;
        scale3x3[2][2] = invScale.z;

        // Set up final matrix with scale & rotation
        destMatrix = scale3x3 * rot3x3;

        destMatrix.setTrans(invTranslate);
    }
    //-----------------------------------------------------------------------
    const String& Node::getName(void) const
    {
        return mName;
    }
    //-----------------------------------------------------------------------
    Material* Node::getMaterial(void) const
    {
        static Material* pMaterial = 0;

        if (!pMaterial)
        {
            pMaterial = (Material*)MaterialManager::getSingleton().getByName("Core/NodeMaterial");
            pMaterial->load();
        }
        return pMaterial;

    }
    //-----------------------------------------------------------------------
    void Node::getRenderOperation(RenderOperation& rend)
    {
        static SubMesh* pSubMesh = 0;
        if (!pSubMesh)
        {
            Mesh *pMesh = MeshManager::getSingleton().load("axes.mesh");
            pSubMesh = pMesh->getSubMesh(0);
        }
        pSubMesh->_getRenderOperation(rend);
    }
    //-----------------------------------------------------------------------
    void Node::getWorldTransforms(Matrix4* xform)
    {
        // Assumes up to date
        *xform = this->_getFullTransform();
    }
    //-----------------------------------------------------------------------
    void Node::setInitialState(void)
    {
        mInitialPosition = mPosition;
        mInitialOrientation = mOrientation;
        mInitialScale = mScale;
    }
    //-----------------------------------------------------------------------
    void Node::resetToInitialState(void)
    {
        mPosition = mInitialPosition;
        mOrientation = mInitialOrientation;
        mScale = mInitialScale;

        // Reset weights
        mAccumAnimWeight = 0.0f;
        mTransFromInitial = Vector3::ZERO;
        mRotFromInitial = Quaternion::IDENTITY;

        mDerivedOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    const Vector3& Node::getInitialPosition(void) const
    {
        return mInitialPosition;
    }
    //-----------------------------------------------------------------------
    const Quaternion& Node::getInitialOrientation(void) const
    {
        return mInitialOrientation;

    }
    //-----------------------------------------------------------------------
    const Vector3& Node::getInitialScale(void) const
    {
        return mInitialScale;
    }
    //-----------------------------------------------------------------------
    Node* Node::getChild(const String& name) const
    {
        ChildNodeMap::const_iterator i = mChildren.find(name);

        if (i == mChildren.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Child node named " + name +
                " does not exist.", "Node::getChild");
        }
        return i->second;

    }
    //-----------------------------------------------------------------------
    Node* Node::removeChild(const String& name)
    {
        ChildNodeMap::iterator i = mChildren.find(name);

        if (i == mChildren.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Child node named " + name +
                " does not exist.", "Node::removeChild");
        }

        Node* ret = i->second;
        mChildren.erase(i);

        return ret;


    }
    //-----------------------------------------------------------------------
    Node::ChildNodeIterator Node::getChildIterator(void)
    {
        return ChildNodeIterator(mChildren.begin(), mChildren.end());
    }
    //-----------------------------------------------------------------------
    void Node::_weightedTransform(Real weight, const Vector3& translate, 
       const Quaternion& rotate)
    {
        // If no previous transforms, we can just apply
        if (mAccumAnimWeight == 0.0f)
        {
            mRotFromInitial = rotate;
            mTransFromInitial = translate;
            mAccumAnimWeight = weight;
        }
        else
        {
            // Blend with existing
            Real factor = weight / (mAccumAnimWeight + weight);
            mTransFromInitial += (translate - mTransFromInitial) * factor;
            mRotFromInitial = 
                Quaternion::Slerp(factor, mRotFromInitial, rotate);
            mAccumAnimWeight += weight;

        }

        // Update final based on bind position + offsets
        mOrientation = mInitialOrientation * mRotFromInitial;
        mPosition = mInitialPosition + mTransFromInitial;
        mDerivedOutOfDate = true;

    }
}

