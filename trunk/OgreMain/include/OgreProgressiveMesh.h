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
        @param ppIdxBuffers Pointer to an list of index buffer pointers, there must be 
            space for numLevels - 1 pointers here. This method will allocate the memory for
            these buffers, but the caller has responsibility for freeing them.
        */
        virtual void build(ushort numLevels, ushort** ppIdxBuffers);

    protected:
        GeometryData* mpGeomData;
        ushort* mpIndexBuffer;
        ushort mNumIndexes;

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
	        void setDetails(Vector3 v, int index);
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

        struct PMWorkingData
        {
            TriangleList mTriList;
            VertexList mVertList;
        };

        typedef std::vector<PMWorkingData> WorkingDataList;
        /// Multiple copies, 1 per vertex buffer
        WorkingDataList mWorkingData;

        /* Map ordered by max cost of each vertex in each buffer.
            By finding the least max cost we find the best collapse
            option for all buffers.
        */
        typedef std::map<Real, ushort> LeastMaxCostMap;
        LeastMaxCostMap mLeastMaxCostMap;

        /// Internal method for building PMWorkingData from geometry data
        void addWorkingData(Real* pPositions, GeometryData* data, ushort* indexBuffer, ushort numIndexes);

        /// Internal calculation method for deriving a collapse cost  from u to v
        Real computeEdgeCollapseCost(PMVertex *src, PMVertex *dest);
        /// Internal method evaluates all collapse costs from this vertex and picks the lowest
        void computeEdgeCostAtVertex(ushort vertIndex);
        /// Internal method for calculating all edge collapse costs for a buffer
        void computeAllEdgeCollapseCostsForBuffer(VertexList* buf);
        /** Internal method to compute edge collapse costs for all buffers and build the
            initial 'least max cost' map. */
        void computeEdgeCollapseCostsForAllBuffers(void);

        /** Internal method, collapses vertex at index srcIdx onto destIdx. 
        @remarks
            This updates the working triangle list to drop a triangle and recalculates
            the edge collapse costs around destIdx. This also updates all the working 
            vertex lists. Note that the vertex srcIdx and a face are deleted but their
            pointer position in the vertlist and trilist are still maintained to enable
            the indices to stay the same.
        */
        void collapse(ushort srcIdx, ushort destIdx);








    };



}

#endif 
