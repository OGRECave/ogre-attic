/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn

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

#include "LexiStdAfx.h"
#include "LexiOgreMeshCompiler.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "LexiExportObject.h"
#include "../XMLConverter/include/OgreXMLMeshSerializer.h"

COgreMeshCompiler::COgreMeshCompiler( CIntermediateMesh* pIntermediateMesh, const CDDObject* pConfig, CExportProgressDlg* pProgressDlg) :
	m_iBind(0),
	m_MaxSquaredLength(0.0),
	m_pOgreMesh(NULL),
	m_bReindex(false),
	m_bExportNormals(false),
	m_bExportColours(false),
	m_bExportTexUVs(false),
	m_bExportSkeleton(false),
	m_bOptimizeBuffers(false),
	m_bPoseAnimation(false),
	m_iNrVerts(0),
	m_iNrPoses(0),
	m_bUseSharedGeometry(true),
	m_bExportEdges(false),
	m_bExportTangents(false),
	m_pProgressDlg(pProgressDlg)
{
	REGISTER_MODULE("Ogre Mesh Compiler")
	m_IndexBitType = Ogre::HardwareIndexBuffer::IT_16BIT;

	m_bExportSkeleton = pIntermediateMesh->GetSkeleton() != NULL ? true : false;

	ReadConfig(pConfig);

	m_pProgressDlg->InitLocal(14);

	if(m_bReindex)
	{
		m_pProgressDlg->LocalStep("Reindexing mesh..");
		ReindexIntermediateBuffers(pIntermediateMesh);
	}

	/////////////////////////////////////
	//MessageBox(NULL, "PreOgreMeshCreate","BREAK!",0);
	/////////////////////////////////////

	m_pProgressDlg->LocalStep("Creating Ogre Mesh Object..");
	CreateOgreMesh(pIntermediateMesh);

	/////////////////////////////////////
	//MessageBox(NULL, "PreCreateBuffer","BREAK!",0);
	/////////////////////////////////////

	CreateBuffers(pIntermediateMesh);

	Ogre::LogManager::getSingletonPtr()->logMessage("OgreMeshCompiler: Creating Bounds");

	int finalCount = 2;
	if(m_bExportEdges) finalCount++;
	if(m_bExportTangents) finalCount++;

	m_pProgressDlg->InitLocal(finalCount);

	// create and register bounding box
	m_pProgressDlg->LocalStep("Calculating Mesh Bounds...");
	CreateMeshBounds();

	// Optimize vertex declarations
	m_pProgressDlg->LocalStep("Optimizing Vertex Declerations...");
	OptimizeVertexDeclaration();

	if(m_bExportEdges)
	{
		m_pProgressDlg->LocalStep("Building Edge List...");
		m_pOgreMesh->buildEdgeList();
	}

	if(m_bExportTangents && m_bExportNormals && m_bExportTexUVs)
	{
		m_pProgressDlg->LocalStep("Building Tangent Vectors...");
		unsigned short SourceCoordSet = 0;
		unsigned short DestCoordSet = 0;
		m_pOgreMesh->suggestTangentVectorBuildParams( Ogre::VES_TANGENT, SourceCoordSet,DestCoordSet);
		m_pOgreMesh->buildTangentVectors( Ogre::VES_TANGENT, SourceCoordSet,DestCoordSet);
		// 
		LOGINFO "Tangents for mesh: (%s) written to coord buffer %i", m_pOgreMesh->getName().c_str(), DestCoordSet);
	}
	// ready for serialization!
}

COgreMeshCompiler::~COgreMeshCompiler()
{
	LOGDEBUG "OgreMeshCompiler cleaned..");

	if(m_bUseSharedGeometry)
	{
		delete m_pOgreMesh->sharedVertexData;
	}
	else
	{
		Ogre::Mesh::SubMeshIterator it = m_pOgreMesh->getSubMeshIterator();
		while(it.hasMoreElements())
		{
			Ogre::SubMesh* curSubMesh = it.getNext();
			delete curSubMesh->vertexData;
		}
	}

	Ogre::MeshManager* pMeshMgr = Ogre::MeshManager::getSingletonPtr();
	pMeshMgr->unloadAll();

	UNREGISTER_MODULE
}


void COgreMeshCompiler::ReadConfig( const CDDObject* pConfig )
{
	assert(pConfig);

	m_bUseSharedGeometry = pConfig->GetBool("useSharedGeometryID", false);
	m_bReindex = pConfig->GetBool("reindexID");
	m_bExportNormals = pConfig->GetBool("normalsID");
	m_bExportColours = pConfig->GetBool("vertexColorsID");
	m_bExportTexUVs = pConfig->GetBool("uvID");
	m_bOptimizeBuffers = pConfig->GetBool("optimizeBuffers",true);
	m_bExportEdges = pConfig->GetBool("exportEdgesID",false);
	m_bExportTangents = pConfig->GetBool("exportTangentsID",false);
}


void COgreMeshCompiler::ReindexIntermediateBuffers( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);

	SharedUtilities::fastvector< CMeshArray* > bufferList;
	bufferList.push_back(pIntermediateMesh->GetArray("position",0));

	CMeshArray* pArray; 

	
	if(m_bExportNormals)
	{
		pArray = pIntermediateMesh->GetArray("normal",0);
		if (pArray == NULL)
			LOGWARNING "No Normals found, ignoring \"export normals\" option.");
		else
			bufferList.push_back(pArray);
	}

	
	if(m_bExportColours)
	{
		pArray = pIntermediateMesh->GetArray("diffuse",0);
		if (pArray != NULL)
			bufferList.push_back(pArray);
	}

	
	if(m_bExportTexUVs)
	{
		// check all uv sets
		for(unsigned int x = 1; x <= 99; x++)
		{
			char temp[32];
			sprintf(temp, "uv%i", x);
			pArray = pIntermediateMesh->GetArray(temp,0);
			if (pArray == NULL)
				continue;
			else
				bufferList.push_back(pArray);
		}
	}

	// Prepare bone vertex assignments
	SharedUtilities::fastvector< CMeshArray* > boneBuffer;

	if(m_bExportSkeleton)
	{
		// extract declaration map into individual arrays for reindexing
	 	CIntermediateSkeleton* pISkel = pIntermediateMesh->GetSkeleton();
		pISkel->ExtractVertexAssignmentsArrays(boneBuffer);

		for(int i=0; i < boneBuffer.size(); i++)
		{
			bufferList.push_back( boneBuffer[i] );
		}
	}

	pIntermediateMesh->Reindex( bufferList );

	if(m_bExportSkeleton)
	{
		CIntermediateSkeleton* pISkel = pIntermediateMesh->GetSkeleton();
		pISkel->ApplyVertexAssignmentsArrays(boneBuffer);
		//Clean up
		for(int i=0; i < boneBuffer.size(); i++)
		{
			delete boneBuffer[i];
		}
		boneBuffer.clear();
	}

}


void COgreMeshCompiler::CreateOgreMesh( CIntermediateMesh* pIntermediateMesh )
{
	Ogre::String sMeshName = pIntermediateMesh->getName();
	Ogre::MeshManager* pMeshMgr = Ogre::MeshManager::getSingletonPtr();
	Ogre::MeshPtr pOgreMesh = pMeshMgr->getByName(sMeshName);

	if(!pOgreMesh.isNull()) {
		pMeshMgr->unload(pOgreMesh->getHandle());
		pMeshMgr->remove(pOgreMesh->getHandle());
		pOgreMesh.setNull();
	}
	pOgreMesh = pMeshMgr->createManual(sMeshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	m_pOgreMesh = pOgreMesh;

	if(m_bUseSharedGeometry)
		m_pOgreMesh->sharedVertexData = new Ogre::VertexData();

	CreateSubMeshes( pIntermediateMesh );
}

void COgreMeshCompiler::CreateSubMeshes( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);
	assert(!m_pOgreMesh.isNull());

	unsigned int iCount = pIntermediateMesh->GetNumMaterials();
	for (int i = 0; i < iCount; i++)
	{
		CIntermediateMaterial* mat = pIntermediateMesh->GetMaterial(i);
		Ogre::SubMesh* pSubMesh = m_pOgreMesh->createSubMesh(mat->GetName());

		if(!m_bUseSharedGeometry)
			pSubMesh->vertexData = new Ogre::VertexData();

		
		m_lMaterialSubMeshMap.insert( std::pair<CIntermediateMaterial*,Ogre::SubMesh*>(mat,pSubMesh) );
	}
}

void COgreMeshCompiler::CreateBuffers( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);
	assert(!m_pOgreMesh.isNull());

	/////////////////////////////////////
	//MessageBox(NULL, "PreVertexBuffer","BREAK!",0);
	/////////////////////////////////////
	CreateVertexBuffer(pIntermediateMesh);

	/////////////////////////////////////
	//MessageBox(NULL, "PreIndexBuffer","BREAK!",0);
	/////////////////////////////////////
	CreateIndexBuffer(pIntermediateMesh);

	if(m_bExportNormals)
	{
		/////////////////////////////////////
		//MessageBox(NULL, "PreNormalBuffer","BREAK!",0);
		/////////////////////////////////////
		CreateNormalBuffer(pIntermediateMesh);
	}

	if(m_bExportColours)
	{
		/////////////////////////////////////
		//MessageBox(NULL, "PreDiffuseBuffer","BREAK!",0);
		/////////////////////////////////////
		CreateDiffuseBuffer(pIntermediateMesh);
	}	
	if (m_bExportTexUVs)
	{
		/////////////////////////////////////
		//MessageBox(NULL, "PreTextureCoordBuffer","BREAK!",0);
		/////////////////////////////////////
		CreateTexCoordBuffer(pIntermediateMesh);
	}


	m_bPoseAnimation = pIntermediateMesh->HasPoseData();
	if( m_bPoseAnimation )
	{
		CreatePoseBuffers(pIntermediateMesh);
	}

	//"specular"
	//"boneindex"
	//"blendweight"
	//"binormal"
	//"tangent"
}

void COgreMeshCompiler::CreateVertexBuffer( CIntermediateMesh* pIntermediateMesh )
{
	START_PROFILE("COgreMeshCompiler::CreateVertexBuffer()");

	assert(pIntermediateMesh);
	CMeshArray* pMeshArray = pIntermediateMesh->GetArray("position",0);
	if(pMeshArray == NULL)
		return;

	

	m_iNrVerts = pMeshArray->Size();

	Ogre::VertexData* vertexData;

	if(m_bUseSharedGeometry)
	{
		// SHARED GEOMETRY BUFFER
		vertexData = m_pOgreMesh->sharedVertexData;

		Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
		m_pVertexDecl = vertexDecl;

		// allocate the position vertex buffer
		vertexDecl->addElement(m_iBind, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		vertexData->vertexCount = m_iNrVerts;

		Ogre::HardwareVertexBufferSharedPtr vBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(m_iBind), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(m_iBind, vBuf);
		const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->getElement(m_iBind);
		unsigned char* vertex = static_cast<unsigned char*>(vBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
		m_iBind++;

		// Ok, Let큦 fill in the buffer with data  :D
		const Ogre::Vector3* data = (const Ogre::Vector3*)pMeshArray->Data();
		Ogre::Real* pReal;
		posElem->baseVertexPointerToElement(vertex, &pReal);

		m_pProgressDlg->LocalStep("Writing Shared Vertex Buffer..");
		
		unsigned int j = 0;
		for(unsigned int i=0; i< m_iNrVerts; i++) {

			Ogre::Vector3 vert = data[i];

			*pReal++ = vert.x;
			*pReal++ = vert.y;
			*pReal++ = vert.z;

			// Bounding Sphere radius
			m_MaxSquaredLength = std::max(m_MaxSquaredLength, vert.squaredLength());

			// update AABB
			m_AABB.merge( vert );

		}
		vBuf->unlock();
	}
	else
	{
		// DEDICATED GEOMETRY BUFFERS
		// create vertex buffer according to submesh indicies.
		const CTriangleArray& triangleArray = pIntermediateMesh->GetTriangles();
		std::map<CIntermediateMaterial*, Ogre::SubMesh*>::iterator it = m_lMaterialSubMeshMap.begin();
		int i=0;
		while( it != m_lMaterialSubMeshMap.end() )
		{
			CIntermediateMaterial* pMat = it->first;
			Ogre::SubMesh* pSubMesh = it->second;
			vertexData = pSubMesh->vertexData;
			Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
			
			std::vector< unsigned int > lMatTriangles;
			pIntermediateMesh->GetTrianglesUsingMaterial( pMat, lMatTriangles);

			// Extract the subMesh data and save the index transform map for later use.
			// Index map record
			std::map< unsigned int, unsigned int> lIndexReMap;
			CMeshArray* pSubMeshData = ExtractSubMeshData_Vec3(pMeshArray, pIntermediateMesh, pMat,lIndexReMap);
			unsigned int iVertCount = pSubMeshData->Size();

			m_iBind = vertexDecl->getElementCount();
			// allocate the position vertex buffer
			vertexDecl->addElement(m_iBind, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
			vertexData->vertexCount = iVertCount;

			Ogre::HardwareVertexBufferSharedPtr vBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(m_iBind), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
			Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;
			binding->setBinding(m_iBind, vBuf); 
			const Ogre::VertexElement* posElem = vertexDecl->findElementBySemantic(Ogre::VES_POSITION);
			unsigned char* vertex = static_cast<unsigned char*>(vBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
			m_iBind++;

			// Ok, Let큦 fill in the buffer with data  :D
			const Ogre::Vector3* data = (const Ogre::Vector3*)pSubMeshData->Data();
			Ogre::Real* pReal;
			posElem->baseVertexPointerToElement(vertex, &pReal);

			m_pProgressDlg->LocalStep("Writing Dedicated Vertex Buffers..");

			unsigned int j = 0;
			for(unsigned int i=0; i< iVertCount; i++) 
			{
				Ogre::Vector3 vert = data[i];

				*pReal++ = vert.x;
				*pReal++ = vert.y;
				*pReal++ = vert.z;

				// Bounding Sphere radius
				m_MaxSquaredLength = std::max(m_MaxSquaredLength, vert.squaredLength());

				// update AABB
				m_AABB.merge( vert );
			}
			vBuf->unlock();

			it++;
			m_lSubMeshIndexReMap.insert( std::pair<Ogre::SubMesh*, std::map< unsigned int, unsigned int>>(pSubMesh, lIndexReMap ) );
			delete pSubMeshData;
		}
	}
 
 	OgreAssert(_CrtCheckMemory(), "Memory corruption");
 
	END_PROFILE("COgreMeshCompiler::CreateVertexBuffer()");
}



void COgreMeshCompiler::CreateIndexBuffer( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);

	START_PROFILE("COgreMeshCompiler::CreateIndexBuffer()");

	const CTriangleArray& triangleArray = pIntermediateMesh->GetTriangles();
	int iNrTriangles = triangleArray.Size();

	// allocate index buffer for each submesh
	std::map<CIntermediateMaterial*, Ogre::SubMesh*>::iterator it = m_lMaterialSubMeshMap.begin();
	while( it != m_lMaterialSubMeshMap.end() )
	{
		Ogre::SubMesh* pSubMesh = it->second;
		CIntermediateMaterial* pMat = it->first;

		std::vector< unsigned int > lMatTriangles;
		pIntermediateMesh->GetTrianglesUsingMaterial( pMat, lMatTriangles);

		//submesh index lookup
		std::map< unsigned int, unsigned int> localMap;
		std::map< unsigned int, unsigned int> *tempRemap = &localMap;
		if(!m_bUseSharedGeometry)
		{
			tempRemap = &(m_lSubMeshIndexReMap.find(pSubMesh)->second);
		}
		std::map< unsigned int, unsigned int> &lIndexReMap = *tempRemap;

		int iNrIndices = lMatTriangles.size() *3;
		pSubMesh->indexData->indexCount = iNrIndices;

		// We should use a 32bit buffer if the vertex buffer has more than 65536 entries (otherwise we cannot reference them)
		// From the vertex buffer decleration we know the vertex count:

		m_IndexBitType = Ogre::HardwareIndexBuffer::IT_16BIT;
		if(m_iNrVerts >= 65536)
			m_IndexBitType = Ogre::HardwareIndexBuffer::IT_32BIT;

		pSubMesh->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(m_IndexBitType, pSubMesh->indexData->indexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		Ogre::HardwareIndexBufferSharedPtr iBuf = pSubMesh->indexData->indexBuffer;
		pSubMesh->useSharedVertices = m_bUseSharedGeometry;
		it++;

		// Ok, Let큦 fill in the buffer with data  :D

		// We lock the buffer before adding content
		if(m_IndexBitType == Ogre::HardwareIndexBuffer::IT_32BIT)
		{	// 32 Bit Index Buffer
			LOGINFO "Locking 32bit IndexBuffer");
			unsigned int* pIndices = static_cast<unsigned int*>(iBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

			m_pProgressDlg->InitLocal(lMatTriangles.size()/100);
			Ogre::String msg = "Writing 32bit Index Buffers";
			if(m_bExportSkeleton) msg+=" and Bone Weights..";
			msg+="..";
			m_pProgressDlg->LocalStep(msg.c_str());

			unsigned int j = 0;
			for(unsigned int i = 0; i< lMatTriangles.size(); i++) 
			{
				if((i%100)==0) m_pProgressDlg->LocalStep();
				const CTriangle& face = triangleArray[ lMatTriangles[i] ];

				if(m_bUseSharedGeometry)
				{
					// indices
					pIndices[j++] = face.m_Vertices[0];
					pIndices[j++] = face.m_Vertices[1];
					pIndices[j++] = face.m_Vertices[2];
				}
				else
				{
					// Dedicated geometry
					pIndices[j++] = lIndexReMap.find(face.m_Vertices[0])->second;
					pIndices[j++] = lIndexReMap.find(face.m_Vertices[1])->second;
					pIndices[j++] = lIndexReMap.find(face.m_Vertices[2])->second;

				}

				// skeleton
				SetBoneAssignments(face,pIntermediateMesh, pSubMesh);
			}
		

			// Unlock
			LOGINFO "Unlocking IndexBuffer");
			iBuf->unlock();
		}
		else
		{	// 16 Bit Index Buffer
			LOGINFO "Locking 16bit IndexBuffer");
			unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

			m_pProgressDlg->InitLocal(lMatTriangles.size()/100);
			Ogre::String msg = "Writing 16bit Index Buffers";
			if(m_bExportSkeleton) msg+=" and Bone Weights";
			msg+="..";
			m_pProgressDlg->LocalStep(msg.c_str());

			unsigned int j = 0;
			for(unsigned int i = 0; i< lMatTriangles.size(); i++) 
			{
				if((i%100)==0) m_pProgressDlg->LocalStep();
				const CTriangle& face = triangleArray[ lMatTriangles[i] ];

				if(m_bUseSharedGeometry)
				{
					// indices
					pIndices[j++] = face.m_Vertices[0];
					pIndices[j++] = face.m_Vertices[1];
					pIndices[j++] = face.m_Vertices[2];
				}
				else
				{
					// Dedicated geometry
					pIndices[j++] = lIndexReMap.find(face.m_Vertices[0])->second;
					pIndices[j++] = lIndexReMap.find(face.m_Vertices[1])->second;
					pIndices[j++] = lIndexReMap.find(face.m_Vertices[2])->second;

				}

				// skeleton
				SetBoneAssignments(face,pIntermediateMesh, pSubMesh);
			}

			// Unlock
			LOGINFO "Unlocking IndexBuffer");
			iBuf->unlock();
		}

		if(pMat != NULL)
			pSubMesh->setMaterialName(pMat->GetName().c_str());
		else
			pSubMesh->setMaterialName("NoMaterial");

		if(iBuf->isLocked())
			iBuf->unlock();
	}

	OgreAssert(_CrtCheckMemory(), "Memory corruption");

	END_PROFILE("COgreMeshCompiler::CreateIndexBuffer()");
}

void COgreMeshCompiler::SetBoneAssignments( const CTriangle& face, CIntermediateMesh* pIntermediateMesh, Ogre::SubMesh* pSubMesh )
{
	if(m_bExportSkeleton) {
		CIntermediateSkeleton* pISkel = pIntermediateMesh->GetSkeleton();
		Ogre::VertexBoneAssignment vertexBoneAssignment;

		for (int w=0; w < 3; w++)
		{
			int vertIndex = face.m_Vertices[w];
			int iBoneCount = pISkel->GetNrOfAssignmentsOnVertex(vertIndex);

			SVertexBoneData maxBoneData;
			maxBoneData.weight = 0;
			int maxVertIndex = 0;
			for (int i=0; i < iBoneCount; i++)
			{
				SVertexBoneData boneData;
				if(pISkel->GetVertexData(vertIndex,i,boneData))
				{
					if(maxBoneData.weight < boneData.weight)
						maxBoneData = boneData;

					if(boneData.weight < 0.001f) continue;


					if(!m_bUseSharedGeometry)
					{
					  OgreAssert(m_lSubMeshIndexReMap.find(pSubMesh) != m_lSubMeshIndexReMap.end(), "submesh in submeshindexremap not found");
						std::map< unsigned int, unsigned int> &lIndexReMap = m_lSubMeshIndexReMap.find(pSubMesh)->second;
						vertexBoneAssignment.vertexIndex = lIndexReMap.find(vertIndex)->second;
					}
					else
					{
						vertexBoneAssignment.vertexIndex	= vertIndex;
					}

					vertexBoneAssignment.boneIndex		= boneData.boneIndex;
					vertexBoneAssignment.weight			= boneData.weight;
	
					if(m_bUseSharedGeometry)
					{
						m_pOgreMesh->addBoneAssignment(vertexBoneAssignment);
					}
					else
					{
						pSubMesh->addBoneAssignment(vertexBoneAssignment);
					}
				}
			}
		}
	}
}

void COgreMeshCompiler::CreateNormalBuffer( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);

	START_PROFILE("COgreMeshCompiler::CreateNormalBuffer()");

	CMeshArray* pNormalArray = pIntermediateMesh->GetArray("normal",0);
	if(pNormalArray == NULL)
		return;

	int iNrNormals = pNormalArray->Size();

	Ogre::VertexData* vertexData;

	if(m_bUseSharedGeometry)
	{
		// Shared Geomatry
		Ogre::VertexData* vertexData = vertexData = m_pOgreMesh->sharedVertexData;
		Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;

		// normals
		vertexDecl->addElement(m_iBind, 0, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);

		// allocate the normal vertex buffer
		vertexData->vertexCount = iNrNormals;
		Ogre::HardwareVertexBufferSharedPtr nBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(m_iBind), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(m_iBind, nBuf);
		const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->getElement(m_iBind);
		unsigned char* vertex = static_cast<unsigned char*>(nBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
		m_iBind++;

		Ogre::Real* pReal;
		posElem->baseVertexPointerToElement(vertex, &pReal);

		m_pProgressDlg->LocalStep("Writing Shared Normal Buffer..");

		const Ogre::Vector3* data = (const Ogre::Vector3*)pNormalArray->Data();
		int j = 0;
		for (int i = 0 ; i < iNrNormals; i++)
		{
			const Ogre::Vector3& vert = data[i];

			*pReal++ = vert.x;
			*pReal++ = vert.y;
			*pReal++ = vert.z;
		}
		nBuf->unlock();
	}
	else
	{
		// DEDICATED GEOMETRY BUFFERS
		const CTriangleArray& triangleArray = pIntermediateMesh->GetTriangles();
		std::map<CIntermediateMaterial*, Ogre::SubMesh*>::iterator it = m_lMaterialSubMeshMap.begin();
		while( it != m_lMaterialSubMeshMap.end() )
		{
			CIntermediateMaterial* pMat = it->first;
			Ogre::SubMesh* pSubMesh = it->second;
			vertexData = pSubMesh->vertexData;
			Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
			
			std::vector< unsigned int > lMatTriangles;
			pIntermediateMesh->GetTrianglesUsingMaterial( pMat, lMatTriangles);

			// Extract the subMesh data and save the index transform map for later use.
			// Index map record
			std::map< unsigned int, unsigned int> lIndexReMap;
			CMeshArray* pSubMeshData = ExtractSubMeshData_Vec3(pNormalArray, pIntermediateMesh, pMat,lIndexReMap);
			unsigned int iVertCount = pSubMeshData->Size();

			// allocate the position vertex buffer
			unsigned int iBind = vertexDecl->getElementCount();
			vertexDecl->addElement(iBind, 0, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
			vertexData->vertexCount = iVertCount;
		
			Ogre::HardwareVertexBufferSharedPtr vBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(iBind), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
			Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;
			binding->setBinding(iBind, vBuf); // HACK!
			const Ogre::VertexElement* posElem = vertexDecl->findElementBySemantic(Ogre::VES_NORMAL);
			unsigned char* vertex = static_cast<unsigned char*>(vBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

			m_pProgressDlg->LocalStep("Writing Dedicated Normal Buffers..");

			// Ok, Let큦 fill in the buffer with data  :D
			const Ogre::Vector3* data = (const Ogre::Vector3*)pSubMeshData->Data();
			Ogre::Real* pReal;
			posElem->baseVertexPointerToElement(vertex, &pReal);

			unsigned int j = 0;
			for(unsigned int i=0; i< iVertCount; i++) 
			{
				Ogre::Vector3 vert = data[i];

				*pReal++ = vert.x;
				*pReal++ = vert.y;
				*pReal++ = vert.z;
			}
			vBuf->unlock();

			it++;

			delete pSubMeshData;
		}
	}

	OgreAssert(_CrtCheckMemory(), "Memory corruption");

	END_PROFILE("COgreMeshCompiler::CreateNormalBuffer()");
}

void COgreMeshCompiler::CreateDiffuseBuffer( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);

	START_PROFILE("COgreMeshCompiler::CreateDiffuseBuffer()");

	CMeshArray* pDiffuseArray = pIntermediateMesh->GetArray("diffuse",0);

	bool bCleanUp = false;
	if(pDiffuseArray == NULL)
	{
		LOGWARNING "No Vertex Colours defined, will export white colors on each vertex!.");

		// Fallback on standard white.
		unsigned int iVIndex = 0;
		CVec4Array* pArray = new CVec4Array(m_iNrVerts);
		bCleanUp = true;
		for(unsigned int x = 0; x < m_iNrVerts; x++)
		{
			Ogre::Vector4& vColor = (*pArray)[iVIndex++];
			vColor.x = 1.0f;
			vColor.y = 1.0f;
			vColor.z = 1.0f;
			vColor.w = 1.0f;
		}
		pDiffuseArray = pArray;
	}

	int iNrVerts = pDiffuseArray->Size();

	Ogre::VertexData* vertexData;
	

	if(m_bUseSharedGeometry)
	{
		// Shared Geometry
		Ogre::VertexData* vertexData = vertexData = m_pOgreMesh->sharedVertexData;
		Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;

		// diffuse colours
		vertexDecl->addElement(m_iBind, 0, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);

		// allocate the diffuse vertex buffer
		vertexData->vertexCount = iNrVerts;
		int vsize = vertexDecl->getVertexSize(m_iBind);
		Ogre::HardwareVertexBufferSharedPtr dBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(m_iBind), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(m_iBind, dBuf);
		const Ogre::VertexElement* diffuseElem = vertexData->vertexDeclaration->getElement(m_iBind);
		unsigned char* vertex = static_cast<unsigned char*>(dBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
		m_iBind++;

		Ogre::ARGB* pReal;
		diffuseElem->baseVertexPointerToElement(vertex, &pReal);

		m_pProgressDlg->LocalStep("Writing Shared Vertex Colour Buffer..");

		const Ogre::Vector4* data = (const Ogre::Vector4*)pDiffuseArray->Data();
		for (int i = 0 ; i < iNrVerts; i++)
		{
			const Ogre::Vector4& vert = data[i];

			*pReal++=	(((unsigned int)(vert.w*255.0))<<24)|
						(((unsigned int)(vert.x*255.0))<<16)|
						(((unsigned int)(vert.y*255.0))<<8)|
						(((unsigned int)(vert.z*255.0)));
		}
		dBuf->unlock();
	}
	else
	{
		// DEDICATED GEOMETRY BUFFERS
		const CTriangleArray& triangleArray = pIntermediateMesh->GetTriangles();
		std::map<CIntermediateMaterial*, Ogre::SubMesh*>::iterator it = m_lMaterialSubMeshMap.begin();
		while( it != m_lMaterialSubMeshMap.end() )
		{
			CIntermediateMaterial* pMat = it->first;
			Ogre::SubMesh* pSubMesh = it->second;
			vertexData = pSubMesh->vertexData;
			Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;

			//submesh index lookup
			OgreAssert(m_lSubMeshIndexReMap.find(pSubMesh) != m_lSubMeshIndexReMap.end(), "submesh not found in submesindexremap");
			std::map< unsigned int, unsigned int> &lIndexReMap
			  = m_lSubMeshIndexReMap.find(pSubMesh)->second;
			CMeshArray* pSubMeshData = ExtractSubMeshDataFromIndexMap_Vec4(pDiffuseArray,lIndexReMap);
			unsigned int iVertCount = pSubMeshData->Size();

			// allocate the position vertex buffer
			unsigned int iBind = vertexDecl->getElementCount();
			vertexDecl->addElement(iBind, 0, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
			vertexData->vertexCount = iVertCount;
		
			Ogre::HardwareVertexBufferSharedPtr dBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(iBind), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
			Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;
			binding->setBinding(iBind, dBuf); // Hmm.. seems like the index parameter really should be the source
			const Ogre::VertexElement* diffuseElem = vertexDecl->findElementBySemantic(Ogre::VES_DIFFUSE);
			unsigned char* vertex = static_cast<unsigned char*>(dBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

			//m_pProgressDlg->InitLocal(iVertCount);
			m_pProgressDlg->LocalStep("Writing Dedicated Vertex Colour Buffers..");

			// Ok, Let큦 fill in the buffer with data  :D
			Ogre::ARGB* pReal;
			diffuseElem->baseVertexPointerToElement(vertex, &pReal);
			const Ogre::Vector4* data = (const Ogre::Vector4*)pSubMeshData->Data();

			for (int i = 0 ; i < iVertCount; i++)
			{
				const Ogre::Vector4& vert = data[i];
				*pReal++=	(((unsigned int)(vert.w*255.0))<<24)|
							(((unsigned int)(vert.x*255.0))<<16)|
							(((unsigned int)(vert.y*255.0))<<8)|
							(((unsigned int)(vert.z*255.0)));
			}

			dBuf->unlock();
			it++;
			delete pSubMeshData;
		}
	}

	if(bCleanUp)
		delete pDiffuseArray;

	OgreAssert(_CrtCheckMemory(), "Memory corruption");

	END_PROFILE("COgreMeshCompiler::CreateDiffuseBuffer()");
}

void COgreMeshCompiler::CreateTexCoordBuffer( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);

	START_PROFILE("COgreMeshCompiler::CreateTexCoordBuffer()");

	unsigned int iUVIndex = 0;
	for(unsigned int x = 1; x <= 99; x++)
	{
		char temp[32];
		sprintf(temp, "uv%i", x);
		CMeshArray* pUVArray = pIntermediateMesh->GetArray(temp,0);

		if(pUVArray == NULL)
			continue;

		
		iUVIndex++;

		if(m_bUseSharedGeometry)
		{
			// Shared
			Ogre::VertexData* vertexData = vertexData = m_pOgreMesh->sharedVertexData;
			Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;

			// UV coords
			vertexDecl->addElement(m_iBind, 0, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES,iUVIndex-1);

			// allocate the UV buffer
			int iNrUVs = pUVArray->Size();
			vertexData->vertexCount = iNrUVs;
			Ogre::HardwareVertexBufferSharedPtr uvBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(m_iBind), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
			Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;

			const Ogre::VertexElement* uvElem = vertexDecl->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES,iUVIndex-1);
			binding->setBinding(m_iBind, uvBuf);

			unsigned char* vertex = static_cast<unsigned char*>(uvBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
			m_iBind++;

			Ogre::Real* pReal;
			uvElem->baseVertexPointerToElement(vertex, &pReal);

			m_pProgressDlg->LocalStep("Writing Shared Texture Coordinate Buffer..");

			const Ogre::Vector2* data = (const Ogre::Vector2*)pUVArray->Data();

			for (int i = 0 ; i < iNrUVs; i++)
			{
				const Ogre::Vector2& vert = data[i];
				*pReal++= vert.x;
				*pReal++= vert.y;
			}
			uvBuf->unlock();
		}
		else
		{
			// Dedicated
			const CTriangleArray& triangleArray = pIntermediateMesh->GetTriangles();
			std::map<CIntermediateMaterial*, Ogre::SubMesh*>::iterator it = m_lMaterialSubMeshMap.begin();
			while( it != m_lMaterialSubMeshMap.end() )
			{
				CIntermediateMaterial* pMat = it->first;
				Ogre::SubMesh* pSubMesh = it->second;
				Ogre::VertexData*  vertexData = pSubMesh->vertexData;
				Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;

				//submesh index lookup
			  OgreAssert(m_lSubMeshIndexReMap.find(pSubMesh) != m_lSubMeshIndexReMap.end(), "submesh not found in submesindexremap");
				std::map< unsigned int, unsigned int> &lIndexReMap
				  = m_lSubMeshIndexReMap.find(pSubMesh)->second;
				CMeshArray* pSubMeshData = ExtractSubMeshDataFromIndexMap_Vec2(pUVArray,lIndexReMap);
				unsigned int iUVCount = pSubMeshData->Size();

				unsigned int iBind = vertexDecl->getElementCount();
				vertexDecl->addElement(iBind, 0, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES,iUVIndex-1);

				// allocate the position vertex buffer
				vertexData->vertexCount = iUVCount;
				Ogre::HardwareVertexBufferSharedPtr uvBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(iBind), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
				Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;
				binding->setBinding(iBind, uvBuf);

				const Ogre::VertexElement* uvElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES, iUVIndex-1);
				unsigned char* vertex = static_cast<unsigned char*>(uvBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

				Ogre::Real* pReal;
				uvElem->baseVertexPointerToElement(vertex, &pReal);
			
				m_pProgressDlg->LocalStep("Writing Dedicated Texture Coordinate Buffers..");

				const Ogre::Vector2* data = (const Ogre::Vector2*)pSubMeshData->Data();

				for (int i = 0 ; i < iUVCount; i++)
				{
					const Ogre::Vector2& vert = data[i];
					*pReal++= vert.x;
					*pReal++= vert.y;
				}
				uvBuf->unlock();
				it++;
			}
		}
	}

	END_PROFILE("COgreMeshCompiler::CreateTexCoordBuffer()");
}

void COgreMeshCompiler::CreateMeshBounds( void )
{
	Ogre::Real width = Ogre::Math::Sqrt(m_MaxSquaredLength);
	m_pOgreMesh->_setBounds(m_AABB);
	m_pOgreMesh->_setBoundingSphereRadius(width);
}

bool COgreMeshCompiler::WriteOgreMesh( const Ogre::String& sFilename, bool bXMLexport )
{
	assert(Ogre::MeshManager::getSingletonPtr());

	START_PROFILE("COgreMeshCompiler::WriteOgreMesh()");

	m_pProgressDlg->InitLocal(2);
	if(bXMLexport)
	{
		m_pProgressDlg->LocalStep("Writing Ogre XML Mesh File..");
		Ogre::String xmlFileName = sFilename;
		xmlFileName.append(".xml");

		Ogre::XMLMeshSerializer* pMeshWriter = new Ogre::XMLMeshSerializer();
		try
		{	
			LOGINFO "Writing Ogre XML Mesh File (%s) ...", xmlFileName.c_str());
			pMeshWriter->exportMesh(m_pOgreMesh.get(), xmlFileName);
		}
		catch (Ogre::Exception& e)
		{
			LOGERROR "OgreException: %s", e.getFullDescription().c_str());
			return false;
		} catch(...)
		{
			LOGERROR "Unhandled exception caught in COgreMeshCompiler::WriteOgreMesh()");
		}
		delete pMeshWriter;
	}
	else
	{
		m_pProgressDlg->LocalStep("Writing Ogre Mesh File..");
		Ogre::MeshSerializer* pMeshWriter = new Ogre::MeshSerializer();
		try
		{	
			LOGINFO "Writing Ogre Mesh (%s) ...", sFilename.c_str());
			pMeshWriter->exportMesh(m_pOgreMesh.get(), sFilename);
		}
		catch (Ogre::Exception& e)
		{
			LOGERROR "OgreException: %s", e.getFullDescription().c_str());
			return false;
		} catch(...)
		{
			LOGERROR "Unhandled exception caught in COgreMeshCompiler::WriteOgreMesh()");
		}
		delete pMeshWriter;
	}

	m_pProgressDlg->LocalStep();

	END_PROFILE("COgreMeshCompiler::WriteOgreMesh()");

	return true;
}

Ogre::MeshPtr COgreMeshCompiler::GetOgreMesh( void )
{
	return m_pOgreMesh;
}

void COgreMeshCompiler::CreatePoseBuffers( CIntermediateMesh* pIntermediateMesh )
{
	START_PROFILE("COgreMeshCompiler::CreatePoseBuffers()");

	// Create static pose data
	unsigned int iPoseCount = pIntermediateMesh->GetPoseCount();

	m_pProgressDlg->InitLocal(iPoseCount);
	m_pProgressDlg->LocalStep("Creating Static Poses..");

	//unsigned int iNrPosesCreated = 0;
	for(unsigned int i=0; i<iPoseCount; i++)
	{
		Ogre::String poseName;
		unsigned int iFrame;
		bool bOptimize;
		pIntermediateMesh->GetPose(i, poseName, iFrame,bOptimize);
		iFrame *= GetTicksPerFrame();

		int* pIndices = NULL;
		unsigned int iNrPosesCreated = CreatePose(&pIndices, pIntermediateMesh, poseName, iFrame, bOptimize );

		m_pProgressDlg->LocalStep();
	}

	// Create pose animations
	unsigned int iPoseAnimCount = pIntermediateMesh->GetPoseAnimCount();

	for(unsigned int i=0; i<iPoseAnimCount; i++)
	{
		

		Ogre::String animName;
		unsigned int iStartFrame;
		unsigned int iEndFrame;
		float fRate;
		bool bOptimize;
		pIntermediateMesh->GetPoseAnimation(i, animName, iStartFrame, iEndFrame, fRate, bOptimize);

		float fps = 1.0 / float(GetFrameRate());
		float fAnimLength = (iEndFrame - iStartFrame) * fps;

		Ogre::Animation* pAnim = m_pOgreMesh->createAnimation(animName,fAnimLength);

		std::map< int, Ogre::VertexAnimationTrack* > lTracks;

		m_pProgressDlg->InitLocal( (iEndFrame-iStartFrame)/fRate );
		Ogre::String progressDesc = "Creating Pose Animation: ";
		progressDesc+= animName;
		m_pProgressDlg->LocalStep(progressDesc.c_str());

		float x = iStartFrame;
		int frameCount = 0;
		while(x <= iEndFrame)
		{
			m_pProgressDlg->LocalStep();

			float time = (x- iStartFrame)*fps; // local in this animation
			
			Ogre::String poseName = animName;
			poseName += "_";
			poseName += Ogre::StringConverter::toString(frameCount++);

			int* pIndices = NULL;
			unsigned int iNrPosesCreated = CreatePose(&pIndices, pIntermediateMesh, poseName, x*GetTicksPerFrame(), bOptimize );

			for(int j=0; j<iNrPosesCreated; j++)
			{
				int poseIndex = pIndices[j]-1;
				int iTarget = m_pOgreMesh->getPose(poseIndex)->getTarget();

				Ogre::VertexAnimationTrack* pTrack = NULL;
				std::map< int, Ogre::VertexAnimationTrack* >::iterator it = lTracks.find(iTarget);
				if(it == lTracks.end())
				{
					pTrack = pAnim->createVertexTrack(iTarget, Ogre::VAT_POSE);
					lTracks.insert(std::pair< int, Ogre::VertexAnimationTrack* >(iTarget, pTrack) );
				}
				else
					pTrack = it->second;
				
				Ogre::VertexPoseKeyFrame* pKeyFrame = pTrack->createVertexPoseKeyFrame(time);
				pKeyFrame->addPoseReference(poseIndex,1.0f);
			}
			x += fRate;
			delete [] pIndices;
		}

		if(bOptimize)
		{
			START_PROFILE("COgreMeshCompiler::CreatePoseBuffers():pAnim->optimise();");
			pAnim->optimise();
			END_PROFILE("COgreMeshCompiler::CreatePoseBuffers():pAnim->optimise();");
		}
	}
	END_PROFILE("COgreMeshCompiler::CreatePoseBuffers()");
}

/** Create pose
	@return Nr of poses created
	@param poseIndex List of indices for the poses created;
*/
unsigned int COgreMeshCompiler::CreatePose( int** poseIndex, CIntermediateMesh* pIntermediateMesh, Ogre::String poseName, unsigned int iFrame, bool bOptimize )
{
	START_PROFILE("COgreMeshCompiler::CreatePose()");

	int PosesCreated = 0;
	Ogre::Pose* pPose = NULL;

	CMeshArray* pVerts = new CVec3Array();
	CMeshArray* pOrigVerts = new CVec3Array();

	// then the pose is the same as the reference, so nothing is needed to be checked
	if(iFrame != 0)
	{
		START_PROFILE("COgreMeshCompiler::CreatePose():Get Buffers");

		CMeshArray* pTmpBuf = pIntermediateMesh->GetArray("position", iFrame);
		pVerts->Create(pTmpBuf->Size(), pTmpBuf->Data());

		pTmpBuf = pIntermediateMesh->GetArray("position", 0);
		pOrigVerts->Create(pTmpBuf->Size(), pTmpBuf->Data());

		END_PROFILE("COgreMeshCompiler::CreatePose():Get Buffers");
		
		if(m_bReindex)
		{
			START_PROFILE("COgreMeshCompiler::CreatePose():Reindexing");

				SharedUtilities::fastvector< CMeshArray* > bufferList;

				bufferList.push_back(pVerts);
				bufferList.push_back(pOrigVerts);

				pIntermediateMesh->PostReindex( bufferList );
			END_PROFILE("COgreMeshCompiler::CreatePose():Reindexing");
		}
	}

	if(m_bUseSharedGeometry)
	{
		pPose = m_pOgreMesh->createPose( 0/*shared geometry buffer*/, poseName);
		m_iNrPoses++;
		PosesCreated++;

		*poseIndex = new int[1];
		(*poseIndex)[0] = m_iNrPoses;

		if(iFrame == 0)
		{
			pPose->clearVertexOffsets();
			return PosesCreated;
		}

		const Ogre::Vector3* data = (const Ogre::Vector3*)pVerts->Data();
		const Ogre::Vector3* origData = (const Ogre::Vector3*)pOrigVerts->Data();

		for(unsigned int j=0; j< pVerts->Size(); j++)
		{
			Ogre::Vector3 vert = data[j];
			Ogre::Vector3 origVert = origData[j];

			vert = vert - origVert;

			if( !bOptimize )
			{
				pPose->addVertex(j,vert);
			}
			else if(vert != Ogre::Vector3::ZERO)
				pPose->addVertex(j,vert);
		}
	}
	else
	{
		// Dedicated Geometry
		const CTriangleArray& triangleArray = pIntermediateMesh->GetTriangles();
		std::map<CIntermediateMaterial*, Ogre::SubMesh*>::iterator it = m_lMaterialSubMeshMap.begin();

		*poseIndex = new int[m_lMaterialSubMeshMap.size()];

		while( it != m_lMaterialSubMeshMap.end() )
		{
			CIntermediateMaterial* pMat = it->first;
			Ogre::SubMesh* pSubMesh = it->second;

			unsigned int iSubmeshIndex = m_pOgreMesh->_getSubMeshIndex(pMat->GetName());
			Ogre::String sSubmeshPoseName = poseName + "_" + pMat->GetName();
			pPose = m_pOgreMesh->createPose( iSubmeshIndex+1, sSubmeshPoseName );
			m_iNrPoses++;
			(*poseIndex)[PosesCreated++]=m_iNrPoses;

			if(iFrame == 0)
			{
				pPose->clearVertexOffsets();
				it++;
				continue;
			}

			// pose reference
			OgreAssert(m_lSubMeshIndexReMap.find(pSubMesh) != m_lSubMeshIndexReMap.end(), "submesh not found in submesindexremap");
			std::map< unsigned int, unsigned int> &lIndexReMap
			    = m_lSubMeshIndexReMap.find(pSubMesh)->second;
			CMeshArray* pSubMeshData = ExtractSubMeshDataFromIndexMap_Vec3(pVerts,lIndexReMap);

			// binding reference (frame 0)
			CMeshArray* pOrigSubMeshData = ExtractSubMeshDataFromIndexMap_Vec3(pOrigVerts,lIndexReMap);

			START_PROFILE("COgreMeshCompiler::CreatePose():Copy data to OgreMesh");
			const Ogre::Vector3* data = (const Ogre::Vector3*)pSubMeshData->Data();
			const Ogre::Vector3* origData = (const Ogre::Vector3*)pOrigSubMeshData->Data();

			for(unsigned int j=0; j< pSubMeshData->Size(); j++)
			{
				Ogre::Vector3 vert = data[j];
				Ogre::Vector3 origVert = origData[j];

				vert = vert - origVert;

				if( !bOptimize )
				{
					pPose->addVertex(j,vert);
				}
				else if(vert != Ogre::Vector3::ZERO)
					pPose->addVertex(j,vert);
			}

			delete pSubMeshData;
			delete pOrigSubMeshData;

			END_PROFILE("COgreMeshCompiler::CreatePose():Copy data to OgreMesh");

			it++;
		}


	}
	delete pOrigVerts;
	delete pVerts;

	OgreAssert(_CrtCheckMemory(), "Memory corruption");

	END_PROFILE("COgreMeshCompiler::CreatePose()");

	return PosesCreated;
}

void COgreMeshCompiler::OptimizeVertexDeclaration( void )
{
	if(m_bOptimizeBuffers)
	{
		
		if(m_bUseSharedGeometry)
		{
			Ogre::VertexDeclaration* new_vd = m_pOgreMesh->sharedVertexData->vertexDeclaration->getAutoOrganisedDeclaration( m_bExportSkeleton, m_bPoseAnimation );
			m_pOgreMesh->sharedVertexData->reorganiseBuffers( new_vd );
		}
		else
		{
			Ogre::Mesh::SubMeshIterator iter = m_pOgreMesh->getSubMeshIterator();
			while(iter.hasMoreElements())
			{
				Ogre::SubMesh* pSubMesh = iter.getNext();
				Ogre::VertexDeclaration* new_vd = pSubMesh->vertexData->vertexDeclaration->getAutoOrganisedDeclaration( m_bExportSkeleton, m_bPoseAnimation );
				pSubMesh->vertexData->reorganiseBuffers( new_vd );
			}
		}
	}
}

/** Builds and returns a mesh array with the verts for a submesh.
	@param lIndexMap returns a map for mapping old indicies (shared buffer) to new indicies (dedicated buffer)
*/
CMeshArray* COgreMeshCompiler::ExtractSubMeshData_Vec3( CMeshArray* pMeshArray, CIntermediateMesh* pIMesh, CIntermediateMaterial* pIMat, std::map< unsigned int, unsigned int>& lIndexMap )
{
	START_PROFILE("COgreMeshCompiler::ExtractSubMeshData_Vec3()");
	// MODIFY TO ONLY WORK ON INDEXES. SINCE THIS SHOULD BE USED FOR VERTICIES AND NORMALS AND WEIGHTS AND DIFFUSE_COLORS .. ETC.

	CMeshArray* pSubMeshArray = NULL;

	const CTriangleArray& triangleArray = pIMesh->GetTriangles();
	const Ogre::Vector3* data = (const Ogre::Vector3*)pMeshArray->Data();

	std::vector< unsigned int > lMatTriangles;
	pIMesh->GetTrianglesUsingMaterial( pIMat, lMatTriangles);

	unsigned int iThird = lMatTriangles.size()/3;
	SharedUtilities::fastintmap< unsigned int > lVertexMap(iThird,iThird);

	START_PROFILE("COgreMeshCompiler::ExtractSubMeshData_Vec3():BuildVertexmap");
	// Pass one: Build ordered list of (unique) indexed verticies.
	for(unsigned int i = 0; i< lMatTriangles.size(); i++) 
	{
		const CTriangle& face = triangleArray[ lMatTriangles[i] ];
		for(unsigned int x = 0; x< 3; x++) 
		{
			unsigned int iOldIndex = face.m_Vertices[x];

			if(m_bReindex)
			{
				unsigned int returnVal;
				if(!lVertexMap.find(iOldIndex,returnVal))
					lVertexMap.map(iOldIndex,iOldIndex);
			}
			else
			{
				lVertexMap.map(iOldIndex,iOldIndex); // If it already exists, its just overwritten
			}
		}
	}
	END_PROFILE("COgreMeshCompiler::ExtractSubMeshData_Vec3():BuildVertexmap");

	// Build list of ordered Verticies AND oldIndex-to-newIndex map
	std::vector< const Ogre::Vector3* > lUniqueVerts;
	std::map< const Ogre::Vector3*, unsigned int > lHelperMap;
	lIndexMap.clear();
	unsigned int newIndex=0;

	START_PROFILE("COgreMeshCompiler::ExtractSubMeshData_Vec3():BuildIndexmap");
	SharedUtilities::fastvector<unsigned int> lVertexList = lVertexMap.data();
	unsigned int iVertCount = lVertexList.size();

	CVec3Array* pArray = new CVec3Array(iVertCount);

	for(unsigned int i = 0; i< iVertCount; i++) 
	{
		unsigned int oldIndex = lVertexList[i];
		const Ogre::Vector3* pVert = &(data[ oldIndex ]);// = it->second;

		(*pArray)[i] = *pVert;

		lIndexMap.insert( std::pair< unsigned int, unsigned int>(oldIndex,i) );// registeredIndex) );
	}
	END_PROFILE("COgreMeshCompiler::ExtractSubMeshData_Vec3():BuildIndexmap");

	pSubMeshArray = pArray;

	END_PROFILE("COgreMeshCompiler::ExtractSubMeshData_Vec3()");
	return pSubMeshArray;
}

CMeshArray* COgreMeshCompiler::ExtractSubMeshData_Vec4( CMeshArray* pMeshArray, CIntermediateMesh* pIMesh, CIntermediateMaterial* pIMat, std::map< unsigned int, unsigned int>& lIndexMap )
{
	START_PROFILE("COgreMeshCompiler::ExtractSubMeshData_Vec4()");
	// MODIFY TO ONLY WORK ON INDEXES. SINCE THIS SHOULD BE USED FOR VERTICIES AND NORMALS AND WEIGHTS AND DIFFUSE_COLORS .. ETC.

	CMeshArray* pSubMeshArray = NULL;

	const CTriangleArray& triangleArray = pIMesh->GetTriangles();
	const Ogre::Vector4* data = (const Ogre::Vector4*)pMeshArray->Data();

	std::vector< unsigned int > lMatTriangles;
	pIMesh->GetTrianglesUsingMaterial( pIMat, lMatTriangles);

	std::map< unsigned int, Ogre::Vector4* > lVertexMap;

	// Pass one: Build ordered list of (unique) indexed verticies.
	for(unsigned int i = 0; i< lMatTriangles.size(); i++) 
	{
		const CTriangle& face = triangleArray[ lMatTriangles[i] ];
		for(unsigned int x = 0; x< 3; x++) 
		{
			unsigned int index = face.m_Vertices[x];
			Ogre::Vector4* pVert = new Ogre::Vector4( data[ index ] );

			std::map<unsigned int, Ogre::Vector4*>::iterator it = lVertexMap.find(index);
			if(it == lVertexMap.end())
				lVertexMap.insert( std::pair<unsigned int, Ogre::Vector4*>(index,pVert) );
		}
	}

	// Build list of ordered Verticies AND oldIndex-to-newIndex map
	std::vector< Ogre::Vector4* > lUniqueVerts;
	std::map< Ogre::Vector4*, unsigned int > lHelperMap;
	lIndexMap.clear();
	unsigned int newIndex=0;

	std::map< unsigned int, Ogre::Vector4* >::iterator it = lVertexMap.begin();
	while(it != lVertexMap.end())
	{
		unsigned int oldIndex = it->first;
		Ogre::Vector4* pVert = it->second;
		std::map< Ogre::Vector4*, unsigned int >::iterator it2 = lHelperMap.find(pVert);
		if(it2 == lHelperMap.end())
		{
			lHelperMap.insert( std::pair< Ogre::Vector4*, unsigned int >(pVert,newIndex) );
			lUniqueVerts.push_back(pVert);
			lIndexMap.insert( std::pair< unsigned int, unsigned int>(oldIndex, newIndex++) );
		}
		else
		{
			unsigned int registeredIndex = it2->second;
			lIndexMap.insert( std::pair< unsigned int, unsigned int>(oldIndex, registeredIndex) );
		}
		it++;
	}

	// Build SubMeshArray
	unsigned int iVertCount = lUniqueVerts.size();
	CVec4Array* pArray = new CVec4Array(iVertCount);
	for(unsigned int j=0; j < iVertCount; j++)
	{
		(*pArray)[j] = *(lUniqueVerts[j]);
	}
	pSubMeshArray = pArray;

	END_PROFILE("COgreMeshCompiler::ExtractSubMeshData_Vec4()");

	return pSubMeshArray;
}

CMeshArray* COgreMeshCompiler::ExtractSubMeshDataFromIndexMap_Vec4( CMeshArray* pMeshArray, std::map< unsigned int, unsigned int>& lIndexMap )
{
	START_PROFILE("COgreMeshCompiler::ExtractSubMeshDataFromIndexMap_Vec4()");
	// MODIFY TO ONLY WORK ON INDEXES. SINCE THIS SHOULD BE USED FOR VERTICIES AND NORMALS AND WEIGHTS AND DIFFUSE_COLORS .. ETC.

	CMeshArray* pSubMeshArray = NULL;

	const Ogre::Vector4* data = (const Ogre::Vector4*)pMeshArray->Data();

	// Build SubMeshArray
	unsigned int iVertCount = lIndexMap.size();
	CVec4Array* pArray = new CVec4Array(iVertCount);

	unsigned int i = 0;
	unsigned int iMeshArraySize = pMeshArray->Size();
	for(unsigned int oldIndex=0; oldIndex < iMeshArraySize; oldIndex++)
	{
		std::map< unsigned int, unsigned int>::iterator it = lIndexMap.find(oldIndex);

		if(it != lIndexMap.end())
		{
			unsigned int newIndex = it->second;
			(*pArray)[newIndex] = data[ oldIndex ];
		}
	}
	pSubMeshArray = pArray;

	END_PROFILE("COgreMeshCompiler::ExtractSubMeshDataFromIndexMap_Vec4()");

	return pSubMeshArray;
}

CMeshArray* COgreMeshCompiler::ExtractSubMeshDataFromIndexMap_Vec3( CMeshArray* pMeshArray, std::map< unsigned int, unsigned int>& lIndexMap )
{
	START_PROFILE("COgreMeshCompiler::ExtractSubMeshDataFromIndexMap_Vec3()");
	// MODIFY TO ONLY WORK ON INDEXES. SINCE THIS SHOULD BE USED FOR VERTICIES AND NORMALS AND WEIGHTS AND DIFFUSE_COLORS .. ETC.

	CMeshArray* pSubMeshArray = NULL;

	const Ogre::Vector3* data = (const Ogre::Vector3*)pMeshArray->Data();

	// Build SubMeshArray
	unsigned int iVertCount = lIndexMap.size();
	CVec3Array* pArray = new CVec3Array(iVertCount);

	unsigned int i = 0;
	unsigned int iMeshArraySize = pMeshArray->Size();
	for(unsigned int oldIndex=0; oldIndex < iMeshArraySize; oldIndex++)
	{
		std::map< unsigned int, unsigned int>::iterator it = lIndexMap.find(oldIndex);

		if(it != lIndexMap.end())
		{
			unsigned int newIndex = it->second;
			(*pArray)[newIndex] = data[ oldIndex ];
		}
	}
	pSubMeshArray = pArray;

	END_PROFILE("COgreMeshCompiler::ExtractSubMeshDataFromIndexMap_Vec3()");

	return pSubMeshArray;
}


CMeshArray* COgreMeshCompiler::ExtractSubMeshDataFromIndexMap_Vec2( CMeshArray* pMeshArray, std::map< unsigned int, unsigned int>& lIndexMap )
{
	START_PROFILE("COgreMeshCompiler::ExtractSubMeshDataFromIndexMap_Vec2()");
	// MODIFY TO ONLY WORK ON INDEXES. SINCE THIS SHOULD BE USED FOR VERTICIES AND NORMALS AND WEIGHTS AND DIFFUSE_COLORS .. ETC.

	CMeshArray* pSubMeshArray = NULL;

	const Ogre::Vector2* data = (const Ogre::Vector2*)pMeshArray->Data();

	// Build SubMeshArray
	unsigned int iVertCount = lIndexMap.size();
	CVec2Array* pArray = new CVec2Array(iVertCount);

	unsigned int i = 0;
	unsigned int iMeshArraySize = pMeshArray->Size();
	for(unsigned int oldIndex=0; oldIndex < iMeshArraySize; oldIndex++)
	{
		std::map< unsigned int, unsigned int>::iterator it = lIndexMap.find(oldIndex);

		if(it != lIndexMap.end())
		{
			unsigned int newIndex = it->second;
			(*pArray)[newIndex] = data[ oldIndex ];
		}
	}
	pSubMeshArray = pArray;

	END_PROFILE("COgreMeshCompiler::ExtractSubMeshDataFromIndexMap_Vec2()");

	return pSubMeshArray;
}