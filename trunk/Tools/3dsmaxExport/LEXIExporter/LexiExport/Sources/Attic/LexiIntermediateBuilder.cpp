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
#include <iInstanceMgr.h>

//

static std::map<Ogre::SceneNode*, unsigned int> g_OgreMaxLink;

//

CIntermediateBuilder::CIntermediateBuilder()
{
	REGISTER_MODULE("Intermediate Builder")

	m_iBoneIndex = 0;
	m_fAnimTotalLength=0.0;
	m_pSkeletonBuilder = NULL;
}

CIntermediateBuilder::~CIntermediateBuilder()
{
	for(std::map<Ogre::String, CIntermediateMaterial*>::iterator it = m_lMaterials.begin(); it != m_lMaterials.end(); it++)
	{
		CIntermediateMaterial* pMat = it->second;
		delete pMat;
	}
	m_lMaterials.clear();

	delete m_pSkeletonBuilder;

	UNREGISTER_MODULE
}

//

void CIntermediateBuilder::Clear()
{
	for(std::map<Ogre::String, CIntermediateMaterial*>::iterator it = m_lMaterials.begin(); it != m_lMaterials.end(); it++)
	{
		CIntermediateMaterial* pMat = it->second;
		delete pMat;
	}
	m_lMaterials.clear();

	delete m_pSkeletonBuilder;
	m_pSkeletonBuilder = NULL;
}

void CIntermediateBuilder::SetConfig( CDDObject* pConfig )
{
	assert(pConfig);
	m_bExportSkeleton = pConfig->GetBool("SkeletonID", false);
	m_iAnimStart = pConfig->GetInt("AnimationStartID", 0);
	m_iAnimEnd = pConfig->GetInt("AnimationEndID", 1);
	m_fSampleRate = pConfig->GetFloat("AnimationSampleRateID", 1.0);
	m_sAnimationName = pConfig->GetString("AnimationNameID", "Anim1");

	m_iBindingPoseFrame = pConfig->GetInt("SkeletonBindingPoseFrameID", 0);

	// Read Animation Settings

	//CDDObject* pAnimContainer = pConfig->GetDDObject("AnimationDataContainer");
	//assert(pAnimContainer);

	//m_AnimationSetting.m_lSettings.clear();

	//fastvector<const CDDObject*> lAnimList = pAnimContainer->GetDDList("Animations");
	//while(!lAnimList.empty())
	//{
	//	const CDDObject* pCurAnim = lAnimList.pop_back();
	//	CAnimationSetting curAnimSetting;
	//	curAnimSetting.m_sAnimName = pCurAnim->GetString("AnimationNameID","NO_NAME");
	//	curAnimSetting.m_iStartFrame = pCurAnim->GetInt("AnimationStartID",0);
	//	curAnimSetting.m_iEndFrame = pCurAnim->GetInt("AnimationEndID",100);
	//	curAnimSetting.m_fSampleRate = pCurAnim->GetFloat("AnimationSampleRateID",1.0);
	//	curAnimSetting.m_bOptimize = pCurAnim->GetBool("AnimationOptimizeID",true);


	//	int iType = pCurAnim->GetInt("AnimationTypeID");
	//	std::string sType;
	//	switch(iType)
	//	{
	//	case 0:
	//		sType = "Bone";
	//		break;
	//	case 1:
	//		sType = "Morph";
	//		break;
	//	case 2:
	//		sType = "Pose";
	//		break;
	//	default:
	//		break;
	//	};
	//	curAnimSetting.m_sType = sType;

	//	m_AnimationSetting.m_lSettings.push_back(curAnimSetting);
	//}
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

	// Build object offset transform (so we can export vertices/normals relative to pivot point)
	Matrix3 objOffsetXform(1);
	Point3 pos = pNode->GetObjOffsetPos();
	objOffsetXform.PreTranslate(pos);
	Quat quat = pNode->GetObjOffsetRot();
	PreRotateMatrix(objOffsetXform, quat);
	ScaleValue scaleValue = pNode->GetObjOffsetScale();
	ApplyScaling(objOffsetXform, scaleValue);

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
			{ // transformed by object offset
				const Point3& v = pMesh->verts[face->v[wind[y]]] * objOffsetXform;
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
				vn = (Ogre::Vector3)(*((Point3*)&vn) * objOffsetXform); // transform by object offset
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
				// Since Max seems to store the colors in reverse (BGR) order we reveses it again into RGB:
				vColor.x = vc.z;
				vColor.y = vc.y;
				vColor.z = vc.x;
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
	LOGDEBUG "CIntermediate::CreateHierarchy() - Main call..");

	Ogre::SceneNode *pSceneNode = CreateHierarchy(iNodeID, NULL, bRecursive, bHidden);

	// Set the base instance object of each object.
	// This is so any objects that are instances of the same Max object can all refer to one object as the "base".
	std::map<unsigned int,CIntermediateObject*>::iterator it;
	for (it = m_mapIObjects.begin(); it != m_mapIObjects.end(); ++it)
	{
		CIntermediateObject *pObject = it->second;
		std::set<unsigned int> *pIIDs = pObject->GetInstanceIDs();
		if (pIIDs->size() > 1)
		{ // choose lowest node ID as base instance
			unsigned int minID = INT_MAX;
			std::set<unsigned int>::iterator itID;
			for (itID = pIIDs->begin(); itID != pIIDs->end(); ++itID)
				if (*itID < minID) minID = *itID;
			pObject->SetBaseInstanceObject(GetObjectByNodeID(minID));
		}
	}

	return pSceneNode;
}

Ogre::SceneNode* CIntermediateBuilder::CreateHierarchy(unsigned int iNodeID, Ogre::SceneNode* pParent, bool bRecursive, bool bHidden)
{
	LOGDEBUG "CIntermediate::CreateHierarchy() - recursive call (MaxNode: %i)..", iNodeID);
	INode* pRoot = GetNodeFromID(iNodeID);
	if(!pRoot)
	{
		LOGERROR "No node with such ID: %d", iNodeID);
		return NULL;
	}

	LOGDEBUG "CIntermediate::CreateHierarchy() - Creating Scenenode");
	Ogre::SceneNode* pSceneNode = new Ogre::SceneNode(NULL, pRoot->GetName());
	g_OgreMaxLink[pSceneNode] = iNodeID;

	LOGDEBUG "CIntermediate::CreateHierarchy() - Checking for mesh");
	CIntermediateMesh* pIMesh = CreateMesh(iNodeID);
	if(pIMesh)
	{
		pSceneNode->attachObject(pIMesh);
		m_mapIObjects[iNodeID] = pIMesh;
	}

	CIntermediateLight* pILight = CreateLight(iNodeID);
	if(pILight)
	{
		pSceneNode->attachObject(pILight);
		m_mapIObjects[iNodeID] = pILight;
	}

	CIntermediateCamera* pICamera = CreateCamera(iNodeID);
	if (pICamera)
	{
		pSceneNode->attachObject(pICamera);
		m_mapIObjects[iNodeID] = pICamera;
	}

	//
	LOGDEBUG "CIntermediate::CreateHierarchy() - Getting transformation");

	Matrix3 mTransform = GetTransformFromNode(0, pRoot);

	INode* pRootParent = pRoot->GetParentNode();
	if(pRootParent)
	{
		Matrix3 mParentT = GetTransformFromNode(0, pRootParent);
		mParentT.Invert();
		mTransform = mTransform * mParentT;
	}

	LOGDEBUG "CIntermediate::CreateHierarchy() - Decomposing");

	// Retrieve orientation and transform into proper coord system
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

	//CIntermediateBuilder::Rotate90DegreesAroundX(quat);
	//Ogre::Quaternion oquat;
	//oquat.x = quat.x;
	//oquat.y = quat.y;
	//oquat.z = quat.z;
	//oquat.w = quat.w;

	pSceneNode->setOrientation(oquat);

	// Transform the position into proper coord system
	Rotate90DegreesAroundX(pos);
	pSceneNode->setPosition(Ogre::Vector3(pos.x, pos.y, pos.z));

	// Scale
	pSceneNode->setScale(Ogre::Vector3(scale.x, scale.z, scale.y));

	//
	LOGDEBUG "CIntermediate::CreateHierarchy() - Add to parent");

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

//

Ogre::SceneNode* CIntermediateBuilder::CreateNodeHierarchy(unsigned int iNodeID, bool bHidden)
{
	g_OgreMaxLink.clear();
	return CreateNodeHierarchy(iNodeID, NULL, bHidden);
}

Ogre::SceneNode* CIntermediateBuilder::CreateNodeHierarchy(unsigned int iNodeID, Ogre::SceneNode* pParent, bool bHidden)
{
	INode* pRoot = GetNodeFromID(iNodeID);
	if(!pRoot) return NULL;

	Ogre::SceneNode* pSceneNode = new Ogre::SceneNode(NULL, pRoot->GetName());
	g_OgreMaxLink[pSceneNode] = iNodeID;

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

	pSceneNode->setOrientation(oquat);

	Rotate90DegreesAroundX(pos);
	pSceneNode->setPosition(Ogre::Vector3(pos.x, pos.y, pos.z));

	pSceneNode->setScale(Ogre::Vector3(scale.x, scale.z, scale.y));

	//

	if(pParent) pParent->addChild(pSceneNode);

	unsigned int iNumSubNodes = pRoot->NumberOfChildren();
	for(unsigned int x = 0; x < iNumSubNodes; x++)
	{
		INode* pSubNode = pRoot->GetChildNode(x);
		CreateNodeHierarchy(pSubNode->GetHandle(), pSceneNode, bHidden);
	}

	return pSceneNode;
}

void CIntermediateBuilder::UpdateNodeHierarchy(TimeValue iTime, Ogre::SceneNode* pSceneNode)
{
	unsigned int iNodeID = g_OgreMaxLink[pSceneNode];
	INode* pRoot = GetNodeFromID(iNodeID);

	Matrix3 mTransform = GetTransformFromNode(iTime, pRoot);

	INode* pRootParent = pRoot->GetParentNode();
	if(pRootParent)
	{
		Matrix3 mParentT = GetTransformFromNode(iTime, pRootParent);
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

	pSceneNode->setOrientation(oquat);
	Rotate90DegreesAroundX(pos);
	pSceneNode->setPosition(Ogre::Vector3(pos.x, pos.y, pos.z));
	pSceneNode->setScale(Ogre::Vector3(scale.x, scale.z, scale.y));

	//

	for(Ogre::Node::ChildNodeIterator it = pSceneNode->getChildIterator(); it.hasMoreElements(); )
	{
		Ogre::SceneNode* pN = (Ogre::SceneNode*)it.getNext();
		UpdateNodeHierarchy(iTime, pN);
	}
}

void CIntermediateBuilder::CleanupNodeHierarchy(Ogre::SceneNode* pNode)
{
	std::vector<Ogre::SceneNode*> lDeleteTheseNodes;

	Ogre::Node::ChildNodeIterator it = pNode->getChildIterator();
	while(it.hasMoreElements())
	{
		Ogre::SceneNode* pChild = (Ogre::SceneNode*)it.peekNextValue();
		CleanupNodeHierarchy(pChild);
		lDeleteTheseNodes.push_back(pChild);
		it.moveNext();
	}

	for(unsigned int x = 0; x < lDeleteTheseNodes.size(); x++)
	{
		Ogre::SceneNode* pChild = lDeleteTheseNodes[x];
		pNode->removeChild(pChild);
		delete pChild;
	}
	lDeleteTheseNodes.clear();

	g_OgreMaxLink.clear();
}

//

void CIntermediateBuilder::CleanUpHierarchy( Ogre::SceneNode* pNode )
{
	if(pNode == NULL)
	{
		return;
	}

	std::vector<Ogre::MovableObject*> lDeleteThese;

	Ogre::SceneNode::ObjectIterator it = pNode->getAttachedObjectIterator();
	while(it.hasMoreElements())
	{
		Ogre::MovableObject* pObj = it.peekNextValue();
		lDeleteThese.push_back(pObj);
		it.moveNext();
	}
	for(int i=0; i < lDeleteThese.size(); i++)
	{
		pNode->detachObject(lDeleteThese[i]);
		delete lDeleteThese[i];
	}

	std::vector<Ogre::SceneNode*> lDeleteTheseNodes;

	Ogre::Node::ChildNodeIterator it2 = pNode->getChildIterator();
	while(it2.hasMoreElements())
	{
		Ogre::SceneNode* pChild = (Ogre::SceneNode*)it2.peekNextValue();
		CleanUpHierarchy(pChild);
		lDeleteTheseNodes.push_back(pChild);
		it2.moveNext();
	}
	for(int i=0; i < lDeleteTheseNodes.size(); i++)
	{
		pNode->removeChild(lDeleteTheseNodes[i]);
		delete lDeleteTheseNodes[i];
	}

	m_mapIObjects.clear();
}

CIntermediateLight *CIntermediateBuilder::CreateLight(unsigned int iNodeID)
{
	CIntermediateLight *pILight = NULL;

	LOGDEBUG "Verify Node ID");

	// Verify Node ID
	INode* pRoot = GetNodeFromID(iNodeID);
	if(!pRoot) return NULL;

	LOGDEBUG "Get object on node");

	// Retrieve object on node
	ObjectState os = pRoot->EvalWorldState(0, TRUE);
	Object* pObj = os.obj;
	if(!pObj) return NULL;

	LOGDEBUG "Validate");

	// We only use the Light objects
	if(pObj->ClassID() == Class_ID(TARGET_CLASS_ID, 0)) return NULL;
	if(pObj->SuperClassID() != LIGHT_CLASS_ID) return NULL;

	LightState ls;
	((LightObject*)pObj)->EvalLightState(0, Interval(0,0), &ls);
	if (ls.type == AMBIENT_LGT)
		return NULL;

	pILight = new CIntermediateLight(iNodeID);
	SetInstanceIDs(pILight);

	switch(ls.type)
	{
		case OMNI_LGT:
			pILight->SetValue("type", "point");
			break;

		case SPOT_LGT:
			pILight->SetValue("type", "spot");
			break;

		case DIRECT_LGT:
			pILight->SetValue("type", "directional");
			break;
	}

	pILight->SetValue("visible", ls.on != 0);

	pILight->SetValue("castShadows", ls.shadow != 0);

	Ogre::ColourValue col = Ogre::ColourValue(ls.color.r, ls.color.g, ls.color.b) * ls.intens;
	pILight->SetValue("colourDiffuse", ls.affectDiffuse ? col : Ogre::ColourValue::Black);
	pILight->SetValue("colourSpecular", ls.affectSpecular ? col : Ogre::ColourValue::Black);

	if (ls.type == SPOT_LGT)
	{
		pILight->SetValue("lightRange.inner", Ogre::Math::DegreesToRadians(ls.hotsize));
		pILight->SetValue("lightRange.outer", Ogre::Math::DegreesToRadians(ls.fallsize));
		pILight->SetValue("lightRange.falloff", 1.0f);
	}

	if (ls.type == SPOT_LGT || ls.type == DIRECT_LGT)
	{ // Max lights look along -z, which is -y in Ogre
		pILight->SetValue("normal", -Ogre::Vector3::UNIT_Y);
	}

	INode* pTarget = pRoot->GetTarget();
	if(pTarget)
	{
		pILight->SetValue("trackTarget.nodeName", pTarget->GetName());
	}

	// TODO: lightAttenuation: range, constant, linear, quadratic

	return pILight;
}

CIntermediateCamera *CIntermediateBuilder::CreateCamera(unsigned int iNodeID)
{
	CIntermediateCamera* pICamera = NULL;

	LOGDEBUG "Verify Node ID");

	// Verify Node ID
	INode* pRoot = GetNodeFromID(iNodeID);
	if(!pRoot) return NULL;

	LOGDEBUG "Get object on node");

	// Retrieve object on node
	ObjectState os = pRoot->EvalWorldState(0, TRUE);
	Object* pObj = os.obj;
	if(!pObj) return NULL;

	LOGDEBUG "Validate");

	// We only use the Camera objects
	if(pObj->ClassID() == Class_ID(TARGET_CLASS_ID, 0)) return NULL;
	if(pObj->SuperClassID() != CAMERA_CLASS_ID) return NULL;

	CameraState cs;
	((CameraObject*)pObj)->EvalCameraState(0, Interval(0,0), &cs);

	pICamera = new CIntermediateCamera(iNodeID);
	SetInstanceIDs(pICamera);

	pICamera->SetValue("fov", cs.fov * 57.295779513f);

	pICamera->SetValue("projectionType", cs.isOrtho ? "orthographic" : "perspective");

	pICamera->SetValue("clipping.near", cs.hither);
	pICamera->SetValue("clipping.far", cs.yon);

	INode* pTarget = pRoot->GetTarget();
	if(pTarget)
	{
		pICamera->SetValue("trackTarget.nodeName", pTarget->GetName());
	}

	// to compensate for Ogre cameras looking along a different axis than Max cameras
	pICamera->SetValue("rotation", Ogre::Quaternion(-Ogre::Radian(Ogre::Math::HALF_PI), Ogre::Vector3::UNIT_X));

	return pICamera;
}

CIntermediateMesh* CIntermediateBuilder::CreateMesh(unsigned int iNodeID)
{
	LOGDEBUG "Verify Node ID");

	// Verify Node ID
	INode* pRoot = GetNodeFromID(iNodeID);
	if(!pRoot) return NULL;

	LOGDEBUG "Get object on node");

	// Retrieve object on node
	ObjectState os = pRoot->EvalWorldState(0, TRUE);
	Object* pObj = os.obj;
	if(!pObj) return NULL;

	LOGDEBUG "Validate");

	// We only use the Mesh objects
	if(pObj->ClassID() == Class_ID(TARGET_CLASS_ID, 0)) return NULL;
	if(pObj->SuperClassID() != GEOMOBJECT_CLASS_ID) return NULL;
	if(!pObj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) return NULL;

	LOGDEBUG "->TRIOBJ_CLASS");

	// Convert object into triangle object (mesh data)
	TriObject* pTri = (TriObject*)pObj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));
	if(!pTri) return NULL;

	bool bDeleteTri = (pObj != pTri) ? true : false;

	LOGDEBUG "Get mesh");
	Mesh* pMesh = &pTri->GetMesh();
	if(!pMesh || !pMesh->getNumFaces())
	{
		if(bDeleteTri) delete pTri;
		return NULL;
	}

	// CALL ON_CREATE_INTERMEDIATE_MESH_START()
	
	LOGDEBUG "Get material");
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

	LOGDEBUG "Construct intermediate mesh");
	unsigned int iNumTriangles = pMesh->getNumFaces();
	CIntermediateMesh* pIMesh = new CIntermediateMesh(iNumTriangles, iNodeID);
	SetInstanceIDs(pIMesh);

	pIMesh->SetValue("castShadows", pRoot->CastShadows() ? "true" : "false");

	if(m_bExportSkeleton)
	{
		LOGDEBUG "Construct intermediate skeletonbuilder");
		if(m_pSkeletonBuilder == NULL)
			m_pSkeletonBuilder = new CIntermediateBuilderSkeleton();

		LOGDEBUG "Build skeleton");
		m_pSkeletonBuilder->BuildIntermediateSkeleton(pRoot);

		LOGDEBUG "Set skeleton");
		pIMesh->SetSkeleton( m_pSkeletonBuilder->GetSkeleton() );
	}

	LOGDEBUG "Iterate triangles");
	//
	const Matrix3& tm = pRoot->GetObjTMAfterWSM(0);
	unsigned int wind[3] = { 0, 1, 2 };
	if(TMNegParity(tm)) { wind[0] = 2; wind[2] = 0; }

	unsigned int iBoneIndex = 0;
	unsigned int iVIndex = 0;
	for(unsigned int x = 0; x < iNumTriangles; x++)
	{
		// Order the face. Due to flat shading where the 
		// first vertex in a face is used to color the entire face. 
		// So faces in the same polygon should share the same start vertex.
		//unsigned int wind[3] = { 0, 1, 2 };
		//if((x%2))
		//{
		//	wind[0]=1;
		//	wind[1]=2; // should be 2,0,1 if we use OpenGL
		//	wind[2]=0;
		//}

		// Create a flat indexing to the verticies.

		CTriangle& tri = pIMesh->GetTriangle(x);
		tri.m_Vertices[wind[0]] = iVIndex++;
		tri.m_Vertices[wind[1]] = iVIndex++;
		tri.m_Vertices[wind[2]] = iVIndex++;

		

		Mtl* pMat = maxMtl;
		if (bMultiMat)
		{
			int subCount = maxMtl->NumSubMtls();
			MtlID matID = pMesh->getFaceMtlIndex(x) %subCount;
			pMat = maxMtl->GetSubMtl( matID );
			if(pMat == NULL)
			{
				char buf[256];
				sprintf (buf, "Mesh \"%s\" references submaterial ID %d.  No such submaterial in Material \"%s\".",
						pRoot->GetName(), matID, maxMtl->GetName());
				LOGERROR buf);
				return NULL;
			}
		}

		tri.m_pMaterial = CreateMaterial( pMat );

		// CALL ON_CREATE_INTERMEDIATE_MESH_TRIANGLE_LOOP(pMesh, idx, tri.m_Vertices[wind[j]], x)

		//if(m_pSkeletonBuilder != NULL)
		if(m_bExportSkeleton)
		{
			for (int j=0; j<3; j++)
			{
				// get current index from max mesh
				int idx = pMesh->faces[x].v[j]; 
				m_pSkeletonBuilder->SetVertexData( idx, tri.m_Vertices[wind[j]] );
			}
		}
	}

	if(m_bExportSkeleton)
	{
		LOGDEBUG "Finalizing intermediate skeleton");
		CIntermediateBuilder::Get()->GetSkeletonBuilder()->Finalize();
	}

	LOGDEBUG "CIntermediate::CreateMesh() - Building Material List..");

	pIMesh->BuildMaterialList();
//	pIMesh->BuildSubmeshIndexMaps();


	// CALL ON_CREATE_INTERMEDIATE_MESH_END()
	LOGDEBUG "Delete pTri");

	if(bDeleteTri) delete pTri;

	return pIMesh;
}

// Let this object know of any nodes it shares an instance with
void CIntermediateBuilder::SetInstanceIDs(CIntermediateObject *pIObject)
{
	INode *pNode = GetNodeFromID(pIObject->GetNodeID());
	if (!pNode) return;

	ObjectState os = pNode->EvalWorldState(0, TRUE);
	Object* pObj = os.obj;
	if(!pObj) return;

	INodeTab instances;
	IInstanceMgr::GetInstanceMgr()->GetInstances(*pNode, instances);
	for (int i = 0; i < instances.Count(); i++)
	{
		os = instances[i]->EvalWorldState(0, TRUE);
		if (os.obj == pObj)
			pIObject->AddInstanceID(instances[i]->GetHandle());
	}
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

void CIntermediateBuilder::Rotate90DegreesAroundX( Quat& inQuat )
{
	static Quat rotMinus90X = Quat(-sqrt(0.5),0,0,sqrt(0.5));
	inQuat = rotMinus90X*inQuat;
}

Ogre::SceneNode* CIntermediateBuilder::CollapseHierarchy(Ogre::SceneNode* pHierarchy, const std::list<std::string>& Arrays, const char* pszName)
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

	pMesh->m_bIsCollapsed = true;

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
	if (pMaxMaterial == NULL)
		return NULL;

	// Tjeck to see if it's a Standard material
	if (pMaxMaterial->ClassID() != Class_ID(DMTL_CLASS_ID, 0))
	{
		Ogre::String str("Material ("+Ogre::String(+pMaxMaterial->GetName())+") Not Standard!");
		LOGWARNING str.c_str());
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
		//LOGDEBUG "newMat->SetOpacity( 1.0f - pMaxMaterial->GetXParency()");
		//newMat->SetOpacity( 1.0f - pMaxMaterial->GetXParency() );

		// Check to see if it's a Standard material
		if (pMaxMaterial->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
		{
			
			StdMat* std = (StdMat *)pMaxMaterial;
			newMat->Set2Sided( std->GetTwoSided() ? true : false );
			newMat->SetWired( std->GetWire() ? true : false );
			//LOGDEBUG "newMat->SetOpacity( std->GetOpacity(0) )");
			newMat->SetOpacity( std->GetOpacity(0) );

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

CIntermediateObject *CIntermediateBuilder::GetObjectByNodeID(unsigned int iNode)
{
	std::map<unsigned int, CIntermediateObject*>::iterator it = m_mapIObjects.find(iNode);
	return (it == m_mapIObjects.end()) ? NULL : it->second;
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

CIntermediateBuilderSkeleton* CIntermediateBuilder::GetSkeletonBuilder( void )
{
	return m_pSkeletonBuilder;
}
