/*
-----------------------------------------------------------------------------
This source file is part of the OGRE Reference Application, a layer built
on top of OGRE(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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

        /// Dynamics properties, must be set up by subclasses if dynamics enabled
        dBody* mOdeBody;
        /// Mass parameters
        dMass mMass;


        /// Collision proxies, must be set up if collision enabled
        typedef std::list<dGeom*> CollisionProxyList;
        CollisionProxyList mCollisionProxies;


        bool mDynamicsEnabled;
        bool mCollisionEnabled;

		Real mBounceCoeffRestitution;
		Real mBounceVelocityThreshold;
		Real mSoftness;
        Real mFriction;

        // Set up method, must override
        virtual void setUp(const String& name) = 0;
        /** Internal method for updating the state of the collision proxies. */
        virtual void updateCollisionProxies(void);

    public:
        ApplicationObject(const String& name);
        virtual ~ApplicationObject();

        /** Sets the position of this object. */
        virtual void setPosition(const Vector3& vec);
        /** Sets the orientation of this object. */
        virtual void setOrientation(const Quaternion& orientation);
        /** Gets the current position of this object. */
        virtual const Vector3& getPosition(void);
        /** Gets the current orientation of this object. */
        virtual const Quaternion& getOrientation(void);

        /// Updates the position of this game object from the simulation
        virtual void _updateFromDynamics(void);

        /// Returns whether or not this object is considered for collision.
        virtual bool isCollisionEnabled(void);
        /** Returns whether or not this object is physically simulated.
        @remarks
            Objects which are not physically simulated only move when their
            SceneNode is manually altered.
        */
        virtual bool isDynamicsEnabled(void);
        /** Sets whether or not this object is considered for collision.
        @remarks
            Objects which have collision enabled must set up an ODE 
            collision proxy as part of their setUp method.
        */
        virtual void setCollisionEnabled(bool enabled);
        /** Sets whether or not this object is physically simulated.
        @remarks
            Objects which are not physically simulated only move when their
            SceneNode is manually altered. Objects which are physically 
            simulated must set up an ODE body as part of their setUp method.
        */
        virtual void setDynamicsEnabled(bool enabled);

		/** Sets the 'bounciness' of this object.
		 * @remarks
		 * Only applies if this object has both collision and dynamics enabled.
		 * When 2 movable objects collide, the greatest bounce parameters 
		 * from both objects apply, so even a non-bouncy object can
		 * bounce if it hits a bouncy surface. 
		 * @param restitutionValue Coeeficient of restitution 
		 * 		(0 for no bounce, 1 for perfect bounciness)
		 * @param velocityThreshold Velocity below which no bounce will occur; 
		 * 		this is a dampening value to ensure small velocities do not
		 * 		cause bounce.
		 */
		virtual void setBounceParameters(Real restitutionValue,	Real velocityThreshold);
		/** Gets the cefficient of restitution (bounciness) for this object. */
		virtual Real getBounceRestitutionValue(void);
		/** Gets the bounce velocity threshold for this object. */
		virtual Real getBounceVelocityThreshold(void);

		/** Sets the softness of this object, which determines how much it is allowed to 
		 * penetrate other objects.
		 * @remarks
		 * 	This parameter only has meaning if collision and dynamics are enabled for this object.
		 * 	@param softness Softness factor (0 is completely hard). Softness will be combined from
		 * 		both objects involved in a collision to determine how much they will penetrate.
		 */
		virtual void setSoftness(Real softness);
		/** Gets the softness factor of this object. */
		virtual Real getSoftness(void);

        /** Sets the Coulomb frictional coefficient for this object.
        @remarks
            This coefficient affects how much an object will slip when it comes
            into contact with another object. 
        @param friction The Coulomb friction coefficient, valid from 0 to Math::POS_INFINITY.
            0 means no friction, Math::POS_INFINITY means infinite friction ie no slippage.
            Note that friction between these 2 bounds is more CPU intensive so use with caution.
        */
        virtual void setFriction(Real friction);
        /** Gets the Coulomb frictional coefficient for this object. */
        virtual Real getFriction(void);
        /** Adds a linear force to this object, in object space, at the position indicated. 
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param direction The force direction in object coordinates.
        @param atPosition The position at which the force is to be applied, in object coordinates.
        */
        virtual void addForce(const Vector3& direction, const Vector3& atPosition = Vector3::ZERO);
        /** Adds a linear force to this object, in world space, at the position indicated. 
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param direction The force direction in world coordinates.
        @param atPosition The position at which the force is to be applied, in world coordinates.
        */
        virtual void addForceWorldSpace(const Vector3& direction, const Vector3& atPosition = Vector3::ZERO);
        /** Adds rotational force to this object, in object space.
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param direction The direction of the torque to apply, in object space. */
        virtual void addTorque(const Vector3& direction);
        /** Adds rotational force to this object, in world space.
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param direction The direction of the torque to apply, in world space. */
        virtual void addTorqueWorldSpace(const Vector3& direction);

        /** Tests to see if there is a detailed collision between this object and the object passed in.
        @remarks
            If there is a collision, both objects will bo notified and if dynamics are enabled
            on these objects, physics will be applied automatically.
        @returns true if collision occurred

        */
        virtual bool testCollide(ApplicationObject* otherObj);

        /** Contains information about a collision; used in the _notifyCollided call. */
        struct CollisionInfo
        {
            /// The position in world coordinates at which the collision occurred
            Vector3 position;
            /// The normal in world coordinates of the collision surface
            Vector3 normal;
            /// Penetration depth 
            Real penetrationDepth;
        };
        /** This method is called automatically if testCollide indicates a real collision. 
        */
        virtual void _notifyCollided(ApplicationObject* otherObj, const CollisionInfo& info);
        /** Gets the SceneNode which is being used to represent this object's position in 
            the OGRE world. */
        SceneNode* getSceneNode(void);
        /** Gets the Entity which is being used to represent this object in the OGRE world. */
        Entity* getEntity(void);
        /** Gets the ODE body used to represent this object's mass and current velocity. */
        dBody* getOdeBody(void);

        /** Set the mass parameters of this object to represent a sphere.
        @remarks
            This method sets the mass and inertia properties of this object such
            that it is like a sphere, ie center of gravity at the origin and
            an even distribution of mass in all directions.
        @param density Density of the sphere in Kg/m^3
        @param radius of the sphere mass
        */
        void setMassSphere(Real density, Real radius);

        /** Set the mass parameters of this object to represent a box. 
        @remarks
            This method sets the mass and inertia properties of this object such
            that it is like a box.
        @param density Density of the box in Kg/m^3
        @param dimensions Width, height and depth of the box.
        @param orientation Optional orientation of the box.
        */
        void setMassBox(Real density, const Vector3& dimensions, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Set the mass parameters of this object to represent a capped cylinder. 
        @remarks
            This method sets the mass and inertia properties of this object such
            that it is like a capped cylinder, by default lying along the Z-axis.
        @param density Density of the cylinder in Kg/m^3
        @param length Length of the cylinder
        @param width Width of the cylinder
        @param orientation Optional orientation if you wish the cylinder to lay 
            along a different axis from Z.
        */
        void setMassCappedCylinder(Real density, Real length, Real width, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Sets the mass parameters manually, use only if you know how!
        @param mass Mass in Kg
        @param center The center of gravity
        @param inertia The inertia matrix describing distribution of the mass around the body.
        */
        void setMassExpert(Real mass, const Vector3 center, const Matrix3 inertia);

        /** Gets the ODE mass parameters for this object. */
        const dMass* getOdeMass(void);


    };


} // namespace

#endif


