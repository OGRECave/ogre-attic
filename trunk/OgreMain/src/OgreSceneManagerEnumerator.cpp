/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreStableHeaders.h"
#include "OgreSceneManagerEnumerator.h"

#include "OgreDynLibManager.h"
#include "OgreDynLib.h"
#include "OgreConfigFile.h"
#include "OgreMaterial.h"
#include "OgreException.h"
#include "OgreRoot.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    template<> SceneManagerEnumerator* Singleton<SceneManagerEnumerator>::ms_Singleton = 0;
    SceneManagerEnumerator* SceneManagerEnumerator::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    SceneManagerEnumerator& SceneManagerEnumerator::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }

    //-----------------------------------------------------------------------
    SceneManagerEnumerator::SceneManagerEnumerator()
    {
        // Create default manager
        mDefaultManager = new SceneManager();

        // All scene types defaulted to begin with (plugins may alter this)
        setSceneManager(ST_GENERIC, mDefaultManager);
        setSceneManager(ST_EXTERIOR_REAL_FAR, mDefaultManager);
        setSceneManager(ST_EXTERIOR_FAR, mDefaultManager);
        setSceneManager(ST_EXTERIOR_CLOSE, mDefaultManager);
        setSceneManager(ST_INTERIOR, mDefaultManager);



    }
    //-----------------------------------------------------------------------
    SceneManagerEnumerator::~SceneManagerEnumerator()
    {
        delete mDefaultManager;
    }
    //-----------------------------------------------------------------------
    SceneManager* SceneManagerEnumerator::getSceneManager(SceneType st)
    {
        SceneManagerList::iterator i = mSceneManagers.find(st);

        if (i != mSceneManagers.end())
        {
            return i->second;
        }
        else
        {
            Except(Exception::ERR_INTERNAL_ERROR, "Cannot find requested SceneManager.", "SceneManagerEnumerator::getSceneManager");
        }
    }



    //-----------------------------------------------------------------------
    void SceneManagerEnumerator::setRenderSystem(RenderSystem* rs)
    {
        std::set<SceneManager*>::iterator i = mUniqueSceneMgrs.begin();

        for(; i != mUniqueSceneMgrs.end(); ++i)
        {
            (*i)->_setDestinationRenderSystem(rs);
        }

    }

    //-----------------------------------------------------------------------
    void SceneManagerEnumerator::setSceneManager(SceneType st, SceneManager* sm)
    {
        // Find entry (may exist)
        SceneManagerList::iterator i = mSceneManagers.find(st);

        if (i == mSceneManagers.end())
        {
            // Insert
            mSceneManagers.insert(SceneManagerList::value_type(st, sm));
        }
        else
        {
            // Override
            i->second = sm;
        }
        // Add to unique set
        mUniqueSceneMgrs.insert(sm);


        // Set rendersystem, incase this one is late & rendersystem already picked
        sm->_setDestinationRenderSystem(Root::getSingleton().getRenderSystem());



    }
    //-----------------------------------------------------------------------
    void SceneManagerEnumerator::shutdownAll(void)
    {
        std::set<SceneManager*>::iterator i;
        for (i = mUniqueSceneMgrs.begin(); i != mUniqueSceneMgrs.end(); ++i)
        {
            (*i)->clearScene();
        }

    }


}
