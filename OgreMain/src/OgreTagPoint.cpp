
#include "OgreTagPoint.h"
#include "OgreMatrix4.h"
#include "OgreMatrix3.h"
#include "OgreEntity.h"
#include "OgreSceneNode.h"
#include "OgreSkeleton.h"
#include "OgreQuaternion.h"


using namespace Ogre;


TagPoint::TagPoint(unsigned short handle, Skeleton* creator): Bone(handle, creator)
{
    mParentEntity = 0; 
    mChildObject = 0; 
}


TagPoint::~TagPoint()
{
}

Entity *TagPoint::getParentEntity(void)
{
	return mParentEntity;
}

void TagPoint::setParentEntity(Entity *pEntity)
{
	mParentEntity = pEntity;
}


void TagPoint::setChildObject(MovableObject *pObject)
{
	mChildObject = pObject;
}

Matrix4 TagPoint::_getFullTransform(void)
{
	return getParentEntityTransform() * Node::_getFullTransform();
}

Matrix4 TagPoint::_getNodeFullTransform(void)
{
	return Node::_getFullTransform();
}


Matrix4 TagPoint::getParentEntityTransform()
{
	
	return mParentEntity->_getParentNodeFullTransform();
}


void TagPoint::_update(bool updateChildren, bool parentHasChanged)
{
	//It's the parent entity that is currently updating the skeleton and his bones
	if(mParentEntity == mCreator->getCurrentEntity())
	{
		Node::_update(updateChildren, parentHasChanged);
	}
}

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




