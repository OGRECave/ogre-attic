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
#ifndef __SceneManagerEnumerator_H__
#define __SceneManagerEnumerator_H__

#include "OgrePrerequisites.h"

#include "OgreSceneManager.h"
#include "OgreSingleton.h"

namespace Ogre {
    /** Classification of a scene to allow a decision of what type of
        SceenManager to provide back to the application.
    */
    enum SceneType
    {
        ST_GENERIC,
        ST_EXTERIOR_CLOSE,
        ST_EXTERIOR_FAR,
        ST_INTERIOR
    };
    /** Enumerates the SceneManager classes available to applications.
        @remarks
            As described in the SceneManager class, SceneManagers are responsible
            for organising the scene and issuing rendering commands to the
            RenderSystem. Certain scene types can benefit from different
            rendering approaches, and it is intended that subclasses will
            be created to special case this.
        @par
            In order to give applications easy access to these implementations,
            the Root object has a getSceneManager method to retrieve a SceneManager
            which is appropriate to the scene type. However, this is the class
            which implements this behaviour and defines the scene types, because
            it is intended that the Root class is not customised by everybody
            (and it may be restricted access in the future).
        @par
            If you customise Ogre and want to add a new SceneManager implementation
            (e.g. an Octree-based scene manager for outside locations), feel
            free to customise this class so that it is passed back where
            required.
        @par
            For this early release of Ogre, only the basic SceneManager implementation
            is passed back for all scene types. This is a highly generic and
            extremely unoptimised reference implementation.
    */
    class _OgreExport SceneManagerEnumerator : public Singleton<SceneManagerEnumerator>
    {
    private:
        // Set of SceneManagers (unique entries)
        std::set<SceneManager*> mUniqueSceneMgrs;
        // Collection of loaded scene managers, keyed by scene type
        typedef std::map<SceneType, SceneManager*> SceneManagerList ;
        SceneManagerList mSceneManagers;

        /// Standard scene manager for default management
        SceneManager* mDefaultManager;


    public:
        SceneManagerEnumerator();
        ~SceneManagerEnumerator();

        /** Sets a SceneManager implementation.
        */
        void setSceneManager(SceneType st, SceneManager* sm);

        /** Implementation of SceneManager retrieval.
        */
        SceneManager* getSceneManager(SceneType st);

        /** Notifies all SceneManagers of the destination rendering system.
        */
        void setRenderSystem(RenderSystem* rs);

        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton implementation is in a .h file,
                which means it gets compiled into anybody who includes it. This is needed for the Singleton
                template to work, but we actually only want it compiled into the implementation of the
                class based on the Singleton, not all of them. If we don't change this, we get link errors
                when trying to use the Singleton-based class from an outside dll.
            @par
                This method just delegates to the template version anyway, but the implementation stays in this
                single compilation unit, preventing link errors.
        */
        static SceneManagerEnumerator& getSingleton(void);

    };


}

#endif
