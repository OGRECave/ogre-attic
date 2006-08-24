/*
** This source file is part of OGRE (Object-oriented Graphics Rendering Engine)
** For the latest info, see http://www.ogre3d.org/
**
** OGRE Copyright goes to Ogre Team
** Hybrid Portal/BSP Scene Manager Copyright (c) 2006 Wael El Oraiby
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
	SceneManager(instanceName), mShowVisiblePortals(false), mShowVisibleCells(false), mToken(0)
{
	LogManager::getSingleton().logMessage("[HybridPortalBspSceneManager(...)] created hybrid portal bsp scene manager instance");
	
	mOccludersRootSceneNode = getRootSceneNode()->createChildSceneNode("OCCLUDERS_ROOT");
	mPortalsRootSceneNode = getRootSceneNode()->createChildSceneNode("PORTALS_ROOT");
	mMoversRootSceneNode = getRootSceneNode()->createChildSceneNode("MOVERS_ROOT");
	mCellsRootSceneNode = getRootSceneNode()->createChildSceneNode("CELLS_ROOT");
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
void HybridPortalBspSceneManager::showVisiblePortals(bool bShow)
{
	mShowVisiblePortals = bShow;
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::showVisibleCells(bool bShow)
{
	mShowVisibleCells = bShow;
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::_findVisibleObjects(Camera *cam, bool onlyShadowCasters)
{
	size_t i;
	std::map<int, Cell>::iterator itc;
	std::map<int, Portal>::iterator itp;

	RenderQueue *queue = getRenderQueue();
	RenderSystem *rs = Root::getSingleton().getRenderSystem();

	// increment token
	mToken++;

	// first all cells are hidden
	for(itc = mCells.begin(); itc != mCells.end(); itc++)
		itc->second.isVisible = false;

	// only the cell where the camera is in, is always visible
	int cellId = getPointCell(cam->getPosition());
	if ( cellId >= 0 )
		mCells[cellId].isVisible = true;

	// pull visible portals
	for( itp = mPortals.begin(); itp != mPortals.end(); itp++ )
	{
		unsigned int n = 0;
		if(!(itp->second.query->isStillOutstanding()))
		{
			itp->second.query->pullOcclusionQuery(&n);
			if ( n ) 
			{	
				// portal is visible
				itp->second.isVisible = true;
			} else {
				itp->second.isVisible = false;
			}
		}
	}

	// pull visible portals again (this is to remove flickering)
	for( itp = mPortals.begin(); itp != mPortals.end(); itp++ )
	{
		if (itp->second.isVisible)
		{
			// extract cell visibility
			int cell0 = itp->second.cells[0];
			if( cell0 >= 0 )
				mCells[cell0].isVisible = true;

			int cell1 = itp->second.cells[1];
			if( cell1 >= 0 )
				mCells[cell1].isVisible = true;	
		}
	}

	// issue occlusion queries to find which portals are visible for the next frame
	rs->_setViewport(cam->getViewport());
	rs->clearFrameBuffer(FBT_DEPTH | FBT_COLOUR);
//	cam->getViewport()->setClearEveryFrame(false);

	// render all occluders first
	rs->_beginFrame();
	queue->clear();
	for( i = 0; i < mOccluders.size(); i++ )
		mOccluders[i]->_findVisibleObjects(cam, queue, true, 
			mDisplayNodes, onlyShadowCasters);
	SceneManager::renderVisibleObjectsDefaultSequence();
	queue->clear();

	// disable writing to the color/depth buffers
	rs->_setColourBufferWriteEnabled(false, false, false, false);
	rs->_setDepthBufferWriteEnabled(false);

	// render portals and issue occlusion queries
	for( itp = mPortals.begin(); itp != mPortals.end(); itp++ )
	{
		// put it only if it's finished
		if (!itp->second.query->isStillOutstanding())
		{
			itp->second.query->beginOcclusionQuery();
			SceneNode *n = itp->second.node;

			n->_findVisibleObjects(cam, queue, true, 
				mDisplayNodes, onlyShadowCasters);
	
			SceneManager::renderVisibleObjectsDefaultSequence();


			itp->second.query->endOcclusionQuery();
			queue->clear();
		}
	}
	
	rs->_endFrame();

	// re-enable writing to the color/depth buffers
	rs->_setColourBufferWriteEnabled(true, true, true, true);
	rs->_setDepthBufferWriteEnabled(true);

	rs->clearFrameBuffer(FBT_DEPTH | FBT_COLOUR);

	/*
	now for all visible cells ask all their scene managers to find their
	visible objects
	*/
	for( itc = mCells.begin(); itc != mCells.end(); itc++ )
	{
		if( itc->second.sm )
			itc->second.sm->_findVisibleObjects(cam, onlyShadowCasters);

		// render visible movers
		std::map<unsigned int, SceneNode*>::iterator itm;
		for(itm = itc->second.movers.begin(); itm != itc->second.movers.end(); itm++)
		{
			std::map<unsigned int, Mover>::iterator it_M;
			it_M = mMovers.find(itm->first);

			// only if the mover exist
			if(it_M != mMovers.end())
			{
				// check to see if it was already rendered
				if(it_M->second.token != mToken)
				{
					it_M->second.token = mToken;
					it_M->second.node->_findVisibleObjects(cam, queue, true, 
						mDisplayNodes, onlyShadowCasters);
				}
			}
		}
	}


	// in case we want to debug the visibility of the scene
	if( mShowVisiblePortals )
	{
		for( itp = mPortals.begin(); itp != mPortals.end(); itp++ )
		{
			if ( itp->second.isVisible ) {
				itp->second.node->_findVisibleObjects(cam, queue, true, 
				mDisplayNodes, onlyShadowCasters);
			}
		}
	}

	// in case we want to debug the visibility of the scene
	if( mShowVisibleCells )
	{
		for( itc = mCells.begin(); itc != mCells.end(); itc++ )
		{
			if ( itc->second.isVisible ) {
				itc->second.node->_findVisibleObjects(cam, queue, true, 
				mDisplayNodes, onlyShadowCasters);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::setCellSceneManager(int id, SceneManager *sm)
{
	if(mCells.find(id) == mCells.end())
		throw "invalid cell index";
	mCells[id].sm = sm;
}

//-----------------------------------------------------------------------------
SceneManager* HybridPortalBspSceneManager::getCellSceneManager(int id)
{
	if(mCells.find(id) == mCells.end())
		throw "invalid cell index";
	return mCells[id].sm;
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
void HybridPortalBspSceneManager::_setBspObject(const BspObject &obj)
{
	mBspObject = obj;
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::_setCellPortals(int cellId, const std::vector<int> &portalId)
{
	if (cellId < 0 || cellId >= static_cast<int>(mCells.size()))
		throw "Cell index out of range";

	mCells[cellId].portals = portalId;
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::createMoverSceneNode(const String &name)
{
	Mover mover;
	
	mover.node = mMoversRootSceneNode->createChildSceneNode(name);
	mover.token = 0;
	
	mMovers[reinterpret_cast<unsigned int>(mover.node)] = mover;

	updateMoverSceneNode(mover.node);

	return mover.node;
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::updateMoverSceneNode(SceneNode *node)
{
	// first remove the mover from all the scene nodes it is in
	unsigned int key = reinterpret_cast<unsigned int>(node);

	// using iterators for mover as we access it many times (search for it one time)
	std::map<unsigned int, Mover>::iterator itm;

	// first try to find if the mover exists
	itm = mMovers.find(key);
	if(itm == mMovers.end())
		throw "mover doesn't exist";

	size_t numCells = itm->second.cells.size();
	for(size_t i = 0; i < numCells; i++)
		mCells[itm->second.cells[i]].movers.erase(key);
	
	// clear all cells
	itm->second.cells.resize(0);

	// next get the node cell
	int cellId = mBspObject.getPointCell(node->getPosition());
	if ( cellId != - 1)
	{
		mCells[cellId].movers[key] = node;

		// update the list of cells this mover has
		itm->second.cells.push_back(cellId);
	}
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::_setPortalCells(int portalId, int cellId1, int cellId2)
{
	if (portalId < 0 || portalId >= static_cast<int>(mPortals.size()))
		throw "portal index out of range";
	mPortals[portalId].cells[0] = cellId1;
	mPortals[portalId].cells[1] = cellId2;
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::_createPortalSceneNode(int id, const String &name)
{
	if(mPortals.find(id) != mPortals.end())
		throw "portal index already exists";

	Portal portal;

	portal.isVisible = true;
	portal.cells[0] = portal.cells[1] = -1;
	portal.node = mPortalsRootSceneNode->createChildSceneNode(name);
	portal.query = Root::getSingleton().getRenderSystem()->createHardwareOcclusionQuery();

	mPortals[id] = portal;

	return portal.node;
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::_createOccluderSceneNode(const String &name)
{
	SceneNode* n = mOccludersRootSceneNode->createChildSceneNode(name);

	mOccluders.push_back(n);

	return n;
}

//-----------------------------------------------------------------------------
SceneNode* HybridPortalBspSceneManager::_createCellSceneNode(int id, const String &name)
{
	if(mCells.find(id) != mCells.end())
		throw "cell index is already in use";

	Cell cell;

	cell.isVisible = true;
	cell.sm = NULL;
	cell.node = mCellsRootSceneNode->createChildSceneNode(name);

	mCells[id] = cell;

	return cell.node;
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::resetAllInternals()
{	
	// clear all the objects
	clearScene();

	// and recreate the root nodes
	mOccludersRootSceneNode = getRootSceneNode()->createChildSceneNode("OCCLUDERS_ROOT");
	mPortalsRootSceneNode = getRootSceneNode()->createChildSceneNode("PORTALS_ROOT");
	mMoversRootSceneNode = getRootSceneNode()->createChildSceneNode("MOVERS_ROOT");
	mCellsRootSceneNode = getRootSceneNode()->createChildSceneNode("CELLS_ROOT");

	// delete all occlusion queries
	for ( std::map<int, Portal>::iterator i = mPortals.begin(); i != mPortals.end(); i++ )
		delete i->second.query;

	// clear
	mPortals.clear();
	mCells.clear();
	mMovers.clear();
	mOccluders.resize(0);
}

//-----------------------------------------------------------------------------
void HybridPortalBspSceneManager::loadLevel(HybridPortalBspSceneLoader *loader, const String &sceneName, const String &groupName)
{
	this->resetAllInternals();
	loader->_load(this, sceneName, groupName);
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
