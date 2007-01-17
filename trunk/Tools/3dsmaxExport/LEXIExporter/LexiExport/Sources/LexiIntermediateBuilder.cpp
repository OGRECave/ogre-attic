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
#include "LexiIntermediateAPI.h"

//

CIntermediateBuilder::CIntermediateBuilder()
{
	m_iBoneIndex = 0;
	m_fAnimTotalLength=0.0;
}

CIntermediateBuilder::~CIntermediateBuilder()
{
	for(std::map<Ogre::String, CIntermediateMaterial*>::iterator it = m_lMaterials.begin(); it != m_lMaterials.end(); it++)
	{
		CIntermediateMaterial* pMat = it->second;
		delete pMat;
	}
	m_lMaterials.clear();
}

//

void CIntermediateBuilder::Clear()
{
	m_lMaterials.clear();
}

void CIntermediateBuilder::SetConfig( CDDObject* pConfig )
{
	assert(pConfig);
	m_bExportSkeleton = pConfig->GetBool("SkeletonID", false);
	m_iAnimStart = pConfig->GetInt("AnimationStartID", 0);
	m_iAnimEnd = pConfig->GetInt("AnimationEndID", 1);
	m_fSampleRate = pConfig->GetFloat("AnimationSampleRateID", 1.0);
	m_sAnimationName = pConfig->GetString("AnimationNameID", "Anim1");

	// Read Animation Settings
	//m_AnimationSetting.m_lSettings.push_back(

	CDDObject* pAnimContainer = pConfig->GetDDObject("AnimationDataContainer");
	assert(pAnimContainer);

	m_AnimationSetting.m_lSettings.clear();

	fastvector<const CDDObject*> lAnimList = pAnimContainer->GetDDList("Animations");
	while(!lAnimList.empty())
	{
		const CDDObject* pCurAnim = lAnimList.pop_back();
		CAnimationSetting curAnimSetting;
		curAnimSetting.m_sAnimName = pCurAnim->GetString("AnimationNameID","NO_NAME");
		curAnimSetting.m_iStartFrame = pCurAnim->GetInt("AnimationStartID",0);
		curAnimSetting.m_iEndFrame = pCurAnim->GetInt("AnimationEndID",100);
		curAnimSetting.m_fSampleRate = pCurAnim->GetFloat("AnimationSampleRateID",1.0);
		curAnimSetting.m_bOptimize = pCurAnim->GetBool("AnimationOptimizeID",true);


		int iType = pCurAnim->GetInt("AnimationTypeID");
		std::string sType;
		switch(iType)
		{
		case 0:
			sType = "Bone";
			break;
		case 1:
			sType = "Morph";
			break;
		case 2:
			sType = "Pose";
			break;
		default:
			break;
		};
		curAnimSetting.m_sType = sType;

		m_AnimationSetting.m_lSettings.push_back(curAnimSetting);
	}
}


//

CIntermediateBuilder* CIntermediateBuilder::Get()
{
	static CIntermediateBuilder* pIB = new CIntermediateBuilder;
	return pIB;
}

//

static void TransformVectors(const Ogre::Matrix4& omat, Ogre::Vector3* pV, unsigned int iCount)
{
	CMatrix mat((const CMatrix&)omat);
	mat.transpose();
	mat.transformVectors((const CVec3*)pV, (CVec3*)pV, iCount);
}

static void TransformPoints(const Ogre::Matrix4& omat, Ogre::Vector3* pV, unsigned int iCount)
{
	CMatrix mat((const CMatrix&)omat);
	mat.transpose();
	mat.transformPoints((const CVec3*)pV, (CVec3*)pV, iCount);
}

//

static Matrix3 GetTransformFromNode(TimeValue time, INode* pNode)
{
	return pNode->GetNodeTM(time);
}

static bool TMNegParity(const Matrix3& m)
{
	return DotProd(CrossProd(m[0], m[1]), m[2]) < 0.0f ? true : false;
}

static void GetVertexNormal(Mesh* mesh, unsigned int faceNo, RVertex* rv, Ogre::Vector3& vOut)
{
	const Face* f = &mesh->faces[faceNo];
	unsigned int smGroup = f->smGroup;
	unsigned int numNormals;

	if(rv->rFlags & SPECIFIED_NORMAL)
	{
		vOut = (const Ogre::Vector3&)rv->rn.getNormal();
		return;
	}
	else if((numNormals = rv->rFlags & NORCT_MASK) && smGroup)
	{
		if(numNormals == 1)
		{
			vOut = (const Ogre::Vector3&)rv->rn.getNormal();
			return;
		}
		else
		{
			for(unsigned int x = 0; x < numNormals; x++)
			{
				if(rv->ern[x].getSmGroup() & smGroup)
				{
					vOut = (const Ogre::Vector3&)rv->ern[x].getNormal();
					return;
				}
			}
		}
	}
	else
	{
		vOut = (const Ogre::Vector3&)mesh->getFaceNormal(faceNo);
		return;
	}
}

//

CMeshArray* CIntermediateBuilder::BuildMeshArray(unsigned int iNodeID, const char* pszTypeName, TimeValue iTime)
{
	INode* pNode = GetNodeFromID(iNodeID);
	if(!pNode) return NULL;

	CMeshArray* pRtnArray = NULL;

	//

	ObjectState os = pNode->EvalWorldState(iTime, TRUE);
	Object* pObj = os.obj;
	if(!pObj) return NULL;

	TriObject* pTri = (TriObject*)pObj->ConvertToType(iTime, Class_ID(TRIOBJ_CLASS_ID, 0));
	bool bDeleteTri = (pObj != pTri) ? true : false;

	Mesh* pMesh = &pTri->GetMesh();

	const Matrix3& tm = pNode->GetObjTMAfterWSM(iTime);
	unsigned int wind[3] = { 0, 1, 2 };
	if(TMNegParity(tm)) { wind[0] = 2; wind[2] = 0; }

	unsigned int iNumTriangles = pMesh->getNumFaces();

	//

	unsigned int iUVIndex = 0;
	for(unsigned int x = 1; x <= 99; x++)
	{
		char temp[32];
		sprintf(temp, "uv%i", x);
		if(!strcmp(pszTypeName, temp))
		{
			iUVIndex = x;
			break;
		}
	}

	//

	if(!strcmp(pszTypeName, "position"))
	{
		CVec3Array* pArray = new CVec3Array(iNumTriangles * 3);
		pRtnArray = pArray;

		unsigned int iVIndex = 0;
		for(unsigned int x = 0; x < iNumTriangles; x++)
		{
			Face* face = &pMesh->faces[x];
			for(unsigned int y = 0; y < 3; y++)
			{
				const Point3& v = pMesh->verts[face->v[wind[y]]];
				(*pArray)[iVIndex++] = Ogre::Vector3(v.x, v.z, -v.y);
			}
		}
	}
	else if(!strcmp(pszTypeName, "normal"))
	{
		CVec3Array* pArray = new CVec3Array(iNumTriangles * 3);
		pRtnArray = pArray;

		pMesh->checkNormals(TRUE);
		Ogre::Vector3 vn;

		unsigned int iVIndex = 0;
		for(unsigned int x = 0; x < iNumTriangles; x++)
		{
			Face* face = &pMesh->faces[x];

			for(unsigned int y = 0; y < 3; y++)
			{
				GetVertexNormal(pMesh, x, pMesh->getRVertPtr(face->v[wind[y]]), vn);
				(*pArray)[iVIndex++] = Ogre::Vector3(vn.x, vn.z, -vn.y);
			}
		}
	}
	else if(!strcmp(pszTypeName, "diffuse") && pMesh->numCVerts)
	{
		CVec4Array* pArray = new CVec4Array(iNumTriangles * 3);
		pRtnArray = pArray;

		unsigned int iVIndex = 0;
		for(unsigned int x = 0; x < iNumTriangles; x++)
		{
			TVFace* face = &pMesh->vcFace[x];

			for(unsigned int y = 0; y < 3; y++)
			{
				const Point3& vc = pMesh->vertCol[face->t[wind[y]]];
				Ogre::Vector4& vColor = (*pArray)[iVIndex++];
				vColor.x = vc.x;
				vColor.y = vc.y;
				vColor.z = vc.z;
				vColor.w = 1.0f;
			}
		}
	}
	else if(iUVIndex > 0 && (pMesh->numMaps > iUVIndex))
	{
		MeshMap* pMeshMap = &pMesh->maps[iUVIndex];
		if(pMeshMap && pMeshMap->tv && pMeshMap->tf)
		{
			if(!(pMeshMap->GetFlag(MESHMAP_USED)) )
				return NULL;

			CVec2Array* pArray = new CVec2Array(iNumTriangles * 3);
			pRtnArray = pArray;

			unsigned int iVIndex = 0;
			for(unsigned int x = 0; x < iNumTriangles; x++)
			{
				TVFace* face = &pMeshMap->tf[x];

				for(unsigned int y = 0; y < 3; y++)
				{
					const Point3& tc = pMeshMap->tv[face->t[wind[y]]];
					Ogre::Vector2& vUV = (*pArray)[iVIndex++];
					vUV.x = tc.x;
					vUV.y = 1.0f - tc.y;
				}
			}
		}
	}

	//

	if(bDeleteTri) delete pTri;

	return pRtnArray;
}

//

Ogre::SceneNode* CIntermediateBuilder::CreateHierarchy(unsigned int iNodeID, bool bRecursive, bool bHidden)
{
	Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateHierarchy() - Main call..");
	return CreateHierarchy(iNodeID, NULL, bRecursive, bHidden);
}

Ogre::SceneNode* CIntermediateBuilder::CreateHierarchy(unsigned int iNodeID, Ogre::SceneNode* pParent, bool bRecursive, bool bHidden)
{
	Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateHierarchy() - recursive call..");
	INode* pRoot = GetNodeFromID(iNodeID);
	if(!pRoot) return NULL;

	Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateHierarchy() - Creating Intermediate Mesh");

	CIntermediateMesh* pIMesh = CreateMesh(iNodeID);
	if(!pIMesh) return NULL;

	Ogre::SceneNode* pSceneNode = new Ogre::SceneNode(NULL, pRoot->GetName());
	pSceneNode->attachObject(pIMesh);

	//

	Matrix3 mTransform = GetTransformFromNode(0, pRoot);

	INode* pRootParent = pRoot->GetParentNode();
	if(pRootParent)
	{
		Matrix3 mParentT = GetTransformFromNode(0, pRootParent);
		mParentT.Invert();
		mTransform = mTransform * mParentT;
	}

	Point3 pos;
	Point3 scale;
	Quat quat;
	DecomposeMatrix(mTransform, pos, quat, scale);
	Ogre::Quaternion oquat;
	oquat.x = quat.x;
	oquat.y = quat.z;
	oquat.z = -quat.y;
	oquat.w = quat.w;
	Ogre::Vector3 axis;
	Ogre::Radian w;
	oquat.ToAngleAxis(w, axis);
	oquat.FromAngleAxis(-w, axis);



	pSceneNode->setPosition(Ogre::Vector3(pos.x, pos.z, -pos.y));
	pSceneNode->setOrientation(oquat);
	pSceneNode->setScale(Ogre::Vector3(scale.x, scale.z, scale.y));

	//

	if(pParent) pParent->addChild(pSceneNode);

	if(bRecursive)
	{
		unsigned int iNumSubNodes = pRoot->NumberOfChildren();
		for(unsigned int x = 0; x < iNumSubNodes; x++)
		{
			INode* pSubNode = pRoot->GetChildNode(x);
			CreateHierarchy(pSubNode->GetHandle(), pSceneNode, true, bHidden);
		}
	}

	return pSceneNode;
}

CIntermediateMesh* CIntermediateBuilder::CreateMesh(unsigned int iNodeID)
{
	// Verify Node ID
	INode* pRoot = GetNodeFromID(iNodeID);
	if(!pRoot) return NULL;

	// Retrieve object on node
	ObjectState os = pRoot->EvalWorldState(0, TRUE);
	Object* pObj = os.obj;
	if(!pObj) return NULL;

	// We only use the Mesh objects
	if(pObj->ClassID() == Class_ID(TARGET_CLASS_ID, 0)) return NULL;
	if(pObj->SuperClassID() != GEOMOBJECT_CLASS_ID) return NULL;
	if(!pObj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) return NULL;

	// Convert object into triangle object (mesh data)
	TriObject* pTri = (TriObject*)pObj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));
	if(!pTri) return NULL;

	bool bDeleteTri = (pObj != pTri) ? true : false;

	Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateMesh() - Retrieving Mesh Data..");


	Mesh* pMesh = &pTri->GetMesh();
	if(!pMesh || !pMesh->getNumFaces())
	{
		if(bDeleteTri) delete pTri;
		return NULL;
	}

	Mtl* maxMtl = pRoot->GetMtl();
	if(maxMtl == NULL)
	{
		StdMat* pBlankMat = NewDefaultStdMat();
		
		pBlankMat->SetDiffuse( Color(1,1,1), 0);
		Ogre::String name = Ogre::String(pRoot->GetName()) + Ogre::StringConverter::toString(pRoot->GetHandle()) + Ogre::String("_material") ;
		pBlankMat->SetName( name.c_str() );
		maxMtl = (Mtl*)pBlankMat;
	}
	BOOL bMultiMat = maxMtl->IsMultiMtl();

	unsigned int iNumTriangles = pMesh->getNumFaces();
	CIntermediateMesh* pIMesh = new CIntermediateMesh(iNumTriangles, iNodeID);

	// build Skeleton if needed
	CIntermediateSkeleton* pISkel = NULL;
	Modifier* pSkinMod = NULL;
	ISkin	* pSkin = NULL;
	ISkinContextData *pSkinContext = NULL;

	//if(m_bExportSkeleton)
	Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateMesh() - Tjeck for bone animation.");
	if(!m_AnimationSetting.m_lSettings.empty())
	{
	Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateMesh() - Tjeck for skin modifier.");
		if(FindSkinModifier(pRoot, &pSkinMod, &pSkin, &pSkinContext))
		{
			Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateMesh() - Creating intermediate skeleton.");
			pISkel = new CIntermediateSkeleton();
			pIMesh->SetSkeleton(pISkel);

			CreateIntermediateBonePool(pISkel,pSkin);
			pISkel->PopulateBoneHandleMap();
			BuildSkeleton( pISkel );
			ConnectLinkedBones( pISkel );

			pISkel->BuildIndexedBoneList();

			SetBindingPose(pISkel, iNodeID, 0);
		}
	}

	//
	unsigned int iBoneIndex = 0;
	unsigned int iVIndex = 0;
	for(unsigned int x = 0; x < iNumTriangles; x++)
	{
		// Order the face. Due to flat shading where the 
		// first vertex in a face is used to color the entire face. 
		// So faces in the same polygon should share the same start vertex.
		unsigned int wind[3] = { 0, 1, 2 };
		if((x%2))
		{
			wind[0]=1;
			wind[1]=2; // should be 2,0,1 if we use OpenGL
			wind[2]=0;
		}

		CTriangle& tri = pIMesh->GetTriangle(x);
		tri.m_Vertices[wind[0]] = iVIndex++;
		tri.m_Vertices[wind[1]] = iVIndex++;
		tri.m_Vertices[wind[2]] = iVIndex++;

		Mtl* pMat = maxMtl;
		if (bMultiMat)
			pMat = maxMtl->GetSubMtl( pMesh->getFaceMtlIndex(x) );

		tri.m_pMaterial = CreateMaterial( pMat );

		if(pISkel != NULL)
		{
			// Record bones and weights (we fill a pool of bones and builds the hierarchy later)
			SVertexBoneData vd;
			for (int j=0; j<3; j++)
			{
				int idx = pMesh->faces[x].v[j]; //tri.m_Vertices[j];
				int nrBones = pSkinContext->GetNumAssignedBones(idx);
				for (int i=0; i < nrBones; i++)
				{
					int iBoneIdx = pSkinContext->GetAssignedBone(idx,i);
					INode* pMaxBone = pSkin->GetBone(iBoneIdx);
					if(pMaxBone)
					{
						CIntermediateBone* pIBone = pISkel->FindBone( pMaxBone->GetHandle() );

						vd.boneIndex = pIBone->GetIndex();
						vd.weight = pSkinContext->GetBoneWeight(idx, i);//iBoneIdx);

						//Ogre::StringUtil::StrStreamType strStream;
						//strStream << "Original Index:\t" << idx << "\tBone:\t" << iBoneIdx << "\tWeight:\t" << vd.weight;
						//Ogre::LogManager::getSingleton().logMessage(strStream.str());

						if(vd.weight < 0.01f) 
							continue;

						pISkel->SetVertexData( tri.m_Vertices[wind[j]], vd );
					}
				}
			}
		}
	}

	Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateMesh() - Building Material List..");

	pIMesh->BuildMaterialList();

	if(pISkel != NULL)
	{
		Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateMesh() - Adding frames for all animations..");
		for(int i=0; i < m_AnimationSetting.m_lSettings.size(); i++)
		{
			CAnimationSetting curAnim = m_AnimationSetting.m_lSettings[i];

			if(curAnim.m_sType.compare("Bone")==0)
			{
				Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateMesh() - Adding frame..");
				Ogre::LogManager::getSingleton().logMessage(curAnim.ToString());

				AddFrame( pISkel, iNodeID, curAnim );
			}
		}

		Ogre::LogManager::getSingleton().logMessage("CIntermediate::CreateMesh() - Normalizing Vertex Assignments..");

		pISkel->NormalizeVertexAssignments();
		pISkel->SetFPS( GetFrameRate() );
		//Ogre::LogManager::getSingletonPtr()->logMessage( pISkel->ToString() );
	}

	if(bDeleteTri) delete pTri;

	return pIMesh;
}

void CIntermediateBuilder::CreateIntermediateBonePool(CIntermediateSkeleton* pSkel, ISkin* pSkin)
{
	int iBoneCount = pSkin->GetNumBones();
	for (int i=0; i < iBoneCount; i++)
	{
		INode* pBone = pSkin->GetBone(i);
		CIntermediateBone* pIBone = new CIntermediateBone( pBone->GetName() );
		pIBone->SetHandle( pBone->GetHandle() );

		INode* pParent = pBone->GetParentNode();
		if(pParent)
			pIBone->SetParentHandle( pParent->GetHandle() );

		pIBone->SetIndex( m_iBoneIndex++ );

		pSkel->AddBone( pIBone, pIBone->GetName().c_str() );
		
	}
}

void CIntermediateBuilder::ConnectLinkedBones(CIntermediateSkeleton* pSkel)
{
	static int i = 0;

	std::vector<CIntermediateBone*>::const_iterator iter = pSkel->GetRootBones().begin();

	while(iter != pSkel->GetRootBones().end())
	{
		CIntermediateBone* pIBone = *iter;
		ULONG handle = pIBone->GetHandle();
		INode* pNode = GetNodeFromID(handle);

		INode* pParentNode = pNode->GetParentNode();
		if(pParentNode)
			if(pParentNode->SuperClassID() == HELPER_CLASS_ID)
			{
				CIntermediateBone* pParentIBone = pSkel->FindBone( pParentNode->GetParentNode()->GetHandle() );
				if(pParentIBone)
				{
					Ogre::StringUtil::StrStreamType strID;
					strID << "Connecting_Bone_" << i++ ;
					CIntermediateBone* pConnectBone = new CIntermediateBone(strID.str().c_str());

					pConnectBone->SetHandle( pParentNode->GetHandle());
					pConnectBone->SetIndex(m_iBoneIndex++);
					pConnectBone->AddBone( pIBone );
					pParentIBone->AddBone( pIBone );
				}
			}
		iter++;
	}
}

bool CIntermediateBuilder::FindSkinModifier(INode* nodePtr, Modifier** pSkinMod,ISkin** pSkin, ISkinContextData** pSkinContext)
{	
	// Get object from node. Abort if no object.
	Object* ObjectPtr = nodePtr->GetObjectRef();			

	if (!ObjectPtr) return false;

	// Is derived object ?
	while (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID && ObjectPtr)
	{
		// Yes -> Cast.
		IDerivedObject *DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);

		// Iterate over all entries of the modifier stack.
		int ModStackIndex = 0;
		while (ModStackIndex < DerivedObjectPtr->NumModifiers())
		{
			// Get current modifier.
			Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

			// Is this Physique ?
			if (ModifierPtr->ClassID() == SKIN_CLASSID)
			{
				// Yes -> Exit.
				*pSkinMod = ModifierPtr;
				*pSkin=(ISkin*)(*pSkinMod)->GetInterface(I_SKIN);

				//get the node's initial transformation matrix and store it in a matrix3
				Matrix3 initTM;
				int msg = (*pSkin)->GetBoneInitTM(nodePtr, initTM);

				//get a pointer to the export context interface
				*pSkinContext=(ISkinContextData*)(*pSkin)->GetContextInterface(nodePtr);
				return true;
			}

			// Next modifier stack entry.
			ModStackIndex++;
		}
		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	// Not found.
	return false;
}


void CIntermediateBuilder::Clamp( Point3& inVec, float threshold )
{
	if(inVec.x > 0.0)
		inVec.x < threshold ? inVec.x = 0 : inVec.x = inVec.x;
	else
		inVec.x > -threshold ? inVec.x = 0 : inVec.x = inVec.x;


	if(inVec.y > 0.0)
		inVec.y < threshold ? inVec.y = 0 : inVec.y = inVec.y;
	else
		inVec.y > -threshold ? inVec.y = 0 : inVec.y = inVec.y;


	if(inVec.z > 0.0)
		inVec.z < threshold ? inVec.z = 0 : inVec.z = inVec.z;
	else
		inVec.z > -threshold ? inVec.z = 0 : inVec.z = inVec.z;
}



void CIntermediateBuilder::Rotate90DegreesAroundX( Point3& inVec )
{
	// transform nodePos to OgreCoords
	float negY = -inVec.y;
	inVec.y = inVec.z;
	inVec.z = negY;
}



void CIntermediateBuilder::AddFrame( CIntermediateSkeleton* pSkel, unsigned int meshNodeID, CAnimationSetting animationSettings )
{
	for (int i=0; i < pSkel->GetBoneCount(); i++)
	{
		CAnimationSetting animSetting = animationSettings;

		CIntermediateBone* pIBone = pSkel->GetBone(i);
		INode* pMaxNode = GetNodeFromID(pIBone->GetHandle());
		if(!pMaxNode) return;

		INode* pParentBone = pMaxNode->GetParentNode();

		// binding pose:
		Ogre::Vector3 pos;
		Ogre::Vector3 scale;
		Ogre::Quaternion oriet;
		pIBone->GetBindingPose(pos,oriet,scale);

		Point3 basePos(pos.x, pos.y, pos.z);
		Point3 baseScale(scale.x, scale.y, scale.z);
		Quat baseOriet(oriet.x, oriet.y, oriet.z, oriet.w);

		int tpf = GetTicksPerFrame();
		float fps = 1.0 / float(GetFrameRate());

		Ogre::LogManager::getSingleton().logMessage("CIntermediate::AddFrame() - Creating Animation on bone..");
		//Ogre::LogManager::getSingleton().logMessage(animSetting.ToString());
		//Ogre::LogManager::getSingleton().logMessage("^AnimSettings");


		bool animCreated = pIBone->CreateAnimation(animSetting);

		if(animCreated)
			Ogre::LogManager::getSingleton().logMessage("CIntermediate::AddFrame() - Animation created..");
		else
			Ogre::LogManager::getSingleton().logMessage("CIntermediate::AddFrame() - Animation NOT created..");

		if(animCreated)
		{
			Ogre::LogManager::getSingleton().logMessage("CIntermediate::AddFrame() - Creating Frames.");
			for(unsigned int x = animSetting.m_iStartFrame; x <= animSetting.m_iEndFrame; x++)
			{
				TimeValue t = (x * animSetting.m_fSampleRate) * tpf;

				Matrix3 nodeTM = pMaxNode->GetNodeTM(t);
				Matrix3 parentTM = pParentBone->GetNodeTM(t);

				if( pParentBone->IsRootNode() )
				{
					INode* pMeshNode = GetNodeFromID(meshNodeID);
					parentTM = pMeshNode->GetObjTMAfterWSM(t);
				}

				Matrix3 relativeTM = nodeTM * Inverse(parentTM);

				Point3 nodePos;
				Point3 nodeScale;
				Quat nodeOriet;

				DecomposeMatrix(relativeTM, nodePos, nodeOriet, nodeScale);

				Ogre::Quaternion oquat;
				oquat.x = nodeOriet.x;
				oquat.y = nodeOriet.z;
				oquat.z = -nodeOriet.y;
				oquat.w = nodeOriet.w;

				Rotate90DegreesAroundX(nodePos);

				Ogre::Vector3 axis;
				Ogre::Radian w;
				oquat.ToAngleAxis(w, axis);
				oquat.FromAngleAxis(-w, axis);

				Point3 relPos = basePos;
				Point3 relScale = baseScale;
				Ogre::Quaternion relOriet = baseOriet;

				// relative transformation
				relPos = nodePos - basePos;
				relScale = (nodeScale + baseScale) / 2;
				relOriet = oriet.Inverse() * oquat;

				//Clamp(relPos, 0.0001);

				float time = (x- animSetting.m_iStartFrame)*fps;
				Ogre::LogManager::getSingleton().logMessage("CIntermediate::AddFrame() - Creating Single Frame.");
				pIBone->AddFrame( 
					animSetting.m_sAnimName,
					x - animSetting.m_iStartFrame, // frame nr relative to this animation
					time,
					Ogre::Vector3(relPos.x, relPos.z, -relPos.y),
					relOriet, 
					Ogre::Vector3(relScale.x, relScale.z, relScale.y)
					);

				Ogre::LogManager::getSingleton().logMessage("CIntermediate::AddFrame() - Single Frame Created.");

				if(m_fAnimTotalLength < time)
					m_fAnimTotalLength = time;
			}
		}
	}
	Ogre::LogManager::getSingleton().logMessage("CIntermediate::AddFrame() - End");
}

Ogre::String CIntermediateBuilder::GetAnimationName( void )
{
	return m_sAnimationName;
}

float CIntermediateBuilder::GetAnimationLength( void )
{
	return m_fAnimTotalLength;
}

void CIntermediateBuilder::SetBindingPose( CIntermediateSkeleton* pISkel, unsigned int meshNodeID, unsigned int frame )
{
	for (int i=0; i < pISkel->GetBoneCount(); i++)
	{
		CIntermediateBone* pIBone = pISkel->GetBone(i);
		INode* pMaxNode = GetNodeFromID(pIBone->GetHandle());
		if(!pMaxNode) return;

		// binding pose:
		INode* pParentBone = pMaxNode->GetParentNode();
		Matrix3 baseParentTM = pParentBone->GetNodeTM(frame);

		Matrix3 baseTM = pMaxNode->GetNodeTM(0);

		if( pParentBone->IsRootNode() )
		{
			INode* pMeshNode = GetNodeFromID(meshNodeID);
			Matrix3 meshTM = pMeshNode->GetObjTMAfterWSM(frame);
			baseTM = baseTM * Inverse(meshTM);
		}
		else
		{
			baseTM = baseTM * Inverse(baseParentTM); // binding pose initial local space transformation matrix
		}
		
		Point3 basePos;
		Point3 baseScale;
		Quat baseOriet;

		DecomposeMatrix(baseTM, basePos, baseOriet, baseScale);

		Ogre::Quaternion oquat;
		oquat.x = baseOriet.x;
		oquat.y = baseOriet.z;
		oquat.z = -baseOriet.y;
		oquat.w = baseOriet.w;

		Ogre::Vector3 axis;
		Ogre::Radian w;
		oquat.ToAngleAxis(w, axis);
		oquat.FromAngleAxis(-w, axis);

		Rotate90DegreesAroundX(basePos);

		pIBone->SetBindingPose(Ogre::Vector3(basePos.x, basePos.y, basePos.z), oquat, Ogre::Vector3(baseScale.x, baseScale.z, baseScale.y));
	}
}

void CIntermediateBuilder::BuildSkeleton( CIntermediateSkeleton* pSkel )
{
	m_iBoneIndex = 0;
	for (int i=0; i < pSkel->GetBoneCount(); i++)
	{
		CIntermediateBone* pIBone = pSkel->GetBone(i);
		INode* pMaxNode = GetNodeFromID(pIBone->GetHandle());
		if(!pMaxNode) return;

		ULONG parentHandle = pIBone->GetParentHandle();
		CIntermediateBone* parent = pSkel->FindBone( parentHandle );
		if(parent)
			parent->AddBone( pIBone );
		else
		{
			pSkel->MarkBoneAsRoot( pIBone );
		}
	}
}

Ogre::SceneNode* CIntermediateBuilder::CollapseHierarchy(Ogre::SceneNode* pHierarchy, const std::list<std::string>& Arrays, const char* pszName) const
{
	unsigned int iNumTriangles = 0;
	CountTriangles(pHierarchy, iNumTriangles);

	//
	CIntermediateMesh* pMesh = new CIntermediateMesh(iNumTriangles, 0);

	for(std::list<std::string>::const_iterator it = Arrays.begin(); it != Arrays.end(); it++)
	{
		const std::string& sArray = *it;
		pMesh->ForceCreateArray(sArray.c_str());
	}

	unsigned int iTriangleOffset = 0;
	unsigned int iVertexOffset = 0;
	CollapseHierarchy(pMesh, iTriangleOffset, iVertexOffset, pHierarchy, Arrays, Ogre::Matrix4::IDENTITY);

	//
	Ogre::SceneNode* pSceneNode = new Ogre::SceneNode(NULL, pszName);
	pSceneNode->attachObject(pMesh);

	return pSceneNode;
}

void CIntermediateBuilder::CollapseHierarchy(CIntermediateMesh* pMesh, unsigned int& iTriangleOffset, unsigned int& iVertexOffset, Ogre::SceneNode* pNode, const std::list<std::string>& Arrays, const Ogre::Matrix4& mat) const
{
	CIntermediateMesh* pNodeMesh = (CIntermediateMesh*)pNode->getAttachedObject(0);

	unsigned int iMatCount = pNodeMesh->m_Materials.size();
	for(unsigned int x = 0; x < iMatCount; x++)
	{
		CIntermediateMaterial* pMat = pNodeMesh->m_Materials[x];
		bool bMatExists = false;

		unsigned int iMC = pMesh->m_Materials.size();
		for(unsigned int y = 0; y < iMC; y++)
		{
			if(pMesh->m_Materials[y] == pMat)
			{
				bMatExists = true;
				break;
			}
		}

		if(!bMatExists) pMesh->m_Materials.push_back(pMat);
	}

	Ogre::Matrix4 om;
	om.makeTransform(pNode->getPosition(), pNode->getScale(), pNode->getOrientation());
	Ogre::Matrix4 nmat = mat * om;

	const CTriangleArray& SourceTriangles = pNodeMesh->GetTriangles();
	CTriangleArray& TargetTriangles = pMesh->GetTriangles();

	unsigned int iNumTriangles = SourceTriangles.Size();

	for(std::list<std::string>::const_iterator it = Arrays.begin(); it != Arrays.end(); it++)
	{
		const std::string& sArray = *it;

		const CMeshArray* pSourceArray = pNodeMesh->GetArray(sArray.c_str(), 0);
		if(!pSourceArray) continue;

		CMeshArray* pTargetArray = pMesh->GetArray(sArray.c_str(), 0);
		if(!pTargetArray) continue;

		unsigned int iElemSize = pSourceArray->ElementSize();
		unsigned int iVO = iVertexOffset;

		unsigned char* pTargetBuffer = (unsigned char*)pTargetArray->Data();
		pTargetBuffer += (iVertexOffset * iElemSize);

		for(unsigned int x = 0; x < iNumTriangles; x++)
		{
			const CTriangle& t = SourceTriangles[x];
			for(unsigned int y = 0; y < 3; y++)
			{
				memcpy(pTargetBuffer, pSourceArray->Data(t.m_Vertices[y]), iElemSize);
				pTargetBuffer += iElemSize;
			}
		}

		if(sArray == "position")
		{
			TransformPoints(nmat, (Ogre::Vector3*)pTargetArray->Data(iVertexOffset), iNumTriangles * 3);
		}
		else if(sArray == "normal")
		{
			TransformVectors(nmat, (Ogre::Vector3*)pTargetArray->Data(iVertexOffset), iNumTriangles * 3);
		}
	}

	//

	for(unsigned int x = 0; x < iNumTriangles; x++)
	{
		const CTriangle& st = SourceTriangles[x];
		CTriangle& tt = TargetTriangles[iTriangleOffset++];

		tt.m_Vertices[0] = iVertexOffset++;
		tt.m_Vertices[1] = iVertexOffset++;
		tt.m_Vertices[2] = iVertexOffset++;
		tt.m_pMaterial = st.m_pMaterial;
	}

	//

	unsigned int iNumNodes = pNode->numChildren();
	for(unsigned int x = 0; x < iNumNodes; x++)
	{
		Ogre::SceneNode* pSubNode = (Ogre::SceneNode*)pNode->getChild(x);
		CollapseHierarchy(pMesh, iTriangleOffset, iVertexOffset, pSubNode, Arrays, nmat);
	}
}

void CIntermediateBuilder::CountTriangles(Ogre::SceneNode* pNode, unsigned int& iNumTriangles) const
{
	CIntermediateMesh* pMesh = (CIntermediateMesh*)pNode->getAttachedObject(0);
	if(pMesh) iNumTriangles += pMesh->GetNumTriangles();

	unsigned int iNumNodes = pNode->numChildren();
	for(unsigned int x = 0; x < iNumNodes; x++)
	{
		Ogre::SceneNode* pSubNode = (Ogre::SceneNode*)pNode->getChild(x);
		CountTriangles(pSubNode, iNumTriangles);
	}
}

CIntermediateMaterial* CIntermediateBuilder::CreateMaterial( Mtl* pMaxMaterial )
{
	// Tjeck to see if it's a Standard material
	if (pMaxMaterial->ClassID() != Class_ID(DMTL_CLASS_ID, 0))
	{
		Ogre::String str("Material ("+Ogre::String(+pMaxMaterial->GetName())+") Not Standard!");
		//MessageBox(NULL,str.c_str(),"ERROR", MB_ICONERROR);
		Ogre::LogManager::getSingletonPtr()->logMessage(str);
		return NULL;
	}

	Ogre::String matName = pMaxMaterial->GetName();
	CIntermediateMaterial* newMat = NULL;

	std::map<Ogre::String, CIntermediateMaterial*>::iterator it = m_lMaterials.find(matName);
	if (it == m_lMaterials.end())
	{
		newMat = new CIntermediateMaterial( matName );

		Color col = pMaxMaterial->GetAmbient();
		newMat->SetAmbientColor( col.r, col.g, col.b );

		col = pMaxMaterial->GetDiffuse();
		newMat->SetDiffuseColor( col.r, col.g, col.b );

		col = pMaxMaterial->GetSpecular();
		newMat->SetSpecularColor( col.r, col.g, col.b );

		col = pMaxMaterial->GetSelfIllumColor();
		newMat->SetEmissiveColor( col.r, col.g, col.b );

		newMat->SetGlosiness( pMaxMaterial->GetShininess() );
		newMat->SetSpecularLevel( pMaxMaterial->GetShinStr() );

		// Check to see if it's a Standard material
		if (pMaxMaterial->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
		{
			StdMat* std = (StdMat *)pMaxMaterial;
			newMat->Set2Sided( std->GetTwoSided() ? true : false );
			newMat->SetWired( std->GetWire() ? true : false );

			//if (std->ClassID() == Class_ID(DMTL2_CLASS_ID, 0))
				newMat->SetFaceted( ((StdMat2 *)std)->IsFaceted() ? true : false );

		}

		RegisterMaps(newMat, pMaxMaterial);

		m_lMaterials.insert( std::pair<Ogre::String, CIntermediateMaterial*>(matName, newMat) );
	}
	else
		newMat = it->second;

	return newMat;
}

bool CIntermediateBuilder::GetMaterials( std::map<Ogre::String, CIntermediateMaterial*>& materialMap ) const
{
	if(m_lMaterials.size() == 0)
		return false;

	materialMap = m_lMaterials;
	return true;
}

void CIntermediateBuilder::RegisterMaps( CIntermediateMaterial* pIMat, Mtl* pMaxMaterial ) 
{
	CIntermediateMaterial* newMat = pIMat;

	Ogre::String identifiers[12] = {
		"ambient_color",
		"diffuse",
		"specular_color",
		"specular_level",
		"glossiness",
		"self_illumination",
		"opacity",
		"filter_color",
		"bump",
		"reflection",
		"refraction",
		"displacement"
	};

	short& mask = pIMat->GetMask();

	Texmap *tmap = NULL;
	for ( int i=0; i <= 11; i++)
	{
		tmap = pMaxMaterial->GetSubTexmap(i);
		if(tmap == NULL)
			continue;
		if(!((StdMat*)pMaxMaterial)->MapEnabled(i))
			continue;

		if(tmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))
		{
			BitmapTex* pBmt = ((BitmapTex*)tmap);
			Ogre::String sMapName = pBmt->GetMapName();

			STextureMapInfo texInfo;
			texInfo.m_sFilename = sMapName;
			texInfo.m_sMapType = identifiers[i];

			if( ((BitmapTex*)tmap)->GetUVWSource()	 == UVWSRC_EXPLICIT )
				texInfo.m_iCoordSet = pBmt->GetMapChannel();
			else
			{
				texInfo.m_iCoordSet = 1;
				// WRITE A WARNING
			}

			StdUVGen* uv = pBmt->GetUVGen();
			int uvflags = uv->GetTextureTiling();				

			if( ((uvflags & U_WRAP) != 0) || ((uvflags & V_WRAP)!=0) )
				texInfo.m_AdressingMode = Ogre::TextureUnitState::TAM_WRAP;
			else if( !(uvflags & U_MIRROR) || !(uvflags & U_MIRROR) )
				texInfo.m_AdressingMode = Ogre::TextureUnitState::TAM_MIRROR;
			else
				texInfo.m_AdressingMode = Ogre::TextureUnitState::TAM_CLAMP;

			// Blend operation is not determined here, but in Ogre-material compiler.

			texInfo.m_fOffset[0] = uv->GetUOffs(0);
			texInfo.m_fOffset[1] = uv->GetVOffs(0);

			texInfo.m_fScale[0] = uv->GetUScl(0);
			texInfo.m_fScale[1] = uv->GetVScl(0);

			texInfo.m_fAngle = uv->GetAng(0);

			texInfo.m_fAmount = ((StdMat*)pMaxMaterial)->GetTexmapAmt(i,0);

			if(pBmt->GetAlphaSource() == ALPHA_FILE)
				texInfo.m_bAlpha = true;

			newMat->AddTextureMap(identifiers[i], texInfo);

			mask = mask | 1<<i;
 		}
	}
}
