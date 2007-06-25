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
Portal.cpp  -  
-----------------------------------------------------------------------------
begin                : Tue Feb 20 2007
author               : Eric Cha
email                : ericc@xenopi.com
current TODO's       : none known
-----------------------------------------------------------------------------
*/

#include "OgrePortal.h"
#include "OgrePCZSceneNode.h"
#include "OgreSphere.h"
#include "OgreCapsule.h"
#include "OgreSegment.h"
#include "OgreRay.h"

using namespace Ogre;

Portal::Portal(const String & name)
{
	mName = name;
    mTargetZone = 0;
	mTargetPortal = 0;
	mNode = 0;
	mRadius = 0.0;
	mLocalsUpToDate = false;
	// flag as uninitialized
	mIsInitialized = false;
}

Portal::~Portal()
{
}

// Set the SceneNode the Portal is associated with
void Portal::setNode( SceneNode * sn )
{
    mNode = sn ;
	mLocalsUpToDate = false;
}
// Set the 1st Zone the Portal connects to
void Portal::setTargetZone( PCZone * z )
{
    mTargetZone = z ;
}
// Set the Portal the Portal connects to
void Portal::setTargetPortal( Portal * p )
{
    mTargetPortal = p ;
}
// Set the local coordinates of one of the portal corners    
void Portal::setCorner( int index, Vector3 & pt)
{
    mCorners[index] = pt ;
	mLocalsUpToDate = false;
}
/** Set the local coordinates of all of the portal corners
*/
void Portal::setCorners( Vector3 &pt1, Vector3 &pt2, Vector3 &pt3, Vector3 &pt4 )
{
    mCorners[0] = pt1;
    mCorners[1] = pt2;
    mCorners[2] = pt3;
    mCorners[3] = pt4;
	mLocalsUpToDate = false;
}

// calculate the local direction of the portal from the corners
void Portal::calcDirectionAndRadius(void)
{
	// first calculate local direction
	Vector3 side1, side2;
	side1 = mCorners[1] - mCorners[0];
	side2 = mCorners[2] - mCorners[0];
	mDirection = side1.crossProduct(side2);
	mDirection.normalise();
	// then calculate radius
	mLocalCP = Vector3::ZERO;
	for (int i=0;i<4;i++)
	{
		mLocalCP += mCorners[i];
	}
	mLocalCP *= 0.25f;
	Vector3 radiusVector = mCorners[0] - mLocalCP;
	mRadius = radiusVector.length();
	mDerivedSphere.setRadius(mRadius);
	// locals are now up to date
	mLocalsUpToDate = true;
}

// Calculate the local bounding sphere of the portal from the corner points
Real Portal::getRadius( void )
{
	if (!mLocalsUpToDate)
	{
		calcDirectionAndRadius();
	}
	return mRadius;
}
//Get the coordinates of one of the portal corners
Vector3 & Portal::getCorner( int index)
{
    return mCorners[index];
}
// Get the derived (world) coordinates of a portal corner (assumes they are up-to-date)
Vector3 & Portal::getDerivedCorner( int index) 
{
    return mDerivedCorners[index];
}
// Get the direction of the portal in world coordinates (assumes  it is up-to-date)
Vector3 & Portal::getDerivedDirection( void ) 
{
    return mDerivedDirection;
}
// Get the position (centerpoint) of the portal in world coordinates (assumes  it is up-to-date)
Vector3 & Portal::getDerivedCP( void ) 
{
    return mDerivedCP;
}
// Get the sphere (centered on DerivedCP) of the portal in world coordinates (assumes  it is up-to-date)
Sphere & Portal::getDerivedSphere( void ) 
{
    return mDerivedSphere;
}
// Get the plane of the portal in world coordinates (assumes  it is up-to-date)
Plane & Portal::getDerivedPlane( void ) 
{
    return mDerivedPlane;
}
// Get the previous position (centerpoint) of the portal in world coordinates (assumes  it is up-to-date)
Vector3 & Portal::getPrevDerivedCP( void ) 
{
    return mPrevDerivedCP;
}
// Get the previous plane of the portal in world coordinates (assumes  it is up-to-date)
Plane & Portal::getPrevDerivedPlane( void ) 
{
    return mPrevDerivedPlane;
}
// Update (Calculate) the world spatial values
void Portal::updateDerivedValues(void)
{
	// make sure local values are up to date
	if (!mLocalsUpToDate)
	{
		calcDirectionAndRadius();
	}
	// calculate derived values
	if (mNode)
	{
		if (mIsInitialized)
		{
			// save off the current DerivedCP
			mPrevDerivedCP = mDerivedCP;
			mDerivedCP = (mNode->_getDerivedOrientation() * mLocalCP) + mNode->_getDerivedPosition();
			for (int i=0;i<4;i++)
			{
				mDerivedCorners[i] = (mNode->_getDerivedOrientation() * mCorners[i]) + mNode->_getDerivedPosition();
			}
			mDerivedSphere.setCenter(mDerivedCP);
			mDerivedDirection = mNode->_getDerivedOrientation() * mDirection;
			// save previous calc'd plane
			mPrevDerivedPlane = mDerivedPlane;
			// calc new plane
			mDerivedPlane = Ogre::Plane(mDerivedDirection, mDerivedCP);
		}
		else
		{
			// this is the first time the derived CP has been calculated, so there
			// is no "previous" value, so set previous = current.
			mDerivedCP = (mNode->_getDerivedOrientation() * mLocalCP) + mNode->_getDerivedPosition();
			mPrevDerivedCP = mDerivedCP;
			mDerivedSphere.setCenter(mDerivedCP);
			for (int i=0;i<4;i++)
			{
				mDerivedCorners[i] = (mNode->_getDerivedOrientation() * mCorners[i]) + mNode->_getDerivedPosition();
			}
			mDerivedDirection = mNode->_getDerivedOrientation() * mDirection;
			// calc new plane
			mDerivedPlane = Ogre::Plane(mDerivedDirection, mDerivedCP);
			// this is first time, so there is no previous, so prev = current.
			mPrevDerivedPlane = mDerivedPlane;
			// flag as initialized
			mIsInitialized = true;
		}
	}
	else // no associated node, so just use the local values as derived values
	{
		// save off the current DerivedCP
		if (mIsInitialized)
		{
			// save off the current DerivedCP
			mPrevDerivedCP = mDerivedCP;
			mDerivedCP = mLocalCP;
			mDerivedSphere.setCenter(mDerivedCP);
			for (int i=0;i<4;i++)
			{
				mDerivedCorners[i] = mCorners[i];
			}
			mDerivedDirection = mDirection;
			// save previous calc'd plane
			mPrevDerivedPlane = mDerivedPlane;
			// calc new plane
			mDerivedPlane = Ogre::Plane(mDerivedDirection, mDerivedCP);
		}
		else
		{
			// this is the first time the derived CP has been calculated, so there
			// is no "previous" value, so set previous = current.
			mDerivedCP = mLocalCP;
			mPrevDerivedCP = mDerivedCP;
			mDerivedSphere.setCenter(mDerivedCP);
			for (int i=0;i<4;i++)
			{
				mDerivedCorners[i] = mCorners[i];
			}
			mDerivedDirection = mDirection;
			// calc new plane
			mDerivedPlane = Ogre::Plane(mDerivedDirection, mDerivedCP);
			// this is first time, so there is no previous, so prev = current.
			mPrevDerivedPlane = mDerivedPlane;
			// flag as initialized
			mIsInitialized = true;
		}
	}
}

// Adjust the portal so that it is centered and oriented on the given node
// NOTE: This function will move/rotate the node as well!
// NOTE: The node will become the portal's "associated" node (mNode).
void Portal::adjustNodeToMatch(SceneNode *node )
{
	int i;

	// make sure local values are up to date
	if (!mLocalsUpToDate)
	{
		calcDirectionAndRadius();
	}

	// move the parent node to the center point
	node->setPosition(mLocalCP);
	// move the corner points to be relative to the node
	for (i=0;i<4;i++)
	{
		mCorners[i] -= mLocalCP;
	}
	// NOTE: UNIT_Z is the basis for our local direction
	// orient the node to match the direction
	Quaternion q;
	q = Vector3::UNIT_Z.getRotationTo(mDirection);
	node->setOrientation(q);
	// set the node as the portal's associated node
	setNode(node);

	return;
}

// Check if a portal intersects an AABB
// NOTE: This check is not exact.
bool Portal::intersects(const AxisAlignedBox & aab)
{
    // since ogre doesn't have built in support for a quad, just check
    // if the box intersects both the sphere of the portal and the plane
    // this can result in false positives, but they will be minimal
    if (!aab.intersects(mDerivedSphere))
    {
        return false;
    }

    if (aab.intersects(mDerivedPlane))
    {
        return true;
    }

    return false;
}

// Check if a portal intersects a sphere
// NOTE: This check is not exact.
bool Portal::intersects(const Sphere & sphere)
{
    // since ogre doesn't have built in support for a quad, just check
    // if the sphere intersects both the sphere of the portal and the plane
    // this can result in false positives, but they will be minimal
    if (!sphere.intersects(mDerivedSphere))
    {
        return false;
    }

    if (sphere.intersects(mDerivedPlane))
    {
        return true;
    }

    return false;
}

// Check if a portal intersects a plane bounded volume
// NOTE: This check is not exact.
// NOTE: UNTESTED as of 5/30/07 (EC)
bool Portal::intersects(const PlaneBoundedVolume & pbv)
{
    // first check sphere of the portal
    if (!pbv.intersects(mDerivedSphere))
    {
        return false;
    }
    // if the portal corners are all outside one of the planes of the pbv, 
    // then the portal does not intersect the pbv. (this can result in 
    // some false positives, but it's the best I can do for now)
    PlaneList::const_iterator it = pbv.planes.begin();
    while (it != pbv.planes.end())
    {
        const Plane& plane = *it;
        // check if all 4 corners of the portal are on negative side of the pbv
        bool allOutside = true;
        for (int i=0;i<4;i++)
        {
		    if (plane.getSide(mDerivedCorners[i]) != pbv.outside)
		    {
                allOutside = false;
            }
        }
        if (allOutside)
        {
            return false;
        }
        it++;
    };

    return true;
}

// Check if a portal intersects a ray
// NOTE: Kinda using my own invented routine here... Better do a lot of testing!
bool Portal::intersects(const Ray & ray )
{
    // since ogre doesn't have built in support for a quad, I'm going to first
    // find the intersection point (if any) of the ray and the portal plane.  Then
    // using the intersection point, I take the cross product of each side of the portal
    // (0,1,intersect), (1,2, intersect), (2,3, intersect), and (3,0,intersect).  If
    // all 4 cross products have vectors pointing in the same direction, then the
    // intersection point is within the portal, otherwise it is outside.

    std::pair<bool, Real> result = ray.intersects(mDerivedPlane);

    if (result.first == true)
    {
        // the ray intersects the plane, now walk around the edges 
        Vector3 isect = ray.getPoint(result.second);
        Vector3 cross, vect1, vect2;
        Vector3 cross2, vect3, vect4;
        vect1 = mDerivedCorners[1] - mDerivedCorners[0];
        vect2 = isect - mDerivedCorners[0];
        cross = vect1.crossProduct(vect2);
        vect3 = mDerivedCorners[2] - mDerivedCorners[1];
        vect4 = isect - mDerivedCorners[1];
        cross2 = vect3.crossProduct(vect4);
        if (cross.dotProduct(cross2) < 0)
        {
            return false;
        }
        vect1 = mDerivedCorners[3] - mDerivedCorners[2];
        vect2 = isect - mDerivedCorners[2];
        cross = vect1.crossProduct(vect2);
        if (cross.dotProduct(cross2) < 0)
        {
            return false;
        }
        vect1 = mDerivedCorners[0] - mDerivedCorners[3];
        vect2 = isect - mDerivedCorners[3];
        cross = vect1.crossProduct(vect2);
        if (cross.dotProduct(cross2) < 0)
        {
            return false;
        }
        // all cross products pointing same way, so intersect
        // must be on the inside of the portal!
        return true;
    }

    return false;
}


/* Test if a scene node intersected a portal during the last time delta 
	* (from last frame time to current frame time).  This function checks
	* if the node "crossed over" the portal also.
*/
Portal::PortalIntersectResult Portal::intersects(PCZSceneNode * pczsn)
{
	if (pczsn == mNode)
	{
		// ignore the scene node if it is the node the portal is associated with
		return Portal::NO_INTERSECT;
	}
	// we model the portal as a line swept sphere (mPrevDerivedCP to mDerivedCP).
	// and the node is modeled as a line segment (prevPostion to currentPosition)
	// intersection test is then between the capsule and the line segment.
	Capsule portalCapsule;
	portalCapsule.set(mPrevDerivedCP, mDerivedCP, mRadius);
	Segment nodeSegment;
	nodeSegment.set(pczsn->getPrevPosition(), pczsn->_getDerivedPosition());

	if (portalCapsule.intersects(nodeSegment))
	{
		// the portal intersected the node at some time from last frame to this frame. 
		// Now check if node "crossed" the portal
		// a crossing occurs if the "side" of the final position of the node compared
		// to the final position of the portal is negative AND the initial position
		// of the node compared to the initial position of the portal is non-negative
		if (mDerivedPlane.getSide(pczsn->_getDerivedPosition()) == Plane::NEGATIVE_SIDE &&
			mPrevDerivedPlane.getSide(pczsn->getPrevPosition()) != Plane::NEGATIVE_SIDE)
		{
			// safety check - make sure the node has at least one dimension which is
			// small enough to fit through the portal! (avoid the "elephant fitting 
			// through a mouse hole" case)
			Vector3 nodeHalfVector = pczsn->_getWorldAABB().getHalfSize();
			Vector3 portalBox = Vector3(mRadius, mRadius, mRadius);
			portalBox.makeFloor(nodeHalfVector);
			if (portalBox.x < mRadius)
			{
				// crossing occurred!
				return Portal::INTERSECT_CROSS;
			}
		}
	}
	// there was no crossing of the portal by the node, but it might be touching
	// the portal.  We check for this by checking the bounding box of the node vs.
	// the sphere of the portal
	if (mDerivedSphere.intersects(pczsn->_getWorldAABB()) &&
		mDerivedPlane.getSide(pczsn->_getWorldAABB()) == Plane::BOTH_SIDE )
	{
		// intersection but no crossing
		// note this means that the node is CURRENTLY touching the portal.
		if (mDerivedPlane.getSide(pczsn->_getDerivedPosition()) != Plane::NEGATIVE_SIDE)
		{
			// the node is on the positive (front) or exactly on the CP of the portal
			return Portal::INTERSECT_NO_CROSS;
		}
		else
		{
			// the node is on the negative (back) side of the portal - it might be in the wrong zone!
			return Portal::INTERSECT_BACK_NO_CROSS;
		}
	}
	// no intersection CURRENTLY.  (there might have been an intersection
	// during the time between last frame and this frame, but it wasn't a portal
	// crossing, and it isn't touching anymore, so it doesn't matter.
	return Portal::NO_INTERSECT;
}

/* This function check if *this* portal "crossed over" the other portal.
*/
// BUGBUG! This routine needs to check for case where one or both objects
//         don't move - resulting in simple sphere tests
bool Portal::crossedPortal(Portal * otherPortal)
{
	// we model both portals as line swept spheres (mPrevDerivedCP to mDerivedCP).
	// intersection test is then between the capsules.
	Capsule portalCapsule, otherPortalCapsule;
	portalCapsule.set( mPrevDerivedCP, mDerivedCP, mRadius);

	otherPortalCapsule.set(otherPortal->getPrevDerivedCP(), 
						   otherPortal->getDerivedCP(),
						   otherPortal->getRadius());

	if (portalCapsule.intersects(otherPortalCapsule))
	{
		// the portal intersected the other portal at some time from last frame to this frame. 
		// Now check if this portal "crossed" the other portal
		// a crossing occurs if the "side" of the final position of this portal compared
		// to the final position of the other portal is negative AND the initial position
		// of this portal compared to the initial position of the other portal is non-negative
		// NOTE: This function assumes that this portal is the smaller portal potentially crossing
		//       over the otherPortal which is larger.
		if (otherPortal->getDerivedPlane().getSide(mDerivedCP) == Plane::NEGATIVE_SIDE &&
			otherPortal->getPrevDerivedPlane().getSide(mPrevDerivedCP) != Plane::NEGATIVE_SIDE)
		{
			// crossing occurred!
			return true;
		}
	}
	// there was no crossing of the portal by this portal. It might be touching
	// the other portal (but we don't care currently)
	return false;
}

// check if portal is close to another portal.  
// Note, both portals are assumed to be stationary
// and DerivedCP is the current position.
// this function is INTENTIONALLY NOT EXACT because
// it is used by PCZSM::connectPortalsToTargetZonesByLocation
// which is a utility function to link up nearby portals
//
bool Portal::closeTo(Portal * otherPortal)
{
	return mDerivedSphere.intersects(otherPortal->getDerivedSphere());
}

