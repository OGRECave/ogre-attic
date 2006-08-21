#ifndef __HpBspDotSceneLoader__H__
#define __HpBspDotSceneLoader__H__
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
#include "tinyxml.h"

#include "BspObject.h"
#include "Decomposer.h"
#include "HybridPortalBspSceneManager.h"

#define PORTAL_NAME_PREFIX		"PORTAL_"
#define OCCLUDER_NAME_PREFIX	"OCCLUDER_"

namespace Ogre
{
	/**
	dotScene hybrid portal bsp loader

	@remarks
		It will load a dotScene file, performs CSG between the occluders and
		the portals and then build the final BSP for the final processed object.
	@note
		For the moment, we will only handle one HybridPortalBSP object per scene,
		this is going to change later, although 1 object is going to suffice in
		most cases.
	@author
		Wael El Oraiby
	@version
		0.1
	*/
	class HpBspDotSceneLoader : public HybridPortalBspSceneLoader
	{
	public:
		/// destructor
		~HpBspDotSceneLoader();

		/**
		load a dotScene file
		@param sceneMgr Reference to the generic scene manager we are going to use
		@param fileName The file name of the scene
		@param groupName the group name of the file
		*/		
		virtual void _load(HybridPortalBspSceneManager *sm, const String &fileName, const String &groupName);

		/**
		get the singleton class
		@returns singleton class
		*/
		virtual HpBspDotSceneLoader* getSingleton();

	protected:
		/// constructor
		HpBspDotSceneLoader();

		/**
		reset internal variables
		*/
		virtual void resetInternals();

		/**
		load a dotScene file
		@param XMLNode Reference to the XML node
		@param pAttach the node to attache the object to
		*/
		virtual void processNode(TiXmlElement *XMLNode,	SceneNode *pAttach);

		/** transform a node's mesh into a set of Bsp polygons
		@param node the node to transform
		@param polys the resulting bsp polygons
		@param cellId associated cell index
		@param portalId associated portal index (-1 means occluder)
		*/
		virtual void buildNodeBspPolys(SceneNode *node, std::vector<BspPolygon> &polys, int cellId, int portalId);

		/** build an Ogre mesh given a set of Bsp polygons (this is not really needed now but it is
			a good debugging feature)
		@param meshName the mesh name to be given
		@param polys input bsp polygons
		@param cellId associated cell index (usefull for material)
		@param portalId associated portal index (usefull for material)
		*/
		virtual void buildMesh(const String &meshName, const std::vector<BspPolygon> &polys, int cellId, int portalId);

		/** extract connectivity information
		@param polys input polygon soup (should be colored already)
		@param numPortals number of portals
		@param numCells number of cells
		*/
		virtual void extractConnectivity(const std::vector<BspPolygon> &polys, int numPortals, int numCells);

		/**
		build the entire loaded scene, it will build the Bsp for the occluder, portals and
		decompose the resulting polygons into different cells
		*/
		virtual void buildScene();

		/// scene manager
		HybridPortalBspSceneManager* mSM;

		/// BSP of the resulting CSG between portals and occluders
		BspObject mBspCSG;

		/// BSP of occluders
		BspObject mBspOccluders;

		/// BSP of portals
		BspObject mBspPortals;		

		/// CSG result mesh
		SceneNode* mCSG;

		/// movers parent scene node
		SceneNode* mParentNode;

		/// original occluders
		std::vector<SceneNode*> mOrigOccluders;

		/// original portals
		std::vector<SceneNode*> mOrigPortals;

		/**
		portal information structure: contains the 2 adjacent cells which the
		portal joins together
		*/
		struct Portal
		{
			/// cell indices, -1 means the portal links to outside (parent cell/skybox?)
			int cellsId[2];

			/// pointer to portal node
			SceneNode* portal;
		};

		/**
		Holds the portal ids that connect the cell to the other neighbouring cells
		*/
		struct Cell
		{
			/// portals
			std::vector<int> portals;

			/// pointer to cell node
			SceneNode* cell;
		};

		/// cells
		std::vector<Cell> mCells;

		/// portals
		std::vector<Portal> mPortals;

	};

};

#endif
