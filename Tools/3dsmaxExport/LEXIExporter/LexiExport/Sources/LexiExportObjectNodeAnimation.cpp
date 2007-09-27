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
#include "LexiDialogNodeAnimationProperties.h"
#include "LexiExportObjectNodeAnimation.h"
#include "LexiExportObjectScene.h"
#include "LexiOgreMeshCompiler.h"
#include "LexiOgreMaterialCompiler.h"
#include "LexiIntermediateBuilder.h"
#include "LexiIntermediateMesh.h"
#include "tinyxml.h"

//

CNodeAnimationPropertiesDlg* CNodeAnimationExportObject::m_pEditDlg = NULL;
CDDObject* CNodeAnimationExportObject::m_pDDMetaDesc = NULL;

//

class KEYFRAME {

	public:

		float m_fTime;
		Ogre::Vector3 m_vTranslation;
		Ogre::Vector3 m_vScale;
		Ogre::Quaternion m_qRotation;

};

typedef std::list<KEYFRAME> KeyframeList;

typedef std::map<Ogre::SceneNode*, KeyframeList> NodeMap;

typedef map<Ogre::SceneNode*, Ogre::NodeAnimationTrack*> AnimMap;

//

CNodeAnimationExportObject::CNodeAnimationExportObject(CDDObject* pConfig) : CExportObject(pConfig)
{
	REGISTER_MODULE("Node Animation Export Object")
}

CNodeAnimationExportObject::~CNodeAnimationExportObject()
{
	UNREGISTER_MODULE
}

//

GDI::Window* CNodeAnimationExportObject::GetEditWindow(GDI::Window* pParent)
{
	if(m_pEditDlg == NULL)
	{
		m_pEditDlg = new CNodeAnimationPropertiesDlg(pParent);
		m_pEditDlg->Create();

		m_pDDMetaDesc = BuildMetaDesc();

		m_pEditDlg->Init(m_pDDMetaDesc, ".nodeanim");	
	}

	m_pEditDlg->SetInstance(m_pDDConfig, this);

	return m_pEditDlg;
}

//

void CNodeAnimationExportObject::CloseEditWindow()
{
	if(m_pEditDlg != NULL)
	{
		m_pEditDlg->EndDialog(0);

		delete m_pEditDlg;
		m_pEditDlg = NULL;

		if(m_pDDMetaDesc)
		{
			m_pDDMetaDesc->Release();
			m_pDDMetaDesc = NULL;
		}
	}
}

//

bool CNodeAnimationExportObject::OnCreate(CExporterPropertiesDlg* pPropDialog)
{
	CExportObject* pParent = GetParent();
	if(pParent == NULL) return false;

	std::string sName = pParent->GetName();
	sName += " NodeAnimation";
	m_pDDConfig->SetString("Name", sName.c_str());

	std::string sFilename = pParent->GetName();
	sFilename += ".nodeanim";
	m_pDDConfig->SetString("Filename", sFilename.c_str());

	return true;
}

//

bool CNodeAnimationExportObject::SupportsParentType(const CExportObject* pParent) const
{
	return strcmp(pParent->GetType(), "scene") == 0 ? true : false;
}

//

CDDObject* CNodeAnimationExportObject::BuildMetaDesc()
{
	int iStartFrame = GetAnimStart() / GetTicksPerFrame();
	int iEndFrame = GetAnimEnd() / GetTicksPerFrame();

	CDDObject* AnimContainer = new CDDObject();

	fastvector<const CDDObject*> lAnimSettings;
	CDDObject* pDDAnimElement;

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID", "AnimationStartID");
	pDDAnimElement->SetString("Type", "Int");
	pDDAnimElement->SetBool("EnableSlider", false);
	pDDAnimElement->SetString("Group", "Animation");
	pDDAnimElement->SetString("Caption", "Start Frame");
	pDDAnimElement->SetString("Help", "Frame at which the animation begins");
	pDDAnimElement->SetInt("Default", iStartFrame);
	lAnimSettings.push_back(pDDAnimElement);

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID", "AnimationEndID");
	pDDAnimElement->SetString("Type", "Int");
	pDDAnimElement->SetBool("EnableSlider", false);
	pDDAnimElement->SetString("Group", "Animation");
	pDDAnimElement->SetString("Caption", "End Frame");
	pDDAnimElement->SetString("Help", "Frame at which the animation stops");
	pDDAnimElement->SetInt("Default", iEndFrame);
	lAnimSettings.push_back(pDDAnimElement);

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID", "AnimationSampleRateID");
	pDDAnimElement->SetString("Type", "Float");
	pDDAnimElement->SetBool("EnableSlider", false);
	pDDAnimElement->SetString("Group", "Animation");
	pDDAnimElement->SetString("Caption", "Samplerate");
	pDDAnimElement->SetString("Help", "Number of samples per second");
	pDDAnimElement->SetFloat("Default", (float)GetFrameRate());
	lAnimSettings.push_back(pDDAnimElement);

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID", "AnimationOptimizeID");
	pDDAnimElement->SetString("Type", "Bool");
	pDDAnimElement->SetString("Group", "Animation");
	pDDAnimElement->SetString("Caption", "Optimize");
	pDDAnimElement->SetString("Help", "Reduces the amount of keyframes by removing redundant data");
	pDDAnimElement->SetBool("Default", true);
	lAnimSettings.push_back(pDDAnimElement);

	AnimContainer->SetDDList("MetaList", lAnimSettings, false);

	return AnimContainer;
}

//

static void BuildNodeList(Ogre::SceneNode* pNode, NodeMap& nodemap)
{
	for(Ogre::Node::ChildNodeIterator it = pNode->getChildIterator(); it.hasMoreElements(); )
	{
		Ogre::SceneNode* pN = (Ogre::SceneNode*)it.getNext();
		nodemap[pN] = KeyframeList();
		BuildNodeList(pN, nodemap);
	}
}

static void ExportToXML(const AnimMap& animmap, const char* pszName, const char* pszFilename, float fLength)
{
	TiXmlDocument* pDoc = new TiXmlDocument();

	TiXmlElement* pAnims = pDoc->InsertEndChild(TiXmlElement("animations"))->ToElement();
	pAnims->SetAttribute("formatVersion", "1.0");

	TiXmlElement* pAnim = pAnims->InsertEndChild(TiXmlElement("animation"))->ToElement();
	pAnim->SetAttribute("name", pszName);
	pAnim->SetDoubleAttribute("length", fLength);

	//

	for(AnimMap::const_iterator it = animmap.begin(); it != animmap.end(); ++it)
	{
		const Ogre::SceneNode* pNode = it->first;
		const Ogre::NodeAnimationTrack* pAnimTrack = it->second;

		unsigned int iNumKeyframes = pAnimTrack->getNumKeyFrames();

		TiXmlElement* pTrack = pAnim->InsertEndChild(TiXmlElement("track"))->ToElement();
		pTrack->SetAttribute("target", pNode->getName());
		pTrack->SetAttribute("keyframes", iNumKeyframes);

		for(unsigned int x = 0; x < iNumKeyframes; x++)
		{
			const Ogre::TransformKeyFrame* pKF = pAnimTrack->getNodeKeyFrame(x);

			float fTime = pKF->getTime();
			TiXmlElement* pKeyframe = pTrack->InsertEndChild(TiXmlElement("keyframe"))->ToElement();
			pKeyframe->SetDoubleAttribute("time", fTime);

			const Ogre::Vector3& vTranslation = pKF->getTranslate();
			TiXmlElement* pPos = pKeyframe->InsertEndChild(TiXmlElement("position"))->ToElement();
			pPos->SetDoubleAttribute("x", vTranslation.x);
			pPos->SetDoubleAttribute("y", vTranslation.y);
			pPos->SetDoubleAttribute("z", vTranslation.z);

			const Ogre::Quaternion& qRotation = pKF->getRotation();
			TiXmlElement* pRot = pKeyframe->InsertEndChild(TiXmlElement("rotation"))->ToElement();
			pRot->SetDoubleAttribute("qx", qRotation.x);
			pRot->SetDoubleAttribute("qy", qRotation.y);
			pRot->SetDoubleAttribute("qz", qRotation.z);
			pRot->SetDoubleAttribute("qw", qRotation.w);

			const Ogre::Vector3& vScale = pKF->getScale();
			TiXmlElement* pScale = pKeyframe->InsertEndChild(TiXmlElement("scale"))->ToElement();
			pScale->SetDoubleAttribute("x", vScale.x);
			pScale->SetDoubleAttribute("y", vScale.y);
			pScale->SetDoubleAttribute("z", vScale.z);
		}
	}

	//

	std::string sFilename = FixupFilename(pszFilename, "animation");
	pDoc->SaveFile(sFilename.c_str());

	delete pDoc;
}

bool CNodeAnimationExportObject::Export(CExportProgressDlg* pProgressDlg, bool bForceAll)
{
	const char* pszName = m_pDDConfig->GetString("Name", "");
	if(!pszName[0]) return false;

	const char* pszFilename = m_pDDConfig->GetString("Filename", "");
	if(!pszFilename[0]) return false;

	CExportObject* pParent = (CExportObject*)GetParent();
	if(!pParent) return false;

	Ogre::SceneNode* pRootNode = CIntermediateBuilder::Get()->CreateNodeHierarchy(pParent->GetMAXNodeID(), false);
	if(pRootNode == NULL) return false;

	NodeMap nodemap;
	BuildNodeList(pRootNode, nodemap);

	//

	unsigned int iStartFrame = m_pDDConfig->GetInt("AnimationStartID", GetAnimStart() / GetTicksPerFrame());
	unsigned int iEndFrame = m_pDDConfig->GetInt("AnimationEndID", GetAnimEnd() / GetTicksPerFrame());
	unsigned int iNumFrames = iEndFrame - iStartFrame + 1;

	float fSPS = m_pDDConfig->GetFloat("AnimationSampleRateID", (float)GetFrameRate());
	if(fSPS < 0.0001f) fSPS = 1.0f;

	float fFPS = (float)GetFrameRate();

	float fStartTime = ((float)iStartFrame) / fFPS;
	float fEndTime = ((float)(iEndFrame + 1)) / fFPS;

	float fTimePerFrame = (fEndTime - fStartTime) / (float)iNumFrames;
	float fIndexAdd = fFPS / fSPS;

	unsigned int iIndex = 0;

	//

	float fAnimLength = iNumFrames / fFPS;

	AnimMap animmap;
	unsigned int iTrackHandle = 0;

	Ogre::Animation* pAnim = new Ogre::Animation(pszName, fAnimLength);

	for(NodeMap::iterator it = nodemap.begin(); it != nodemap.end(); ++it)
	{
		Ogre::SceneNode* pNode = it->first;
		Ogre::NodeAnimationTrack* pTrack = pAnim->createNodeTrack(iTrackHandle++);
		animmap[pNode] = pTrack;
	}

	//

	while(true)
	{
		float fCurrentTime = fStartTime + (fTimePerFrame * fIndexAdd * (float)iIndex);
		if(fCurrentTime > fEndTime) fCurrentTime = fEndTime;

		CIntermediateBuilder::Get()->UpdateNodeHierarchy((TimeValue)(fCurrentTime * 4800.0f), pRootNode);

		for(NodeMap::iterator it = nodemap.begin(); it != nodemap.end(); ++it)
		{
			Ogre::SceneNode* pNode = it->first;

			Ogre::NodeAnimationTrack* pTrack = animmap[pNode];
			Ogre::TransformKeyFrame* pKF = pTrack->createNodeKeyFrame(fCurrentTime - fStartTime);

			pKF->setTranslate(pNode->getPosition());
			pKF->setScale(pNode->getScale());
			pKF->setRotation(pNode->getOrientation());
		}

		if(fCurrentTime >= fEndTime) break;

		iIndex++;
	}

	//

	bool bOptimize = m_pDDConfig->GetBool("AnimationOptimizeID", true);
	if(bOptimize)
	{
		for(NodeMap::iterator it = nodemap.begin(); it != nodemap.end(); ++it)
		{
			Ogre::SceneNode* pNode = it->first;
			Ogre::AnimationTrack* pTrack = animmap[pNode];
			pTrack->optimise();
		}
	}

	//

	ExportToXML(animmap, pszName, pszFilename, fAnimLength);

	//

	delete pAnim;

	nodemap.clear();

	CIntermediateBuilder::Get()->CleanupNodeHierarchy(pRootNode);
	delete pRootNode;

	return true;
}

//

