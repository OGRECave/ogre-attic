#ifndef __Decomposer__H__
#define __Decomposer__H__
/*
** This source file is part of OGRE (Object-oriented Graphics Rendering Engine)
** For the latest info, see http://www.ogre3d.org/
**
** OGRE Copyright goes for Ogre Team
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

#include "BspObject.h"

namespace Ogre {

	/** decompose a mesh given as an array of BspPolygon(s)

	@remarks The algorithm is straight forward, just pick a vertex and walk all
		the polygons that share this vertex. If the output from the CSG operation
		is ok, the polygons will be colored (each assigned to a respective cell)
	@author
		Wael El Oraiby
	@version
		0.1
	*/
	class Decomposer {
	protected:
		
		struct Vec3Less {
			bool operator()(const Vector3 v0, const Vector3 v1) const
			{
				if (fabs(v0.x - v1.x) > HPBSP_EPSILON)
					return (v0.x < v1.x);
                if (fabs(v0.y - v1.y) > HPBSP_EPSILON)
					return (v0.y < v1.y);
				if (fabs(v0.z - v1.z) > HPBSP_EPSILON)
					return (v0.z < v1.z);
				return false;
			}
		};

		struct Vertex {
			std::vector<int>	polys;
			int					cellId;

			Vertex(): cellId(-1) {}
		};

		/// vertex map iterator
		typedef std::map<Vector3, Vertex, Vec3Less>::iterator v_iter;

		/// vertex map
		std::map<Vector3, Vertex, Vec3Less>	mVerts;

		/// polygons
		std::vector<BspPolygon>			mPolys;

		/** add a polygon to the polygon list of a vertex
		@param vertex the vertex to add
		@param polyId polygon index to add
		*/
		void addVertex(const Vector3 &vertex, int polyId);

		/** walk a vertex assigning all polygons to the same cell
		@param vertex the vertex to go through
		@param cellId the cell to assign to polygons
		*/
		void walkVertex(const Vector3 &vertex, int cellId);

	public:

		/// default constructor
		Decomposer();

		/** decompose the polygons (better say color them) into different cells
		@param inPolys polygons to be colored
		*/
		int decompose(std::vector<BspPolygon> &inPolys);
	}; // class Decomposer
}; // namespace HybridProcLib

#endif