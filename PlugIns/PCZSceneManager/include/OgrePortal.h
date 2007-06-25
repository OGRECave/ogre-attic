/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
Portal.h  -  Portals are special constructs which which are used to connect 
			 two Zones in a PCZScene.  Portals are defined by 4 coplanr 
             corners and a direction.  Portals are contained within Zones and 
             are essentially "one way" connectors.  Objects and entities can
			 use them to travel to other Zones, but to return, there must be
			 a corresponding Portal which connects back to the original zone
			 from the new zone.

-----------------------------------------------------------------------------
begin                : Thu Feb 22 2007
author               : Eric Cha
email                : ericc@xenopi.com
Code Style Update	 : Apr 5, 2007
-----------------------------------------------------------------------------
*/

#ifndef PORTAL_H
#define PORTAL_H

#include "OgrePCZPrerequisites.h"
#include "OgrePCZSceneNode.h"
#include "OgreAxisAlignedBox.h"
#include "OgreSphere.h"

namespace Ogre
{
    class PCZone;
	class PCZSceneNode;

    /** Portal datastructure for connecting zones.
    @remarks
    */

	class _OgrePCZPluginExport Portal 
    {
    public:
        Portal(const String &);
        ~Portal();

		/* Returns the name of the portal
		*/
		const String & getName(void) const { return mName; }
        /** Set the SceneNode the Portal is associated with
        */
        void setNode( SceneNode * );
        /** Set the Zone the Portal targets (connects to)
        */
        void setTargetZone( PCZone * );
		/** Set the target portal pointer
		*/
		void setTargetPortal( Portal * );
        /** Set the local coordinates of one of the portal corners
        */
        void setCorner( int , Vector3 & );
        /** Set the local coordinates of all of the portal corners
        */
        void setCorners( Vector3 &, Vector3 &, Vector3 &, Vector3 & );
		/* Calculate the local direction of the portal
		*/
		void calcDirectionAndRadius( void );
        /** Calculate the radius of the portal from corner points
        */
		Real getRadius( void );
        /** Get the Zone the Portal connects to
        */
        PCZone * getTargetZone() {return mTargetZone;}
		/** Get the connected portal (if any)
		*/
		Portal * getTargetPortal() {return mTargetPortal;}
        /** Get the coordinates of one of the portal corners in local space
        */
        Vector3 & getCorner( int );
        /** Get the derived (world) coordinates of one of the portal corners 
        */
        Vector3 & getDerivedCorner( int ) ;
        /** Get the direction of the portal in world coordinates
        */
        Vector3 & getDerivedDirection( void ) ;
        /** Get the position (centerpoint) of the portal in world coordinates
        */
        Vector3 & getDerivedCP( void ) ;
		/** Get the sphere centered on the derived CP of the portal in world coordinates
		*/
		Sphere & getDerivedSphere( void );
		/* Get the portal plane in world coordinates
		*/
		Plane & getDerivedPlane(void) ;
        /** Get the previous position (centerpoint) of the portal in world coordinates
        */
        Vector3 & getPrevDerivedCP( void ) ;
		/* Get the previous portal plane in world coordinates
		*/
		Plane & getPrevDerivedPlane(void) ;
        /** Update the derived values
        */
        void updateDerivedValues(void);
		/* Adjust the portal so that it is centered and oriented on the given node
		*/
		void adjustNodeToMatch(SceneNode *);
		enum PortalIntersectResult
		{
			NO_INTERSECT,
			INTERSECT_NO_CROSS,
			INTERSECT_BACK_NO_CROSS,
			INTERSECT_CROSS
		};
        /* check if portal intersects an aab
        */
        bool intersects(const AxisAlignedBox & aab);

        /* check if portal intersects an sphere
        */
        bool intersects(const Sphere & sphere);

        /* check if portal intersects a plane bounded volume
        */
        bool intersects(const PlaneBoundedVolume & pbv);

        /* check if portal intersects a ray
        */
        bool intersects(const Ray & ray);

		/* check for intersection between portal & scenenode (also determines
		 * if scenenode crosses over portal
		 */
		PortalIntersectResult intersects(PCZSceneNode *);
		/* check if portal crossed over portal
		 */
		bool crossedPortal(Portal *);
		/* check if portal touches another portal
		*/
		bool closeTo(Portal *);

    protected:
		// Name (identifier) for the Portal - must be unique
		String mName;
		/// SceneNode (if any) this portal is attached to
		SceneNode * mNode;
        ///connected Zone
        PCZone * mTargetZone;
		///Matching Portal in the target zone (usually in same world space 
        // as this portal, but pointing the opposite direction)
		Portal * mTargetPortal;
        /// 4 Corners of the portal - coordinates are relative to the sceneNode
        Vector3 mCorners[4];
		/// Direction ("Norm") of the portal - determined by the 1st 3 corners
		Vector3 mDirection;
		/// Radius of the sphere enclosing the portal
		Real mRadius;
		// Local Centerpoint of the portal
		Vector3 mLocalCP;
        /// 4 Derived (world coordinates) Corners of the portal
        Vector3 mDerivedCorners[4];
		/// Derived (world coordinates) direction of the portal
		Vector3 mDerivedDirection;
		/// Derived (world coordinates) of portal (center point)
		Vector3 mDerivedCP;
		/// Sphere of the portal centered on the derived CP
		Sphere mDerivedSphere;
		/// Derived (world coordinates) Plane of the portal
		Plane mDerivedPlane;
		/// Previous frame portal cp (in world coordinates)
		Vector3 mPrevDerivedCP;
		/// Previous frame derived plane 
		Plane mPrevDerivedPlane;
		/// flag indicating whether or not local values are up-to-date
		bool mLocalsUpToDate;
		// flag indicating whether or not the portal has been through it's first update
		bool mIsInitialized;
    };

}

#endif


