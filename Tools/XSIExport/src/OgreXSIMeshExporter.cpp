/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreXSIMeshExporter.h"
#include <xsi_model.h>
#include <xsi_primitive.h>
#include <xsi_polygonnode.h>
#include <xsi_polygonmesh.h>
#include <xsi_material.h>
#include <xsi_vertex.h>
#include <xsi_trianglevertex.h>
#include <xsi_cluster.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>

#include "OgreException.h"
#include "OgreXSIHelper.h"
#include "OgreLogManager.h"
#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMeshManager.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "OgreMeshSerializer.h"

using namespace XSI;


namespace Ogre {
    //-----------------------------------------------------------------------
    XsiMeshExporter::UniqueVertex::UniqueVertex()
        : initialised(false), position(Vector3::ZERO), normal(Vector3::ZERO), colour(0), nextIndex(0)
    {
        for (int i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; ++i)
            uv[i] = Vector3::ZERO;
    }
    //-----------------------------------------------------------------------
    bool XsiMeshExporter::UniqueVertex::operator==(const UniqueVertex& rhs) const
    {
        bool ret = position == rhs.position && 
            normal == rhs.normal && 
            colour == rhs.colour;
        if (!ret) return ret;

        for (int i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS && ret; ++i)
        {
            ret = ret && (uv[i] == rhs.uv[i]);
        }

        return ret;
        

    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    XsiMeshExporter::XsiMeshExporter()
    {
    }
    //-----------------------------------------------------------------------
    XsiMeshExporter::~XsiMeshExporter()
    {
    }
    //-----------------------------------------------------------------------
    void XsiMeshExporter::exportMesh(const CString& fileName, 
        const CString& objectName, bool edgeLists, bool tangents)
    {
        LogManager logMgr;
		logMgr.createLog("OgreXSIExporter.log", true);
		ResourceGroupManager rgm;
        MeshManager *meshMgr = new MeshManager();
        DefaultHardwareBufferManager *hardwareBufMgr = new DefaultHardwareBufferManager();

        logMgr.createLog("OgreXSIExport.log", true);

        // Derive the scene root
        X3DObject sceneRoot(mXsiApp.GetActiveSceneRoot());

        // Construct mesh
        MeshPtr pMesh = MeshManager::getSingleton().createManual("XSIExport", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

        if (objectName.IsEmpty())
        {
            // export the entire scene
            exportX3DObject(pMesh.getPointer(), sceneRoot);
        }
        else
        {
            // find the named object
            X3DObject obj = sceneRoot.FindChild(
                objectName, 
                CString(), // no type criteria
                CStringArray() // no family criteria
                );
            // Check we got it
            if (!obj.IsValid())
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                    "No object named " + XSItoOgre(objectName) + " found.",
                    "XsiMeshExporter::exportMesh");
            }

            exportX3DObject(pMesh.getPointer(), obj);

        }

        if(edgeLists)
        {
            mXsiApp.LogMessage(L"Calculating edge lists");
            pMesh->buildEdgeList();
        }

        if(tangents)
        {
            mXsiApp.LogMessage(L"Calculating tangents");
            unsigned short src, dest;
            if (pMesh->suggestTangentVectorBuildParams(src, dest))
            {
                pMesh->buildTangentVectors(src, dest);
            }
            else
            {
                mXsiApp.LogMessage(L"Could not derive tangents parameters");
            }

        }

        MeshSerializer serializer;
        serializer.exportMesh(pMesh.getPointer(), XSItoOgre(fileName));

        delete meshMgr;
        delete hardwareBufMgr;
    }
    //-----------------------------------------------------------------------
    void XsiMeshExporter::exportX3DObject(Mesh* pMesh, X3DObject& x3dObj)
    {
        // Check validity of current object
        if (!x3dObj.IsValid())
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Invalid X3DObject found",
                "XsiMeshExporter::exportX3DObject");
        }

#ifdef _DEBUG
        // Log a message in script window
        CString name = x3dObj.GetName() ;
        mXsiApp.LogMessage(L"-- Exporting " +  name) ;
#endif


        // locate any geometry
        if (!x3dObj.IsA(siCameraID) && 
            !x3dObj.IsA(siLightID) && 
            !x3dObj.IsA(siNullID) && 
            !x3dObj.IsA(siModelID))
        {
            Primitive prim(x3dObj.GetActivePrimitive());
            if (prim.IsValid())
            {
                Geometry geom(prim.GetGeometry());
                exportSubMesh(pMesh, x3dObj, geom);
            }

        }

        // Cascade into children
        CRefArray children = x3dObj.GetChildren();

        for(long i = 0; i < children.GetCount(); i++)
        {
            X3DObject childObj = children[i];
            exportX3DObject(pMesh, childObj);
        }

        

    }
    //-----------------------------------------------------------------------
    void XsiMeshExporter::exportSubMesh(Mesh* pMesh, X3DObject& x3dObj, 
        XSI::Geometry& geom)
    {
        // Check we're dealing with a polygon mesh
        if (geom.GetRef().GetClassID() != siPolygonMeshID)
            return;

        // NB, we don't deal with poly clusters right now
        // In XSI, poly clusters can be used to change material for a sub-part
        // of a PolygonMesh, but the SDK doesn't appear to let you derive the
        // list of triangles from it (only facets, which do not appear to have a 
        // link with triangles in the API for some reason)
        // Material changes are only supported per PolygonMesh for this reason

        SubMesh* sm = 0;
        if (x3dObj.GetName().IsEmpty())
        {
            // anonymous submesh
            sm = pMesh->createSubMesh();
        }
        else
        {
            // named submesh
            sm = pMesh->createSubMesh(XSItoOgre(x3dObj.GetName()));
        }

        // Set material
        sm->setMaterialName(XSItoOgre(x3dObj.GetMaterial().GetName()));

        PolygonMesh xsiMesh(geom);

        // Retrieve all the XSI relevant summary info
        CPointRefArray pointArray(xsiMesh.GetPoints());
        MATH::CVector3Array srcPosArray = pointArray.GetPositionArray();
        CPolygonNodeRefArray nodeArray(xsiMesh.GetNodes());
        MATH::CVector3Array srcNormArray = nodeArray.GetNormalArray();
        CTriangleRefArray triArray = xsiMesh.GetTriangles();

        // derive number of UVs
        size_t numUVs = 0;
        CRefArray clusterRefArray;
        // Filter to 'sample' types
        xsiMesh.GetClusters().Filter(siSampledPointCluster,CStringArray(),L"",clusterRefArray);

        Cluster samplePointClusterUV;
        CRefArray uvClusterPropertiesRefArray;
        
        long i;
        for(i = 0; i < clusterRefArray.GetCount(); ++i)
        {
            Cluster cluster(clusterRefArray[i]);		
            // Now filter all the 'uvspace' children
            // there is one of these per UV set
            if(cluster.GetProperties().Filter(
                siClsUVSpaceTxtType, CStringArray(), L"", 
                uvClusterPropertiesRefArray) == CStatus::OK)
            {
                samplePointClusterUV = cluster;			
                break;
            }
        }

        // Ok, we now have our array of UV sets
        numUVs = uvClusterPropertiesRefArray.GetCount();
		size_t numSamplerPoints = xsiMesh.GetNodes().GetCount();
		// list of UVs stored in order of sampler points (use 3D coords by default)
		std::vector<Vector3*> uvSamplerSets;
		std::vector<ushort> uvSamplerDimensions; // actual dimensions
		uvSamplerSets.reserve(numUVs);
		uvSamplerDimensions.reserve(numUVs);
        for(i = 0; i < numUVs; ++i)
        {
			// init sampler points
			Vector3* samplerUVs = new Vector3[numSamplerPoints];
			uvSamplerSets.push_back(samplerUVs);

			// Detect the dimensions by figuring out if any are all 0
			bool hasU, hasV, hasW;
			hasU = hasV = hasW = false;

			// Pull out all the UV data for this set and reorder it based on 
			// samples, we'll need this for reference later
			// get Elements from uvspace Property
            ClusterProperty uvProp(uvClusterPropertiesRefArray[i]);
			CClusterPropertyElementArray uvElements = uvProp.GetElements();

			// Now, each Element here is actually a CDoubleArray of the u,v,w values
			// However it's not in order of samplers, we need to use the Array
			// linked to from the Elements collection under the cluster (not the 
			// cluster property, confusing I know) to figure out what sampler 
			// index it is, i.e.
			// samplerUVs[Array[j]] = Element[j]
			// In all cases in XSI element properties are referenced back to 
			// their original poly index via this array, ie all cluster properties
			// in the same cluster are dereferenced in the same way. 
			CLongArray derefArray = samplePointClusterUV.GetElements().GetArray();

			for (int j = 0; j < uvElements.GetCount(); ++j)
			{
				CDoubleArray curUVW(uvElements.GetItem(j));
				size_t samplerIndex = derefArray[j];
				samplerUVs[samplerIndex].x = curUVW[0];//u
				samplerUVs[samplerIndex].y = curUVW[1];//v
				samplerUVs[samplerIndex].z = curUVW[2];//w

				if (!hasU && curUVW[0] > 0)
					hasU = true;
				if (!hasV && curUVW[1] > 0)
					hasV = true;
				if (!hasW && curUVW[2] > 0)
					hasW = true;

			}
			
			// save dimensions
			uvSamplerDimensions.push_back((hasU?1:0) + (hasV?1:0) + (hasW?1:0));

        }


        // do we have vertex colours?
        bool hasVertexColours = false;

        ClusterProperty vertexColourClusterProperty = xsiMesh.GetCurrentVertexColor();
        if (vertexColourClusterProperty.IsValid())
            hasVertexColours = true;

#if _DEBUG
        StringUtil::StrStreamType msg;
        msg << "Points: " << pointArray.GetCount() << std::endl;
        msg << "Triangles: " << triArray.GetCount() << std::endl;
        msg << "Normals: " << srcNormArray.GetCount() << std::endl;
        msg << "Num UVs: " << numUVs << std::endl;
        String str = msg.str();
        mXsiApp.LogMessage(OgretoXSI(str));
#endif
        // never use shared geometry
        sm->useSharedVertices = false;
        sm->vertexData = new VertexData();
        // always do triangle list
        sm->indexData->indexCount = static_cast<size_t>(triArray.GetCount() * 3);
        // identify the unique vertices, write to a temp index buffer

        startPolygonMesh(pointArray.GetCount(), triArray.GetCount()*3);

		// Save transform
		MATH::CTransformation xsiTransform = x3dObj.GetKinematics().GetGlobal().GetTransform();

        // Iterate through all the triangles
        // There will often be less poisitions than normals and UVs
        // But TrianglePoint
        for (long t = 0; t < triArray.GetCount(); ++t)
        {
            Triangle tri(triArray[t]);
			// derive sampler indices for triangle
			size_t samplerIndices[3];
			CPolygonFaceRefArray polys(xsiMesh.GetPolygons());
			deriveSamplerIndices(tri, polys[tri.GetPolygonIndex()], samplerIndices);

            CTriangleVertexRefArray points = tri.GetPoints();
            for (long p = 0; p < 3; ++p)
            {
                TriangleVertex point(points[p]);
                long posIndex = point.GetIndex(); // unique position index
                UniqueVertex vertex;
				// Get position
				MATH::CVector3 xsipos = point.GetPosition();
				// Apply global SRT
				xsipos.MulByTransformationInPlace(xsiTransform);
                vertex.position = XSItoOgre(xsipos);
				// Get normal
				MATH::CVector3 xsinorm = point.GetNormal();
				// Apply global rotation
				MATH::CTransformation rotTrans;
				rotTrans.SetRotation(xsiTransform.GetRotation());
				xsinorm *= rotTrans;
                vertex.normal = XSItoOgre(xsinorm);

				for (int i = 0; i < numUVs; ++i)
				{
					// sampler indices can correctly dereference to sampler-order
					// uv sets we built earlier
					vertex.uv[i] = (uvSamplerSets[i])[samplerIndices[p]];
				}
                
                if (hasVertexColours)
                    vertex.colour = XSItoOgre(point.GetColor());

                size_t index = createOrRetrieveUniqueVertex(posIndex, vertex);
                mIndices.push_back(index);

            }
        }

		// free temp UV data now
		for(i = 0; i < numUVs; ++i)
		{
			// init sampler points
			delete [] uvSamplerSets[i];
		}
        uvSamplerSets.clear();

        // Now bake final geometry
        sm->vertexData->vertexCount = mUniqueVertices.size();
        // Determine index size
        bool use32BitIndexes = false;
        if (mUniqueVertices.size() > 65536)
        {
            use32BitIndexes = true;
        }

        sm->indexData->indexBuffer = 
            HardwareBufferManager::getSingleton().createIndexBuffer(
            use32BitIndexes ? HardwareIndexBuffer::IT_32BIT : HardwareIndexBuffer::IT_16BIT,
            triArray.GetCount() * 3,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        if (use32BitIndexes)
        {
            uint32* pIdx = static_cast<uint32*>(
                sm->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
            writeIndexes(pIdx);
            sm->indexData->indexBuffer->unlock();
        }
        else
        {
            uint16* pIdx = static_cast<uint16*>(
                sm->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
            writeIndexes(pIdx);
            sm->indexData->indexBuffer->unlock();
        }


        // define vertex declaration
        unsigned buf = 0;
        size_t offset = 0;
        sm->vertexData->vertexDeclaration->addElement(buf, offset, VET_FLOAT3, VES_POSITION);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
        sm->vertexData->vertexDeclaration->addElement(buf, offset, VET_FLOAT3, VES_NORMAL);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
        // TODO - split vertex data here if animated
        /*
        if (animated)
        {
            buf = 0;
            offset = 0;
        }
        */
        if(hasVertexColours)
        {
            sm->vertexData->vertexDeclaration->addElement(buf, offset, VET_COLOUR, VES_DIFFUSE);
            offset += VertexElement::getTypeSize(VET_COLOUR);
        }
        // Write uvs
        for (unsigned short uvi = 0; uvi < numUVs; ++uvi)
        {
			VertexElementType uvType = 
				VertexElement::multiplyTypeCount(
					VET_FLOAT1, uvSamplerDimensions[uvi]);
            sm->vertexData->vertexDeclaration->addElement(
				buf, offset, uvType, VES_TEXTURE_COORDINATES, uvi);
            offset += VertexElement::getTypeSize(uvType);
        }

        // create & fill buffer(s)
        for (unsigned short b = 0; b <= sm->vertexData->vertexDeclaration->getMaxSource(); ++b)
        {
            createVertexBuffer(sm->vertexData, b);
        }

        // Bounds definitions
        Real squaredRadius = 0.0f;
        Vector3 min, max;
        bool first = true;
        for (i = 0; i < srcPosArray.GetCount(); ++i)
        {
            Vector3 pos = XSItoOgre(srcPosArray[i]);
            if (first)
            {
                squaredRadius = pos.squaredLength();
                min = max = pos;
                first = false;
            }
            else
            {
                squaredRadius = std::max(squaredRadius, pos.squaredLength());
                min.makeFloor(pos);
                max.makeCeil(pos);
            }

        }
        AxisAlignedBox box;
        box.setExtents(min, max);
        box.merge(pMesh->getBounds());
        pMesh->_setBounds(box);
        pMesh->_setBoundingSphereRadius(std::max(pMesh->getBoundingSphereRadius(), 
            Math::Sqrt(squaredRadius)));



    }
	//-----------------------------------------------------------------------
	void XsiMeshExporter::deriveSamplerIndices(const Triangle& tri, const PolygonFace& face, 
		size_t* samplerIndices)
	{
		//We want to find what is the SampleIndex associated with 3 vertex in a Triangle
		CPointRefArray facePoints = face.GetPoints();

		//Get the position of the 3 vertex in the triangle
		MATH::CVector3Array triPos = tri.GetPositionArray();

		//Get the position of the N Points in the polygon
		MATH::CVector3Array facePos = facePoints.GetPositionArray();

		//To know if the 3 vertex have a point in the same position
		bool found[3];
		found[0] = false;
		found[1] = false;
		found[2] = false;

		int p,t;
		//For the 3 triangle vertices
		for(t=0; t<3 ; t++)
		{       //for each polygon point
			for(p=0; p<facePos.GetCount() && !found[t]; p++)
			{
				//Check if the position is the same
				if(triPos[t] == facePos[p])
				{
					//if so, we know the PolygonPointIndex of the TriangleVertex
					//then, we must find what is the sample index associated with this Point
					samplerIndices[t] = getSamplerIndex(Facet(face), facePoints[p]);
					found[t] = true;
				}
			}

		}

		if (!found[0] || !found[1] || !found[2] )
		{
			// Problem!
			mXsiApp.LogMessage(L"!! Couldn't find a matching UV point!");
		}

	}
	//-----------------------------------------------------------------------
	size_t XsiMeshExporter::getSamplerIndex(const Facet &f, const Point &p)
	{
		//This function check if a Sample is shared by a Facet and a Point
		//just by using the operator=
		//Only one Sample can be shared.

		Sample curFacetSample;
		CSampleRefArray facetSamples( f.GetSamples() );
		CSampleRefArray pointSamples( p.GetSamples() );

		for(int i = 0; i < facetSamples.GetCount(); i++ )
		{

			curFacetSample = Sample( facetSamples[i] );

			for(int j = 0; j < pointSamples.GetCount(); j++)
			{
				if(curFacetSample == Sample(pointSamples[j]))
				{
					return curFacetSample.GetIndex();
				}
			}
		}
		// Problem!
		mXsiApp.LogMessage(L"!! Couldn't find a matching sample point!");
		return 0;
	}
    //-----------------------------------------------------------------------
    template <typename T> 
    void XsiMeshExporter::writeIndexes(T* buf)
    {
        IndexList::const_iterator i, iend;
        iend = mIndices.end();
        for (i = mIndices.begin(); i != iend; ++i)
        {
            *buf++ = static_cast<T>(*i);
        }
    }
    //-----------------------------------------------------------------------
    void XsiMeshExporter::createVertexBuffer(VertexData* vd, unsigned short bufIdx)
    {
        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            vd->vertexDeclaration->getVertexSize(bufIdx),
            vd->vertexCount, 
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        vd->vertexBufferBinding->setBinding(bufIdx, vbuf);
        size_t vertexSize = vd->vertexDeclaration->getVertexSize(bufIdx);

        char* pBase = static_cast<char*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

        VertexDeclaration::VertexElementList elems = vd->vertexDeclaration->findElementsBySource(bufIdx);
        VertexDeclaration::VertexElementList::iterator ei, eiend;
        eiend = elems.end();
        float* pFloat;
        RGBA* pRGBA;

        UniqueVertexList::iterator srci = mUniqueVertices.begin();

        for (size_t v = 0; v < vd->vertexCount; ++v, ++srci)
        {
            for (ei = elems.begin(); ei != eiend; ++ei)
            {
                VertexElement& elem = *ei;
                switch(elem.getSemantic())
                {
                case VES_POSITION:
                    elem.baseVertexPointerToElement(pBase, &pFloat);
                    *pFloat++ = srci->position.x;
                    *pFloat++ = srci->position.y;
                    *pFloat++ = srci->position.z;
                    break;
                case VES_NORMAL:
                    elem.baseVertexPointerToElement(pBase, &pFloat);
                    *pFloat++ = srci->normal.x;
                    *pFloat++ = srci->normal.y;
                    *pFloat++ = srci->normal.z;
                    break;
                case VES_DIFFUSE:
                    elem.baseVertexPointerToElement(pBase, &pRGBA);
                    *pRGBA = srci->colour;
                    break;
                case VES_TEXTURE_COORDINATES:
                    elem.baseVertexPointerToElement(pBase, &pFloat);
					for (int t = 0; t < VertexElement::getTypeCount(elem.getType()); ++t)
					{
						Real val = srci->uv[elem.getIndex()][t];
						*pFloat++ = val;
					}
                    break;
                }
            }
            pBase += vertexSize;
        }
        vbuf->unlock();

    }
    //-----------------------------------------------------------------------
    void XsiMeshExporter::startPolygonMesh(size_t count, size_t indexCount)
    {
        mUniqueVertices.clear();
        mUniqueVertices.resize(count);
        mIndices.clear();
        mIndices.reserve(indexCount); // intentionally reserved, not resized
    }
    //-----------------------------------------------------------------------
    size_t XsiMeshExporter::createOrRetrieveUniqueVertex(size_t originalPositionIndex, 
        const UniqueVertex& vertex)
    {
        UniqueVertex& orig = mUniqueVertices[originalPositionIndex];

        if (!orig.initialised)
        {
            orig = vertex;
            orig.initialised = true;
            return originalPositionIndex;
        }
        else if (orig == vertex)
        {
            return originalPositionIndex;
        }
        else
        {
            // no match, go to next or create new
            if (orig.nextIndex)
            {
                // cascade
                return createOrRetrieveUniqueVertex(orig.nextIndex, vertex);
            }
            else
            {
                // get new index
                size_t newindex = mUniqueVertices.size();
                orig.nextIndex = newindex;
                // create new (NB invalidates 'orig' reference)
                mUniqueVertices.push_back(vertex);
                // set initialised
                mUniqueVertices[newindex].initialised = true;

                return newindex;
            }
        }
    }

}
