/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
#include "OgreStableHeaders.h"
#include "OgreEdgeListBuilder.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreVertexIndexData.h"
#include "OgreException.h"

namespace Ogre {

    void EdgeData::log(Log* l)
    {
        EdgeGroupList::iterator i, iend;
        EdgeList::iterator ei, eiend;
        TriangleList::iterator ti, tiend;
        tiend = triangles.end();
        l->logMessage("Edge Data");
        l->logMessage("---------");
        size_t num = 0;
        for (ti = triangles.begin(); ti != tiend; ++ti, ++num)
        {
            Triangle& t = *ti;
            l->logMessage("Triangle " + StringConverter::toString(num) + " = {" +
                "indexSet=" + StringConverter::toString(t.indexSet) + ", " + 
                "vertexSet=" + StringConverter::toString(t.vertexSet) + ", " + 
                "v0=" + StringConverter::toString(t.vertIndex[0]) + ", " + 
                "v1=" + StringConverter::toString(t.vertIndex[1]) + ", " + 
                "v2=" + StringConverter::toString(t.vertIndex[2]) + "}"); 
        }
        iend = edgeGroups.end();
        for (i = edgeGroups.begin(); i != iend; ++i)
        {
            num = 0;
            eiend = i->edges.end();
            l->logMessage("Edge Group vertexSet=" + StringConverter::toString(i->vertexSet));
            for (ei = i->edges.begin(); ei != eiend; ++ei, ++num)
            {
                Edge& e = *ei;
                l->logMessage(
                    "Edge " + StringConverter::toString(num) + " = {\n" + 
                    "  tri0=" + StringConverter::toString(e.triIndex[0]) + ", \n" + 
                    "  tri1=" + StringConverter::toString(e.triIndex[1]) + ", \n" + 
                    "  v0=" + StringConverter::toString(e.vertIndex[0]) + ", \n" + 
                    "  v1=" + StringConverter::toString(e.vertIndex[1]) + ", \n"
                    "  degenerate=" + StringConverter::toString(e.degenerate) + " \n"
                    "}"); 
            }
        }
    }
    //---------------------------------------------------------------------
    EdgeListBuilder::EdgeListBuilder()
        : mWeldVerticesAcrossSets(true), mWeldVerticesWithinSets(true)
    {
    }
    //---------------------------------------------------------------------
    EdgeListBuilder::~EdgeListBuilder()
    {
    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::addVertexData(const VertexData* vertexData)
    {
        mVertexDataList.push_back(vertexData);
    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::addIndexData(const IndexData* indexData, 
        size_t vertexSet, RenderOperation::OperationType opType)
    {
        mIndexDataList.push_back(indexData);
        mIndexDataVertexDataSetList.push_back(vertexSet);
        mOperationTypeList.push_back(opType);
    }
    //---------------------------------------------------------------------
    EdgeData* EdgeListBuilder::build(void)
    {
        /* Ok, here's the algorithm:
        For each set of indices in turn
          // First pass, create triangles and create edges
          For each set of 3 indexes
            Create a new Triangle entry in the list
            For each vertex referenced by the tri indexes
              Get the position of the vertex as a Vector3 from the correct vertex buffer
              Attempt to locate this position in the existing common vertex set
              If not found
                Create a new common vertex entry in the list
              End If
              Populate the original vertex index and common vertex index 
            Next vertex
            If commonIndex[0] < commonIndex[1]
                Create a new edge 
            End If
            If commonIndex[1] < commonIndex[2]
                Create a new edge 
            End If
            If commonIndex[2] < commonIndex[0]
                Create a new edge 
            End If
          Next set of 3 indexes
        Next index set
        // Identify shared edges (works across index sets)
        For each triangle in the common triangle list
        If commonIndex[0] > commonIndex[1]
            Find existing edge and update with second side
        End If
        If commonIndex[1] > commonIndex[2]
            Find existing edge and update with second side
        End If
        If commonIndex[2] > commonIndex[0]
            Find existing edge and update with second side
        End If
        Next triangle

        Note that all edges 'belong' to the index set which originally caused them
        to be created, which also means that the 2 vertices on the edge are both referencing the 
        vertex buffer which this index set uses.
        */

        // Default to try to form a combined hull from multiple non-manifold parts
        mWeldVerticesAcrossSets = true;
        mWeldVerticesWithinSets = true;

        mEdgeData = new EdgeData();
        // resize the edge group list to equal the number of vertex sets
        mEdgeData->edgeGroups.resize(mVertexDataList.size());
        // Initialise edge group data
        for (unsigned short vSet = 0; vSet < mVertexDataList.size(); ++vSet)
        {
            mEdgeData->edgeGroups[vSet].vertexSet = vSet;
            mEdgeData->edgeGroups[vSet].vertexData = mVertexDataList[vSet];
        }

        IndexDataList::iterator i, iend;
        std::vector<size_t>::iterator mapi, mapiend;
        mapiend = mIndexDataVertexDataSetList.end();
        mapi = mIndexDataVertexDataSetList.begin();
        iend = mIndexDataList.end();
        // Stage 1, build triangles and initial edge list
        size_t indexSet = 0;
        for (i = mIndexDataList.begin(); i != iend; ++i, ++mapi, ++indexSet)
        {
            try 
            {
                buildTrianglesEdges(indexSet, *mapi);
            }
            catch (Exception& e)
            {
                if (e.getNumber() == Exception::ERR_DUPLICATE_ITEM)
                {
                    // Ok, we found a case where there are too many triangles
                    // attached to a single edge. This can happen in valid cases
                    // where a mesh has multiple submeshes which 'but up' against
                    // each other perfectly, but are individually closed
                    // In this case, we try disabling the 'welding across vertex sets'
                    // option, which means we ignore it when 2 vertices from different
                    // sets are in the same position (thus we stop trying to form
                    // a complete hull from multiple non-manifold submeshes). 
                    // This is on the assumption that all submeshes have their own
                    // geometry and are manifold in their own right. If this results
                    // in degenerates, then it's the modellers fault (and unavoidable 
                    // since the 'too many tris on an edge' case would have resulted in 
                    // artefacts anyway)

                    if (mWeldVerticesAcrossSets || mWeldVerticesWithinSets)
                    {
                        if (mWeldVerticesAcrossSets)
                        {
                            LogManager::getSingleton().logMessage(
                                "RECOVERY: attempting to find valid hull "
                                "by assuming all individual submeshes are "
                                "manifold with their own geometry...");

                            mWeldVerticesAcrossSets = false;
                        }
                        else if (mWeldVerticesWithinSets)
                        {
                            LogManager::getSingleton().logMessage(
                                "WARNING: edge calculation process detected "
                                "ambiguous hull. This is almost certainly going "
                                "to result in stencil shadow artefacts.");

                            mWeldVerticesWithinSets = false;
                        }

                        // reset
                        mUniqueEdges.clear();
                        mEdgeData->triangles.clear();
                        EdgeData::EdgeGroupList::iterator egi, egiend;
                        egiend = mEdgeData->edgeGroups.end();
                        for(egi = mEdgeData->edgeGroups.begin(); egi != egiend; ++egi)
                        {
                            egi->edges.clear();
                        }
                        // try again (reset and issue 1st)
                        // Any exceptions this time will be thrown to parent
                        i = mIndexDataList.begin();
                        mapi = mIndexDataVertexDataSetList.begin();
                        indexSet = 0;
                        try 
                        {

                            buildTrianglesEdges(indexSet, *mapi);
                        }
                        catch (Exception& e2)
                        {
                            if (e.getNumber() == Exception::ERR_DUPLICATE_ITEM)
                            {
                                if (mWeldVerticesWithinSets)
                                {
                                    // Oh, for feck sakes
                                    // This means that, even splitting by vertex set
                                    // we still managed to get a situation where there
                                    // are too many faces meeting at the same edge
                                    // This is likely because the source of this mesh
                                    // has co-incident vertices, but they're not meant
                                    // to be treated as the same edge
                                    // So we don't weld at all now
                                    mWeldVerticesWithinSets = false;

                                    LogManager::getSingleton().logMessage(
                                        "WARNING: edge calculation process detected "
                                        "ambiguous hull. This is almost certainly going "
                                        "to result in stencil shadow artefacts.");
                                    // reset
                                    mUniqueEdges.clear();
                                    mEdgeData->triangles.clear();
                                    EdgeData::EdgeGroupList::iterator egi, egiend;
                                    egiend = mEdgeData->edgeGroups.end();
                                    for(egi = mEdgeData->edgeGroups.begin(); egi != egiend; ++egi)
                                    {
                                        egi->edges.clear();
                                    }
                                    // try again (reset and issue 1st) - LAST CHANCE!
                                    // Any exceptions this time will be thrown to parent
                                    i = mIndexDataList.begin();
                                    mapi = mIndexDataVertexDataSetList.begin();
                                    indexSet = 0;
                                    buildTrianglesEdges(indexSet, *mapi);
                                }
                                else
                                {
                                    throw;
                                }

                            }
                            else
                            {
                                throw;
                            }
                        }
                    }
                    else
                    {
                        throw;
                    }
                }
                else
                {
                    throw;
                }

            }
        }
        // Stage 2, link edges
        connectEdges();

        // Log
        //log(LogManager::getSingleton().createLog("EdgeListBuilder.log"));

        return mEdgeData;


    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::buildTrianglesEdges(size_t indexSet, size_t vertexSet)
    {
        const IndexData* indexData = mIndexDataList[indexSet];
        RenderOperation::OperationType opType = mOperationTypeList[indexSet];

        size_t iterations;
        
        switch (opType)
        {
        case RenderOperation::OT_TRIANGLE_LIST:
            iterations = indexData->indexCount / 3;
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
        case RenderOperation::OT_TRIANGLE_STRIP:
            iterations = indexData->indexCount - 2;
            break;
        default:
            break;
        };



		// locate position element & the buffer to go with it
        const VertexData* vertexData = mVertexDataList[vertexSet];
		const VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
		HardwareVertexBufferSharedPtr vbuf = 
			vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
		// lock the buffer for reading
		unsigned char* pBaseVertex = static_cast<unsigned char*>(
			vbuf->lock(HardwareBuffer::HBL_READ_ONLY));

        // Get the indexes ready for reading
        unsigned short* p16Idx;
        unsigned int* p32Idx;

        if (indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
        {
            p32Idx = static_cast<unsigned int*>(
                indexData->indexBuffer->lock(HardwareBuffer::HBL_READ_ONLY));
        }
        else
        {
            p16Idx = static_cast<unsigned short*>(
                indexData->indexBuffer->lock(HardwareBuffer::HBL_READ_ONLY));
        }

        // Iterate over all the groups of 3 indexes
        Real* pReal;
        // Get the triangle start, if we have more than one index set then this
        // will not be zero
        size_t triStart = mEdgeData->triangles.size();
        // Pre-reserve memory for less thrashing
        mEdgeData->triangles.reserve(triStart + iterations);
        for (size_t t = 0; t < iterations; ++t)
        {
            EdgeData::Triangle tri;
            tri.indexSet = indexSet;
            tri.vertexSet = vertexSet;

            unsigned int index[3];
            Vector3 v[3];
            for (size_t i = 0; i < 3; ++i)
            {
                // Standard 3-index read for tri list or first tri in strip / fan
                if (opType == RenderOperation::OT_TRIANGLE_LIST ||
                    t == 0)
                {
                    if (indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
                    {
                        index[i] = *p32Idx++;
                    }
                    else
                    {
                        index[i] = *p16Idx++;
                    }
                }
                else
                {
                    // Strips and fans are formed from last 2 indexes plus the 
                    // current one for triangles after the first
                    if (indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
                    {
                        index[i] = p32Idx[i-2];
                    }
                    else
                    {
                        index[i] = p16Idx[i-2];
                    }
                    // Perform single-index increment at the last tri index
                    if (i == 2)
                    {
                        if (indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
                        {
                            p32Idx++;
                        }
                        else
                        {
                            p16Idx++;
                        }

                    }

                }

                // Populate tri original vertex index
                tri.vertIndex[i] = index[i];

                // Retrieve the vertex position
                unsigned char* pVertex = pBaseVertex + (index[i] * vbuf->getVertexSize());
                posElem->baseVertexPointerToElement(pVertex, &pReal);
                v[i].x = *pReal++;
                v[i].y = *pReal++;
                v[i].z = *pReal++;
                // find this vertex in the existing vertex map, or create it
                tri.sharedVertIndex[i] = findOrCreateCommonVertex(v[i], vertexSet);
            }
            // Calculate triangle normal (NB will require recalculation for 
            // skeletally animated meshes)
            tri.normal = Math::calculateFaceNormal(v[0], v[1], v[2]);
            // Add triangle to list
            mEdgeData->triangles.push_back(tri);
            // Create edges from common list
            try {
                if (tri.sharedVertIndex[0] < tri.sharedVertIndex[1])
                {
                    createEdge(vertexSet, triStart + t, 
                        tri.vertIndex[0], tri.vertIndex[1], 
                        tri.sharedVertIndex[0], tri.sharedVertIndex[1]);
                }
                if (tri.sharedVertIndex[1] < tri.sharedVertIndex[2])
                {
                    createEdge(vertexSet, triStart + t, 
                        tri.vertIndex[1], tri.vertIndex[2], 
                        tri.sharedVertIndex[1], tri.sharedVertIndex[2]);
                }
                if (tri.sharedVertIndex[2] < tri.sharedVertIndex[0])
                {
                    createEdge(vertexSet, triStart + t, 
                        tri.vertIndex[2], tri.vertIndex[0], 
                        tri.sharedVertIndex[2], tri.sharedVertIndex[0]);
                }
            }
            catch (Exception& e)
            {
                // unlock buffers so this is repeatable if required
                indexData->indexBuffer->unlock();
                vbuf->unlock();
                throw;
            }

        }
        indexData->indexBuffer->unlock();
        vbuf->unlock();




    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::createEdge(size_t vertexSet, size_t triangleIndex, 
        size_t vertIndex0, size_t vertIndex1, size_t sharedVertIndex0, 
        size_t sharedVertIndex1)
    {
        // Check unique on shared vertices
        std::pair<size_t, size_t> vertPair;
        vertPair.first = sharedVertIndex0;
        vertPair.second = sharedVertIndex1;
        UniqueEdgeSet::iterator ui = mUniqueEdges.find(vertPair);
        if (ui != mUniqueEdges.end())
        {
            Except(Exception::ERR_DUPLICATE_ITEM, 
                "Edge is shared by too many triangles", 
                "EdgeListBuilder::createEdge");
        }
        mUniqueEdges.insert(vertPair);

        EdgeData::Edge e;
        e.degenerate = true; // initialise as degenerate

        // Set only first tri, the other will be completed in connectEdges
        e.triIndex[0] = triangleIndex;
        e.sharedVertIndex[0] = sharedVertIndex0;
        e.sharedVertIndex[1] = sharedVertIndex1;
        e.vertIndex[0] = vertIndex0;
        e.vertIndex[1] = vertIndex1;
        mEdgeData->edgeGroups[vertexSet].edges.push_back(e);

    }
    //---------------------------------------------------------------------
    size_t EdgeListBuilder::findOrCreateCommonVertex(const Vector3& vec, size_t vertexSet)
    {
        // Iterate over existing list
        CommonVertexList::iterator i, iend;
        iend = mVertices.end();
        size_t index = 0;
        for (i = mVertices.begin(); i != iend; ++i, ++index)
        {
            const CommonVertex& commonVec = *i;

            if (Math::RealEqual(vec.x, commonVec.position.x, 1e-04) && 
                Math::RealEqual(vec.y, commonVec.position.y, 1e-04) && 
                Math::RealEqual(vec.z, commonVec.position.z, 1e-04) && 
                ((commonVec.vertexSet == vertexSet && mWeldVerticesWithinSets)
                || mWeldVerticesAcrossSets))
            {
                return index;
            }

        }
        // Not found, insert
        CommonVertex newCommon;
        newCommon.index = mVertices.size();
        newCommon.position = vec;
        newCommon.vertexSet = vertexSet;
        mVertices.push_back(newCommon);
        return newCommon.index;
    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::connectEdges(void)
    {
        // Iterate over existing triangles
        EdgeData::TriangleList::iterator ti, tiend;
        tiend = mEdgeData->triangles.end();
        size_t triIndex = 0;
        for (ti = mEdgeData->triangles.begin(); ti != tiend; ++ti, ++triIndex)
        {
            EdgeData::Triangle& tri = *ti;
            EdgeData::Edge* e;
            if (tri.sharedVertIndex[0] > tri.sharedVertIndex[1])
            {
                e = findEdge(tri.sharedVertIndex[1], tri.sharedVertIndex[0]);
                if (e)
                {
                    e->triIndex[1] = triIndex;
                    e->degenerate = false;
                }
            }
            if (tri.sharedVertIndex[1] > tri.sharedVertIndex[2])
            {
                // Find the existing edge (should be reversed order)
                e = findEdge(tri.sharedVertIndex[2], tri.sharedVertIndex[1]);
                if (e)
                {
                    e->triIndex[1] = triIndex;
                    e->degenerate = false;
                }
            }
            if (tri.sharedVertIndex[2] > tri.sharedVertIndex[0])
            {
                e = findEdge(tri.sharedVertIndex[0], tri.sharedVertIndex[2]);
                if (e)
                {
                    e->triIndex[1] = triIndex;
                    e->degenerate = false;
                }
            }

        }


    }
    //---------------------------------------------------------------------
    EdgeData::Edge* EdgeListBuilder::findEdge(size_t sharedIndex1, size_t sharedIndex2)
    {
        // Iterate over the existing edges
        EdgeData::EdgeGroupList::iterator i, iend;
        EdgeData::EdgeList::iterator ei, eiend;

        iend = mEdgeData->edgeGroups.end();
        for (i = mEdgeData->edgeGroups.begin(); i != iend; ++i)
        {
            eiend = i->edges.end();
            for (ei = i->edges.begin(); ei != eiend; ++ei)
            {
                EdgeData::Edge& e = *ei;
                if (e.sharedVertIndex[0] == sharedIndex1 && e.sharedVertIndex[1] == sharedIndex2)
                {
                    return &(*ei);
                }
            }
        }
        
        // no edge
        return 0;

    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void EdgeData::updateTriangleLightFacing(const Vector4& lightPos)
    {
        // Iterate over the triangles, and determine if they are light facing
        EdgeData::TriangleList::iterator ti, tiend;
        tiend = triangles.end();
        Vector3 vertToLight;
        for (ti = triangles.begin(); ti != tiend; ++ti)
        {
            EdgeData::Triangle& t = *ti;
            // Get pointer to positions, and reference the first position

            Real dp = t.normal.dotProduct(lightPos);
            t.lightFacing = (dp > 0);

        }

    }
    //---------------------------------------------------------------------
    void EdgeData::updateFaceNormals(size_t vertexSet, 
        HardwareVertexBufferSharedPtr positionBuffer)
    {
        assert (positionBuffer->getVertexSize() == sizeof(Real) * 3
            && "Position buffer should contain only positions!");

        // Lock buffer for reading
        Real* pVert = static_cast<Real*>(
            positionBuffer->lock(HardwareBuffer::HBL_READ_ONLY));

        // Iterate over the triangles
        EdgeData::TriangleList::iterator i, iend;
        iend = triangles.end();
        for (i = triangles.begin(); i != iend; ++i)
        {
            Triangle& t = *i;
            // Only update tris which are using this vertex set
            if (t.vertexSet == vertexSet)
            {
                size_t offset = t.vertIndex[0]*3;
                Vector3 v1(pVert + offset);

                offset = t.vertIndex[1]*3;
                Vector3 v2(pVert + offset);

                offset = t.vertIndex[2]*3;
                Vector3 v3(pVert + offset);

                t.normal = Math::calculateFaceNormal(v1, v2, v3);
            }
        }


        // unlock the buffer
        positionBuffer->unlock();
    }
    //---------------------------------------------------------------------
    void EdgeListBuilder::log(Log* l)
    {
        l->logMessage("EdgeListBuilder Log");
        l->logMessage("-------------------");
        l->logMessage("Number of vertex sets: " + StringConverter::toString(mVertexDataList.size()));
        l->logMessage("Number of index sets: " + StringConverter::toString(mIndexDataList.size()));
        
        size_t i, j;
        // Log original vertex data
        for(i = 0; i < mVertexDataList.size(); ++i)
        {
            const VertexData* vData = mVertexDataList[i];
            l->logMessage(".");
            l->logMessage("Original vertex set " + 
                StringConverter::toString(i) + " - vertex count " + 
                StringConverter::toString(vData->vertexCount));
            const VertexElement* posElem = vData->vertexDeclaration->findElementBySemantic(VES_POSITION);
            HardwareVertexBufferSharedPtr vbuf = 
                vData->vertexBufferBinding->getBuffer(posElem->getSource());
            // lock the buffer for reading
            unsigned char* pBaseVertex = static_cast<unsigned char*>(
                vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
            Real* pReal;
            for (j = 0; j < vData->vertexCount; ++j)
            {
                posElem->baseVertexPointerToElement(pBaseVertex, &pReal);
                l->logMessage("Vertex " + StringConverter::toString(j) + 
                    ": (" + StringConverter::toString(pReal[0]) + 
                    ", " + StringConverter::toString(pReal[1]) + 
                    ", " + StringConverter::toString(pReal[2]) + ")");
                pBaseVertex += vbuf->getVertexSize();
            }
            vbuf->unlock();
        }

        // Log original index data
        for(i = 0; i < mIndexDataList.size(); i++)
        {
            const IndexData* iData = mIndexDataList[i];
            l->logMessage(".");
            l->logMessage("Original triangle set " + 
                StringConverter::toString(i) + " - index count " + 
                StringConverter::toString(iData->indexCount) + " - " + 
            "vertex set " + StringConverter::toString(mIndexDataVertexDataSetList[i]) + " - " + 
            "operationType " + StringConverter::toString(mOperationTypeList[i]));
            // Get the indexes ready for reading
            unsigned short* p16Idx;
            unsigned int* p32Idx;

            if (iData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
            {
                p32Idx = static_cast<unsigned int*>(
                    iData->indexBuffer->lock(HardwareBuffer::HBL_READ_ONLY));
            }
            else
            {
                p16Idx = static_cast<unsigned short*>(
                    iData->indexBuffer->lock(HardwareBuffer::HBL_READ_ONLY));
            }

            for (j = 0; j < iData->indexCount;  )
            {
                if (iData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
                {
                    if (mOperationTypeList[i] == RenderOperation::OT_TRIANGLE_LIST
                        || j == 0)
                    {
                        l->logMessage("Triangle " + StringConverter::toString(j) + 
                            ": (" + StringConverter::toString(*p32Idx++) + 
                            ", " + StringConverter::toString(*p32Idx++) + 
                            ", " + StringConverter::toString(*p32Idx++) + ")");
                        j += 3;
                    }
                    else
                    {
                        l->logMessage("Triangle " + StringConverter::toString(j) + 
                            ": (" + StringConverter::toString(*p32Idx++) + ")");
                        j++;
                    }
                }
                else
                {
                    if (mOperationTypeList[i] == RenderOperation::OT_TRIANGLE_LIST
                        || j == 0)
                    {
                        l->logMessage("Index " + StringConverter::toString(j) + 
                            ": (" + StringConverter::toString(*p16Idx++) + 
                            ", " + StringConverter::toString(*p16Idx++) + 
                            ", " + StringConverter::toString(*p16Idx++) + ")");
                        j += 3;
                    }
                    else
                    {
                        l->logMessage("Triangle " + StringConverter::toString(j) + 
                            ": (" + StringConverter::toString(*p16Idx++) + ")");
                        j++;
                    }
                }


            }

            iData->indexBuffer->unlock();


            // Log common vertex list
            l->logMessage(".");
            l->logMessage("Common vertex list - vertex count " + 
                StringConverter::toString(mVertices.size()));
            for (i = 0; i < mVertices.size(); ++i)
            {
                CommonVertex& c = mVertices[i];
                l->logMessage("Common vertex " + StringConverter::toString(i) + 
                    ": (vertexSet=" + StringConverter::toString(c.vertexSet) + 
                    ", originalIndex=" + StringConverter::toString(c.index) + 
                    ", position=" + StringConverter::toString(c.position));
            }
        }

    }



}

