/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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
#include "OgreString.h"
#include <algorithm>

#include <iostream>

std::ofstream of;

namespace Ogre {
	#define NEVER_COLLAPSE_COST 99999.9f
    //---------------------------------------------------------------------
    ProgressiveMesh::ProgressiveMesh(GeometryData* data, 
        ushort* indexBuffer, ushort numIndexes)
    {
        addWorkingData(data->pVertices, data, indexBuffer, numIndexes);
        mpGeomData = data;
        mpIndexBuffer = indexBuffer;
        mNumIndexes = numIndexes;
        mWorstCosts.resize(data->numVertices);



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
    void ProgressiveMesh::build(ushort numLevels, LODGeometryList* outList, 
			VertexReductionQuota quota, Real reductionValue)
    {
        LODGeometryData newLod;

#if OGRE_DEBUG_MODE
		dumpContents("pm_before.log");
#endif

        computeAllCosts();
        // Init
        mCurrNumIndexes = mNumIndexes;
        ushort numVerts, numCollapses;
        numVerts = mpGeomData->numVertices;
		
		// Bake top-level LOD
        bakeNewLOD(&newLod);
        outList->push_back(newLod);

		PMVertex* test = &(mWorkingData[0].mVertList[347]);

		of.open("progressivemesh.log");

		numCollapses = 0;
		while (numLevels--)
        {
			if (quota == VRQ_PROPORTIONAL)
			{
				numCollapses = numVerts * reductionValue;
			}
			else 
			{
				numCollapses = reductionValue;
			}
            // Minimum 3 verts!
            if ( (numVerts - numCollapses) < 3) 
                numCollapses = numVerts - 3;
			// Store new number of verts
			numVerts = numVerts - numCollapses;

			while(numCollapses--)
            {
                ushort nextIndex = getNextCollapser();
                // Collapse on every buffer
                WorkingDataList::iterator idata, idataend;
                idataend = mWorkingData.end();
                for (idata = mWorkingData.begin(); idata != idataend; ++idata)
                {
                    PMVertex* collapser = &( idata->mVertList.at( nextIndex ) );
                    // This will reduce mCurrNumIndexes and recalc costs as required
					of << "Collapsing index " << collapser->index << "(border: "<< collapser->isBorder() <<
						") to " << collapser->collapseTo->index << "(border: "<< collapser->collapseTo->isBorder() <<
						std::endl;

					assert(collapser->collapseTo->removed == false);

                    collapse(collapser);
                }
            }
#if OGRE_DEBUG_MODE
			char logname[20];
			sprintf(logname, "pm_level%d.log", numLevels);
			dumpContents(logname);
#endif

            // Bake a new LOD and add it to the list
            bakeNewLOD(&newLod);
            outList->push_back(newLod);
			

        }



    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::addWorkingData(Real* pPositions, GeometryData* data, 
        ushort* indexBuffer, ushort numIndexes)
    {
        // Insert blank working data, then fill 
        mWorkingData.push_back(PMWorkingData());

        PMWorkingData& work = mWorkingData.back();

        uint i;
        // Build vertex list
        work.mVertList.resize(data->numVertices);
        Real* pReal = data->pVertices;
        Vector3 pos;
        for (i = 0; i < data->numVertices; ++i)
        {
            pos.x = *pReal++;
            pos.y = *pReal++;
            pos.z = *pReal++;
            work.mVertList[i].setDetails(pos, i);
            work.mVertList[i].removed = false;
        }

        // Build tri list
        ushort numTris = numIndexes / 3;
        ushort* pIdx = indexBuffer;
        work.mTriList.resize(numTris); // assumed tri list
        for (i = 0; i < numTris; ++i)
        {
			PMVertex *v0, *v1, *v2;
			ushort vindex = *pIdx++;
			v0 = &(work.mVertList[vindex]);
			vindex = *pIdx++;
			v1 = &(work.mVertList[vindex]);
			vindex = *pIdx++;
			v2 = &(work.mVertList[vindex]);

			work.mTriList[i].setDetails(i, v0, v1, v2);

			/*
            work.mTriList[i].setDetails(
                &(work.mVertList[*pIdx++]),
                &(work.mVertList[*pIdx++]),
                &(work.mVertList[*pIdx++]) );
			*/

            work.mTriList[i].removed = false;

        }


		// Scan for border vertices which are connected to others
		// This happens when we need duplicate verts to hold different texture coords etc at seams
        for (i = 0; i < data->numVertices; ++i)
        {
			PMVertex *thisVert = &(work.mVertList[i]);
			if (thisVert->isBorder())
			{
				// Look for other borders in the same place
				int j;
				for (j = 0; j < data->numVertices; ++j)
				{
					PMVertex* otherVert = &(work.mVertList[j]);
					if (thisVert != otherVert &&
						otherVert->isBorder() && 
						otherVert->position == thisVert->position)
					{
						// Link together
						otherVert->borderJoined.insert(thisVert);
						thisVert->borderJoined.insert(otherVert);

						// Copy neighbour info both ways
						otherVert->neighbor.insert(
							thisVert->neighbor.begin(), thisVert->neighbor.end());
						thisVert->neighbor.insert(
							otherVert->neighbor.begin(), otherVert->neighbor.end());
						// Copy face links both ways
						otherVert->face.insert(
							thisVert->face.begin(), thisVert->face.end());
						thisVert->face.insert(
							otherVert->face.begin(), otherVert->face.end());

					}

				}
			}
		}


    }
    //---------------------------------------------------------------------
    Real ProgressiveMesh::computeEdgeCollapseCost(PMVertex *src, PMVertex *dest)
    {
        // if we collapse edge uv by moving src to dest then how 
        // much different will the model change, i.e. how much "error".
        // The method of determining cost was designed in order 
        // to exploit small and coplanar regions for
        // effective polygon reduction.
        Vector3 edgeVector = src->position - dest->position;

        Real cost;
		Real curvature = 0.001f;

        // find the "sides" triangles that are on the edge uv
        PMVertex::FaceList sides;
        PMVertex::FaceList::iterator srcface, srcfaceEnd;
        srcfaceEnd = src->face.end();
        // Iterate over src's faces and find 'sides' of the shared edge which is being collapsed
        for(srcface = src->face.begin(); srcface != srcfaceEnd; ++srcface)
        {
            // Check if this tri also has dest in it (shared edge)
            if( (*srcface)->hasVertex(dest) )
            {
                sides.insert(*srcface);
            }
        }

		// Special cases
		// If we're looking at a border vertex
        if(src->isBorder())
        {
			if (sides.size() > 1) 
			{
				// src is on a border, but the src-dest edge has more than one tri on it
				// So it must be collapsing inwards
				// Mark as very high-value cost
				//curvature = 1.0f;
				cost = NEVER_COLLAPSE_COST;
			}
			else
			{
				// Collapsing ALONG a border
				// We can't use curvature to measure the effect on the model
				// Instead, see what effect it has on 'pulling' the other border edges
				// The more colinear, the less effect it will have
				// So measure the 'kinkiness' (for want of a better term)
				// Normally there can be at most 1 other border edge attached to this
				// However in weird cases there may be more, so find the worst
				Vector3 otherBorderEdge;
				Real kinkiness, maxKinkiness;
				PMVertex::NeighborList::iterator n, nend;
				nend = src->neighbor.end();
				maxKinkiness = 0.0f;
				for (n = src->neighbor.begin(); n != nend; ++n)
				{
					if (*n != dest && (*n)->isBorder())
					{
						otherBorderEdge = src->position - (*n)->position;
						// This time, the nearer the dot is to -1, the better, because that means
						// the edges are opposite each other, therefore less kinkiness
						// Scale into [0..1]
						kinkiness = (otherBorderEdge.dotProduct(edgeVector) + 1.002f) * 0.5f;
						maxKinkiness = std::max(kinkiness, maxKinkiness);

					}
				}

				cost = maxKinkiness + 0.1; // bias a little to prefer inner verts

			}
        } 
		else // not a border
		{

			// Standard inner vertex
			// Calculate curvature
			// use the triangle facing most away from the sides 
			// to determine our curvature term
			// Iterate over src's faces again
			for(srcface = src->face.begin(); srcface != srcfaceEnd; ++srcface) 
			{
				Real mincurv = 1.0f; // curve for face i and closer side to it
				// Iterate over the sides
				PMVertex::FaceList::iterator sidesFace, sidesFaceEnd;
				sidesFaceEnd = sides.end();
				for(sidesFace = sides.begin(); sidesFace != sidesFaceEnd; ++sidesFace) 
				{
					// Dot product of face normal gives a good delta angle
					Real dotprod = (*srcface)->normal.dotProduct( (*sidesFace)->normal );
					// NB we do (1-..) to invert curvature where 1 is high curvature [0..1]
					// Whilst dot product is high when angle difference is low
					mincurv =  std::min(mincurv,(1.002f - dotprod) * 0.5f);
				}
				curvature = std::max(curvature, mincurv);
			}
			cost = curvature;
		}

		// Degenerate case check
		// Are we going to invert a face normal of one of the neighbouring faces?
		// Can occur when we have a very small remaining edge and collapse crosses it
		// Look for a face normal changing by > 90 degrees
		for(srcface = src->face.begin(); srcface != srcfaceEnd; ++srcface) 
		{
			// Ignore the deleted faces (those including src & dest)
			if( !(*srcface)->hasVertex(dest) )
			{
				// Test the new face normal
				PMVertex *v0, *v1, *v2;
				// Replace src with dest wherever it is
				v0 = ( (*srcface)->vertex[0] == src) ? dest : (*srcface)->vertex[0];
				v1 = ( (*srcface)->vertex[1] == src) ? dest : (*srcface)->vertex[1];
				v2 = ( (*srcface)->vertex[1] == src) ? dest : (*srcface)->vertex[2];

				// Cross-product 2 edges
				Vector3 e1 = v1->position - v0->position; 
				Vector3 e2 = v2->position - v1->position;

				Vector3 newNormal = e1.crossProduct(e2);
				newNormal.normalise();

				// Dot old and new face normal
				// If < 0 then more than 90 degree difference
				if (newNormal.dotProduct( (*srcface)->normal ) < 0.0f )
				{
					// Don't do it!
					cost = NEVER_COLLAPSE_COST;
					break; // No point continuing
				}


			}
		}



        // check for texture seam ripping
        /* Not reimplemented (This is Stan's code below so won't compile here!)
        Because we have to duplicate vertices where texcoords do not agree, then
        a texture seam actually becomes a geometry border now, which will
        be dealt with in the section above. 
        int nomatch=0;
        for(i=0;i<u->face.num;i++) {
        for(int j=0;j<sides.num;j++) {
        // perhaps we should actually compare the positions in uv space
        if(u->face[i]->texat(u) == sides[j]->texat(u)) break;
        }
        if(j==sides.num) 
        {
        // we didn't find a triangle with edge uv that shares texture coordinates
        // with face i at vertex u
        nomatch++;
        }
        }
        if(nomatch) {
        curvature=1;
        }
        */


        // Enable this next part if we want to keep ALL border vertices
        // Will guarantee shape remains in flat objects but limits reduction
        /*
        if(u->isBorder()) 
        {
        curvature = 9999.9f;
        }
        */
		
		return cost;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::initialiseEdgeCollapseCosts(void)
    {
        WorkingDataList::iterator i, iend;
        iend = mWorkingData.end();
        for (i = mWorkingData.begin(); i != iend; ++i)
        {
            VertexList::iterator v, vend;
            vend = i->mVertList.end();
            for (v = i->mVertList.begin(); v != vend; ++v)
            {
                v->collapseTo = NULL;
                v->collapseCost = NEVER_COLLAPSE_COST;
            }
        }

        
    }
    //---------------------------------------------------------------------
    Real ProgressiveMesh::computeEdgeCostAtVertexForBuffer(WorkingDataList::iterator idata, ushort vertIndex)
    {
        // compute the edge collapse cost for all edges that start
        // from vertex v.  Since we are only interested in reducing
        // the object by selecting the min cost edge at each step, we
        // only cache the cost of the least cost edge at this vertex
        // (in member variable collapse) as well as the value of the 
        // cost (in member variable objdist).

        VertexList::iterator v = idata->mVertList.begin();
        v += vertIndex;

        if(v->neighbor.empty()) {
            // v doesn't have neighbors so it costs nothing to collapse
            v->collapseTo = NULL;
            v->collapseCost = -0.01f;
            return v->collapseCost;
        }

        // Init metrics
        v->collapseCost = 1000000;
        v->collapseTo = NULL;

        // search all neighboring edges for "least cost" edge
        PMVertex::NeighborList::iterator n, nend;
        nend = v->neighbor.end();
        Real cost;
        for(n = v->neighbor.begin(); n != nend; ++n) 
        {
            cost = computeEdgeCollapseCost(&(*v), *n);
            if( (!v->collapseTo) || cost < v->collapseCost) 
            {
                v->collapseTo = *n;  // candidate for edge collapse
                v->collapseCost = cost;             // cost of the collapse
            }
        }

        return cost;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::computeAllCosts(void)
    {
        initialiseEdgeCollapseCosts();
        ushort i;
        for (i = 0; i < mpGeomData->numVertices; ++i)
        {
            computeEdgeCostAtVertex(i);
        }
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::collapse(PMVertex *src)
    {
        PMVertex *dest = src->collapseTo;

		// Abort if we're never supposed to collapse
		if (src->collapseCost == NEVER_COLLAPSE_COST) 
			return;

		// Remove this vertex from the running for the next check
		src->collapseTo = 0;
		src->collapseCost = NEVER_COLLAPSE_COST;
		mWorstCosts[src->index] = NEVER_COLLAPSE_COST;

		// Collapse the edge uv by moving vertex u onto v
	    // Actually remove tris on uv, then update tris that
	    // have u to have v, and then remove u.
	    if(!dest) {
		    // src is a vertex all by itself 
		    return;
	    }

	    // delete triangles on edge src-dest
        // Notify others to replace src with dest
        PMVertex::FaceList::iterator f, fend;
        fend = src->face.end();
		// Queue of faces for removal / replacement
		// prevents us screwing up the iterators while we parse
		PMVertex::FaceList faceRemovalList, faceReplacementList;
	    for(f = src->face.begin(); f != fend; ++f) 
        {
		    if((*f)->hasVertex(dest)) 
            {
                // Tri is on src-dest therefore is gone
				faceRemovalList.insert(*f);
                // Reduce index count by 3 (useful for quick allocation later)
                mCurrNumIndexes -= 3;
		    }
            else
            {
                // Only src involved, replace with dest
				faceReplacementList.insert(*f);
            }
	    }

		// Remove all the faces queued for removal
	    for(f = faceRemovalList.begin(); f != faceRemovalList.end(); ++f) 
		{
			(*f)->notifyRemoved();
		}
		// Replace all the faces queued for replacement
	    for(f = faceReplacementList.begin(); f != faceReplacementList.end(); ++f) 
		{
            (*f)->replaceVertex(src, dest);
		}

        // Notify the vertex that it is gone
        src->notifyRemoved();

        // recompute the edge collapse costs for dest and it's neighboring vertices
        // Dest
        computeEdgeCostAtVertex(dest->index);
        // Neighbors of dest
        PMVertex::NeighborList::iterator n, nend;
        nend = dest->neighbor.end();
	    for(n = dest->neighbor.begin(); n != nend; ++n)
        {
		    computeEdgeCostAtVertex((*n)->index);
	    }



    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::computeEdgeCostAtVertex(ushort vertIndex)
    {
		// Call computer for each buffer on this vertex
        Real worstCost = -0.01f;
        WorkingDataList::iterator i, iend;
        iend = mWorkingData.end();
        for (i = mWorkingData.begin(); i != iend; ++i)
        {
            worstCost = std::max(worstCost, 
                computeEdgeCostAtVertexForBuffer(i, vertIndex));
        }
        // Save the worst cost
        mWorstCosts[vertIndex] = worstCost;
    }
    //---------------------------------------------------------------------
    ushort ProgressiveMesh::getNextCollapser(void)
    {
        // Scan
        // Not done as a sort because want to keep the lookup simple for now
        Real bestVal = NEVER_COLLAPSE_COST;
        ushort i, bestIndex;
		bestIndex = 0; // NB this is ok since if nothing is better than this, nothing will collapse
        for (i = 0; i < mpGeomData->numVertices; ++i)
        {
            if (mWorstCosts[i] < bestVal)
            {
                bestVal = mWorstCosts[i];
                bestIndex = i;
            }
        }
        return bestIndex;
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::bakeNewLOD(LODGeometryData* pData)
    {
        // Zip through the tri list of any working data copy and bake
        pData->numIndexes = mCurrNumIndexes;
        pData->pIndexes = new ushort[mCurrNumIndexes];

        ushort* pIndex = pData->pIndexes;
        TriangleList::iterator tri, triend;
        // Use the first working data buffer, they are all the same index-wise
        WorkingDataList::iterator pWork = mWorkingData.begin();
        triend = pWork->mTriList.end();
        for (tri = pWork->mTriList.begin(); tri != triend; ++tri)
        {
            if (!tri->removed)
            {
                *pIndex++ = tri->vertex[0]->index;
                *pIndex++ = tri->vertex[1]->index;
                *pIndex++ = tri->vertex[2]->index;
            }
        }

    }
    //---------------------------------------------------------------------
    ProgressiveMesh::PMTriangle::PMTriangle() : removed(false)
    {
    }
    //---------------------------------------------------------------------
    void ProgressiveMesh::PMTriangle::setDetails(ushort newindex, PMVertex *v0, PMVertex *v1, 
        PMVertex *v2)
    {
        assert(v0!=v1 && v1!=v2 && v2!=v0);

        index = newindex;
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
    void ProgressiveMesh::PMVertex::setDetails(const Vector3& v, int newindex)
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
            //assert(count>0); // Must be at least one!
            // This edge has only 1 tri on it, it's a border
            if(count == 1) 
				return true;
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
    //---------------------------------------------------------------------
    void ProgressiveMesh::dumpContents(const String& log)
	{
		std::ofstream ofdump(log);

		// Just dump 1st working data for now
		WorkingDataList::iterator worki = mWorkingData.begin();

		VertexList::iterator vi, vend;
		vend = worki->mVertList.end();
		ofdump << "-------== VERTEX LIST ==-----------------" << std::endl;
		for (vi = worki->mVertList.begin(); vi != vend; ++vi)
		{
			ofdump << "Vertex " << vi->index << " pos: " << vi->position << " removed: " 
				<< vi->removed << " isborder: " << vi->isBorder() << std::endl;
			ofdump << "    Faces:" << std::endl;
			PMVertex::FaceList::iterator f, fend;
			fend = vi->face.end();
			for(f = vi->face.begin(); f != fend; ++f)
			{
				ofdump << "    Triangle index " << (*f)->index << std::endl;
			}
			ofdump << "    Neighbours:" << std::endl;
			PMVertex::NeighborList::iterator n, nend;
			nend = vi->neighbor.end();
			for (n = vi->neighbor.begin(); n != nend; ++n)
			{
				ofdump << "    Vertex index " << (*n)->index << std::endl;
			}

		}

		TriangleList::iterator ti, tend;
		tend = worki->mTriList.end();
		ofdump << "-------== TRIANGLE LIST ==-----------------" << std::endl;
		for(ti = worki->mTriList.begin(); ti != tend; ++ti)
		{
			ofdump << "Triangle " << ti->index << " norm: " << ti->normal << " removed: " << ti->removed << std::endl;
			ofdump << "    Vertex 0: " << ti->vertex[0]->index << std::endl;
			ofdump << "    Vertex 1: " << ti->vertex[1]->index << std::endl;
			ofdump << "    Vertex 2: " << ti->vertex[2]->index << std::endl;
		}

		ofdump.close();
	}


}
