/*
** This source file is part of OGRE (Object-oriented Graphics Rendering Engine)
** For the latest info, see http://www.ogre3d.org/
** 
** Copyright (c) 2006 Wael El Oraiby
** 
** This program is free software; you can redistribute it and/or modify it under
** the terms of the GNU Lesser General Public License as published by the Free Software
** Foundation; either version 2 of the License, or (at your option) any later
** version.
** 
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public License along with
** this program; if not, write to the Free Software Foundation, Inc., 59 Temple
** Place - Suite 330, Boston, MA 02111-1307, USA, or go to
** http://www.gnu.org/copyleft/lesser.txt.
*/

#include "Ogre.h"

#include "BspObject.h"

#include "HybridPortalBspSceneManagerFactory.h"
#include "HybridPortalBspSceneManager.h"

using namespace Ogre;
using namespace std;

//-----------------------------------------------------------------------------
// HybridPortalBspSceneManager
//-----------------------------------------------------------------------------
HybridPortalBspSceneManager::HybridPortalBspSceneManager(const String &instanceName):
	SceneManager(instanceName)
{
	LogManager::getSingleton().logMessage("[HybridPortalBspSceneManager(...)] created hybrid portal bsp scene manager instance");
	
	mOccludersRootSceneNode = getRootSceneNode()->createChildSceneNode("OCCLUDERS_ROOT");
	mPortalsRootSceneNode = getRootSceneNode()->createChildSceneNode("PORTALS_ROOT");
	mMoversRootSceneNode = getRootSceneNode()->createChildSceneNode("MOVERS_ROOT");
}

//-----------------------------------------------------------------------------
HybridPortalBspSceneManager::~HybridPortalBspSceneManager()
{
}

//-----------------------------------------------------------------------------
const String& HybridPortalBspSceneManager::getTypeName(void) const
{
	return HybridPortalBspSceneManagerFactory::FACTORY_TYPE_NAME;
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::_findVisibleObjects(Camera *cam, bool onlyShadowCasters)
{

	RenderQueue *queue = getRenderQueue();
	RenderSystem *rs = Root::getSingleton().getRenderSystem();

	// issue occlusion queries to find which portals are visible
	{
		queue->clear();
		
		size_t i;

		rs->_setViewport(cam->getViewport());
		rs->clearFrameBuffer(FBT_DEPTH | FBT_COLOUR);
//		cam->getViewport()->setClearEveryFrame(false);

		rs->_beginFrame();

		for( i = 0; i < mOccluders.size(); i++ )
			mOccluders[i]->_findVisibleObjects(cam, queue, true, 
				mDisplayNodes, onlyShadowCasters);

		SceneManager::renderVisibleObjectsDefaultSequence();

		// disable writing to the color/depth buffers
		rs->_setColourBufferWriteEnabled(false, false, false, false);
		rs->_setDepthBufferWriteEnabled(false);


		// render portals and issue occlusion queries
		for( i = 0; i < mPortals.size(); i++ )
		{
			mPortals[i].query->beginOcclusionQuery();
			SceneNode *n = mPortals[i].node;
/*			
			{
				Node::ChildNodeIterator it = n->getChildIterator();

				while( it.hasMoreElements() )
				{
					SceneNode *child = (SceneNode*)it.getNext();
					SceneNode::ObjectIterator obit = child->getAttachedObjectIterator();
					while(obit.hasMoreElements())
					{
						MovableObject *obj = obit.getNext();
						obj->_updateRenderQueue(queue);
					}
				}
			}
			*/
			queue->clear();

			n->_findVisibleObjects(cam, queue, true, 
				mDisplayNodes, onlyShadowCasters);
		
			SceneManager::renderVisibleObjectsDefaultSequence();

			queue->clear();

			mPortals[i].query->endOcclusionQuery();
		}



		// pull visible portals
		for( i = 0; i < mPortals.size(); i++ )
		{
			unsigned int n = 0;
			while(mPortals[i].query->isStillOutstanding());
			mPortals[i].query->pullOcclusionQuery(&n);
			if ( n ) {
				mPortals[i].isVisible = true;
			} else {
				mPortals[i].isVisible = false;
			}
		}
		rs->_endFrame();

		queue->clear();

		// re-enable writing to the color/depth buffers
		rs->_setColourBufferWriteEnabled(true, true, true, true);
		rs->_setDepthBufferWriteEnabled(true);
/*
		// add occluders to the render queue
		for( i = 0; i < mOccluders.size(); i++ )
			mOccluders[i]->_findVisibleObjects(cam, queue, true, 
				mDisplayNodes, onlyShadowCasters);
*/
		// and visible portals
		for( i = 0; i < mPortals.size(); i++ )
		{
			if ( mPortals[i].isVisible ) {
				mPortals[i].node->_findVisibleObjects(cam, queue, true, 
				mDisplayNodes, onlyShadowCasters);
			}
		}

	}

	//mSceneRoot->getChild
    // Tell nodes to find, cascade down all nodes
    //mSceneRoot->_findVisibleObjects(cam, getRenderQueue(), true, 
    //    mDisplayNodes, onlyShadowCasters);

	//mSceneRoot->removeAndDestroyAllChildren();
		
	//SceneManager::_findVisibleObjects(cam, onlyShadowCasters);
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::createPortalSceneNode(String &name)
{
	Portal portal;
	portal.node = getRootSceneNode()->createChildSceneNode(name);
	portal.query = Root::getSingleton().getRenderSystem()->createHardwareOcclusionQuery();
	portal.cells[0] = portal.cells[1] = -1;
	portal.isVisible = true;

	mPortals.push_back(portal);

	// TODO: add portal to dictionary

	return portal.node;
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::createCellSceneNode(String &name)
{
	Cell cell;
	cell.node = getRootSceneNode()->createChildSceneNode(name);
	cell.isVisible = true;

	mCells.push_back(cell);

	// TODO: add cell to dictionary

	return cell.node;
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::createOccluderSceneNode(String &name)
{
	SceneNode* occluder = getRootSceneNode()->createChildSceneNode(name);

	mOccluders.push_back(occluder);

	// TODO: add occluder to dictionary

	return occluder;
}

//-----------------------------------------------------------------------------
int HybridPortalBspSceneManager::addPortalSceneNode(SceneNode *node)
{
	Portal portal;

	node->getParent()->removeChild(node);
	getRootSceneNode()->addChild(node);

	portal.node = node;
	portal.query = Root::getSingleton().getRenderSystem()->createHardwareOcclusionQuery();
	portal.cells[0] = portal.cells[1] = -1;
	portal.isVisible = true;

	mPortals.push_back(portal);

	// TODO: add portal to dictionary

	return static_cast<int>(mPortals.size() - 1);
}

//-----------------------------------------------------------------------------
int HybridPortalBspSceneManager::addCellSceneNode(SceneNode *node)
{
	Cell cell;

	node->getParent()->removeChild(node);
	getRootSceneNode()->addChild(node);

	cell.node = node;
	cell.isVisible = true;

	mCells.push_back(cell);

	// TODO: add cell to dictionary

	return static_cast<int>(mPortals.size() - 1);
}

//-----------------------------------------------------------------------------
int HybridPortalBspSceneManager::addOccluderSceneNode(SceneNode *node)
{
	SceneNode* occluder = node;

	node->getParent()->removeChild(node);
	getRootSceneNode()->addChild(node);

	mOccluders.push_back(occluder);

	// TODO: add occluder to dictionary

	return static_cast<int>(mPortals.size() - 1);
}

//-----------------------------------------------------------------------------
int HybridPortalBspSceneManager::getCellCount()
{
	return static_cast<int>(mCells.size());
}

//-----------------------------------------------------------------------------
int HybridPortalBspSceneManager::getPortalCount()
{
	return static_cast<int>(mPortals.size());
}

//-----------------------------------------------------------------------------
int HybridPortalBspSceneManager::getOccluderCount()
{
	return static_cast<int>(mOccluders.size());
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::getPortalSceneNode(int portalIdx)
{
	if (portalIdx < 0 || portalIdx >= static_cast<int>(mPortals.size()))
		throw String("[getPortalSceneNode] invalid portal index");

	return mPortals[portalIdx].node;
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::getCellSceneNode(int cellIdx)
{
	if (cellIdx < 0 || cellIdx >= static_cast<int>(mCells.size()))
		throw String("[getCellSceneNode] invalid cell index");

	return mCells[cellIdx].node;
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::getNeighboringPortals(int portalId, std::vector<int> &outNeighbors)
{
	size_t i;
	int j, cell[2];

	outNeighbors.resize(0);

	if (portalId < 0 || portalId >= static_cast<int>(mPortals.size()))
		throw "[getNeighboringPortals] portal index out of range";

	getPortalCells(portalId, cell[0], cell[1]);

	for (j = 0; j < 2; j++ )
		for (i = 0; i < mCells[cell[j]].portals.size(); i++)
		{
			int c_portal = mCells[cell[j]].portals[i];
			if (c_portal != portalId)
			{
				size_t k = 0;
				for (; k < outNeighbors.size(); k++)
					if (outNeighbors[k] == c_portal)
						break;

				if (k == outNeighbors.size())
					outNeighbors.push_back(c_portal);
			}
		}
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::getPortalCells(int portalId, int &firstCell, int &secondCell)
{
	if (portalId < 0 || portalId >= static_cast<int>(mPortals.size()))
		throw "[getPortalCells] portal index out of range";

	firstCell = mPortals[portalId].cells[0];
	secondCell = mPortals[portalId].cells[1];
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::getNeighboringCells(int cellId, std::vector<int> &outNeighbors)
{
	outNeighbors.resize(0);

	if (cellId == -1) // solid space
		return;

	if (cellId < 0 || cellId >= static_cast<int>(mCells.size()))
		throw "[getNeighboringCells] Cell index is out of range";

	for (size_t i = 0; i < mCells[cellId].portals.size(); i++)
	{
		for (int j = 0; j < 2; j++)
		{
			int p_cell = mPortals[mCells[cellId].portals[i]].cells[j];
			if ( p_cell != cellId)
			{
				// check if the cell exists in the list
				size_t k = 0;
				for (; k < outNeighbors.size(); k++)
					if (outNeighbors[k] == p_cell)
						break;
				if (k == outNeighbors.size())
					outNeighbors.push_back(p_cell);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::getCellPortals(int cellId, std::vector<int> &outPortals)
{
	outPortals.resize(0);

	if (cellId == -1) // solid space
		return;

	if (cellId < 0 || cellId >= static_cast<int>(mCells.size()))
		throw "[getCellPortals] Cell index is out of range";

	outPortals = mCells[cellId].portals;
}

//-----------------------------------------------------------------------------
int HybridPortalBspSceneManager::getPointCell(const Vector3 &point)
{
	return mBspObject.getPointCell(point);
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::getOccludersRootSceneNode()
{
	return mOccludersRootSceneNode;
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::getPortalsRootSceneNode()
{
	return mPortalsRootSceneNode;
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::getMoversRootSceneNode()
{
	return mMoversRootSceneNode;
}


//-----------------------------------------------------------------------------
// HybridPortalBspSceneManagerFactory
//-----------------------------------------------------------------------------
const String HybridPortalBspSceneManagerFactory::FACTORY_TYPE_NAME = "HybridPortalBspSceneManager";

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManagerFactory::initMetaData(void) const
{
	mMetaData.typeName = FACTORY_TYPE_NAME;
	mMetaData.description = "Scene manager for loading Hybrid Portal/BSP .scene files.";
	mMetaData.sceneTypeMask = ST_GENERIC;
	mMetaData.worldGeometrySupported = false;
}

//-----------------------------------------------------------------------------
SceneManager* HybridPortalBspSceneManagerFactory::createInstance(
	const String& instanceName)
{
	return new HybridPortalBspSceneManager(instanceName);
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManagerFactory::destroyInstance(SceneManager* instance)
{
	delete instance;
}
