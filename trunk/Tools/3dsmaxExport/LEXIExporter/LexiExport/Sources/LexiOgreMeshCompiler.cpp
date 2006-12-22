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

COgreMeshCompiler::COgreMeshCompiler( CIntermediateMesh* pIntermediateMesh, const CDDObject* pConfig, Ogre::String filename) :
	m_iBind(0),
	m_MaxSquaredLength(0.0),
	m_pOgreMesh(NULL),
	m_bReindex(false),
	m_bExportNormals(false),
	m_bExportColours(false),
	m_bExportTexUVs(false),
	m_bExportSkeleton(false)
{
	m_IndexBitType = Ogre::HardwareIndexBuffer::IT_16BIT;

	InitializeOgreComponents();

	ReadConfig(pConfig);

	if(m_bReindex)
		ReindexIntermediateBuffers(pIntermediateMesh);

	Ogre::LogManager::getSingletonPtr()->logMessage("OgreMeshCompiler: Creating Mesh");
	// create basic ogre mesh structure
	CreateOgreMesh(pIntermediateMesh);

	

	Ogre::LogManager::getSingletonPtr()->logMessage("OgreMeshCompiler: Creating HWBuffers");
	// create hw buffers i.e.(vertex,index,normal..)
	CreateBuffers(pIntermediateMesh);

	if(m_bExportSkeleton)
		if(pIntermediateMesh->GetSkeleton()!= NULL)
			m_pSkeletonCompiler = new COgreSkeletonCompiler( pIntermediateMesh->GetSkeleton(), pConfig, filename, m_pOgreMesh );
		else
		{
			MessageBox(NULL, "No Skin Modifier found. Ignoring..", "Warning!", MB_ICONWARNING);
			m_bExportSkeleton = false;
		}



	Ogre::LogManager::getSingletonPtr()->logMessage("OgreMeshCompiler: Creating Bounds");
	// create and register bounding box
	CreateMeshBounds();

	// ready for serialization!
}

COgreMeshCompiler::~COgreMeshCompiler()
{
	if(m_bExportSkeleton)
		delete m_pSkeletonCompiler;
}

void COgreMeshCompiler::InitializeOgreComponents( void )
{
	//if(Ogre::Root::getSingletonPtr() == NULL )
	//	new Ogre::Root();
	//else if(!Ogre::Root::getSingletonPtr()->isInitialised())
	//{
	//	Ogre::LogManager::getSingletonPtr()->logMessage("Ogre::Root Created. Starting Initializing....");
	//	Ogre::Root::getSingletonPtr()->initialise(false,"foorbar");
	//	Ogre::LogManager::getSingletonPtr()->logMessage("Ogre::Root Created. Initialization Done.");
	//}


	//if(Ogre::LogManager::getSingletonPtr() == NULL)
	//{
	//	new Ogre::LogManager();
	//	Ogre::LogManager::getSingletonPtr()->createLog("Ogre.log");
	//}

	//if(Ogre::ResourceGroupManager::getSingletonPtr() == NULL) 
	//	new Ogre::ResourceGroupManager();

	//if(Ogre::MeshManager::getSingletonPtr() == NULL) 
	//	new Ogre::MeshManager();


}


void COgreMeshCompiler::ReadConfig( const CDDObject* pConfig )
{
	assert(pConfig);

	m_bReindex = pConfig->GetBool("reindexID");
	m_bExportNormals = pConfig->GetBool("normalsID");
	m_bExportColours = pConfig->GetBool("vertexColorsID");
	m_bExportTexUVs = pConfig->GetBool("uvID");
	m_bExportSkeleton = pConfig->GetBool("SkeletonID");
}


void COgreMeshCompiler::ReindexIntermediateBuffers( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);

	SharedUtilities::fastvector< CMeshArray* > bufferList;
	bufferList.push_back(pIntermediateMesh->GetArray("position",0));

	CMeshArray* pArray; 

	pArray = pIntermediateMesh->GetArray("normal",0);
	if(m_bExportNormals)
	{
		if (pArray == NULL)
			MessageBox(NULL, "No Normals found. Ignoring..", "Warning!", MB_ICONWARNING);
		else
			bufferList.push_back(pArray);
	}

	pArray = pIntermediateMesh->GetArray("diffuse",0);
	if(m_bExportColours)
	{
		if (pArray == NULL)
			MessageBox(NULL, "No Vertex Colours defined. Ignoring..", "Warning!", MB_ICONWARNING);
		else
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

	pIntermediateMesh->Reindex( bufferList );
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
		Ogre::SubMesh* pSubMesh = m_pOgreMesh->createSubMesh();
		CIntermediateMaterial* mat = pIntermediateMesh->GetMaterial(i);
		m_lMaterialSubMeshMap.insert( std::pair<CIntermediateMaterial*,Ogre::SubMesh*>(mat,pSubMesh) );
	}
}

void COgreMeshCompiler::CreateBuffers( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);
	assert(!m_pOgreMesh.isNull());

	CreateVertexBuffer(pIntermediateMesh);
	CreateIndexBuffer(pIntermediateMesh);

	if(m_bExportNormals)
		CreateNormalBuffer(pIntermediateMesh);

	if(m_bExportColours)
		CreateDiffuseBuffer(pIntermediateMesh);
	
	if (m_bExportTexUVs)
		CreateTexCoordBuffer(pIntermediateMesh);

	//"specular"
	//"boneindex"
	//"blendweight"
	//"binormal"
	//"tangent"
}

void COgreMeshCompiler::CreateVertexBuffer( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);
	CMeshArray* pMeshArray = pIntermediateMesh->GetArray("position",0);
	if(pMeshArray == NULL)
		return;

	unsigned int iNrVerts = pMeshArray->Size();

	Ogre::VertexData* vertexData = m_pOgreMesh->sharedVertexData;
	Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;

	// allocate the position vertex buffer
	vertexDecl->addElement(m_iBind, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
	vertexData->vertexCount = iNrVerts;

	{	// DEBUG INFO
		Ogre::StringUtil::StrStreamType strStrm;
		strStrm << "Export: Vertex Size: " << vertexDecl->getVertexSize(m_iBind) << " | Vertex Count: " << vertexData->vertexCount << " | Buffer Size: " << vertexDecl->getVertexSize(m_iBind)*vertexData->vertexCount;
		Ogre::LogManager::getSingletonPtr()->logMessage(strStrm.str());
	}

	Ogre::HardwareVertexBufferSharedPtr vBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(m_iBind), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
	Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;
	binding->setBinding(m_iBind, vBuf);
	const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->getElement(m_iBind);
	unsigned char* vertex = static_cast<unsigned char*>(vBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
	m_iBind++;

	// Ok, Let´s fill in the buffer with data  :D

	const Ogre::Vector3* data = (const Ogre::Vector3*)pMeshArray->Data();

	Ogre::Real* pReal;
	posElem->baseVertexPointerToElement(vertex, &pReal);

	unsigned int j = 0;
	for(unsigned int i=0; i< iNrVerts; i++) {

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

	Ogre::StringUtil::StrStreamType strStrm;
	strStrm << "Export: #Vertices: " << iNrVerts;
	Ogre::LogManager::getSingletonPtr()->logMessage(strStrm.str());
	
}

void COgreMeshCompiler::CreateIndexBuffer( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);

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

		int iNrIndices = lMatTriangles.size() *3;
		pSubMesh->indexData->indexCount = iNrIndices;

		if(iNrIndices >= 65536)
			m_IndexBitType = Ogre::HardwareIndexBuffer::IT_32BIT;

		pSubMesh->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(m_IndexBitType, pSubMesh->indexData->indexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		Ogre::HardwareIndexBufferSharedPtr iBuf = pSubMesh->indexData->indexBuffer;
		pSubMesh->useSharedVertices = true;
		it++;

		// Ok, Let´s fill in the buffer with data  :D

		// We lock the buffer before adding content
		if(m_IndexBitType == Ogre::HardwareIndexBuffer::IT_32BIT)
		{	// 32 Bit Index Buffer
			unsigned int* pIndices = static_cast<unsigned int*>(iBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

			Ogre::StringUtil::StrStreamType strStrm3;
			strStrm3 << "Export: #Indices: " << iBuf->getNumIndexes();
			Ogre::LogManager::getSingletonPtr()->logMessage(strStrm3.str());

			unsigned int j = 0;
			for(unsigned int i = 0; i< lMatTriangles.size(); i++) {

				const CTriangle& face = triangleArray[ lMatTriangles[i] ];

				// indices
				pIndices[j++] = face.m_Vertices[0];
				pIndices[j++] = face.m_Vertices[1];
				pIndices[j++] = face.m_Vertices[2];

				// skeleton
				CIntermediateSkeleton* pISkel = pIntermediateMesh->GetSkeleton();
				if(pISkel != NULL) {

					Ogre::VertexBoneAssignment vertexBoneAssignment;

					for (int w=0; w < 3; w++)
					{
						int vertIndex = j-w;
						int iBoneCount = pISkel->GetNrOfAssignmentsOnVertex(vertIndex);

						for (int i=0; i < iBoneCount; i++)
						{
							SVertexBoneData boneData;
							vertexBoneAssignment.boneIndex = pISkel->GetVertexData(vertIndex,0,boneData);

							vertexBoneAssignment.vertexIndex = vertIndex;
							vertexBoneAssignment.boneIndex = boneData.boneIndex;
							vertexBoneAssignment.weight = boneData.weight;

							m_pOgreMesh->addBoneAssignment(vertexBoneAssignment);
						}

						//Vertex::BoneData::iterator it = smData.mVertexDeque[i].mBoneData.begin();
						//Vertex::BoneData::iterator iend = smData.mVertexDeque[i].mBoneData.end();

						//for(; it != iend; ++it) {
						//	VertexBoneAssignment& vertexBoneData = (*it);
						//	vertexBoneAssginment.boneIndex = vertexBoneData.mBoneIndex;
						//	vertexBoneAssginment.vertexIndex = i;
						//	vertexBoneAssginment.weight = vertexBoneData.mWeight;

						//	pSubMesh->addBoneAssignment(vertexBoneAssginment);

						//	/*
						//	if(!pSubMesh->useSharedVertices)
						//	pSubMesh->addBoneAssignment(vertexBoneAssginment);
						//	else
						//	mMesh->addBoneAssignment(vertexBoneAssginment);
						//	*/
						//}

					}
				}

			}

			// Unlock
			iBuf->unlock();
		}
		else
		{	// 16 Bit Index Buffer
			unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

			Ogre::StringUtil::StrStreamType strStrm3;
			strStrm3 << "Export: #Indices: " << iBuf->getNumIndexes();
			Ogre::LogManager::getSingletonPtr()->logMessage(strStrm3.str());

			unsigned int j = 0;
			for(unsigned int i = 0; i< lMatTriangles.size(); i++) {

				const CTriangle& face = triangleArray[ lMatTriangles[i] ];

				// indices
				pIndices[j++] = face.m_Vertices[0];
				pIndices[j++] = face.m_Vertices[1];
				pIndices[j++] = face.m_Vertices[2];


				// skeleton
				CIntermediateSkeleton* pISkel = pIntermediateMesh->GetSkeleton();
				if(pISkel != NULL) {

					Ogre::VertexBoneAssignment vertexBoneAssignment;

					for (int w=0; w < 3; w++)
					{
						int vertIndex = j-3+w;
						int iBoneCount = pISkel->GetNrOfAssignmentsOnVertex(vertIndex);

						//vertexBoneAssignment.vertexIndex = vertIndex;
						//vertexBoneAssignment.boneIndex = 2;
						//vertexBoneAssignment.weight = 1.0f;
						//m_pOgreMesh->addBoneAssignment(vertexBoneAssignment);

						//for (int i=0; i < 1; i++)
						for (int i=0; i < iBoneCount; i++)
						{
							SVertexBoneData boneData;
							pISkel->GetVertexData(vertIndex,i,boneData);

							//if(boneData.weight < 0.001f) continue;
							//boneData.weight = 1.0f;

							
							//int boneIndex = 0;
							//if(boneData.boneHandle == 2)
							//	boneIndex = 0;
							//else if (boneData.boneHandle == 3)
							//	boneIndex = 1;
							//else if (boneData.boneHandle == 5)
							//	boneIndex = 2;

							vertexBoneAssignment.vertexIndex	= vertIndex;
							vertexBoneAssignment.boneIndex		= boneData.boneIndex;
							vertexBoneAssignment.weight			= boneData.weight;

							Ogre::StringUtil::StrStreamType str;
							str << vertIndex << ": " << vertexBoneAssignment.boneIndex << "\t" << vertexBoneAssignment.weight;
							Ogre::LogManager::getSingletonPtr()->logMessage(str.str());

							m_pOgreMesh->addBoneAssignment(vertexBoneAssignment);
						}

						//Vertex::BoneData::iterator it = smData.mVertexDeque[i].mBoneData.begin();
						//Vertex::BoneData::iterator iend = smData.mVertexDeque[i].mBoneData.end();

						//for(; it != iend; ++it) {
						//	VertexBoneAssignment& vertexBoneData = (*it);
						//	vertexBoneAssginment.boneIndex = vertexBoneData.mBoneIndex;
						//	vertexBoneAssginment.vertexIndex = i;
						//	vertexBoneAssginment.weight = vertexBoneData.mWeight;

						//	pSubMesh->addBoneAssignment(vertexBoneAssginment);

						//	/*
						//	if(!pSubMesh->useSharedVertices)
						//	pSubMesh->addBoneAssignment(vertexBoneAssginment);
						//	else
						//	mMesh->addBoneAssignment(vertexBoneAssginment);
						//	*/
						//}

					}
				}
			}

			// Unlock
			iBuf->unlock();
		}

		if(pMat != NULL)
			pSubMesh->setMaterialName(pMat->GetName().c_str());
		else
			pSubMesh->setMaterialName("NoMaterial");
	}
}

void COgreMeshCompiler::CreateNormalBuffer( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);

	CMeshArray* pNormalArray = pIntermediateMesh->GetArray("normal",0);
	if(pNormalArray == NULL)
		return;

	int iNrNormals = pNormalArray->Size();

	Ogre::VertexData* vertexData = m_pOgreMesh->sharedVertexData;
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

	Ogre::StringUtil::StrStreamType strStrm3;
	strStrm3 << "Export: #Normals2Export: " << iNrNormals;
	Ogre::LogManager::getSingletonPtr()->logMessage(strStrm3.str());

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

void COgreMeshCompiler::CreateDiffuseBuffer( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);

	CMeshArray* pDiffuseArray = pIntermediateMesh->GetArray("diffuse",0);
	if(pDiffuseArray == NULL)
		return;

	int iNrVerts = pDiffuseArray->Size();

	Ogre::VertexData* vertexData = m_pOgreMesh->sharedVertexData;
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

	Ogre::StringUtil::StrStreamType strStrm3;
	strStrm3 << "Export: #Diffuse2Export: " << iNrVerts;
	Ogre::LogManager::getSingletonPtr()->logMessage(strStrm3.str());

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

void COgreMeshCompiler::CreateTexCoordBuffer( CIntermediateMesh* pIntermediateMesh )
{
	assert(pIntermediateMesh);


	unsigned int iUVIndex = 0;
	for(unsigned int x = 1; x <= 99; x++)
	{
		char temp[32];
		sprintf(temp, "uv%i", x);
		CMeshArray* pUVArray = pIntermediateMesh->GetArray(temp,0);
		iUVIndex = x;

		if(pUVArray == NULL)
			continue;

		Ogre::String msg = Ogre::String("Exporting UV Channel: ")+Ogre::String(temp);
		Ogre::LogManager::getSingletonPtr()->logMessage(msg);


		int iNrUVs = pUVArray->Size();

		Ogre::VertexData* vertexData = m_pOgreMesh->sharedVertexData;
		Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;

		// normals
		vertexDecl->addElement(m_iBind, 0, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES,iUVIndex-1);

		// allocate the position vertex buffer
		vertexData->vertexCount = iNrUVs;
		Ogre::HardwareVertexBufferSharedPtr uvBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(m_iBind), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		Ogre::VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(m_iBind, uvBuf);

		const Ogre::VertexElement* uvElem = vertexData->vertexDeclaration->getElement(m_iBind);
		unsigned char* vertex = static_cast<unsigned char*>(uvBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
		m_iBind++;

		Ogre::Real* pReal;
		uvElem->baseVertexPointerToElement(vertex, &pReal);

		const Ogre::Vector2* data = (const Ogre::Vector2*)pUVArray->Data();

		for (int i = 0 ; i < iNrUVs; i++)
		{
			const Ogre::Vector2& vert = data[i];
			*pReal++= vert.x;
			*pReal++= vert.y;
		}
		uvBuf->unlock();
		
	}
}

void COgreMeshCompiler::CreateMeshBounds( void )
{
	Ogre::Real width = Ogre::Math::Sqrt(m_MaxSquaredLength);
	m_pOgreMesh->_setBounds(m_AABB);
	m_pOgreMesh->_setBoundingSphereRadius(width);
}

void COgreMeshCompiler::PrintVertexDataToLog( void )
{
	Ogre::VertexData* vertexData = m_pOgreMesh->sharedVertexData;
	Ogre::VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;

	Ogre::Real* pReal;
	const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
	Ogre::HardwareVertexBufferSharedPtr vBuf = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
	unsigned char* vertex = static_cast<unsigned char*>(vBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

	for(size_t j = 0; j < vertexData->vertexCount; ++j, vertex += vBuf->getVertexSize())
	{
		posElem->baseVertexPointerToElement(vertex, &pReal);
		Ogre::StringUtil::StrStreamType str;
		str << "x: " << (*pReal++) << "\ty: " << (*pReal++) << "\tz:" << (*pReal++);
		Ogre::LogManager::getSingletonPtr()->logMessage( str.str());
	}
}

bool COgreMeshCompiler::WriteOgreMesh( const Ogre::String& sFilename )
{
	assert(Ogre::MeshManager::getSingletonPtr());

	Ogre::MeshSerializer* pMeshWriter = new Ogre::MeshSerializer();
	try
	{	
		pMeshWriter->exportMesh(m_pOgreMesh.get(), sFilename);
	}
	catch (Ogre::Exception& e)
	{
		MessageBox( NULL, e.getFullDescription().c_str(), "ERROR", MB_ICONERROR);
		return false;
	}
	delete pMeshWriter;

	if(m_bExportSkeleton)
		if( !m_pSkeletonCompiler->WriteOgreSkeleton( sFilename+".skeleton") )
			return false;

	return true;
}

Ogre::MeshPtr COgreMeshCompiler::GetOgreMesh( void )
{
	return m_pOgreMesh;
}
