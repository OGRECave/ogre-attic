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
#include "OgreXSIMeshExporter.h"
#include <xsi_model.h>
#include <xsi_primitive.h>
#include <xsi_point.h>
#include <xsi_polygonnode.h>
#include <xsi_polygonmesh.h>
#include <xsi_facet.h>
#include <xsi_material.h>

#include "OgreException.h"
#include "OgreXSIHelper.h"
#include "OgreLogManager.h"
#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreMeshManager.h"
#include "OgreDefaultHardwareBufferManager.h"

using namespace XSI;

namespace Ogre {
    //-----------------------------------------------------------------------
    XsiMeshExporter::XsiMeshExporter()
    {
        // software buffer manager
        mBufferManager = new DefaultHardwareBufferManager();
        mMeshManager = new MeshManager();
    }
    //-----------------------------------------------------------------------
    XsiMeshExporter::~XsiMeshExporter()
    {
        delete mMeshManager;
        delete mBufferManager;
    }
    //-----------------------------------------------------------------------
    void XsiMeshExporter::exportMesh(const CString& fileName, 
        const CString& objectName, bool edgeLists, bool tangents)
    {
        // Derive the scene root
        X3DObject sceneRoot(mXsiApp.GetActiveSceneRoot());

        // Construct mesh
        Mesh* pMesh = MeshManager::getSingleton().createManual("XSIExport");

        if (objectName.IsEmpty())
        {
            // export the entire scene
            exportX3DObject(pMesh, sceneRoot);
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
                Except(Exception::ERR_ITEM_NOT_FOUND, 
                    "No object named " + XSItoOgre(objectName) + " found.",
                    "XsiMeshExporter::exportMesh");
            }

            exportX3DObject(pMesh, obj);

        }
    }
    //-----------------------------------------------------------------------
    void XsiMeshExporter::exportX3DObject(Mesh* pMesh, X3DObject& x3dObj)
    {
        // Check validity of current object
        if (!x3dObj.IsValid())
        {
            Except(Exception::ERR_INTERNAL_ERROR, 
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

        CFacetRefArray facetArray(geom.GetFacets());

#if _DEBUG
        StringUtil::StrStreamType msg;
        msg << "Points: " << pointArray.GetCount() << std::endl;
        msg << "Nodes: " << nodeArray.GetCount() << std::endl;
        msg << "Position Array Size: " << srcPosArray.GetCount() << std::endl;
        msg << "Normal Array Size: " << srcNormArray.GetCount() << std::endl;
        msg << "Facets: " << facetArray.GetCount() << std::endl;
        msg << "Indexes: " << (facetArray.GetCount() * 3) << std::endl;
        String str = msg.str();
        mXsiApp.LogMessage(OgretoXSI(str));
#endif
        // never use shared geometry
        sm->useSharedVertices = false;
        sm->vertexData = new VertexData();
        sm->vertexData->vertexCount = static_cast<size_t>(pointArray.GetCount());
        // always do triangle list
        sm->indexData->indexCount = static_cast<size_t>(facetArray.GetCount() * 3);





    }
}
