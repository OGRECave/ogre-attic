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
#include "OgreStringConverter.h"



namespace Ogre {

    //---------------------------------------------------------------------
    XMLMeshSerializer::XMLMeshSerializer()
    {
    }
    //---------------------------------------------------------------------
    XMLMeshSerializer::~XMLMeshSerializer()
    {
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

        mXMLDoc = new TiXmlDocument();
        mXMLDoc->InsertEndChild(TiXmlElement("mesh"));
        TiXmlElement* rootNode = mXMLDoc->RootElement();

        LogManager::getSingleton().logMessage("Populating DOM...");

            
        // Write materials if required
        if (includeMaterials)
        {
            MaterialManager& matMgr = MaterialManager::getSingleton();
            LogManager::getSingleton().logMessage("Writing Materials...");
            // Insert a 'Materials' node to contain
            TiXmlElement* matsNode = 
                rootNode->InsertEndChild(TiXmlElement("materials"))->ToElement();
            
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
        mXMLDoc->SaveFile(filename);

    
        delete mXMLDoc;

        LogManager::getSingleton().logMessage("XMLMeshSerializer export successful.");

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeMesh(const Mesh* pMesh)
    {
        TiXmlElement* rootNode = mXMLDoc->RootElement();
        // Write geometry
        TiXmlElement* geomNode = 
            rootNode->InsertEndChild(TiXmlElement("sharedgeometry"))->ToElement();
        writeGeometry(geomNode, &pMesh->sharedGeometry);

        // Write Submeshes
        TiXmlElement* subMeshesNode = 
            rootNode->InsertEndChild(TiXmlElement("submeshes"))->ToElement();
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
                TiXmlElement* boneAssignNode = 
                    rootNode->InsertEndChild(TiXmlElement("boneassignments"))->ToElement();

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
    void XMLMeshSerializer::writeMaterial(TiXmlElement* mMaterialsNode, const Material* m)
    {
        // Create Node
        TiXmlElement* matNode = 
            mMaterialsNode->InsertEndChild(TiXmlElement("material"))->ToElement();

        // Name
        matNode->SetAttribute("name", m->getName());

        // Ambient
        TiXmlElement* subNode = 
            matNode->InsertEndChild(TiXmlElement("ambient"))->ToElement();
        const ColourValue& ambient = m->getAmbient();
        subNode->SetAttribute("red", StringConverter::toString(ambient.r));
        subNode->SetAttribute("green", StringConverter::toString(ambient.g));
        subNode->SetAttribute("blue", StringConverter::toString(ambient.b));

        // Diffuse
        subNode = 
            matNode->InsertEndChild(TiXmlElement("diffuse"))->ToElement();
        const ColourValue& diffuse = m->getDiffuse();
        subNode->SetAttribute("red", StringConverter::toString(diffuse.r));
        subNode->SetAttribute("green", StringConverter::toString(diffuse.g));
        subNode->SetAttribute("blue", StringConverter::toString(diffuse.b));

        // Specular
        subNode = 
            matNode->InsertEndChild(TiXmlElement("specular"))->ToElement();
        const ColourValue& specular = m->getSpecular();
        subNode->SetAttribute("red", StringConverter::toString(specular.r));
        subNode->SetAttribute("green", StringConverter::toString(specular.g));
        subNode->SetAttribute("blue", StringConverter::toString(specular.b));

        // Shininess
        subNode = matNode->InsertEndChild(TiXmlElement("shininess"))->ToElement();
        subNode->SetAttribute("value", StringConverter::toString(m->getShininess()));

        // Nested texture layers
        TiXmlElement* mLayersNode = 
             matNode->InsertEndChild(TiXmlElement("texturelayers"))->ToElement();
        for (int i = 0; i < m->getNumTextureLayers(); ++i)
        {
            writeTextureLayer(mLayersNode, m->getTextureLayer(i));
        }
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeTextureLayer(TiXmlElement* mLayersNode, const Material::TextureLayer* pTex)
    {
        TiXmlElement* texNode = 
            mLayersNode->InsertEndChild(TiXmlElement("texturelayer"))->ToElement();

        texNode->SetAttribute("texture", pTex->getTextureName());
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSubMesh(TiXmlElement* mMeshNode, const SubMesh* s)
    {
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeGeometry(TiXmlElement* mParentNode, const GeometryData* pGeom)
    {
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSkeletonLink(TiXmlElement* mMeshNode, const Skeleton* pSkel)
    {
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeMeshBoneAssignment(TiXmlElement* mBoneAssignNode, const VertexBoneAssignment* assign)
    {
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSubMeshBoneAssignment(TiXmlElement* mBoneAssignNode, const VertexBoneAssignment* assign)
    {
    }
    //---------------------------------------------------------------------
}