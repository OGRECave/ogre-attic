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
DefaultZone.cpp  -  Default PCZone implementation
-----------------------------------------------------------------------------
begin                : Tue Feb 20 2007
author               : Eric Cha
email                : ericc@xenopi.com

-----------------------------------------------------------------------------
*/

#include "OgreDefaultZone.h"
#include "OgreSceneNode.h"
#include "OgrePortal.h"
#include "OgrePCZSceneNode.h"
#include "OgrePCZSceneManager.h"
#include "OgreEntity.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgrePCZLight.h"

namespace Ogre
{

    DefaultZone::DefaultZone( PCZSceneManager * creator, const String& name )
		: PCZone(creator, name)
    {
		mZoneTypeName = "ZoneType_Default";
	}

    DefaultZone::~DefaultZone()
    {
    }

	/** Set the enclosure node for this Zone
	*/
	void DefaultZone::setEnclosureNode(PCZSceneNode * node)
	{
		mEnclosureNode = node;
		// anchor the node to this zone
		node->anchorToHomeZone(this);
	}

    // this call adds the given node to either the zone's list
	// of nodes at home in the zone, or to the list of visiting nodes
	// NOTE: The list is decided by the node's homeZone value, so 
	// that must be set correctly before calling this function.
    void DefaultZone::_addNode( PCZSceneNode * n )
    {
		if (n->getHomeZone() == this)
		{
			// add a reference to this node in the "nodes at home in this zone" list
			mHomeNodeList.insert( n );
		}
		else
		{
			// add a reference to this node in the "nodes visiting this zone" list
			mVisitorNodeList.insert( n );
		}
    }

    void DefaultZone::removeNode( PCZSceneNode * n )
    {
		if (n->getHomeZone() == this)
		{
			mHomeNodeList.erase( n );
		}
		else
		{
			mVisitorNodeList.erase( n );
		}
    }

	/** Indicates whether or not this zone requires zone-specific data for 
		*  each scene node
		*/
	bool DefaultZone::requiresZoneSpecificNodeData(void)
	{
		// regular DefaultZones don't require any zone specific node data
		return false;
	}

	/* Add a portal to the zone
	*/
	void DefaultZone::_addPortal( Portal * newPortal)
	{
		if (newPortal)
		{
			// make sure portal is unique (at least in this zone)
			PortalList::iterator it = std::find( mPortals.begin(), mPortals.end(), newPortal );
			if (it != mPortals.end())
			{
				OGRE_EXCEPT(
					Exception::ERR_DUPLICATE_ITEM,
					"A portal with the name " + newPortal->getName() + " already exists",
					"DefaultZone::_addPortal" );
			}
			// add portal to portals list
	        mPortals.push_back( newPortal );
			// tell the portal which zone it's currently in
			newPortal->setCurrentHomeZone(this);
		}
	}

	/* Remove a portal from the zone (does not erase the portal object, just removes reference)
	*/
	void DefaultZone::_removePortal( Portal * removePortal)
	{
		if (removePortal)
		{
			mPortals.erase( std::find( mPortals.begin(), mPortals.end(), removePortal ) );
		}
	}

	/* Recursively check for intersection of the given scene node
	 * with zone portals.  If the node touches a portal, then the
	 * connected zone is assumed to be touched.  The zone adds
	 * the node to its node list and the node adds the zone to 
	 * its visiting zone list. 
	 *
	 * NOTE: This function assumes that the home zone of the node 
	 *       is correct.  The function "_updateHomeZone" in PCZSceneManager
	 *		 takes care of this and should have been called before 
	 *		 this function.
	 */

	void DefaultZone::_checkNodeAgainstPortals(PCZSceneNode * pczsn, Portal * ignorePortal)
	{
		if (pczsn == mEnclosureNode ||
			pczsn->allowedToVisit() == false)
		{
			// don't do any checking of enclosure node versus portals
			return;
		}

		PCZone * connectedZone;
        for ( PortalList::iterator it = mPortals.begin(); it != mPortals.end(); ++it )
        {
			Portal * p = *it;
			//Check if the portal intersects the node
			if (p != ignorePortal &&
				p->intersects(pczsn) != Portal::NO_INTERSECT)
			{
				// node is touching this portal
				connectedZone = p->getTargetZone();
				// add zone to the nodes visiting zone list unless it is the home zone of the node
				if (connectedZone != pczsn->getHomeZone() &&
					!pczsn->isVisitingZone(connectedZone))
				{
					pczsn->addZoneToVisitingZonesMap(connectedZone);
					// tell the connected zone that the node is visiting it
					connectedZone->_addNode(pczsn);
					//recurse into the connected zone
					connectedZone->_checkNodeAgainstPortals(pczsn, p->getTargetPortal());
				}
			}
        }
	}

    /** (recursive) check the given light against all portals in the zone
    * NOTE: This is the default implementation, which doesn't take advantage
    *       of any zone-specific optimizations for checking portal visibility
    */
    void DefaultZone::_checkLightAgainstPortals(PCZLight *light, 
                                                long frameCount, 
                                                PCZFrustum *portalFrustum,
                                                Portal * ignorePortal)
    {
		for ( PortalList::iterator it = mPortals.begin(); it != mPortals.end(); ++it )
		{
			Portal * p = *it;
            if (p != ignorePortal)
            {
                // calculate the direction vector from light to portal
                Vector3 lightToPortal = p->getDerivedCP() - light->getDerivedPosition();
                if (portalFrustum->isVisible(p))
                {
                    // portal is facing the light, but some light types need to
                    // check illumination radius too.
                    PCZone * targetZone = p->getTargetZone();
                    switch(light->getType())
                    {
                    case Light::LT_POINT:
                        // point lights - just check if within illumination range
                        if (lightToPortal.length() <= light->getAttenuationRange())
                        {
 							// if portal is quad portal it must be pointing towards the light 
							if ((p->getType() == Portal::PORTAL_TYPE_QUAD && lightToPortal.dotProduct(p->getDerivedDirection()) < 0.0) ||
								(p->getType() != Portal::PORTAL_TYPE_QUAD))
							{
								if (!light->affectsZone(targetZone))
								{
									light->addZoneToAffectedZonesList(targetZone);
									if (targetZone->getLastVisibleFrame() == frameCount)
									{
										light->setAffectsVisibleZone(true);
									}
									// set culling frustum from the portal
									portalFrustum->addPortalCullingPlanes(p);
									// recurse into the target zone of the portal
									p->getTargetZone()->_checkLightAgainstPortals(light, 
																				frameCount, 
																				portalFrustum,
																				p->getTargetPortal());
									// remove the planes added by this portal
									portalFrustum->removePortalCullingPlanes(p);
								}
							}
                        }
                        break;
                    case Light::LT_DIRECTIONAL:
                        // directionals have infinite range, so just make sure
                        // the direction is facing the portal
                        if (lightToPortal.dotProduct(light->getDerivedDirection()) >= 0.0)
                        {
 							// if portal is quad portal it must be pointing towards the light 
							if ((p->getType() == Portal::PORTAL_TYPE_QUAD && lightToPortal.dotProduct(p->getDerivedDirection()) < 0.0) ||
								(p->getType() != Portal::PORTAL_TYPE_QUAD))
							{
								if (!light->affectsZone(targetZone))
								{
									light->addZoneToAffectedZonesList(targetZone);
									if (targetZone->getLastVisibleFrame() == frameCount)
									{
										light->setAffectsVisibleZone(true);
									}
									// set culling frustum from the portal
									portalFrustum->addPortalCullingPlanes(p);
									// recurse into the target zone of the portal
									p->getTargetZone()->_checkLightAgainstPortals(light, 
																				frameCount, 
																				portalFrustum,
																				p->getTargetPortal());
									// remove the planes added by this portal
									portalFrustum->removePortalCullingPlanes(p);
								}
							}
                        }
                        break;
                    case Light::LT_SPOTLIGHT:
                        // spotlights - just check if within illumination range
                        // Technically, we should check if the portal is within
                        // the cone of illumination, but for now, we'll leave that
                        // as a future optimisation.
                        if (lightToPortal.length() <= light->getAttenuationRange())
                        {
 							// if portal is quad portal it must be pointing towards the light 
							if ((p->getType() == Portal::PORTAL_TYPE_QUAD && lightToPortal.dotProduct(p->getDerivedDirection()) < 0.0) ||
								(p->getType() != Portal::PORTAL_TYPE_QUAD))
							{
								if (!light->affectsZone(targetZone))
								{
									light->addZoneToAffectedZonesList(targetZone);
									if (targetZone->getLastVisibleFrame() == frameCount)
									{
										light->setAffectsVisibleZone(true);
									}
									// set culling frustum from the portal
									portalFrustum->addPortalCullingPlanes(p);
									// recurse into the target zone of the portal
									p->getTargetZone()->_checkLightAgainstPortals(light, 
																				frameCount, 
																				portalFrustum,
																				p->getTargetPortal());
									// remove the planes added by this portal
									portalFrustum->removePortalCullingPlanes(p);
								}
							}
                        }
                        break;
                    }
                }
            }
        }           
    }

	/** Update the spatial data for the portals in the zone
	* NOTE: All scenenodes must be up-to-date before calling this routine.
	*/
	void DefaultZone::updatePortalsSpatially(void)
	{
		// update each portal spatial data
		for ( PortalList::iterator it = mPortals.begin(); it != mPortals.end(); ++it )
		{
			Portal * p = *it;
			p->updateDerivedValues();
		}
	}

	/** Update the zone data for the portals in the zone
	* NOTE: All portal spatial data must be up-to-date before calling this routine.
	*/
	void DefaultZone::updatePortalsZoneData(void)
	{
		PortalList transferPortalList;
		// check each portal to see if it's intersecting another portal of greater size
		for ( PortalList::iterator it = mPortals.begin(); it != mPortals.end(); ++it )
		{
			Portal * p = *it;
			Real pRadius = p->getRadius();
			// First we check against portals in the SAME zone (and only if they have a 
			// target zone different from the home zone)
			for ( PortalList::iterator it2 = mPortals.begin(); it2 != mPortals.end(); ++it2 )
			{
				Portal * p2 = (*it2);
				// only check against bigger portals (this will also prevent checking against self)
				// and only against portals which point to another zone
				if (pRadius < p2->getRadius() && p2->getTargetZone() != this)
				{
					// Portal#2 is bigger than Portal1, check for crossing
					if (p->crossedPortal(p2) && p->getCurrentHomeZone() != p2->getTargetZone())
					{
						// portal#1 crossed portal#2 - flag portal#1 to be moved to portal#2's target zone
						p->setNewHomeZone(p2->getTargetZone());
						transferPortalList.push_back(p);
						break;
					}
				}
			}

			// Second we check against portals in the target zone (and only if that target
			// zone is different from the home zone)
			PCZone * tzone = p->getTargetZone();
			if (tzone != this)
			{
				for ( PortalList::iterator it3 = tzone->mPortals.begin(); it3 != tzone->mPortals.end(); ++it3 )
				{
					Portal * p3 = (*it3);
					// only check against bigger portals
					if (pRadius < p3->getRadius())
					{
						// Portal#3 is bigger than Portal#1, check for crossing
						if (p->crossedPortal(p3) && 
							p->getCurrentHomeZone() != p3->getTargetZone())
						{
							// Portal#1 crossed Portal#3 - switch target zones for Portal#1
							p->setTargetZone(p3->getTargetZone());
							break;
						}
					}
				}
			}
		}
		// transfer any portals to new zones that have been flagged
		for ( PortalList::iterator it = transferPortalList.begin(); it != transferPortalList.end(); ++it )
		{
			Portal * p = *it;
			if (p->getNewHomeZone() != 0)
			{
				_removePortal(p);
				p->getNewHomeZone()->_addPortal(p);
				p->setNewHomeZone(0);
			}
		}
		transferPortalList.clear();
	}

    /* The following function checks if a node has left it's current home zone.
	* This is done by checking each portal in the zone.  If the node has crossed
	* the portal, then the current zone is no longer the home zone of the node.  The
	* function then recurses into the connected zones.  Once a zone is found where
	* the node does NOT cross out through a portal, that zone is the new home zone.
	NOTE: For this function to work, the node must start out in the proper zone to
	      begin with!
	*/
    PCZone* DefaultZone::updateNodeHomeZone( PCZSceneNode * pczsn, bool allowBackTouches )
    {
		// default to newHomeZone being the current home zone
		PCZone * newHomeZone = pczsn->getHomeZone();

		// Check all portals of the start zone for crossings!
		Portal* portal;
		PortalList::iterator pi, piend;
		piend = mPortals.end();
		for (pi = mPortals.begin(); pi != piend; pi++)
		{
			portal = *pi;
			Portal::PortalIntersectResult pir = portal->intersects(pczsn);
			switch (pir)
			{
			default:
			case Portal::NO_INTERSECT: // node does not intersect portal - do nothing
			case Portal::INTERSECT_NO_CROSS:// node intersects but does not cross portal - do nothing				
				break;
			case Portal::INTERSECT_BACK_NO_CROSS:// node intersects but on the back of the portal
				if (allowBackTouches)
				{
					// node is on wrong side of the portal - fix if we're allowing backside touches
					if (portal->getTargetZone() != this &&
						portal->getTargetZone() != pczsn->getHomeZone())
					{
						// set the home zone of the node to the target zone of the portal
						pczsn->setHomeZone(portal->getTargetZone());
						// continue checking for portal crossings in the new zone
						newHomeZone = portal->getTargetZone()->updateNodeHomeZone(pczsn, false);
					}
				}
				break;
			case Portal::INTERSECT_CROSS:
				// node intersects and crosses the portal - recurse into that zone as new home zone
				if (portal->getTargetZone() != this &&
					portal->getTargetZone() != pczsn->getHomeZone())
				{
					// set the home zone of the node to the target zone of the portal
					pczsn->setHomeZone(portal->getTargetZone());
					// continue checking for portal crossings in the new zone
					newHomeZone = portal->getTargetZone()->updateNodeHomeZone(pczsn, true);
				}
				break;
			}
		}

		// return the new home zone
		return newHomeZone;

    }

    /*
    // Recursively walk the zones, adding all visible SceneNodes to the list of visible nodes.
    */
    void DefaultZone::findVisibleNodes(PCZCamera *camera, 
								  NodeList & visibleNodeList,
								  RenderQueue * queue,
								  VisibleObjectsBoundsInfo* visibleBounds, 
								  bool onlyShadowCasters,
								  bool displayNodes,
								  bool showBoundingBoxes)
    {

        //return immediately if nothing is in the zone.
		if (mHomeNodeList.size() == 0 &&
			mVisitorNodeList.size() == 0 &&
			mPortals.size() == 0)
            return ;

        // Else, the zone is automatically assumed to be visible since either
		// it is the camera the zone is in, or it was reached because
		// a connecting portal was deemed visible to the camera.  

		// enable sky if called to do so for this zone
		if (mHasSky)
		{
			// enable sky 
			mPCZSM->enableSky(true);
		}

		// find visible nodes at home in the zone
        bool vis;
        PCZSceneNodeList::iterator it = mHomeNodeList.begin();
        while ( it != mHomeNodeList.end() )
        {
			PCZSceneNode * pczsn = *it;
            // if the scene node is already visible, then we can skip it
            if (pczsn->getLastVisibleFrame() != mLastVisibleFrame ||
				pczsn->getLastVisibleFromCamera() != camera)
            {
				// for a scene node, check visibility using AABB
				vis = camera ->isVisible( pczsn -> _getWorldAABB() );
				if ( vis )
				{
					// add it to the list of visible nodes
					visibleNodeList.push_back( pczsn );
					// add the node to the render queue
					pczsn -> _addToRenderQueue(camera, queue, onlyShadowCasters, visibleBounds );
					// if we are displaying nodes, add the node renderable to the queue
					if ( displayNodes )
					{
						queue -> addRenderable( pczsn );
					}
					// if the scene manager or the node wants the bounding box shown, add it to the queue
					if (pczsn->getShowBoundingBox() || showBoundingBoxes)
					{
						pczsn->_addBoundingBoxToQueue(queue);
					}
					// flag the node as being visible this frame
					pczsn->setLastVisibleFrame(mLastVisibleFrame);
					pczsn->setLastVisibleFromCamera(camera);
				}
            }
            ++it;
        }
		// find visible visitor nodes
        it = mVisitorNodeList.begin();
        while ( it != mVisitorNodeList.end() )
        {
			PCZSceneNode * pczsn = *it;
            // if the scene node is already visible, then we can skip it
            if (pczsn->getLastVisibleFrame() != mLastVisibleFrame ||
				pczsn->getLastVisibleFromCamera() != camera)
            {
				// for a scene node, check visibility using AABB
				vis = camera ->isVisible( pczsn -> _getWorldAABB() );
				if ( vis )
				{
					// add it to the list of visible nodes
					visibleNodeList.push_back( pczsn );
					// add the node to the render queue
					pczsn->_addToRenderQueue(camera, queue, onlyShadowCasters, visibleBounds );
					// if we are displaying nodes, add the node renderable to the queue
					if ( displayNodes )
					{
						queue -> addRenderable( pczsn );
					}
					// if the scene manager or the node wants the bounding box shown, add it to the queue
					if (pczsn->getShowBoundingBox() || showBoundingBoxes)
					{
						pczsn->_addBoundingBoxToQueue(queue);
					}
					// flag the node as being visible this frame
					pczsn->setLastVisibleFrame(mLastVisibleFrame);
					pczsn->setLastVisibleFromCamera(camera);
				}
            }
            ++it;
        }

		// find visible portals in the zone and recurse into them
        PortalList::iterator pit = mPortals.begin();

        while ( pit != mPortals.end() )
        {
            Portal * portal = *pit;
			// for portal, check visibility using world bounding sphere & direction
			vis = camera->isVisible(portal);
			if (vis)
			{
				// portal is visible. Add the portal as extra culling planes to camera
				int planes_added = camera->addPortalCullingPlanes(portal);
				// tell target zone it's visible this frame
				portal->getTargetZone()->setLastVisibleFrame(mLastVisibleFrame);
				portal->getTargetZone()->setLastVisibleFromCamera(camera);
				// recurse into the connected zone 
				portal->getTargetZone()->findVisibleNodes(camera, 
														  visibleNodeList, 
														  queue, 
														  visibleBounds, 
														  onlyShadowCasters,
														  displayNodes,
														  showBoundingBoxes);
				if (planes_added > 0)
				{
					// Then remove the extra culling planes added before going to the next portal in this zone.
					camera->removePortalCullingPlanes(portal);
				}
			}
			pit++;
		}
    }
    // --- find nodes which intersect various types of BV's ---
	void DefaultZone::_findNodes( const AxisAlignedBox &t, 
							      PCZSceneNodeList &list, 
                                  PortalList &visitedPortals,
						 		  bool includeVisitors,
								  bool recurseThruPortals,
								  PCZSceneNode *exclude )
    {
		// if this zone has an enclosure, check against the enclosure AABB first
		if (mEnclosureNode)
		{
			if (!mEnclosureNode->_getWorldAABB().intersects(t))
			{
				// AABB of zone does not intersect t, just return.
				return;
			}
		}

		// check nodes at home in this zone
	    PCZSceneNodeList::iterator it = mHomeNodeList.begin();
	    while ( it != mHomeNodeList.end() )
	    {
			PCZSceneNode * pczsn = *it;
			if ( pczsn != exclude )
			{
				// make sure node is not already in the list (might have been added in another
				// zone it was visiting)
				PCZSceneNodeList::iterator it2 = list.find(pczsn);
				if (it2 == list.end())
				{
					bool nsect = t.intersects( pczsn -> _getWorldAABB() );
					if ( nsect )
					{
						list.insert( pczsn );
					}
				}
			}
		    ++it;
	    }

		if (includeVisitors)
		{
			// check visitor nodes
			PCZSceneNodeList::iterator it = mVisitorNodeList.begin();
			while ( it != mVisitorNodeList.end() )
			{
				PCZSceneNode * pczsn = *it;
				if ( pczsn != exclude )
				{
					// make sure node is not already in the list (might have been added in another
					// zone it was visiting)
					PCZSceneNodeList::iterator it2 = list.find(pczsn);
					if (it2 == list.end())
					{
						bool nsect = t.intersects( pczsn -> _getWorldAABB() );
						if ( nsect )
						{
							list.insert( pczsn );
						}
					}
				}
				++it;
			}
		}

        // if asked to, recurse through portals
        if (recurseThruPortals)
        {
            PortalList::iterator pit = mPortals.begin();
            while ( pit != mPortals.end() )
            {
                Portal * portal = *pit;
			    // check portal versus bounding box
			    if (portal->intersects(t))
			    {
                    // make sure portal hasn't already been recursed through
                    PortalList::iterator pit2 = std::find(visitedPortals.begin(), visitedPortals.end(), portal);
                    if (pit2 == visitedPortals.end())
                    {
                        // save portal to the visitedPortals list
                        visitedPortals.push_front(portal);
				        // recurse into the connected zone 
				        portal->getTargetZone()->_findNodes(t, 
														    list, 
                                                            visitedPortals,
														    includeVisitors, 
														    recurseThruPortals, 
														    exclude);
                    }
			    }
			    pit++;
		    }
        }
    }

    void DefaultZone::_findNodes(const Sphere &t, 
							     PCZSceneNodeList &list, 
                                 PortalList &visitedPortals,
						 	     bool includeVisitors,
							     bool recurseThruPortals,
							     PCZSceneNode *exclude )
    {
		// if this zone has an enclosure, check against the enclosure AABB first
		if (mEnclosureNode)
		{
			if (!mEnclosureNode->_getWorldAABB().intersects(t))
			{
				// AABB of zone does not intersect t, just return.
				return;
			}
		}

		// check nodes at home in this zone
	    PCZSceneNodeList::iterator it = mHomeNodeList.begin();
	    while ( it != mHomeNodeList.end() )
	    {
			PCZSceneNode * pczsn = *it;
			if ( pczsn != exclude )
			{
				// make sure node is not already in the list (might have been added in another
				// zone it was visiting)
				PCZSceneNodeList::iterator it2 = list.find(pczsn);
				if (it2 == list.end())
				{
					bool nsect = t.intersects( pczsn -> _getWorldAABB() );
					if ( nsect )
					{
						list.insert( pczsn );
					}
				}
			}
		    ++it;
	    }

		if (includeVisitors)
		{
			// check visitor nodes
			PCZSceneNodeList::iterator it = mVisitorNodeList.begin();
			while ( it != mVisitorNodeList.end() )
			{
				PCZSceneNode * pczsn = *it;
				if ( pczsn != exclude )
				{
					// make sure node is not already in the list (might have been added in another
					// zone it was visiting)
					PCZSceneNodeList::iterator it2 = list.find(pczsn);
					if (it2 == list.end())
					{
						bool nsect = t.intersects( pczsn -> _getWorldAABB() );
						if ( nsect )
						{
							list.insert( pczsn );
						}
					}
				}
				++it;
			}
		}

        // if asked to, recurse through portals
        if (recurseThruPortals)
        {
            PortalList::iterator pit = mPortals.begin();
            while ( pit != mPortals.end() )
            {
                Portal * portal = *pit;
			    // check portal versus boundign box
			    if (portal->intersects(t))
			    {
                    // make sure portal hasn't already been recursed through
                    PortalList::iterator pit2 = std::find(visitedPortals.begin(), visitedPortals.end(), portal);
                    if (pit2 == visitedPortals.end())
                    {
                        // save portal to the visitedPortals list
                        visitedPortals.push_front(portal);
				        // recurse into the connected zone 
				        portal->getTargetZone()->_findNodes(t, 
														    list, 
                                                            visitedPortals,
														    includeVisitors, 
														    recurseThruPortals, 
														    exclude);
                    }
			    }
			    pit++;
		    }
        }

    }

    void DefaultZone::_findNodes( const PlaneBoundedVolume &t, 
							      PCZSceneNodeList &list, 
                                  PortalList &visitedPortals,
						 	      bool includeVisitors,
							      bool recurseThruPortals,
							      PCZSceneNode *exclude)
    {
		// if this zone has an enclosure, check against the enclosure AABB first
		if (mEnclosureNode)
		{
			if (!t.intersects(mEnclosureNode->_getWorldAABB()))
			{
				// AABB of zone does not intersect t, just return.
				return;
			}
		}

		// check nodes at home in this zone
	    PCZSceneNodeList::iterator it = mHomeNodeList.begin();
	    while ( it != mHomeNodeList.end() )
	    {
			PCZSceneNode * pczsn = *it;
			if ( pczsn != exclude )
			{
				// make sure node is not already in the list (might have been added in another
				// zone it was visiting)
				PCZSceneNodeList::iterator it2 = list.find(pczsn);
				if (it2 == list.end())
				{
					bool nsect = t.intersects( pczsn -> _getWorldAABB() );
					if ( nsect )
					{
						list.insert( pczsn );
					}
				}
			}
		    ++it;
	    }

		if (includeVisitors)
		{
			// check visitor nodes
			PCZSceneNodeList::iterator it = mVisitorNodeList.begin();
			while ( it != mVisitorNodeList.end() )
			{
				PCZSceneNode * pczsn = *it;
				if ( pczsn != exclude )
				{
					// make sure node is not already in the list (might have been added in another
					// zone it was visiting)
					PCZSceneNodeList::iterator it2 = list.find(pczsn);
					if (it2 == list.end())
					{
						bool nsect = t.intersects( pczsn -> _getWorldAABB() );
						if ( nsect )
						{
							list.insert( pczsn );
						}
					}
				}
				++it;
			}
		}

        // if asked to, recurse through portals
        if (recurseThruPortals)
        {
            PortalList::iterator pit = mPortals.begin();
            while ( pit != mPortals.end() )
            {
                Portal * portal = *pit;
			    // check portal versus boundign box
			    if (portal->intersects(t))
			    {
                    // make sure portal hasn't already been recursed through
                    PortalList::iterator pit2 = std::find(visitedPortals.begin(), visitedPortals.end(), portal);
                    if (pit2 == visitedPortals.end())
                    {
                        // save portal to the visitedPortals list
                        visitedPortals.push_front(portal);
				        // recurse into the connected zone 
				        portal->getTargetZone()->_findNodes(t, 
														    list, 
                                                            visitedPortals,
														    includeVisitors, 
														    recurseThruPortals, 
														    exclude);
                    }
			    }
			    pit++;
		    }
        }

    }

    void DefaultZone::_findNodes( const Ray &t, 
							      PCZSceneNodeList &list, 
                                  PortalList &visitedPortals,
						 	      bool includeVisitors,
							      bool recurseThruPortals,
							      PCZSceneNode *exclude )
    {
		// if this zone has an enclosure, check against the enclosure AABB first
		if (mEnclosureNode)
		{
			std::pair<bool, Real> nsect = t.intersects(mEnclosureNode->_getWorldAABB());
			if (!nsect.first)
			{
				// AABB of zone does not intersect t, just return.
				return;
			}
		}

		// check nodes at home in this zone
	    PCZSceneNodeList::iterator it = mHomeNodeList.begin();
	    while ( it != mHomeNodeList.end() )
	    {
			PCZSceneNode * pczsn = *it;
			if ( pczsn != exclude )
			{
				// make sure node is not already in the list (might have been added in another
				// zone it was visiting)
				PCZSceneNodeList::iterator it2 = list.find(pczsn);
				if (it2 == list.end())
				{
					std::pair<bool, Real> nsect = t.intersects( pczsn -> _getWorldAABB() );
					if ( nsect.first )
					{
						list.insert( pczsn );
					}
				}
			}
		    ++it;
	    }

		if (includeVisitors)
		{
			// check visitor nodes
			PCZSceneNodeList::iterator it = mVisitorNodeList.begin();
			while ( it != mVisitorNodeList.end() )
			{
				PCZSceneNode * pczsn = *it;
				if ( pczsn != exclude )
				{
					// make sure node is not already in the list (might have been added in another
					// zone it was visiting)
					PCZSceneNodeList::iterator it2 = list.find(pczsn);
					if (it2 == list.end())
					{
						std::pair<bool, Real> nsect = t.intersects( pczsn -> _getWorldAABB() );
						if ( nsect.first )
						{
							list.insert( pczsn );
						}
					}
				}
				++it;
			}
		}

        // if asked to, recurse through portals
        if (recurseThruPortals)
        {
            PortalList::iterator pit = mPortals.begin();
            while ( pit != mPortals.end() )
            {
                Portal * portal = *pit;
			    // check portal versus boundign box
			    if (portal->intersects(t))
			    {
                    // make sure portal hasn't already been recursed through
                    PortalList::iterator pit2 = std::find(visitedPortals.begin(), visitedPortals.end(), portal);
                    if (pit2 == visitedPortals.end())
                    {
                        // save portal to the visitedPortals list
                        visitedPortals.push_front(portal);
				        // recurse into the connected zone 
				        portal->getTargetZone()->_findNodes(t, 
														    list, 
                                                            visitedPortals,
														    includeVisitors, 
														    recurseThruPortals, 
														    exclude);
                    }
			    }
			    pit++;
		    }
        }

    }

	/* Set option for the zone */
	bool DefaultZone::setOption( const String &name, const void *value )
	{
		return false;
	}

	/** called when the scene manager creates a camera because
		some zone managers (like TerrainZone) need the camera info.
	*/
	void DefaultZone::notifyCameraCreated( Camera* c )
	{
	}
	//-------------------------------------------------------------------------
	void DefaultZone::notifyWorldGeometryRenderQueue(uint8 qid)
	{
	}
	//-------------------------------------------------------------------------
    void DefaultZone::notifyBeginRenderScene(void)
    {
	}
	//-------------------------------------------------------------------------
	void DefaultZone::setZoneGeometry(const String &filename, PCZSceneNode * parentNode)
	{
		String entityName, nodeName;
		entityName = this->getName() + "_entity";
		nodeName = this->getName() + "_Node";
		Entity *ent = mPCZSM->createEntity(entityName , filename );
		// create a node for the entity
		PCZSceneNode * node;
		node = (PCZSceneNode*)(parentNode->createChildSceneNode(nodeName));
		// attach the entity to the node
		node->attachObject(ent);
		// set the node as the enclosure node
		setEnclosureNode(node);
	}

}
