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
#include "OgreBspSceneNode.h"
#include "OgreBspSceneManager.h"

namespace Ogre {


    void BspSceneNode::_update(bool updateChildren, bool parentHasChanged)
    {
        // Call superclass
        SceneNode::_update(updateChildren, parentHasChanged);

        // Check membership of attached objects
        ObjectMap::const_iterator it, itend;
        itend = mObjectsByName.end();
        for (it = mObjectsByName.begin(); it != itend; ++it)
        {
            MovableObject* mov = it->second;

            static_cast<BspSceneManager*>(mCreator)->_notifyObjectMoved(
                mov, this->_getDerivedPosition());

        }

    }
	//-------------------------------------------------------------------------
	MovableObject* BspSceneNode::detachObject(unsigned short index)
	{
		MovableObject* ret = SceneNode::detachObject(index);
		static_cast<BspSceneManager*>(mCreator)->_notifyObjectDetached(ret);
		return ret;
		
	}
	//-------------------------------------------------------------------------
	MovableObject* BspSceneNode::detachObject(const String& name)
	{
		MovableObject* ret = SceneNode::detachObject(name);
		static_cast<BspSceneManager*>(mCreator)->_notifyObjectDetached(ret);
		return ret;
	}
	//-------------------------------------------------------------------------
	void BspSceneNode::detachAllObjects(void)
	{
		ObjectMap::const_iterator i, iend;
		iend = mObjectsByName.end();
		for (i = mObjectsByName.begin(); i != iend; ++i)
		{
			static_cast<BspSceneManager*>(mCreator)
				->_notifyObjectDetached(i->second);
		}
		SceneNode::detachAllObjects();
	}

}



