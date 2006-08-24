#ifndef __BSP__H__
#define __BSP__H__
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

namespace Ogre {

#define HPBSP_EPSILON	(1.0f / 128.0f)

	/**	BSP friendly polygon

	@remarks
		BspPolygon is always a convex polygons since polygons inside a BSP tree are
		always convex. It only includes vertices and no indices. If portalId < 0 then
		this means that this a solid occluder polygon.
	*/
	struct BspPolygon
	{
		/// polygon cell Id
		int							cellId;

		/// portal Id (if it's not a portal polygon this must be < 0)
		int							portalId;

		/// polygon's plane
		Plane						p;

		/// vertices
		std::vector<Vector3>		verts;

		/// standard constructor
		BspPolygon() : cellId(-1), portalId(-1) {}

		/**	compute heron surface of all triangles in the polygon
		@returns the area of the polygon
		*/
		float surface()
		{
			Vector3	v0, v1, v2;

			v0		= verts[0];

			float	S = 0.0f;

			for (size_t i = 1; i < verts.size() - 1; i++ )
			{
				v1	= verts[i];
				v2	= verts[i + 1];
				Vector3	e0 = v1 - v0;
				Vector3	e1 = v2 - v1;
				Vector3	e2 = v0 - v2;

				float s = 0.5f * (e0.length() + e1.length() + e2.length());
				S += sqrtf(s * (s - e0.length()) * (s - e1.length()) * (s - e2.length()));
			}

			return S;
		} // surface

	}; // struct BspPolygon
	
	/**	BSP tree node

	@remarks
		The Bsp Tree we are using is kindof compact, it doesn't include any polygons
		in nodes or leafs.
	@author
		Wael El Oraiby
	@version
		0.1
	*/
	struct BspNode
	{
		/// node splitter plane
		Plane						splitter;

		/// front node index or cell ( if < 0 it indicates a cell )
		int							frontNode;

		/// back node index or cell ( if < 0 it indicates a cell )
		int							backNode;
	};

	/**	Bsp Object:	this class build or use an existing Bsp tree, it expose some necessary functions
		to get some infromation about where a point lies. (it will build a varient of a solid
		node Bsp tree)
	*/
	class BspObject {
	protected:

		/// nodes of the bsp tree
		std::vector<BspNode>		mNodes;

		/** find the best splitter of a polygon array if it exists
		@param polys input polygon array
		@param splitter best splitter plane found
		@returns the splitter's polygon index
		*/
		int findSplitter(const std::vector<BspPolygon> &polys, Plane &splitter);

		/** build a Bsp node for a given polygon array
		@param inPolys input polygon array
		@param nodeId node index to fill
		*/
		void buildBspNode(const std::vector<BspPolygon> &inPolys, int nodeId);

		/** get the cell id for a given point
		@param v the given point
		@param nodeId the root node id
		@returns the cell of the point (-1 means solid space)
		*/
		int getPointCell(const Vector3 &v, int nodeId);

		/** clip a set of polygons by the Bsp tree
		@param inPolys input polygons
		@param outPolys output clipped polygons
		@param nodeId the root node id
		*/
		void clipPolygons(const std::vector<BspPolygon> &inPolys, std::vector<BspPolygon> &outPolys, int nodeId);

	public:
		/// default constructor
		BspObject() {}
		
		/** construct the Bsp tree of given polygons
		@param faces the polygons
		*/
		BspObject(const std::vector<BspPolygon> &faces);

		/** fill the Bsp tree with the given nodes
		@param nodes the nodes to fill in the tree with
		*/
		BspObject(const std::vector<BspNode> &nodes);
		
		/** clip a set of polygons by the Bsp tree
		@param inPolys input polygons
		@param outPolys output clipped polygons
		*/
		void clipPolygons(const std::vector<BspPolygon> &inPolys, std::vector<BspPolygon> &outPolys);

		/** get the cell id for a given point
		@param v the given point
		@returns the cell of the point (-1 means solid space)
		*/
		int getPointCell(const Vector3 &v);

		/** retrive all nodes of the Bsp tree (in case we want to save them)
		@param outNodes the nodes of the tree
		*/
		void getBspNodes(std::vector<BspNode> &outNodes);
	};

}; // namespace Ogre
#endif