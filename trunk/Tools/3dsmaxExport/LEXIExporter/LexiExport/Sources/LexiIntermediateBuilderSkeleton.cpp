/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg

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

CIntermediateBuilderSkeleton::CIntermediateBuilderSkeleton()
{
	//
	m_iBoneIndex = 0;
	m_pMaxNode = NULL;
	m_pISkel = NULL;
	m_pProgressDlg = NULL;

}

CIntermediateBuilderSkeleton::~CIntermediateBuilderSkeleton()
{
	//
}

bool CIntermediateBuilderSkeleton::BuildIntermediateSkeleton( INode* pMaxNode )
{
	m_pMaxNode = pMaxNode;

	//LOGDEBUG "CIntermediate::CreateMesh() - Tjeck for skin modifier.");
	if(FindSkinModifier( m_pMaxNode ))
	{
		LOGDEBUG "Creating intermediate skeleton.");

		if(m_pISkel != NULL)
			delete m_pISkel;

		m_pISkel = new CIntermediateSkeleton();
		//pIMesh->SetSkeleton(m_pISkel); // Needed for ReIndexing!

		CreateBonePool();
		m_pISkel->PopulateBoneHandleMap();
		AssembleBones();
		ConnectLinkedBones();

		SetBoneIndexes();

		m_pISkel->BuildIndexedBoneList();

		SetBindingPose(0);

		//LOGDEBUG "%s", m_pISkel->ToString().c_str() );


		return true;
	}
	
	return false;
}

void CIntermediateBuilderSkeleton::Finalize( void )
{
	if(m_pISkel!=NULL)
	{
		m_pISkel->TrimVertexAssignments(4);
		m_pISkel->NormalizeVertexAssignments();
		m_pISkel->SetFPS( GetFrameRate() );
	}
}

// Index the bones according to a depth first search. This way we can add them to the ogre skeleton in the right order.
void CIntermediateBuilderSkeleton::SetBoneIndexes( void )
{
	std::vector< CIntermediateBone* > lRootBones = m_pISkel->GetRootBones();
	std::vector< CIntermediateBone* >::iterator rootIter = lRootBones.begin();

	while(rootIter != lRootBones.end())
	{
		CIntermediateBone* pCurBone = (*rootIter);
		pCurBone->SetIndex(m_iBoneIndex++);

		for(int i=0; i < pCurBone->GetBoneCount(); i++)
		{
			SetBoneIndexesRecursive(pCurBone->GetBone(i));
		}

		rootIter++;
	}
}

void CIntermediateBuilderSkeleton::SetBoneIndexesRecursive( CIntermediateBone* pIBone)
{
	pIBone->SetIndex(m_iBoneIndex++);

	for(int i=0; i < pIBone->GetBoneCount(); i++)
	{
		SetBoneIndexesRecursive(pIBone->GetBone(i));
	}
}

void CIntermediateBuilderSkeleton::CreateAnimation( CDDObject* pDDConfig, CExportProgressDlg *pProgressDlg )
{
	assert(pDDConfig);
	if(pProgressDlg != NULL)
		m_pProgressDlg = pProgressDlg;

	CAnimationSetting curAnim;
	curAnim.m_iStartFrame = pDDConfig->GetInt("AnimationStartID", 0);
	curAnim.m_iEndFrame = pDDConfig->GetInt("AnimationEndID", 1);
	curAnim.m_fSampleRate = pDDConfig->GetFloat("AnimationSampleRateID", 1.0);
	curAnim.m_sAnimName = pDDConfig->GetString("Name", "Anim1");

	AddFrame( curAnim );
}


void CIntermediateBuilderSkeleton::CreateBonePool( void )
{
	int iBoneCount = m_pSkin->GetNumBones();
	for (int i=0; i < iBoneCount; i++)
	{
		INode* pBone = m_pSkin->GetBone(i);
		CIntermediateBone* pIBone = new CIntermediateBone( pBone->GetName() );
		pIBone->SetHandle( pBone->GetHandle() );

		INode* pParent = pBone->GetParentNode();
		if(pParent)
			pIBone->SetParentHandle( pParent->GetHandle() );

		//pIBone->SetIndex( m_iBoneIndex++ );
		
		m_pISkel->AddBone( pIBone, pIBone->GetName().c_str() );
	}
}

void CIntermediateBuilderSkeleton::ConnectLinkedBones( void )
{
	static int i = 0;

	std::vector<CIntermediateBone*>::const_iterator iter = m_pISkel->GetRootBones().begin();

	while(iter != m_pISkel->GetRootBones().end())
	{
		CIntermediateBone* pIBone = *iter;
		ULONG handle = pIBone->GetHandle();
		INode* pNode = GetNodeFromID(handle);

		INode* pParentNode = pNode->GetParentNode();
		if(pParentNode)
			if(pParentNode->SuperClassID() == HELPER_CLASS_ID)
			{
				CIntermediateBone* pParentIBone = m_pISkel->FindBone( pParentNode->GetParentNode()->GetHandle() );
				if(pParentIBone)
				{
					Ogre::StringUtil::StrStreamType strID;
					strID << "Connecting_Bone_" << i++ ;
					CIntermediateBone* pConnectBone = new CIntermediateBone(strID.str().c_str());

					pConnectBone->SetHandle( pParentNode->GetHandle());
					//pConnectBone->SetIndex(m_iBoneIndex++);
					pConnectBone->AddBone( pIBone );
					pParentIBone->AddBone( pIBone );
				}
			}
		iter++;
	}
}

bool CIntermediateBuilderSkeleton::FindSkinModifier( INode* pMaxNode )
{	
	// Get object from node. Abort if no object.
	Object* ObjectPtr = pMaxNode->GetObjectRef();			

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
				m_pSkinMod = ModifierPtr;
				m_pSkin=(ISkin*)(m_pSkinMod)->GetInterface(I_SKIN);

				//get the node's initial transformation matrix and store it in a matrix3
				Matrix3 initTM;
				int msg = (m_pSkin)->GetBoneInitTM(pMaxNode, initTM);

				//get a pointer to the export context interface
				m_pSkinContext = (ISkinContextData*)(m_pSkin)->GetContextInterface(pMaxNode);
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

bool CIntermediateBuilderSkeleton::QuerySkinModifier( INode* pMaxNode )
{	
	// Get object from node. Abort if no object.
	Object* ObjectPtr = pMaxNode->GetObjectRef();			

	if (!ObjectPtr) return false;

	Modifier*			pSkinMod = NULL;
	ISkin*				pSkin = NULL;
	ISkinContextData*	pSkinContext = NULL;

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
				pSkinMod = ModifierPtr;
				pSkin=(ISkin*)(pSkinMod)->GetInterface(I_SKIN);

				//get the node's initial transformation matrix and store it in a matrix3
				Matrix3 initTM;
				int msg = (pSkin)->GetBoneInitTM(pMaxNode, initTM);

				//get a pointer to the export context interface
				pSkinContext = (ISkinContextData*)(pSkin)->GetContextInterface(pMaxNode);
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



void CIntermediateBuilderSkeleton::AddFrame( CAnimationSetting animationSettings )
{
	if(m_pProgressDlg != NULL)
		m_pProgressDlg->InitLocal(m_pISkel->GetBoneCount());

	for (int i=0; i < m_pISkel->GetBoneCount(); i++)
	{
		if(m_pProgressDlg != NULL)
			m_pProgressDlg->LocalStep("Exporting Bones");

		CAnimationSetting animSetting = animationSettings;

		CIntermediateBone* pIBone = m_pISkel->GetBone(i);
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

		//If bone is root in the modifier we must not make it relative to its parent
		bool bModifierRoot = false;
		std::vector<CIntermediateBone*> lRoots = m_pISkel->GetRootBones();
		for(int i=0; i<lRoots.size(); i++)
		{
			if(lRoots[i] == pIBone)
				bModifierRoot = true;
		}


		int tpf = GetTicksPerFrame();
		float fps = 1.0 / float(GetFrameRate());

		bool animCreated = pIBone->CreateAnimation(animSetting);

		if(animCreated)
		{
			float x = animSetting.m_iStartFrame;
			int frameCount = 0;
			while(x <= animSetting.m_iEndFrame)
			{
				TimeValue t = x*tpf;
				float time = (x- animSetting.m_iStartFrame)*fps;

				// adjust for next sample according to sampleRate
				x = x + animSetting.m_fSampleRate;

				Matrix3 nodeTM = pMaxNode->GetNodeTM(t);
				Matrix3 parentTM = pParentBone->GetNodeTM(t);
				Matrix3 relativeTM = nodeTM;

				if( pParentBone->IsRootNode() || bModifierRoot )
				{
					INode* pMeshNode = m_pMaxNode; //GetNodeFromID(meshNodeID);
					parentTM = pMeshNode->GetObjTMAfterWSM(t);
				}
	
				//if(!bModifierRoot)
				relativeTM = nodeTM * Inverse(parentTM);

				Point3 nodePos;
				Point3 nodeScale;
				Quat nodeOriet;

				DecomposeMatrix(relativeTM, nodePos, nodeOriet, nodeScale);

				Ogre::Quaternion oquat;
				oquat.x = nodeOriet.x;
				oquat.y = nodeOriet.z;
				oquat.z = -nodeOriet.y;
				oquat.w = nodeOriet.w;

				Ogre::Vector3 axis;
				Ogre::Radian w;
				oquat.ToAngleAxis(w, axis);
				oquat.FromAngleAxis(-w, axis);

				//CIntermediateBuilder::Rotate90DegreesAroundX(nodeOriet);
				//Ogre::Quaternion oquat;
				//oquat.x = nodeOriet.x;
				//oquat.y = nodeOriet.y;
				//oquat.z = nodeOriet.z;
				//oquat.w = nodeOriet.w;

				CIntermediateBuilder::Rotate90DegreesAroundX(nodePos);


				Point3 relPos = basePos;
				Point3 relScale = baseScale;
				Ogre::Quaternion relOriet = baseOriet;

				// relative transformation
				relPos = nodePos - basePos;
				relScale = (nodeScale + baseScale) / 2;
				relOriet = oriet.Inverse() * oquat;
				//relOriet =  oquat * oriet.Inverse();

				relScale = Point3(1,1,1);

				//Clamp(relPos, 0.0001);

				
				pIBone->AddFrame( 
					animSetting.m_sAnimName,
					frameCount++, // frame nr relative to this animation
					time,
					Ogre::Vector3(relPos.x, relPos.y, relPos.z), // already rotated..
					relOriet, 
					Ogre::Vector3(relScale.x, relScale.z, relScale.y)
					);

				//if(m_fAnimTotalLength < time)
				//	m_fAnimTotalLength = time;
			}
		}
	}
	//LOGDEBUG "CIntermediate::AddFrame() - End");
}


void CIntermediateBuilderSkeleton::SetBindingPose( unsigned int frame )
{
	for (int i=0; i < m_pISkel->GetBoneCount(); i++)
	{
		CIntermediateBone* pIBone = m_pISkel->GetBone(i);
		INode* pMaxNode = GetNodeFromID(pIBone->GetHandle());
		if(!pMaxNode) return;

		// binding pose:
		INode* pParentBone = pMaxNode->GetParentNode();
		Matrix3 baseParentTM = pParentBone->GetNodeTM(frame);
		Matrix3 baseTM = pMaxNode->GetNodeTM(frame);

		//If bone is root in the modifier we must not make it relative to its parent
		bool bModifierRoot = false;
		std::vector<CIntermediateBone*> lRoots = m_pISkel->GetRootBones();
		for(int i=0; i<lRoots.size(); i++)
		{
			if(lRoots[i] == pIBone)
				bModifierRoot = true;
		}

		if( pParentBone->IsRootNode() ||  bModifierRoot)
		{
			// NOTICE! Double tjeck this seciton! Are Root bones handled correctly?
			Matrix3 meshTM = m_pMaxNode->GetObjTMAfterWSM(frame);
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

		//CIntermediateBuilder::Rotate90DegreesAroundX(baseOriet);
		//Ogre::Quaternion oquat;
		//oquat.x = baseOriet.x;
		//oquat.y = baseOriet.y;
		//oquat.z = baseOriet.z;
		//oquat.w = baseOriet.w;

		CIntermediateBuilder::Rotate90DegreesAroundX(basePos);

		Ogre::Vector3 scale = Ogre::Vector3(baseScale.x, baseScale.z, baseScale.y);

		pIBone->SetBindingPose(Ogre::Vector3(basePos.x, basePos.y, basePos.z), oquat, scale);
	}
}

void CIntermediateBuilderSkeleton::AssembleBones( void )
{
	//m_iBoneIndex = 0;
	for (int i=0; i < m_pISkel->GetBoneCount(); i++)
	{
		CIntermediateBone* pIBone = m_pISkel->GetBone(i);
		INode* pMaxNode = GetNodeFromID(pIBone->GetHandle());
		if(!pMaxNode) return;

		ULONG parentHandle = pIBone->GetParentHandle();
		CIntermediateBone* parent = m_pISkel->FindBone( parentHandle );
		if(parent)
			parent->AddBone( pIBone );
		else
		{
			m_pISkel->MarkBoneAsRoot( pIBone );
		}
	}
}

// Set the vertex data from the max vertex with index maxIdx to Intermediate Mesh vertex with intermediateIdx.
void CIntermediateBuilderSkeleton::SetVertexData( unsigned int maxIdx, unsigned int intermediateIdx )
{
	if(m_pISkel==NULL)
		return;
	SVertexBoneData vd;
	int nrBones = m_pSkinContext->GetNumAssignedBones(maxIdx);
	for (int i=0; i < nrBones; i++)
	{
		int iBoneIdx = m_pSkinContext->GetAssignedBone(maxIdx,i);
		INode* pMaxBone = m_pSkin->GetBone(iBoneIdx);
		if(pMaxBone)
		{
			CIntermediateBone* pIBone = m_pISkel->FindBone( pMaxBone->GetHandle() );

			vd.boneIndex = pIBone->GetIndex();
			vd.weight = m_pSkinContext->GetBoneWeight(maxIdx, i);//iBoneIdx);

			if(vd.weight < 0.001f) 
				continue;

			m_pISkel->AddVertexData( intermediateIdx, vd );
		}
	}
}

CIntermediateSkeleton* CIntermediateBuilderSkeleton::GetSkeleton( void )
{
	return m_pISkel;
}
