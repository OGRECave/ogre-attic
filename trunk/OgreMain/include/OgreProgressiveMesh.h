/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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
// The underlying algorithms in this class are based heavily on:
/*
 *  Progressive Mesh type Polygon Reduction Algorithm
 *  by Stan Melax (c) 1998
 */

#ifndef __ProgressiveMesh_H_
#define __ProgressiveMesh_H_

#include "OgrePrerequisites.h"
#include "OgreVector3.h"

namespace Ogre {

    /** This class reduces the complexity of the geometry it is given.
        This class is dedicated to reducing the number of triangles in a given mesh
        taking into account seams in both geometry and texture co-ordinates and meshes 
        which have multiple frames.
    @par
        The primary use for this is generating LOD versions of Mesh objects, but it can be
        used by any geometry provider. The only limitation at the moment is that the 
        provider uses a common vertex buffer for all LODs and one index buffer per LOD.
        Therefore at the moment this class can only handle indexed geometry.
    @par
        NB the interface of this class will certainly change when compiled vertex buffers are
        supported.
    */
    class _OgreExport ProgressiveMesh
    {
    public:

        /// Struct for holding the returned LOD geometry information
        struct LODGeometryData
        {
            ushort numIndexes;
            ushort* pIndexes;
        };

        typedef std::vector<LODGeometryData> LODGeometryList;

        /** Constructor, takes the geometry data and index buffer. */
        ProgressiveMesh(GeometryData* data, ushort* indexBuffer, ushort numIndexes);
        virtual ~ProgressiveMesh();

        /** Adds an extra vertex position buffer. 
        @remarks
            As well as the main vertex buffer, the client of this class may add extra versions
            of the vertex buffer which will also be taken into account when the cost of 
            simplifying the mesh is taken into account. This is because the cost of
            simplifying an animated mesh cannot be calculated from just the reference position,
            multiple positions needs to be assessed in order to find the best simplification option.
        @param buffer Pointer to x/y/z buffer with vertex positions. The number of vertices
            must be the same as in the original GeometryData passed to the constructor.
        */
        virtual void addExtraVertexPositionBuffer(Real* buffer);

        /** Builds the progressive mesh with the specified number of levels.
        @param numLevels The number of levels to include in the mesh including the full detail version.
            Each level will have half as many vertices as the previous one.
        @param outList Pointer to a list of LOD geometry data which will be completed by the application
        */
        virtual void build(ushort numLevels, LODGeometryList* outList);

    protected:
        GeometryData* mpGeomData;
        ushort* mpIndexBuffer;
        ushort mNumIndexes;
        ushort mCurrNumIndexes;

        // Internal classes
        class PMTriangle;
        class PMVertex;
        /** A triangle in the progressive mesh, holds extra info like face normal. */
        class PMTriangle {
        public:
            PMTriangle();
            void setDetails(PMVertex *v0, PMVertex *v1, PMVertex *v2);
	        void computeNormal(void);
	        void replaceVertex(PMVertex *vold, PMVertex *vnew);
	        bool  hasVertex(PMVertex *v);
	        void notifyRemoved(void);

	        PMVertex* vertex[3]; // the 3 points that make this tri
	        Vector3   normal;    // unit vector othogonal to this face
            bool      removed;   // true if this tri is now removed
        };

        /** A vertex in the progressive mesh, holds info like collapse cost etc. */
        class PMVertex {
        public:
            PMVertex();
	        void setDetails(const Vector3& v, int index);
	        void removeIfNonNeighbor(PMVertex *n);
	        bool isBorder(void);
            void notifyRemoved(void);

            Vector3  position;  // location of point in euclidean space
	        ushort index;       // place of vertex in original list
            typedef std::set<PMVertex *> NeighborList;
            NeighborList neighbor; // adjacent vertices
	        typedef std::set<PMTriangle *> FaceList;
            FaceList face;     // adjacent triangles

	        Real collapseCost;  // cached cost of collapsing edge
	        PMVertex * collapseTo; // candidate vertex for collapse
            bool      removed;   // true if this vert is now removed
        };

        typedef std::vector<PMTriangle> TriangleList;
        typedef std::vector<PMVertex> VertexList;
        typedef std::vector<Real> WorstCostList;

        /// Data used to calculate the collapse costs
        struct PMWorkingData
        {
            TriangleList mTriList; /// List of faces
            VertexList mVertList; // The vertex details referenced by the triangles
        };

        typedef std::vector<PMWorkingData> WorkingDataList;
        /// Multiple copies, 1 per vertex buffer
        WorkingDataList mWorkingData;

        /// The worst collapse cost from all vertex buffers for each vertex
        WorstCostList mWorstCosts;

        /// Internal method for building PMWorkingData from geometry data
        void addWorkingData(Real* pPositions, GeometryData* data, ushort* indexBuffer, ushort numIndexes);

        /// Internal method for initialising the edge collapse costs
        void initialiseEdgeCollapseCosts(void);
        /// Internal calculation method for deriving a collapse cost  from u to v
        Real computeEdgeCollapseCost(PMVertex *src, PMVertex *dest);
        /// Internal method evaluates all collapse costs from this vertex and picks the lowest for a single buffer
        Real computeEdgeCostAtVertexForBuffer(WorkingDataList::iterator idata, ushort vertIndex);
        /// Internal method evaluates all collapse costs from this vertex for every buffer and returns the worst
        void computeEdgeCostAtVertex(ushort vertIndex);
        /// Internal method to compute edge collapse costs for all buffers /
        void computeAllCosts(void);
        /// Internal method for getting the index of next best vertex to collapse
        ushort getNextCollapser(void);
        /// Internal method builds an new LOD based on the current state
        void bakeNewLOD(LODGeometryData* pData);

        /** Internal method, collapses vertex onto it's saved collapse target. 
        @remarks
            This updates the working triangle list to drop a triangle and recalculates
            the edge collapse costs around the collapse target. 
            This also updates all the working vertex lists for the relevant buffer. 
        */
        void collapse(PMVertex *collapser);








    };



}

#endif 
