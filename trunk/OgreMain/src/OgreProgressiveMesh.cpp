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

// The algorithm in this file is based heavily on:
/*
 *  Progressive Mesh type Polygon Reduction Algorithm
 *  by Stan Melax (c) 1998
 */

#include "OgreProgressiveMesh.h"
#include "OgreGeometryData.h"

namespace Ogre {
    //---------------------------------------------------------------------
    ProgressiveMesh::ProgressiveMesh(GeometryData* data, 
        ushort* indexBuffer, ushort numIndexes)
    {
        addWorkingData(data->pVertices, data, indexBuffer, numIndexes);
        mpGeomData = data;
        mpIndexBuffer = indexBuffer;
        mNumIndexes = numIndexes;

    }
    //---------------------------------------------------------------------
    ProgressiveMesh::~ProgressiveMesh()
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::addExtraVertexPositionBuffer(Real* buffer)
    {
        addWorkingData(buffer, mpGeomData, mpIndexBuffer, mNumIndexes);
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::build(ushort numLevels, ushort** ppIdxBuffers)
    {
        computeEdgeCollapseCostsForAllBuffers();

        
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::addWorkingData(Real* pPositions, GeometryData* data, 
        ushort* indexBuffer, ushort numIndexes)
    {
    }
    //---------------------------------------------------------------------
    Real ProgressiveMesh::computeEdgeCollapseCost(PMVertex *src, PMVertex *dest)
    {
        // TODO
        return 0.0f;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::computeEdgeCostAtVertex(ushort vertIndex)
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::computeAllEdgeCollapseCostsForBuffer(VertexList* buf)
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::computeEdgeCollapseCostsForAllBuffers(void)
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::collapse(ushort srcIdx, ushort destIdx)
    {
    }
    //---------------------------------------------------------------------
    ProgressiveMesh::PMTriangle::PMTriangle() : removed(false)
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::setDetails(PMVertex *v0, PMVertex *v1, 
        PMVertex *v2)
    {
	    assert(v0!=v1 && v1!=v2 && v2!=v0);

        vertex[0]=v0;
	    vertex[1]=v1;
	    vertex[2]=v2;

        computeNormal();

        // Add tri to vertices
        // Also tell vertices they are neighbours
        for(int i=0;i<3;i++) {
		    vertex[i]->face.insert(this);
		    for(int j=0;j<3;j++) if(i!=j) {
			    vertex[i]->neighbor.insert(vertex[j]);
		    }
	    }
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::notifyRemoved(void)
    {
        int i;
	    for(i=0; i<3; i++) {
            // remove this tri from the vertices
		    if(vertex[i]) vertex[i]->face.erase(this);
	    }
	    for(i=0; i<3; i++) {
		    int i2 = (i+1)%3;
		    if(!vertex[i] || !vertex[i2]) continue;
            // Check remaining vertices and remove if not neighbours anymore
            // NB May remain neighbours if other tris link them
		    vertex[i ]->removeIfNonNeighbor(vertex[i2]);
		    vertex[i2]->removeIfNonNeighbor(vertex[i ]);
	    }

        removed = true;
    }
    //---------------------------------------------------------------------
    bool ProgressiveMesh::PMTriangle::hasVertex(PMVertex *v) 
    {
	    return (v==vertex[0] ||v==vertex[1] || v==vertex[2]);
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::computeNormal()
    {
	    Vector3 v0=vertex[0]->position;
	    Vector3 v1=vertex[1]->position;
	    Vector3 v2=vertex[2]->position;
        // Cross-product 2 edges
        Vector3 e1 = v1 - v0; 
        Vector3 e2 = v2 - v1;

        normal = e1.crossProduct(e2);
        normal.normalise();
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::replaceVertex(PMVertex *vold, PMVertex *vnew) 
    {
	    assert(vold && vnew);
	    assert(vold==vertex[0] || vold==vertex[1] || vold==vertex[2]);
	    assert(vnew!=vertex[0] && vnew!=vertex[1] && vnew!=vertex[2]);
	    if(vold==vertex[0]){
		    vertex[0]=vnew;
	    }
	    else if(vold==vertex[1]){
		    vertex[1]=vnew;
	    }
	    else {
		    assert(vold==vertex[2]);
		    vertex[2]=vnew;
	    }
	    int i;
	    vold->face.erase(this);
	    vnew->face.insert(this);
	    for(i=0;i<3;i++) {
		    vold->removeIfNonNeighbor(vertex[i]);
		    vertex[i]->removeIfNonNeighbor(vold);
	    }
	    for(i=0;i<3;i++) {
		    assert(vertex[i]->face.find(this) != vertex[i]->face.end());
		    for(int j=0;j<3;j++) if(i!=j) {
			    vertex[i]->neighbor.insert(vertex[j]);
		    }
	    }
	    computeNormal();
    }
    //---------------------------------------------------------------------
    ProgressiveMesh::PMVertex::PMVertex() : removed(false)
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMVertex::setDetails(Vector3 v, int newindex)
    {
	    position = v;
	    index = newindex;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMVertex::notifyRemoved(void)
    {
        NeighborList::iterator i, iend;
        iend = neighbor.end();
        for (i = neighbor.begin(); i != iend; ++i)
        {
            // Remove me from neighbor
            (*i)->neighbor.erase(this);
	    }
        removed = true;
    }
    //---------------------------------------------------------------------
    bool ProgressiveMesh::PMVertex::isBorder() 
    {
        // Look for edges which only have one tri attached, this is a border

        NeighborList::iterator i, iend;
        iend = neighbor.end();
        // Loop for each neighbor
	    for(i = neighbor.begin(); i != iend; ++i) 
        {
            // Count of tris shared between the edge between this and neighbor
		    ushort count = 0;
            // Loop over each face, looking for shared ones
            FaceList::iterator j, jend;
            jend = face.end();
		    for(j = face.begin(); j != jend; ++j) 
            {
			    if((*j)->hasVertex(*i))
                {
                    // Shared tri
				    count ++;
			    }
		    }
		    assert(count>0); // Must be at least one!
            // This edge has only 1 tri on it, it's a border
		    if(count == 1) return true;
	    }
	    return false;
    } 
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMVertex::removeIfNonNeighbor(PMVertex *n) 
    {
	    // removes n from neighbor list if n isn't a neighbor.
        NeighborList::iterator i = neighbor.find(n);
        if (i == neighbor.end())
            return; // Not in neighbor list anyway

        FaceList::iterator f, fend;
        fend = face.end();
        for(f = face.begin(); f != fend; ++f) 
        {
		    if((*f)->hasVertex(n)) return; // Still a neighbor
	    }

	    neighbor.erase(n);
    }


}
