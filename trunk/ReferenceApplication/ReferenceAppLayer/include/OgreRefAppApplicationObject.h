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
#ifndef __REFAPP_APPLICATIONOBJECT_H__
#define __REFAPP_APPLICATIONOBJECT_H__

#include "OgreRefAppPrerequisites.h"

namespace OgreRefApp {


    /** This object is the base class for all discrete objects in the application.
    @remarks
        This object holds a reference to the underlying OGRE entity / entities which 
        comprise it, plus links to the additional properties required to make it
        work in the application world.
    @remarks
        It extends the OGRE UserDefinedObject to allow reverse links from Ogre::Entity.
        Note that this class does not override the UserDefinedObject's getTypeId method 
        because this class is abstract.
    */
    class _OgreRefAppExport ApplicationObject : public UserDefinedObject
    {
    protected:
        // Visual component
        SceneNode* mSceneNode;
        Entity* mEntity;

        // Dynamics properties, must be set up by subclasses if dynamics enabled
        dBody* mOdeBody;

        // Collision proxies, must be set up if collision enabled
        typedef std::list<dGeom*> CollisionProxyList;
        CollisionProxyList mCollisionProxies;


        bool mDynamicsEnabled;
        bool mCollisionEnabled;


        // Set up method, must override
        virtual void setUp(const String& name) = 0;
        /** Internal method for updating the state of the collision proxies. */
        virtual void updateCollisionProxies(void);

    public:
        ApplicationObject(const String& name);
        virtual ~ApplicationObject();

        virtual void setPosition(const Vector3& vec);
        virtual void setOrientation(const Quaternion& orientation);
        virtual const Vector3& getPosition(void);
        virtual const Quaternion& getOrientation(void);

        /// Updates the position of this game object from the simulation
        virtual void _updateFromDynamics(void);

        virtual bool isCollisionEnabled(void);
        virtual bool isDynamicsEnabled(void);
        virtual void setCollisionEnabled(bool enabled);
        virtual void setDynamicsEnabled(bool enabled);

        virtual void addForce(const Vector3& direction, const Vector3& atPosition = Vector3::ZERO);
        virtual void addForceWorldSpace(const Vector3& direction, const Vector3& atPosition = Vector3::ZERO);
        virtual void addTorque(const Vector3& direction);
        virtual void addTorqueWorldSpace(const Vector3& direction);

        virtual void testCollide(ApplicationObject* otherObj);

        SceneNode* getSceneNode(void);
        Entity* getEntity(void);
        dBody* getOdeBody(void);


    };


} // namespace

#endif


