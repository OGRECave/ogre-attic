/*
-----------------------------------------------------------------------------
This source file is part of the OGRE Reference Application, a layer built
on top of OGRE(Object-oriented Graphics Rendering Engine)
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
#ifndef __REFAPP_WORLD_H__
#define __REFAPP_WORLD_H__

#include "OgreRefAppPrerequisites.h"
#include <OgreSingleton.h>

namespace OgreRefApp {

    class _OgreRefAppExport World : public Singleton<World>
    {
    protected:
        /// Pointer to OGRE's scene manager
        SceneManager* mSceneMgr;

        typedef std::map<String, ApplicationObject*> ObjectMap;
        /// Main list of objects
        ObjectMap mObjects;

        typedef std::set<ApplicationObject*> ObjectSet;
        /// Set of dynamics objects (those to perform physics on)
        ObjectSet mDynamicsObjects;

        // ODE world object
        dWorld* mOdeWorld;

        /// Contact joint group
        dJointGroup* mOdeContactGroup;

        Vector3 mGravity;

        IntersectionSceneQuery* mIntersectionQuery;

    public:
        /** Creates an instance of the world, must have a pointer to SceneManager. */
        World(SceneManager* sceneMgr);
        ~World();

        /// Get the scene manager for this world
        SceneManager* getSceneManager(void);

        /** Create an OGRE head object. */
        OgreHead* createOgreHead(const String& name, const Vector3& pos = Vector3::ZERO, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Create a plane object. */
        OgreRefApp::Plane* createPlane(const String& name, Real width, Real height, const Vector3& pos = Vector3::ZERO, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Create a sphere object. */
        OgreRefApp::Sphere* createSphere(const String& name, Real radius, const Vector3& pos = Vector3::ZERO, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        void clear(void);

        dWorld* getOdeWorld(void);
        dJointGroup* getOdeContactJointGroup(void);

        /** Detects all the collisions in the world and acts on them.
        @remarks
            This method performs the appropriate queries to detect all the colliding objects
            in the world, tells the objects about it and adds the appropriate physical simulation
            constructs required to apply collision response when applyDynamics is called.
        */
        void applyCollision(void);

        /** Updates the world simulation. */
        void applyDynamics(Real timeElapsed);

        /** Internal method for notifying the world of a change in the dynamics status of an object. */
        void _notifyDynamicsStateForObject(ApplicationObject* obj, bool dynamicsEnabled);

        /** Sets the gravity vector, units are in m/s^2.
        @remarks
            The world defaults to no gravity.
            Tip: Earth gravity is Vector3(0, -9.81, 0);
        */
        void setGravity(const Vector3& vec);

        /** Gets the gravity vector. */
        const Vector3& getGravity(void);

        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton
                implementation is in a .h file, which means it gets compiled
                into anybody who includes it. This is needed for the
                Singleton template to work, but we actually only want it
                compiled into the implementation of the class based on the
                Singleton, not all of them. If we don't change this, we get
                link errors when trying to use the Singleton-based class from
                an outside dll.
            @par
                This method just delegates to the template version anyway,
                but the implementation stays in this single compilation unit,
                preventing link errors.
        */
        static World& getSingleton(void);
    };


}

#endif

