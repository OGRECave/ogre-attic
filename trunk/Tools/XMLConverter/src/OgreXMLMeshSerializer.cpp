/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/


#include "OgreXMLMeshSerializer.h"
#include "OgreSubMesh.h"
#include "OgreMaterialManager.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"



namespace Ogre {

    //---------------------------------------------------------------------
    XMLMeshSerializer::XMLMeshSerializer()
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch(const XMLException& toCatch)
        {
            char msg[256];
            XMLString::transcode(toCatch.getMessage(), msg, 256);
            String fullMsg = "Error during Xerces-c Initialization: ";
            fullMsg << msg;
            LogManager::getSingleton().logMessage(fullMsg);
        }
    }
    //---------------------------------------------------------------------
    XMLMeshSerializer::~XMLMeshSerializer()
    {
        XMLPlatformUtils::Terminate();
    }
    //---------------------------------------------------------------------
    Mesh* XMLMeshSerializer::importMesh(const String& filename, Mesh* pMesh)
    {
        // TODO 
        return NULL;
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::exportMesh(const Mesh* pMesh, const String& filename, bool includeMaterials)
    {
        LogManager::getSingleton().logMessage("XMLMeshSerializer writing mesh data to " + filename + "...");

        DOMImplementation* impl =  
            DOMImplementationRegistry::getDOMImplementation(X("Core"));

        mXMLDoc = impl->createDocument(
                    0,                    // root element namespace URI.
                    X("mesh"),            // root element name
                    0);                   // document type object (DTD).

        LogManager::getSingleton().logMessage("Populating DOM...");

            
        // Write materials if required
        if (includeMaterials)
        {
            MaterialManager& matMgr = MaterialManager::getSingleton();
            LogManager::getSingleton().logMessage("Writing Materials...");
            // Insert a 'Materials' node to contain
            DOMElement* rootNode = mXMLDoc->getDocumentElement();
            DOMElement* matsNode = mXMLDoc->createElement(X("materials"));
            rootNode->appendChild(matsNode);
            
            for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
            {
                SubMesh* sm = pMesh->getSubMesh(i);
                Material* pMat = (Material*)matMgr.getByName(sm->getMaterialName());
                if (pMat)
                {
                    LogManager::getSingleton().logMessage("Exporting Material '" + pMat->getName() + "'...");
                    writeMaterial(matsNode, pMat);
                    LogManager::getSingleton().logMessage("Material '" + pMat->getName() + "' exported.");
                }
            }
        }
            
        // Write to DOM
        writeMesh(pMesh);
        LogManager::getSingleton().logMessage("DOM populated, writing XML file..");

        // Write out to a file
        DOMWriter* writer = impl->createDOMWriter();
        LocalFileFormatTarget target(X(filename));
        writer->writeNode(&target, *mXMLDoc);

        writer->release();

    
        mXMLDoc->release();

        LogManager::getSingleton().logMessage("XMLMeshSerializer export successful.");

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeMesh(const Mesh* pMesh)
    {
        DOMElement* rootNode = mXMLDoc->getDocumentElement();
        // Write geometry
        DOMElement* geomNode = mXMLDoc->createElement(X("sharedgeometry"));
        rootNode->appendChild(geomNode);
        writeGeometry(geomNode, &pMesh->sharedGeometry);

        // Write Submeshes
        DOMElement* subMeshesNode = mXMLDoc->createElement(X("submeshes"));
        rootNode->appendChild(subMeshesNode);
        for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            LogManager::getSingleton().logMessage("Writing submesh...");
            writeSubMesh(subMeshesNode, pMesh->getSubMesh(i));
            LogManager::getSingleton().logMessage("Submesh exported.");
        }

        // Write skeleton info if required
        if (pMesh->hasSkeleton())
        {
            LogManager::getSingleton().logMessage("Exporting skeleton link...");
            // Write skeleton link
            writeSkeletonLink(rootNode, pMesh->getSkeleton());
            LogManager::getSingleton().logMessage("Skeleton link exported.");

            // Write bone assignments
            if (!pMesh->mBoneAssignments.empty())
            {
                LogManager::getSingleton().logMessage("Exporting shared geometry bone assignments...");
                DOMElement* boneAssignNode = mXMLDoc->createElement(X("boneassignments"));
                rootNode->appendChild(boneAssignNode);

                Mesh::VertexBoneAssignmentList::const_iterator vi;
                for (vi = pMesh->mBoneAssignments.begin(); 
                vi != pMesh->mBoneAssignments.end(); ++vi)
                {
                    writeMeshBoneAssignment(boneAssignNode, &(vi->second));
                }

                LogManager::getSingleton().logMessage("Shared geometry bone assignments exported.");
            }
        }





    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeMaterial(DOMElement* mMaterialsNode, const Material* m)
    {
        // Create Node
        DOMElement* matNode = mXMLDoc->createElement(X("material"));
        mMaterialsNode->appendChild(matNode);

        // Name
        matNode->setAttribute(X("name"), X(m->getName()));

        // Ambient
        DOMElement* subNode = mXMLDoc->createElement(X("ambient"));
        matNode->appendChild(subNode);
        const ColourValue& ambient = m->getAmbient();
        subNode->setAttribute(X("red"), X(ambient.r));
        subNode->setAttribute(X("green"), X(ambient.g));
        subNode->setAttribute(X("blue"), X(ambient.b));

        // Diffuse
        subNode = mXMLDoc->createElement(X("diffuse"));
        matNode->appendChild(subNode);
        const ColourValue& diffuse = m->getDiffuse();
        subNode->setAttribute(X("red"), X(diffuse.r));
        subNode->setAttribute(X("green"), X(diffuse.g));
        subNode->setAttribute(X("blue"), X(diffuse.b));

        // Specular
        subNode = mXMLDoc->createElement(X("specular"));
        matNode->appendChild(subNode);
        const ColourValue& specular = m->getSpecular();
        subNode->setAttribute(X("red"), X(specular.r));
        subNode->setAttribute(X("green"), X(specular.g));
        subNode->setAttribute(X("blue"), X(specular.b));

        // Shininess
        subNode = mXMLDoc->createElement(X("shininess"));
        matNode->appendChild(subNode);
        subNode->setAttribute(X("value"), X(m->getShininess()));

        // Nested texture layers
        DOMElement* mLayersNode = mXMLDoc->createElement(X("texturelayers"));
        matNode->appendChild(mLayersNode);
        for (int i = 0; i < m->getNumTextureLayers(); ++i)
        {
            writeTextureLayer(mLayersNode, m->getTextureLayer(i));
        }
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeTextureLayer(DOMElement* mLayersNode, const Material::TextureLayer* pTex)
    {
        DOMElement* texNode = mXMLDoc->createElement(X("texturelayer"));
        mLayersNode->appendChild(texNode);

        texNode->setAttribute(X("texture"), X(pTex->getTextureName()));
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSubMesh(DOMElement* mMeshNode, const SubMesh* s)
    {
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeGeometry(DOMElement* mParentNode, const GeometryData* pGeom)
    {
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSkeletonLink(DOMElement* mMeshNode, const Skeleton* pSkel)
    {
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeMeshBoneAssignment(DOMElement* mBoneAssignNode, const VertexBoneAssignment* assign)
    {
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSubMeshBoneAssignment(DOMElement* mBoneAssignNode, const VertexBoneAssignment* assign)
    {
    }
    //---------------------------------------------------------------------
}