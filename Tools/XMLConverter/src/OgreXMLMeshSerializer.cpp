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


#include "OgreXMLMeshSerializer.h"
#include "OgreSubMesh.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"
#include "OgreStringConverter.h"
#include "OgreHardwareBufferManager.h"


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
        LogManager::getSingleton().logMessage("XMLMeshSerializer reading mesh data from " + filename + "...");
        mpMesh = pMesh;
        mXMLDoc = new TiXmlDocument(filename);
        mXMLDoc->LoadFile();

        TiXmlElement* elem;

        TiXmlElement* rootElem = mXMLDoc->RootElement();

        // shared geometry
        elem = rootElem->FirstChildElement("sharedgeometry");
        if (elem)
            readGeometry(elem, mpMesh->sharedVertexData);

        // submeshes
        elem = rootElem->FirstChildElement("submeshes");
        if (elem)
            readSubMeshes(elem);

        // skeleton link
        elem = rootElem->FirstChildElement("skeletonlink");
        if (elem)
            readSkeletonLink(elem);

        // bone assignments
        elem = rootElem->FirstChildElement("boneassignments");
        if (elem)
            readBoneAssignments(elem);

		//Lod
		elem = rootElem->FirstChildElement("levelofdetail");
		if (elem)
			readLodInfo(elem);

        delete mXMLDoc;

        LogManager::getSingleton().logMessage("XMLMeshSerializer import successful.");
        
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::exportMesh(const Mesh* pMesh, const String& filename)
    {
        LogManager::getSingleton().logMessage("XMLMeshSerializer writing mesh data to " + filename + "...");
        
        mpMesh = const_cast<Mesh*>(pMesh);

        mXMLDoc = new TiXmlDocument();
        mXMLDoc->InsertEndChild(TiXmlElement("mesh"));
        TiXmlElement* rootNode = mXMLDoc->RootElement();

        LogManager::getSingleton().logMessage("Populating DOM...");

            
           
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
		if (pMesh->sharedVertexData)
		{
			TiXmlElement* geomNode = 
				rootNode->InsertEndChild(TiXmlElement("sharedgeometry"))->ToElement();
			writeGeometry(geomNode, pMesh->sharedVertexData);
		}

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
		if (pMesh->getNumLodLevels() > 1)
		{
            LogManager::getSingleton().logMessage("Exporting LOD information...");
			writeLodInfo(rootNode, pMesh);
            LogManager::getSingleton().logMessage("LOD information exported.");
		}





    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::writeSubMesh(TiXmlElement* mSubMeshesNode, const SubMesh* s)
    {
        TiXmlElement* subMeshNode = 
            mSubMeshesNode->InsertEndChild(TiXmlElement("submesh"))->ToElement();

        // Material name
        subMeshNode->SetAttribute("material", s->getMaterialName());
        // bool useSharedVertices
        subMeshNode->SetAttribute("usesharedvertices", 
            StringConverter::toString(s->useSharedVertices) );
        // bool use32BitIndexes
		bool use32BitIndexes = (s->indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);
        subMeshNode->SetAttribute("use32bitindexes", 
            StringConverter::toString( use32BitIndexes ));

        // Faces
        TiXmlElement* facesNode = 
            subMeshNode->InsertEndChild(TiXmlElement("faces"))->ToElement();
        facesNode->SetAttribute("count", StringConverter::toString(s->indexData->indexCount / 3));
        // Write each face in turn
        ushort i;
		unsigned int* pInt;
		unsigned short* pShort;
		HardwareIndexBufferSharedPtr ibuf = s->indexData->indexBuffer;
		if (use32BitIndexes)
		{
			pInt = static_cast<unsigned int*>(
				ibuf->lock(0, ibuf->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY)); 
		}
		else
		{
			pShort = static_cast<unsigned short*>(
				ibuf->lock(0, ibuf->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY)); 
		}
        for (i = 0; i < s->indexData->indexCount; i += 3)
        {
            TiXmlElement* faceNode = 
                facesNode->InsertEndChild(TiXmlElement("face"))->ToElement();
			if (use32BitIndexes)
			{
				faceNode->SetAttribute("v1", StringConverter::toString(*pInt++));
				faceNode->SetAttribute("v2", StringConverter::toString(*pInt++));
				faceNode->SetAttribute("v3", StringConverter::toString(*pInt++));
			}
			else
			{
				faceNode->SetAttribute("v1", StringConverter::toString(*pShort++));
				faceNode->SetAttribute("v2", StringConverter::toString(*pShort++));
				faceNode->SetAttribute("v3", StringConverter::toString(*pShort++));
			}
        }

        // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
        if (!s->useSharedVertices)
        {
            TiXmlElement* geomNode = 
                subMeshNode->InsertEndChild(TiXmlElement("geometry"))->ToElement();
            writeGeometry(geomNode, s->vertexData);
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
    void XMLMeshSerializer::writeGeometry(TiXmlElement* mParentNode, const VertexData* vertexData)
    {
        // Write a vertex buffer per element

        TiXmlElement *vbNode, *vertexNode, *dataNode;

        // Set num verts on parent
        mParentNode->SetAttribute("vertexcount", StringConverter::toString(vertexData->vertexCount));

		VertexDeclaration* decl = vertexData->vertexDeclaration;
		VertexBufferBinding* bind = vertexData->vertexBufferBinding;

		VertexBufferBinding::VertexBufferBindingMap::const_iterator b, bend;
		bend = bind->getBindings().end();
		// Iterate over buffers
		for(b = bind->getBindings().begin(); b != bend; ++b)
		{
			vbNode = mParentNode->InsertEndChild(TiXmlElement("vertexbuffer"))->ToElement();
			const HardwareVertexBufferSharedPtr vbuf = b->second;
			unsigned short bufferIdx = b->first;
			// Get all the elements that relate to this buffer			
			VertexDeclaration::VertexElementList elems = decl->findElementsBySource(bufferIdx);
			VertexDeclaration::VertexElementList::iterator i, iend;
			iend = elems.end();

			// Set up the data access for this buffer (lock read-only)
			unsigned char* pVert;
			Real* pReal;
			RGBA* pColour;

			pVert = static_cast<unsigned char*>(
				vbuf->lock(0, vbuf->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY));

            // Skim over the elements to set up the general data
            unsigned short numTextureCoords = 0;
			for (i = elems.begin(); i != iend; ++i)
			{
				VertexElement& elem = *i;
				switch(elem.getSemantic())
				{
				case VES_POSITION:
					vbNode->SetAttribute("positions","true");
                    break;
				case VES_NORMAL:
					vbNode->SetAttribute("normals","true");
                    break;
				case VES_DIFFUSE:
					vbNode->SetAttribute("colours_diffuse","true");
                    break;
				case VES_SPECULAR:
					vbNode->SetAttribute("colours_specular","true");
                    break;
                case VES_TEXTURE_COORDINATES:
                    vbNode->SetAttribute(
                        "texture_coord_dimensions_" + StringConverter::toString(numTextureCoords), 
                        StringConverter::toString(VertexElement::getTypeCount(elem.getType())));
                    ++numTextureCoords;
                    break;
                }
            }
            if (numTextureCoords > 0)
            {
                vbNode->SetAttribute("texture_coords", 
                    StringConverter::toString(numTextureCoords));
            }

			// For each vertex
			for (size_t v = 0; v < vertexData->vertexCount; ++v)
			{
				// Iterate over the elements
				for (i = elems.begin(); i != iend; ++i)
				{
					VertexElement& elem = *i;
					vertexNode = 
						vbNode->InsertEndChild(TiXmlElement("vertex"))->ToElement();
					switch(elem.getSemantic())
					{
					case VES_POSITION:
						pReal = static_cast<Real*>(static_cast<void*>(pVert + elem.getOffset()));
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("position"))->ToElement();
						dataNode->SetAttribute("x", StringConverter::toString(pReal[0]));
						dataNode->SetAttribute("y", StringConverter::toString(pReal[1]));
						dataNode->SetAttribute("z", StringConverter::toString(pReal[2]));
						break;
					case VES_NORMAL:
						pReal = static_cast<Real*>(static_cast<void*>(pVert + elem.getOffset()));
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("position"))->ToElement();
						dataNode->SetAttribute("x", StringConverter::toString(pReal[0]));
						dataNode->SetAttribute("y", StringConverter::toString(pReal[1]));
						dataNode->SetAttribute("z", StringConverter::toString(pReal[2]));
						break;
					case VES_DIFFUSE:
						pColour = static_cast<RGBA*>(static_cast<void*>(pVert + elem.getOffset()));
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("colour_diffuse"))->ToElement();
						
						dataNode->SetAttribute("value", StringConverter::toString(*pColour++));
						break;
					case VES_SPECULAR:
						pColour = static_cast<RGBA*>(static_cast<void*>(pVert + elem.getOffset()));
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("colour_specular"))->ToElement();
						
						dataNode->SetAttribute("value", StringConverter::toString(*pColour++));
						break;
					case VES_TEXTURE_COORDINATES:
						pReal = static_cast<Real*>(static_cast<void*>(pVert + elem.getOffset()));
						dataNode = 
							vertexNode->InsertEndChild(TiXmlElement("texcoord"))->ToElement();

						switch(elem.getType())
                        {
                        case VET_FLOAT1:
    						dataNode->SetAttribute("u", StringConverter::toString(*pReal++));
                            break;
                        case VET_FLOAT2:
    						dataNode->SetAttribute("u", StringConverter::toString(*pReal++));
    						dataNode->SetAttribute("v", StringConverter::toString(*pReal++));
                            break;
                        case VET_FLOAT3:
    						dataNode->SetAttribute("u", StringConverter::toString(*pReal++));
    						dataNode->SetAttribute("v", StringConverter::toString(*pReal++));
    						dataNode->SetAttribute("w", StringConverter::toString(*pReal++));
                            break;
                        }
						break;


					}
				}
				pVert += vbuf->getVertexSize();
			}
			vbuf->unlock();
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

        assignNode->SetAttribute("vertexindex", 
            StringConverter::toString(assign->vertexIndex));
        assignNode->SetAttribute("boneindex", 
            StringConverter::toString(assign->boneIndex));
        assignNode->SetAttribute("weight",
            StringConverter::toString(assign->weight));


    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readSubMeshes(TiXmlElement* mSubmeshesNode)
    {
        LogManager::getSingleton().logMessage("Reading submeshes...");

        for (TiXmlElement* smElem = mSubmeshesNode->FirstChildElement();
            smElem != 0; smElem = smElem->NextSiblingElement())
        {
            // All children should be submeshes 
            SubMesh* sm = mpMesh->createSubMesh();

            const char* mat = smElem->Attribute("material");
            if (mat)
                sm->setMaterialName(mat);
            sm->useSharedVertices = StringConverter::parseBool(smElem->Attribute("useSharedVertices"));
            bool use32BitIndexes = StringConverter::parseBool(smElem->Attribute("use32bitindexes"));
            
            // Faces
            TiXmlElement* faces = smElem->FirstChildElement("faces");
            sm->indexData->indexCount = StringConverter::parseInt(faces->Attribute("count")) * 3;

            // Allocate space
            HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
                createIndexBuffer(
                    use32BitIndexes? HardwareIndexBuffer::IT_32BIT : HardwareIndexBuffer::IT_16BIT, 
                    sm->indexData->indexCount, 
                    HardwareBuffer::HBU_DYNAMIC,
                    false);
            unsigned int *pInt;
            unsigned short *pShort;
            if (use32BitIndexes)
            {
                pInt = static_cast<unsigned int*>(
                    ibuf->lock(0, ibuf->getSizeInBytes(), HardwareBuffer::HBL_DISCARD));
            }
            else
            {
                pShort = static_cast<unsigned short*>(
                    ibuf->lock(0, ibuf->getSizeInBytes(), HardwareBuffer::HBL_DISCARD));
            }
            TiXmlElement* faceElem;
            for (faceElem = faces->FirstChildElement();
                faceElem != 0; faceElem = faceElem->NextSiblingElement())
            {
                if (use32BitIndexes)
                {
                    *pInt++ = StringConverter::parseInt(faceElem->Attribute("v1"));
                    *pInt++ = StringConverter::parseInt(faceElem->Attribute("v2"));
                    *pInt++ = StringConverter::parseInt(faceElem->Attribute("v3"));
                }
                else
                {
                    *pShort++ = StringConverter::parseInt(faceElem->Attribute("v1"));
                    *pShort++ = StringConverter::parseInt(faceElem->Attribute("v2"));
                    *pShort++ = StringConverter::parseInt(faceElem->Attribute("v3"));
                }
            }
            ibuf->unlock();

            // Geometry
            if (!sm->useSharedVertices)
            {
                TiXmlElement* geomNode = smElem->FirstChildElement("geometry");
                if (geomNode)
                    readGeometry(geomNode, sm->vertexData);
            }

            // Bone assignments
            TiXmlElement* boneAssigns = smElem->FirstChildElement("boneassignments");
            if(boneAssigns)
                readBoneAssignments(boneAssigns, sm);

        }
        LogManager::getSingleton().logMessage("Submeshes done.");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readGeometry(TiXmlElement* mGeometryNode, VertexData* vertexData)
    {
        /* TODO
        LogManager::getSingleton().logMessage("Reading geometry...");
        Real *pPos, *pNorm, *pTex[9];
        RGBA* pCol;

        vertexData->vertexCount = StringConverter::parseInt(mGeometryNode->Attribute("count"));
        // Skip empty 
        if (vertexData->vertexCount <= 0) return;
        

        // Iterate over all children (vertexbuffer entries)
        for (TiXmlElement* vbElem = mGeometryNode->FirstChildElement();
            vbElem != 0; vbElem = vbElem->NextSiblingElement())
        {
            // Skip non-vertexbuffer elems
            if (stricmp(vbElem->Value(), "vertexbuffer")) continue;

            
            const char* attrib = vbElem->Attribute("positions");
            if (attrib && StringConverter::parseBool(attrib))
            {
                vertexData->pVertices = new Real[vertexData->numVertices * 3];
                // TODO change when we do shared bufs
                vertexData->vertexStride = 0;
                pPos = vertexData->pVertices;

            }
            attrib = vbElem->Attribute("normals");
            if (attrib && StringConverter::parseBool(attrib))
            {
                vertexData->hasNormals = true;
                vertexData->pNormals = new Real[vertexData->numVertices * 3];
                // TODO change when we do shared bufs
                vertexData->normalStride = 0;
                
                pNorm = vertexData->pNormals;
            }
            attrib = vbElem->Attribute("colours");
            if (attrib && StringConverter::parseBool(attrib))
            {
                vertexData->hasColours = true;
                vertexData->pColours = new RGBA[vertexData->numVertices];
                // TODO change when we do shared bufs
                vertexData->colourStride = 0;

                pCol = vertexData->pColours;

            }
            attrib = vbElem->Attribute("numtexcoords");
            if (attrib && StringConverter::parseInt(attrib))
            {
                vertexData->numTexCoords = StringConverter::parseInt(vbElem->Attribute("numtexcoords"));
                String sets = vbElem->Attribute("texcoordsets");
                String dims = vbElem->Attribute("texcoorddimensions");

                std::vector<String> vecSets = sets.split(" ");
                std::vector<String> vecDims = dims.split(" ");

                for (size_t v = 0; v < vecSets.size(); ++v)
                {
                    int set = StringConverter::parseInt(vecSets[v]);
                    int dim = StringConverter::parseInt(vecDims[v]);
                    vertexData->numTexCoordDimensions[set] = dim;
                    vertexData->pTexCoords[set] = new Real[vertexData->numVertices * dim];
                    // TODO change when we do shared bufs
                    vertexData->texCoordStride[set] = 0;

                    pTex[set] = vertexData->pTexCoords[set];

                }
            } 

            // Now the buffers are set up, parse all the vertices
            for (TiXmlElement* vertexElem = vbElem->FirstChildElement();
            vertexElem != 0; vertexElem = vertexElem->NextSiblingElement())
            {
                ushort currentSet = 0;
                // Each vertex can have 1 or more components
                for (TiXmlElement* cmpElem = vertexElem->FirstChildElement();
                cmpElem != 0; cmpElem = cmpElem->NextSiblingElement())
                {
                    if (!stricmp(cmpElem->Value(), "position"))
                    {
                        *pPos++ = StringConverter::parseReal(
                            cmpElem->Attribute("x"));
                        *pPos++ = StringConverter::parseReal(
                            cmpElem->Attribute("y"));
                        *pPos++ = StringConverter::parseReal(
                            cmpElem->Attribute("z"));

                        pPos += vertexData->vertexStride;
                    }
                    else if (!stricmp(cmpElem->Value(), "normal"))
                    {
                        *pNorm++ = StringConverter::parseReal(
                            cmpElem->Attribute("x"));
                        *pNorm++ = StringConverter::parseReal(
                            cmpElem->Attribute("y"));
                        *pNorm++ = StringConverter::parseReal(
                            cmpElem->Attribute("z"));

                        pNorm += vertexData->normalStride;
                    }
                    else if (!stricmp(cmpElem->Value(), "texcoord"))
                    {
                        *pTex[currentSet]++ = StringConverter::parseReal(
                            cmpElem->Attribute("u"));
                        if (vertexData->numTexCoordDimensions[currentSet] > 1)
                        {
                            *pTex[currentSet]++ = StringConverter::parseReal(
                                cmpElem->Attribute("v"));
                        }
                        if (vertexData->numTexCoordDimensions[currentSet] > 2)
                        {
                            *pTex[currentSet]++ = StringConverter::parseReal(
                                cmpElem->Attribute("w"));
                        }

                        pTex[currentSet] += vertexData->texCoordStride[currentSet];
                        currentSet++;
                    }
                    else if (!stricmp(cmpElem->Value(), "colour"))
                    {
                        *pCol++ = StringConverter::parseLong(
                            cmpElem->Attribute("value"));
                    }
                }// position / normal / texcoord / colour 
            } // vertex
        } // vertexbuffer
        */

        LogManager::getSingleton().logMessage("Geometry done...");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readSkeletonLink(TiXmlElement* mSkelNode)
    {
        mpMesh->setSkeletonName(mSkelNode->Attribute("name"));
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readBoneAssignments(TiXmlElement* mBoneAssignmentsNode)
    {
        LogManager::getSingleton().logMessage("Reading bone assignments...");

        // Iterate over all children (vertexboneassignment entries)
        for (TiXmlElement* elem = mBoneAssignmentsNode->FirstChildElement();
        elem != 0; elem = elem->NextSiblingElement())
        {
            VertexBoneAssignment vba;
            vba.vertexIndex = StringConverter::parseInt(
                elem->Attribute("vertexIndex"));
            vba.boneIndex = StringConverter::parseInt(
                elem->Attribute("boneIndex"));
            vba.weight= StringConverter::parseReal(
                elem->Attribute("weight"));

            mpMesh->addBoneAssignment(vba);
        }

        LogManager::getSingleton().logMessage("Bone assignments done.");
    }
    //---------------------------------------------------------------------
    void XMLMeshSerializer::readBoneAssignments(TiXmlElement* mBoneAssignmentsNode, SubMesh* sm)
    {
        LogManager::getSingleton().logMessage("Reading bone assignments...");
        // Iterate over all children (vertexboneassignment entries)
        for (TiXmlElement* elem = mBoneAssignmentsNode->FirstChildElement();
        elem != 0; elem = elem->NextSiblingElement())
        {
            VertexBoneAssignment vba;
            vba.vertexIndex = StringConverter::parseInt(
                elem->Attribute("vertexIndex"));
            vba.boneIndex = StringConverter::parseInt(
                elem->Attribute("boneIndex"));
            vba.weight= StringConverter::parseReal(
                elem->Attribute("weight"));

            sm->addBoneAssignment(vba);
        }
        LogManager::getSingleton().logMessage("Bone assignments done.");
    }
    //---------------------------------------------------------------------
	void XMLMeshSerializer::writeLodInfo(TiXmlElement* mMeshNode, const Mesh* pMesh)
	{
        TiXmlElement* lodNode = 
            mMeshNode->InsertEndChild(TiXmlElement("levelofdetail"))->ToElement();

		unsigned short numLvls = pMesh->getNumLodLevels();
		bool manual = pMesh->isLodManual();
		lodNode->SetAttribute("numLevels", StringConverter::toString(numLvls));
		lodNode->SetAttribute("manual", StringConverter::toString(manual));

		// Iterate from level 1, not 0 (full detail)
		for (unsigned short i = 1; i < numLvls; ++i)
		{
			const Mesh::MeshLodUsage& usage = pMesh->getLodLevel(i);
			if (manual)
			{
				writeLodUsageManual(lodNode, i, usage);
			}
			else
			{
				writeLodUsageGenerated(lodNode, i, usage, pMesh);
			}
		}

	}
    //---------------------------------------------------------------------
	void XMLMeshSerializer::writeLodUsageManual(TiXmlElement* usageNode, 
		unsigned short levelNum, const Mesh::MeshLodUsage& usage)
	{
		TiXmlElement* manualNode = 
			usageNode->InsertEndChild(TiXmlElement("lodmanual"))->ToElement();

		manualNode->SetAttribute("fromDepthSquared", 
			StringConverter::toString(usage.fromDepthSquared));
		manualNode->SetAttribute("meshName", usage.manualName);

	}
    //---------------------------------------------------------------------
	void XMLMeshSerializer::writeLodUsageGenerated(TiXmlElement* usageNode, 
		unsigned short levelNum,  const Mesh::MeshLodUsage& usage, 
		const Mesh* pMesh)
	{
		TiXmlElement* generatedNode = 
			usageNode->InsertEndChild(TiXmlElement("lodgenerated"))->ToElement();
		generatedNode->SetAttribute("fromDepthSquared", 
			StringConverter::toString(usage.fromDepthSquared));

		// Iterate over submeshes at this level
		unsigned short numsubs = pMesh->getNumSubMeshes();

		for (unsigned short subi = 0; subi < numsubs; ++subi)
		{
			TiXmlElement* subNode = 
				generatedNode->InsertEndChild(TiXmlElement("lodfacelist"))->ToElement();
			SubMesh* sub = pMesh->getSubMesh(subi);
			subNode->SetAttribute("submeshindex", StringConverter::toString(subi));
			// NB level - 1 because SubMeshes don't store the first index in geometry
		    IndexData* facedata = sub->mLodFaceList[levelNum - 1];
			subNode->SetAttribute("numfaces", StringConverter::toString(facedata->indexCount / 3));

			// Write each face in turn
		    bool use32BitIndexes = (facedata->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);

            // Write each face in turn
		    unsigned int* pInt;
		    unsigned short* pShort;
		    HardwareIndexBufferSharedPtr ibuf = facedata->indexBuffer;
		    if (use32BitIndexes)
		    {
			    pInt = static_cast<unsigned int*>(
				    ibuf->lock(0, ibuf->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY)); 
		    }
		    else
		    {
			    pShort = static_cast<unsigned short*>(
				    ibuf->lock(0, ibuf->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY)); 
		    }
			
			for (size_t f = 0; f < facedata->indexCount; f += 3)
			{
				TiXmlElement* faceNode = 
					subNode->InsertEndChild(TiXmlElement("face"))->ToElement();
                if (use32BitIndexes)
                {
				    faceNode->SetAttribute("v1", StringConverter::toString(*pInt++));
				    faceNode->SetAttribute("v2", StringConverter::toString(*pInt++));
				    faceNode->SetAttribute("v3", StringConverter::toString(*pInt++));
                }
                else
                {
				    faceNode->SetAttribute("v1", StringConverter::toString(*pShort++));
				    faceNode->SetAttribute("v2", StringConverter::toString(*pShort++));
				    faceNode->SetAttribute("v3", StringConverter::toString(*pShort++));
                }

			}



		}

	}
    //---------------------------------------------------------------------
	void XMLMeshSerializer::readLodInfo(TiXmlElement*  lodNode)
	{
		
        LogManager::getSingleton().logMessage("Parsing LOD information...");

		const char* val = lodNode->Attribute("numLevels");
		unsigned short numLevels = static_cast<unsigned short>(
			StringConverter::parseUnsignedInt(val));

		val = lodNode->Attribute("manual");
		bool manual = StringConverter::parseBool(val);

		// Set up the basic structures
		mpMesh->_setLodInfo(numLevels, manual);

		// Parse the detail, start from 1 (the first sub-level of detail)
		unsigned short i = 1;
		TiXmlElement* usageElem;
		if (manual)
		{
			usageElem = lodNode->FirstChildElement("lodmanual");
		}
		else
		{
			usageElem = lodNode->FirstChildElement("lodgenerated");
		}
		while (usageElem)
		{
			if (manual)
			{
				readLodUsageManual(usageElem, i);
				usageElem = usageElem->NextSiblingElement();
			}
			else
			{
				readLodUsageGenerated(usageElem, i);
				usageElem = usageElem->NextSiblingElement();
			}
			++i;
		}
		
        LogManager::getSingleton().logMessage("LOD information done.");
		
	}
    //---------------------------------------------------------------------
	void XMLMeshSerializer::readLodUsageManual(TiXmlElement* manualNode, unsigned short index)
	{
		Mesh::MeshLodUsage usage;
		const char* val = manualNode->Attribute("fromDepthSquared");
		usage.fromDepthSquared = StringConverter::parseReal(val);
		usage.manualName = manualNode->Attribute("meshName");

		mpMesh->_setLodUsage(index, usage);
	}
    //---------------------------------------------------------------------
	void XMLMeshSerializer::readLodUsageGenerated(TiXmlElement* genNode, unsigned short index)
	{
        /*  TODO
		Mesh::MeshLodUsage usage;
		const char* val = genNode->Attribute("fromDepthSquared");
		usage.fromDepthSquared = StringConverter::parseReal(val);
		usage.manualMesh = NULL;
		usage.manualName = "";

		mpMesh->_setLodUsage(index, usage);

		// Read submesh face lists
		TiXmlElement* faceListElem = genNode->FirstChildElement("lodfacelist");
		while (faceListElem)
		{
			val = faceListElem->Attribute("submeshindex");
			unsigned short subidx = StringConverter::parseUnsignedInt(val);
			val = faceListElem->Attribute("numFaces");
			unsigned short numFaces = StringConverter::parseUnsignedInt(val);

			// Assign memory: this will be deleted by the submesh 
			unsigned short* pIndexes = new unsigned short[numFaces * 3];
			unsigned short* pCurr = pIndexes;

			TiXmlElement* faceElem = faceListElem->FirstChildElement("face");
			for (unsigned int face = 0; face < numFaces; ++face, faceElem->NextSiblingElement())
			{
				val = faceElem->Attribute("v1");
				*pCurr++ = StringConverter::parseUnsignedInt(val);
				val = faceElem->Attribute("v2");
				*pCurr++ = StringConverter::parseUnsignedInt(val);
				val = faceElem->Attribute("v3");
				*pCurr++ = StringConverter::parseUnsignedInt(val);

			}

			ProgressiveMesh::LODFaceData facedata;
			facedata.numIndexes = numFaces * 3;
			facedata.pIndexes = pIndexes;
			mpMesh->_setSubMeshLodFaceList(subidx, index, facedata);

			faceListElem = faceListElem->NextSiblingElement();
		}
        */
	}

}

