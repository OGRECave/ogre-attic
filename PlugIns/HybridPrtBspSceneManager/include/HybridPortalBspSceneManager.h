#ifndef __HybridPortalBspSceneManager__H__
#define __HybridPortalBspSceneManager__H__
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

namespace Ogre {

	class HybridPortalBspSceneLoader;
	class HybridPortalBspSceneManager;

	/**
	Hybrid Portal/BSP based scene manager.

	This scene manager uses a BSP tree to figure out where the camera/nodes are,
	it then uses occlusion queries on portals to figure out which portal is
	visible and then which cell. Cells that are visible are then drawn, as well
	as objects spanning portals.

	@author
		Wael El Oraiby
	@version
		1.0
	*/
	class HybridPortalBspSceneManager : public SceneManager {
	public:

		/// default constructor
		HybridPortalBspSceneManager(const String &instanceName);

		/// default destructor
		virtual ~HybridPortalBspSceneManager();

		/// find the visible nodes
		virtual void _findVisibleObjects(Camera *cam, bool onlyShadowCasters);

		/// get the scene manager type name
		virtual const String& getTypeName(void) const;

		/**
		get the portal count
		@returns portal count
		*/
		virtual int getPortalCount();

		/**
		get the cell count
		@returns cell count
		*/
		virtual int getCellCount();

		/**
		get the occluder count
		@returns occluder count
		*/
		virtual int getOccluderCount();

		/**
		get a portal SceneNode
		@param portalIdx portal index
		@returns portal SceneNode pointer
		*/
		virtual SceneNode* getPortalSceneNode(int portalIdx);

		/**
		get a cell SceneNode
		@param cellIdx cell index
		@returns cell SceneNode pointer
		*/
		virtual SceneNode* getCellSceneNode(int cellIdx);

		/**
		load an hybrid portal bsp scene using a level editor
		@param sceneLoader the scene loader to use
		@param sceneName the scene name
		*/
		virtual void loadLevel(HybridPortalBspSceneLoader *loader, const String &sceneName, const String &groupName);

		/**
		create the portal scene node
		@param id portal id
		@param name portal name
		@returns the portal scene node
		*/
		virtual SceneNode* _createPortalSceneNode(int id, const String &name);

		/**
		create the occluder scene node
		@param id occluder id
		@param name occluder name
		@returns the occluder scene node
		*/
		virtual SceneNode* _createOccluderSceneNode(const String &name);

		/**
		create the cell scene node
		@param id cell id
		@param name cell name
		@returns the cell scene node
		*/
		virtual SceneNode* _createCellSceneNode(int id, const String &name);

		/**
		set the bsp object
		@param obj BspObject
		*/
		virtual void _setBspObject(const BspObject &obj);

		/**
		set the portals cells
		@param portalId portal id
		@param cellId1 first cell id (-1 means outside/skybox)
		@param cellId2 second cell id (-1 means outside/skybox)
		*/
		virtual void _setPortalCells(int portalId, int cellId1, int cellId2);

		/**
		adds a portal to a cell
		@param cellId cell id
		@param portalId portal id
		*/
		virtual void _setCellPortals(int cellId, const std::vector<int> &portalId);

		/**
		get portal cells
		@param portalId the portal index
		@param firstCell the first adjacent cell
		@param secondCell the second adjacent cell
		*/
		virtual void getPortalCells(int portalId, int &firstCell, int &secondCell);

		/**
		get the cell portals
		@param cellId the cell index
		@param outPortals the portals array
		*/
		virtual void getCellPortals(int cellId, std::vector<int> &outPortals);

		/**
		gets the occluders root scene node
		@returns occluders root scene node
		*/
		virtual SceneNode* getOccludersRootSceneNode();

		/**
		gets the portals root scene node
		@returns portals root scene node
		*/
		virtual SceneNode* getPortalsRootSceneNode();

		/**
		gets the Movers root scene node
		@returns Movers root scene node
		*/
		virtual SceneNode* getMoversRootSceneNode();

		/**
		get the current point cell
		@param point the point
		@returns cell index
		*/
		virtual int getPointCell(const Vector3 &point);

		/**
		get neighbouring cells
		@param cellId the cell index
		@param outNeighbors the neighbouring cells
		*/
		virtual void getNeighboringCells(int cellId, std::vector<int> &outNeighbors);

		/**
		get neighboring portals
		@param portalId the portal index
		@param outNeighbors the neighboring portals
		*/
		virtual void getNeighboringPortals(int portalId, std::vector<int> &outNeighbors);

		/**
		reset all internals
		*/
		virtual void resetAllInternals();

	protected:
		/// the root of all occluders
		SceneNode* mOccludersRootSceneNode;

		/// the root of all portals
		SceneNode* mPortalsRootSceneNode;

		/// the root of all movers
		SceneNode* mMoversRootSceneNode;

		/// the root for all cells
		SceneNode* mCellsRootSceneNode;

		/// BspObject to determine where a point spacially lies (which cell)
		BspObject mBspObject;

		/**
		Portal information/data place holder
		*/
		struct Portal {
			/// portal mesh node
			SceneNode* node;

			/// portal occlusion query
			HardwareOcclusionQuery* query;

			/// list of spanning nodes
			std::map<unsigned int, SceneNode*> spanningNodes;

			/// the cells this portal connects (-1 means skybox)
			int cells[2];

			/// visibility of the portal
			bool isVisible;
		};

		/**
		Cell information/data place holder
		*/
		struct Cell {
			/// cell mesh node
			SceneNode* node;

			/// list of movers
			std::map<unsigned int, SceneNode*> moverObjecs;

			// cell SceneManager and holder of the globally static(localy mover) objects
			SceneManager* sm;

			/// the portals connected to this cell
			std::vector<int> portals;

			/// visibility of the frame
			bool isVisible;
		};

		/// portals
		std::map<int, Portal> mPortals;

		/// cells
		std::map<int, Cell> mCells;

		/// occluders
		std::vector<SceneNode*> mOccluders;
	};

	//----------------------------------------------------------------------------

	/** This is the hybrid portal/bsp level loader.

	@remarks
		this is a pure virtual class and should be implemented.
	@note
		For the moment, we will only handle one BspObject per scene,
		this is going to change later, although 1 object is going to suffice in
		most cases.
	@author
		Wael El Oraiby
	@version
		1.0
	*/
	class HybridPortalBspSceneLoader
	{
	public:
		/// constructor
		HybridPortalBspSceneLoader() {}

		/// destructor
		virtual ~HybridPortalBspSceneLoader() {};

		/**
		load a dotScene file
		@param sceneMgr Reference to the generic scene manager we are going to use
		@param fileName The file name of the scene
		@param groupName the group name of the file
		*/		
		virtual void _load(HybridPortalBspSceneManager *sm, const String &fileName, const String &groupName) = 0;
	};

	//-------------------------------------------------------------------------

	/** Factory for HybridPortalBspSceneManager

	this class will create an instance of the hybrid portal bsp scene manager
	*/
	class HybridPortalBspSceneManagerFactory : public SceneManagerFactory
	{
	protected:
		void initMetaData(void) const;
	public:
		HybridPortalBspSceneManagerFactory() {}
		~HybridPortalBspSceneManagerFactory() {}
		/// Factory type name
		static const String FACTORY_TYPE_NAME;
		SceneManager* createInstance(const String& instanceName);
		void destroyInstance(SceneManager* instance);
	};
};

#endif