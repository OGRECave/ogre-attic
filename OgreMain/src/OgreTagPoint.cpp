/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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

#include "OgreTagPoint.h"
#include "OgreMatrix4.h"
#include "OgreMatrix3.h"
#include "OgreEntity.h"
#include "OgreSceneNode.h"
#include "OgreSkeleton.h"
#include "OgreQuaternion.h"


namespace Ogre {

    //-----------------------------------------------------------------------------
    TagPoint::TagPoint(unsigned short handle, Skeleton* creator): Bone(handle, creator)
    {
        mParentEntity = 0; 
        mChildObject = 0; 
    }
    //-----------------------------------------------------------------------------
    TagPoint::~TagPoint()
    {
    }
    //-----------------------------------------------------------------------------
    Entity *TagPoint::getParentEntity(void)
    {
        return mParentEntity;
    }
    //-----------------------------------------------------------------------------
    void TagPoint::setParentEntity(Entity *pEntity)
    {
        mParentEntity = pEntity;
    }
    //-----------------------------------------------------------------------------
    void TagPoint::setChildObject(MovableObject *pObject)
    {
        mChildObject = pObject;
    }
    //-----------------------------------------------------------------------------
    Matrix4 TagPoint::_getFullTransform(void)
    {
        //return getParentEntityTransform() * Node::_getFullTransform();
        // Now that we're overriding updateFromParent(), this doesn't need to be different
        return Node::_getFullTransform();
    }
    //-----------------------------------------------------------------------------
    Matrix4 TagPoint::_getFullLocalTransform(void)
    {
        return mFullLocalTransform;
    }
    //-----------------------------------------------------------------------------
    Matrix4 TagPoint::getParentEntityTransform()
    {

        return mParentEntity->_getParentNodeFullTransform();
    }
    //-----------------------------------------------------------------------------
    void TagPoint::_update(bool updateChildren, bool parentHasChanged)
    {
        Node::_update(updateChildren, parentHasChanged);
    }
    //-----------------------------------------------------------------------------
    void TagPoint::needUpdate()
    {
        // We need to tell parent entities node
        if (mParentEntity)
        {
            Node* n = mParentEntity->getParentNode();
            if (n)
            {
                n->needUpdate();
            }

        }

    }
    //-----------------------------------------------------------------------------
    void TagPoint::_updateFromParent(void) const
    {
        // Call superclass
        Bone::_updateFromParent();

        // Save transform for local skeleton
        makeTransform(mDerivedPosition, mDerivedScale, mDerivedOrientation, mFullLocalTransform);

        // Include Entity transform
        if (mParentEntity)
        {
            Node* entityParentNode = mParentEntity->getParentNode();
            if (entityParentNode)
            {
                // Combine orientation with that of parent entity
                Quaternion mParentQ = entityParentNode->_getDerivedOrientation();
                mDerivedOrientation = mParentQ * mDerivedOrientation;

                Vector3 mParentS = entityParentNode->_getDerivedScale();

                if (mInheritScale)
                {
                    // Incorporate parent entity scale
                    mDerivedScale *= mParentS;
                }

                // Change position vector based on parent entity's orientation & scale
                mDerivedPosition = mParentQ * (mParentS * mDerivedPosition);

                // Add altered position vector to parent entity
                mDerivedPosition += entityParentNode->_getDerivedPosition();
            }
        }


    }
    //-----------------------------------------------------------------------------
    const LightList& TagPoint::getLights(void) const
    {
        return mParentEntity->getParentSceneNode()->findLights(mParentEntity->getBoundingRadius());
    }

}
