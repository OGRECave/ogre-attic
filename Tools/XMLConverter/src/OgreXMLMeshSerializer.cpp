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
    void XMLMeshSerializer::importMesh(const String& filename, Mesh* pMesh)
    {
        mpMesh = pMesh;
        mXMLDoc = new TiXmlDocument(filename);
        mXMLDoc->LoadFile();

        TiXmlElement* elem;

        // materials
        elem = mXMLDoc->FirstChildElement("materials");
        if (elem)
            readMaterials(elem);

        // shared geometry
        elem = mXMLDoc->FirstChildElement("sharedgeometry");
        if (elem)
            readGeometry(elem, &mpMesh->sharedGeometry);

        // submeshes
        elem = mXMLDoc->FirstChildElement("submeshes");
        if (elem)
            readSubMeshes(elem);

        // skeleton link
        elem = mXMLDoc->FirstChildElement("skeletonlink");
        if (elem)
            readSkeletonLink(elem);

        // bone assignments
        elem = mXMLDoc->FirstChildElement("boneassignments");
        if (elem)
            readBoneAssignments(elem);
        
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::exportMesh(const Mesh* pMesh, const String& filename, bool includeMaterials)
    {
        LogManager::getSingleton().logMessage("XMLMeshSerializer writing mesh data to " + filename + "...");
        
        mpMesh = const_cast<Mesh*>(pMesh);

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
            writeSkeletonLink(rootNode, pMesh->getSkeletonName());
            LogManager::getSingleton().logMessage("Skeleton link exported.");

            // Write bone assignments
            Mesh::BoneAssignmentIterator bi = const_cast<Mesh*>(pMesh)->getBoneAssignmentIterator();
            if (bi.hasMoreElements())
            {
                LogManager::getSingleton().logMessage("Exporting shared geometry bone assignments...");
                TiXmlElement* boneAssignNode = 
                    rootNode->InsertEndChild(TiXmlElement("boneassignments"))->ToElement();

                while (bi.hasMoreElements())
                {
                    writeBoneAssignment(boneAssignNode, &(bi.getNext()));
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
        subNode->SetAttribute("alpha", StringConverter::toString(ambient.a));

        // Diffuse
        subNode = 
            matNode->InsertEndChild(TiXmlElement("diffuse"))->ToElement();
        const ColourValue& diffuse = m->getDiffuse();
        subNode->SetAttribute("red", StringConverter::toString(diffuse.r));
        subNode->SetAttribute("green", StringConverter::toString(diffuse.g));
        subNode->SetAttribute("blue", StringConverter::toString(diffuse.b));
        subNode->SetAttribute("alpha", StringConverter::toString(diffuse.a));

        // Specular
        subNode = 
            matNode->InsertEndChild(TiXmlElement("specular"))->ToElement();
        const ColourValue& specular = m->getSpecular();
        subNode->SetAttribute("red", StringConverter::toString(specular.r));
        subNode->SetAttribute("green", StringConverter::toString(specular.g));
        subNode->SetAttribute("blue", StringConverter::toString(specular.b));
        subNode->SetAttribute("alpha", StringConverter::toString(specular.a));

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
    void XMLMeshSerializer::writeSubMesh(TiXmlElement* mSubMeshesNode, const SubMesh* s)
    {
        TiXmlElement* subMeshNode = 
            mSubMeshesNode->InsertEndChild(TiXmlElement("submesh"))->ToElement();

        // Material name
        subMeshNode->SetAttribute("material", s->getMaterialName());
        // bool useSharedVertices
        subMeshNode->SetAttribute("useSharedVertices", 
            StringConverter::toString(s->useSharedVertices) );

        // Faces
        TiXmlElement* facesNode = 
            subMeshNode->InsertEndChild(TiXmlElement("faces"))->ToElement();
        facesNode->SetAttribute("count", StringConverter::toString(s->numFaces));
        // Write each face in turn
        ushort i;
        ushort* pFace = s->faceVertexIndices;
        for (i = 0; i < s->numFaces * 3; i += 3)
        {
            TiXmlElement* faceNode = 
                facesNode->InsertEndChild(TiXmlElement("face"))->ToElement();
            faceNode->SetAttribute("v1", StringConverter::toString(*pFace++));
            faceNode->SetAttribute("v2", StringConverter::toString(*pFace++));
            faceNode->SetAttribute("v3", StringConverter::toString(*pFace++));
        }

        // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
        if (!s->useSharedVertices)
        {
            TiXmlElement* geomNode = 
                subMeshNode->InsertEndChild(TiXmlElement("geometry"))->ToElement();
            writeGeometry(geomNode, &s->geometry);
        }

        // Bone assignments
        if (mpMesh->hasSkeleton())
        {
            SubMesh::BoneAssignmentIterator bi = const_cast<SubMesh*>(s)->getBoneAssignmentIterator();
            LogManager::getSingleton().logMessage("Exporting dedicated geometry bone assignments...");

            TiXmlElement* boneAssignNode = 
                subMeshNode->InsertEndChild(TiXmlElement("boneassignments"))->ToElement();
            while (bi.hasMoreElements())
            {
                writeBoneAssignment(boneAssignNode, &bi.getNext());
            }
        }
        LogManager::getSingleton().logMessage("Dedicated geometry bone assignments exported.");

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeGeometry(TiXmlElement* mParentNode, const GeometryData* pGeom)
    {
        // Write a vertex buffer per element
        // TODO when we do VBs properly, we probably want to create some 
        //   shared buffers

        TiXmlElement *vbNode, *vertexNode, *dataNode;
        Real* pReal;
        ushort i;

        // Set num verts on parent
        mParentNode->SetAttribute("count", StringConverter::toString(pGeom->numVertices));

        vbNode = mParentNode->InsertEndChild(TiXmlElement("vertexbuffer"))->ToElement();
        vbNode->SetAttribute("positions","true");
        vbNode->SetAttribute("normals","false");
        vbNode->SetAttribute("colours","false");
        vbNode->SetAttribute("numtexcoords","0");
        pReal = pGeom->pVertices;
        for (i = 0; i < pGeom->numVertices; ++i)
        {
            vertexNode = 
                vbNode->InsertEndChild(TiXmlElement("vertex"))->ToElement();
            dataNode = 
                vertexNode->InsertEndChild(TiXmlElement("position"))->ToElement();
            dataNode->SetAttribute("x", StringConverter::toString(*pReal++));
            dataNode->SetAttribute("y", StringConverter::toString(*pReal++));
            dataNode->SetAttribute("z", StringConverter::toString(*pReal++));
        }

        if (pGeom->hasNormals)
        {
            vbNode = mParentNode->InsertEndChild(TiXmlElement("vertexbuffer"))->ToElement();
            vbNode->SetAttribute("count", StringConverter::toString(pGeom->numVertices));
            vbNode->SetAttribute("positions","false");
            vbNode->SetAttribute("normals","true");
            vbNode->SetAttribute("colours","false");
            vbNode->SetAttribute("numtexcoords","0");
            pReal = pGeom->pNormals;
            for (i = 0; i < pGeom->numVertices; ++i)
            {
                vertexNode = 
                    vbNode->InsertEndChild(TiXmlElement("vertex"))->ToElement();
                dataNode = 
                    vertexNode->InsertEndChild(TiXmlElement("normal"))->ToElement();
                dataNode->SetAttribute("x", StringConverter::toString(*pReal++));
                dataNode->SetAttribute("y", StringConverter::toString(*pReal++));
                dataNode->SetAttribute("z", StringConverter::toString(*pReal++));
            }
        }

        if (pGeom->hasColours)
        {
            vbNode = mParentNode->InsertEndChild(TiXmlElement("vertexbuffer"))->ToElement();
            vbNode->SetAttribute("count", StringConverter::toString(pGeom->numVertices));
            vbNode->SetAttribute("positions","false");
            vbNode->SetAttribute("normals","false");
            vbNode->SetAttribute("colours","true");
            vbNode->SetAttribute("numtexcoords","0");
            RGBA* pColour = pGeom->pColours;
            for (i = 0; i < pGeom->numVertices; ++i)
            {
                vertexNode = 
                    vbNode->InsertEndChild(TiXmlElement("vertex"))->ToElement();
                dataNode = 
                    vertexNode->InsertEndChild(TiXmlElement("colour"))->ToElement();

                dataNode->SetAttribute("value", StringConverter::toString(*pColour++));
            }
        }

        for (int t = 0; t < pGeom->numTexCoords; ++t)
        {
            vbNode = mParentNode->InsertEndChild(TiXmlElement("vertexbuffer"))->ToElement();
            vbNode->SetAttribute("count", StringConverter::toString(pGeom->numVertices));
            vbNode->SetAttribute("positions","false");
            vbNode->SetAttribute("normals","false");
            vbNode->SetAttribute("colours","false");
            vbNode->SetAttribute("numtexcoords","1");
            // NB if later we do shared buffers this will be space-separated per set
            vbNode->SetAttribute("texcoordsets", StringConverter::toString(t));
            // NB if later we do shared buffers this will be space-separated per set
            vbNode->SetAttribute("texcoorddimensions", 
                StringConverter::toString(pGeom->numTexCoordDimensions[t]));
            pReal = pGeom->pTexCoords[t];
            for (i = 0; i < pGeom->numVertices; ++i)
            {
                vertexNode = 
                    vbNode->InsertEndChild(TiXmlElement("vertex"))->ToElement();
                dataNode = 
                    vertexNode->InsertEndChild(TiXmlElement("texcoord"))->ToElement();

                dataNode->SetAttribute("u", StringConverter::toString(*pReal++));
                if (pGeom->numTexCoordDimensions[t] > 1)
                {
                    dataNode->SetAttribute("v", StringConverter::toString(*pReal++));
                }
                if (pGeom->numTexCoordDimensions[t] > 2)
                {
                    dataNode->SetAttribute("w", StringConverter::toString(*pReal++));
                }
            }
        }

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSkeletonLink(TiXmlElement* mMeshNode, const String& skelName)
    {

        TiXmlElement* skelNode = 
            mMeshNode->InsertEndChild(TiXmlElement("skeletonlink"))->ToElement();
        skelNode->SetAttribute("name", skelName);
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeBoneAssignment(TiXmlElement* mBoneAssignNode, const VertexBoneAssignment* assign)
    {
        TiXmlElement* assignNode = 
            mBoneAssignNode->InsertEndChild(
            TiXmlElement("vertexboneassignment"))->ToElement();

        assignNode->SetAttribute("vertexIndex", 
            StringConverter::toString(assign->vertexIndex));
        assignNode->SetAttribute("boneIndex", 
            StringConverter::toString(assign->boneIndex));
        assignNode->SetAttribute("weight",
            StringConverter::toString(assign->weight));


    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readMaterials(TiXmlElement* mMaterialsNode)
    {
        MaterialManager matMgr = MaterialManager::getSingleton();

        // All children will be materials
        for (TiXmlElement* matElem = mMaterialsNode->FirstChildElement();
            matElem != 0; matElem = matElem->NextSiblingElement())
        {
            String name = matElem->Attribute("name");
            Material* newMat = (Material*)matMgr.createDeferred(name);

            ColourValue col;
            TiXmlElement* colElem;
            // Ambient
            colElem = matElem->FirstChildElement("ambient");
            if (colElem)
            {
                col.r = StringConverter::parseReal(colElem->Attribute("red"));
                col.g = StringConverter::parseReal(colElem->Attribute("green"));
                col.b = StringConverter::parseReal(colElem->Attribute("blue"));
                col.a = StringConverter::parseReal(colElem->Attribute("alpha"));
                newMat->setAmbient(col);
            }
            // Diffuse
            colElem = matElem->FirstChildElement("diffuse");
            if (colElem)
            {
                col.r = StringConverter::parseReal(colElem->Attribute("red"));
                col.g = StringConverter::parseReal(colElem->Attribute("green"));
                col.b = StringConverter::parseReal(colElem->Attribute("blue"));
                col.a = StringConverter::parseReal(colElem->Attribute("alpha"));
                newMat->setDiffuse(col);
            }
            // Specular
            colElem = matElem->FirstChildElement("diffuse");
            if (colElem)
            {
                col.r = StringConverter::parseReal(colElem->Attribute("red"));
                col.g = StringConverter::parseReal(colElem->Attribute("green"));
                col.b = StringConverter::parseReal(colElem->Attribute("blue"));
                col.a = StringConverter::parseReal(colElem->Attribute("alpha"));
                newMat->setSpecular(col);
            }
            // Shininess
            colElem = matElem->FirstChildElement("shininess");
            if (colElem)
            {
                newMat->setShininess(StringConverter::parseReal(colElem->Attribute("value")));
            }

            // Texture layers
            TiXmlElement* texLayersElem = matElem->FirstChildElement("texturelayers");
            if (texLayersElem)
            {
                for (TiXmlElement* texElem = texLayersElem->FirstChildElement();
                    texElem != 0; texElem = texElem->NextSiblingElement())
                {
                    newMat->addTextureLayer(texElem->Attribute("texture"));
                }

            } // texture layers

        } // material

    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readSubMeshes(TiXmlElement* mSubmeshesNode)
    {
        for (TiXmlElement* smElem = mSubmeshesNode->FirstChildElement();
            smElem != 0; smElem = smElem->NextSiblingElement())
        {
            // All children should be submeshes 
            SubMesh* sm = mpMesh->createSubMesh();

            sm->setMaterialName(smElem->Attribute("material"));
            sm->useSharedVertices = StringConverter::parseBool(smElem->Attribute("useSharedVertices"));

            // Faces
            TiXmlElement* faces = smElem->FirstChildElement("faces");
            sm->numFaces = StringConverter::parseInt(faces->Attribute("count"));
            // Allocate space
            sm->faceVertexIndices = new unsigned short[sm->numFaces * 3];
            int faceIdx;
            TiXmlElement* faceElem;
            for (faceElem = faces->FirstChildElement(), faceIdx = 0;
                faceElem != 0; faceElem = faceElem->NextSiblingElement(), faceIdx += 3)
            {
                sm->faceVertexIndices[faceIdx] = StringConverter::parseInt(faceElem->Attribute("v1"));
                sm->faceVertexIndices[faceIdx+1] = StringConverter::parseInt(faceElem->Attribute("v2"));
                sm->faceVertexIndices[faceIdx+2] = StringConverter::parseInt(faceElem->Attribute("v3"));
            }

            // Geometry
            if (!sm->useSharedVertices)
            {
                TiXmlElement* geomNode = smElem->FirstChildElement("geometry");
                if (geomNode)
                    readGeometry(geomNode, &sm->geometry);
            }

            // Bone assignments
            TiXmlElement* boneAssigns = smElem->FirstChildElement("boneassignments");
            if(boneAssigns)
                readBoneAssignments(boneAssigns, sm);

        }
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readGeometry(TiXmlElement* mGeometryNode, GeometryData* pGeom)
    {
        pGeom->numVertices = StringConverter::parseInt(mGeometryNode->Attribute("count"));
        // Skip empty 
        if (pGeom->numVertices <= 0) return;

        // Iterate over all children (vertexbuffer entries)
        for (TiXmlElement* vbElem = mGeometryNode->FirstChildElement();
            vbElem != 0; vbElem = vbElem->NextSiblingElement())
        {
            // Skip non-vertexbuffer elems
            if (stricmp(vbElem->Value(), "vertexbuffer")) continue;

            // Determine type(s) present per vertex
            pGeom->hasNormals = pGeom->hasColours = false;
            pGeom->numTexCoords = 0;

            const char* attrib = vbElem->Attribute("positions");
            if (attrib && StringConverter::parseBool(attrib))
            {
                pGeom->pVertices = new Real[pGeom->numVertices * 3];
                // TODO change when we do shared bufs
                pGeom->vertexStride = 0;
            }
            attrib = vbElem->Attribute("normals");
            if (attrib && StringConverter::parseBool(attrib))
            {
                pGeom->hasNormals = true;
                pGeom->pNormals = new Real[pGeom->numVertices * 3];
                // TODO change when we do shared bufs
                pGeom->normalStride = 0;
            }
            attrib = vbElem->Attribute("colours");
            if (attrib && StringConverter::parseBool(attrib))
            {
                pGeom->hasColours = true;
                pGeom->pColours = new RGBA[pGeom->numVertices];
                // TODO change when we do shared bufs
                pGeom->colourStride = 0;
            }
            attrib = vbElem->Attribute("texcoords");
            if (attrib && StringConverter::parseBool(attrib))
            {
                pGeom->numTexCoords = StringConverter::parseInt(vbElem->Attribute("numtexcoords"));
                String sets = vbElem->Attribute("texcoordsets");
                String dims = vbElem->Attribute("texcoorddimensions");

                std::vector<String> vecSets = sets.split(" ");
                std::vector<String> vecDims = dims.split(" ");

                for (int v = 0; v < vecSets.size(); ++v)
                {
                    int set = StringConverter::parseInt(vecSets[v]);
                    int dim = StringConverter::parseInt(vecDims[v]);
                    pGeom->numTexCoordDimensions[set] = dim;
                    pGeom->pTexCoords[set] = new Real[pGeom->numVertices * dim];
                    // TODO change when we do shared bufs
                    pGeom->texCoordStride[set] = 0;

                }


            }




        }


    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readSkeletonLink(TiXmlElement* mSkelNode)
    {
        // TODO
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readBoneAssignments(TiXmlElement* mBoneAssignmentsNode)
    {
        // TODO
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readBoneAssignments(TiXmlElement* mBoneAssignmentsNode, SubMesh* sm)
    {
        // TODO
    }

}