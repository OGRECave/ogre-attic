/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
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
#ifndef __TagPoint_H_
#define __TagPoint_H_

#include "OgrePrerequisites.h"

#include "OgreBone.h"
#include "OgreQuaternion.h"
#include "OgreVector3.h"
#include "OgreMatrix4.h"

namespace Ogre	{


	
    /** A tagged point on a skeleton, which can be used to attach entities to on specific
        other entities.
    @remarks
        A Skeleton, like a Mesh, is shared between Entity objects and simply updated as required
        when it comes to rendering. However there are times when you want to attach another object
        to an animated entity, and make sure that attachment follows the parent entity's animation
        (for example, a character holding a gun in his / her hand). This class simply identifies
        attachment points on a skeleton which can be used to attach child objects. 
    @par
        The child objects themselves are not physically attached to this class; as it's name suggests
        this class just 'tags' the area. The actual child objects are attached to the Entity using the
        skeleton which has this tag point. Use the Entity::attachMovableObjectToBone method to attach
        the objects, which creates a new TagPoint on demand.
    */
    class TagPoint : public Bone
	{

	public:
		TagPoint(unsigned short handle, Skeleton* creator);
		virtual ~TagPoint();

		Entity *getParentEntity(void);
		
		void setParentEntity(Entity *pEntity);
		void setChildObject(MovableObject *pObject);

		//Matrix4 getCombinedTransform();
		Matrix4 getParentEntityTransform();

        void _update(bool updateChildren, bool parentHasChanged);

        Matrix4 _getFullTransform(void);

		Matrix4 _getNodeFullTransform(void);

	private:
		Entity *mParentEntity;
		MovableObject *mChildObject;
		
	};


} //namespace


#endif//__TagPoint_H_
