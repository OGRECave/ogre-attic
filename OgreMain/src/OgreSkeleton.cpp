/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
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
#include "OgreSkeleton.h"
#include "OgreBone.h"


namespace Ogre {

    //---------------------------------------------------------------------
    Skeleton::Skeleton(String name) 
    {
        mName = name;
        // Start auto handles high to avoid problems with combinations
        mNextAutoHandle = 32768;

        // Create root bone, handle 0
        mRootBone = createBone(0);

    }
    //---------------------------------------------------------------------
    Skeleton::~Skeleton()
    {
        unload();
        // delete root bone
        delete mRootBone;
    }
    //---------------------------------------------------------------------
    void Skeleton::load(void)
    {
        //TODO
    }
    //---------------------------------------------------------------------
    void Skeleton::unload(void)
    {
        //TODO

        // destroy bones, except root
        BoneList::iterator i;
        for (i = mBoneList.begin(); i != mBoneList.end(); ++i)
        {
            if (i->second != mRootBone) // If not root
                delete i->second;
        }
        mBoneList.clear();
    }
    //---------------------------------------------------------------------
    Bone* Skeleton::createBone(void)
    {
        // use autohandle
        return createBone(mNextAutoHandle++);
    }
    //---------------------------------------------------------------------
    Bone* Skeleton::createBone(unsigned short handle)
    {
        Bone* ret = new Bone(handle, this);
        mBoneList[handle] = ret;
        return ret;

    }
    //---------------------------------------------------------------------
    Bone* Skeleton::getRootBone(void)
    {
        return mRootBone;

    }



}
