/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Marty Rabens
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
#include "LexiDialogSceneProperties.h"
#include "LexiIntermediateLight.h"
#include "LexiExportObjectScene.h"
#include "LexiDialogSelectNode.h"
#include "LexiMaxExport.h"
#include "tinyxml.h"

#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")

CScenePropertiesDlg* CSceneExportObject::m_pEditDlg=NULL;
CDDObject* CSceneExportObject::m_pDDMetaDesc=NULL;
//

CSceneExportObject::CSceneExportObject(CDDObject* pConfig) : CExportObject(pConfig)
{
	REGISTER_MODULE("Scene Export Object")
}

CSceneExportObject::~CSceneExportObject()
{
	UNREGISTER_MODULE
}

// Check if ExportObject supports a given ExportObject instance as parent
bool CSceneExportObject::SupportsParentType(const CExportObject* pParent) const
{	
	// Scene can only have root as parent
	if(pParent==NULL || stricmp(pParent->GetType(), "root")!=0) return false;
	return true;
}

// Get window for editing ExportObject properties
GDI::Window* CSceneExportObject::GetEditWindow(GDI::Window* pParent)
{
	if(m_pEditDlg==NULL)
	{
		m_pEditDlg=new CScenePropertiesDlg(pParent);
		m_pEditDlg->Create();
		m_pDDMetaDesc=BuildMetaDesc();
		m_pEditDlg->Init(m_pDDMetaDesc, ".scene");	
	}

	m_pEditDlg->SetInstance(m_pDDConfig, this);
	//LOGDEBUG "Window Created:\t%i (handle)", m_pEditDlg->m_hWnd);
	return m_pEditDlg;
}

// End edit
void CSceneExportObject::CloseEditWindow()
{
	if(m_pEditDlg!=NULL)
	{
		//LOGDEBUG "Window Ended:\t%i (handle)", m_pEditDlg->m_hWnd);
		m_pEditDlg->EndDialog(0);
		delete m_pEditDlg;		
		m_pEditDlg=NULL;
		if(m_pDDMetaDesc) 
		{
			m_pDDMetaDesc->Release();
			m_pDDMetaDesc=NULL;
		}
	}
}

// Called when object is first created [by user].
// This allows for wizard-style editing of required data
// If this function returns false, the object is not created
bool CSceneExportObject::OnCreate(CExporterPropertiesDlg* pPropDialog)
{
	if(GetMAXNodeID()==0xFFFFFFFF)
	{
		CSelectNodeDlg dlg((GDI::Window*)pPropDialog, this);
		if(dlg.DoModal() != IDOK) return false;	// user must select a valid node
	}

	// Initialize name and filename
	std::string sName = GetNameFromID(m_pDDConfig->GetInt("NodeID"));
	RemoveIllegalChars(sName);
	m_pDDConfig->SetString("Name", sName.c_str());

	sName += ".scene";
	m_pDDConfig->SetString("FileName", sName.c_str());

	return true;
}

CDDObject* CSceneExportObject::BuildMetaDesc( void )
{
	CDDObject* pDDMetaDesc = new CDDObject();

	fastvector< const CDDObject* > lSettings;

	pDDMetaDesc->SetDDList("MetaList", lSettings, false);

	return pDDMetaDesc;
}

//
bool CSceneExportObject::Export(CExportProgressDlg* pProgressDlg, bool bForceAll)
{
	START_PROFILE("CSceneExportObject::Export()");
	bool returnVal = false;
	Ogre::SceneNode* pNode = NULL;
	TiXmlDocument* pSceneDoc = NULL;

	if(m_bEnabled || bForceAll)
	{
		try
		{
			pProgressDlg->InitLocal(3);

			LOGDEBUG "Starting...");
			CIntermediateBuilder::Get()->Clear();
			CIntermediateBuilder::Get()->SetConfig(m_pDDConfig);

			pProgressDlg->LocalStep("Scene: Reading max data");
			LOGDEBUG "Creating Hierarchy...");

			pNode = CIntermediateBuilder::Get()->CreateHierarchy(GetMAXNodeID(), true, false);
			if(pNode == NULL)
			{
				LOGERROR "No node named \"%s\" with ID: %d", GetName(), GetMAXNodeID());
				return false;
			}

			LOGINFO "Exporting scene: %s", m_pDDConfig->GetString("FileName"));

			returnVal = CExportObject::Export(pProgressDlg, bForceAll);

			pProgressDlg->LocalStep("Scene: Generating DotScene XML...");
			pSceneDoc = DotSceneFromHierarchy(pNode);

			const Ogre::String& sOrigFilename = m_pDDConfig->GetString("FileName");
			const Ogre::String& sSceneFilename = FixupFilename(sOrigFilename.c_str(), "scene");

			Ogre::String sBaseName;
			Ogre::String sScenePath;

			Ogre::StringUtil::splitFilename(sSceneFilename, sBaseName, sScenePath);
			int n = sScenePath.find("/");
			while(n != Ogre::String::npos)
			{
				sScenePath.replace(n,1,"\\");
				n = sScenePath.find("/");
			}

			if(!::MakeSureDirectoryPathExists(sScenePath.c_str()))
			{
				LOGERROR "Error while attempting to create path: %s", sScenePath.c_str());
				delete pSceneDoc;
				CIntermediateBuilder::Get()->CleanUpHierarchy( pNode );
				CIntermediateBuilder::Get()->Clear();
				return false;
			}

			pProgressDlg->LocalStep("Scene: Writing DotScene XML...");
			pSceneDoc->SaveFile(sSceneFilename);

		} catch(Ogre::Exception e)
		{
			LOGERROR "OgreException caught: %s", e.getDescription().c_str());
		} catch(...)
		{
			LOGERROR "Caught unhandled exception in CSceneExportObject::Export()");
		}

		LOGDEBUG "Cleaning up...");
		delete pSceneDoc;
		CIntermediateBuilder::Get()->CleanUpHierarchy( pNode );
		CIntermediateBuilder::Get()->Clear();

		LOGINFO "..Done!");
	}

	END_PROFILE("CSceneExportObject::Export()");

	return returnVal;
}


TiXmlDocument* CSceneExportObject::DotSceneFromHierarchy(Ogre::SceneNode* pRootNode)
{
	TiXmlDocument* pDoc = new TiXmlDocument();

	TiXmlElement* pElem = pDoc->InsertEndChild(TiXmlElement("scene"))->ToElement();
	pElem->SetAttribute("formatVersion", "");

	pElem = pElem->InsertEndChild(TiXmlElement("environment"))->ToElement();
	pElem = pElem->InsertEndChild(TiXmlElement("colourAmbient"))->ToElement();
	Point3 col = CExporter::GetMax()->GetAmbient(0, Interval(0,0));
	pElem->SetDoubleAttribute("r", col.x); pElem->SetDoubleAttribute("g", col.y); pElem->SetDoubleAttribute("b", col.z);
	pElem = pElem->Parent()->ToElement()->InsertEndChild(TiXmlElement("colourBackground"))->ToElement();
	col = CExporter::GetMax()->GetBackGround(0, Interval(0,0));
	pElem->SetDoubleAttribute("r", col.x); pElem->SetDoubleAttribute("g", col.y); pElem->SetDoubleAttribute("b", col.z);
	pElem = pElem->Parent()->Parent()->ToElement();

	pElem = pElem->InsertEndChild(TiXmlElement("nodes"))->ToElement();
	DotSceneAddNode(pRootNode, pElem);

	return pDoc;
}


void CSceneExportObject::DotSceneAddNode(Ogre::SceneNode* pNode, TiXmlElement* pElem)
{
	TiXmlElement* pNodeElem = pElem->InsertEndChild(TiXmlElement("node"))->ToElement();
	pNodeElem->SetAttribute("name", pNode->getName());

	Ogre::Vector3 v;
	Ogre::Quaternion q;

	v = pNode->getPosition();
	pElem = pNodeElem->InsertEndChild(TiXmlElement("position"))->ToElement();
	pElem->SetDoubleAttribute("x", v.x); pElem->SetDoubleAttribute("y", v.y); pElem->SetDoubleAttribute("z", v.z);

	q = pNode->getOrientation();
	pElem = pNodeElem->InsertEndChild(TiXmlElement("rotation"))->ToElement();
	pElem->SetDoubleAttribute("qx", q.x); pElem->SetDoubleAttribute("qy", q.y); pElem->SetDoubleAttribute("qz", q.z); pElem->SetDoubleAttribute("qw", q.w);

	v = pNode->getScale();
	pElem = pNodeElem->InsertEndChild(TiXmlElement("scale"))->ToElement();
	pElem->SetDoubleAttribute("x", v.x); pElem->SetDoubleAttribute("y", v.y); pElem->SetDoubleAttribute("z", v.z);

	if(pNode->numAttachedObjects() > 0)
	{
		Ogre::MovableObject* pObject = pNode->getAttachedObject(0);

		if(pObject->getMovableType() == "CIntermediateMesh")
		{
			CIntermediateMesh* pIMesh = (CIntermediateMesh*)pObject;
			pElem = pNodeElem->InsertEndChild(TiXmlElement("entity"))->ToElement();
			pElem->SetAttribute("name", pIMesh->getName());
			pElem->SetAttribute("meshFile", ((CIntermediateMesh*)pIMesh->GetBaseInstanceObject())->getName() + ".mesh");
			pElem->SetAttribute("static", (pIMesh->GetSkeleton() == NULL) ? "true" : "false");
			pElem->SetAttribute("castShadows", pIMesh->GetStringValue("castShadows"));
		}
		else if(pObject->getMovableType() == "CIntermediateLight")
		{
			CIntermediateLight* pILight = (CIntermediateLight*)pObject;
			pElem = pNodeElem->InsertEndChild(TiXmlElement("light"))->ToElement();
			pElem->SetAttribute("name", pILight->getName());
			pElem->SetAttribute("type", pILight->GetStringValue("type"));
			pElem->SetAttribute("visible", pILight->GetBoolValue("visible") ? "true" : "false");
			pElem->SetAttribute("castShadows", pILight->GetBoolValue("castShadows") ? "true" : "false");

			Ogre::ColourValue col(pILight->GetColourValue("colourDiffuse"));
			pElem = pElem->InsertEndChild(TiXmlElement("colourDiffuse"))->ToElement();
			pElem->SetDoubleAttribute("r", col.r);
			pElem->SetDoubleAttribute("g", col.g);
			pElem->SetDoubleAttribute("b", col.b);
			pElem = pElem->Parent()->ToElement();

			col = pILight->GetColourValue("colourSpecular");
			pElem = pElem->InsertEndChild(TiXmlElement("colourSpecular"))->ToElement();
			pElem->SetDoubleAttribute("r", col.r);
			pElem->SetDoubleAttribute("g", col.g);
			pElem->SetDoubleAttribute("b", col.b);
			pElem = pElem->Parent()->ToElement();

			if(pILight->GetStringValue("type") == "spot")
			{
				pElem = pElem->InsertEndChild(TiXmlElement("lightRange"))->ToElement();
				pElem->SetDoubleAttribute("inner", pILight->GetFloatValue("lightRange.inner"));
				pElem->SetDoubleAttribute("outer", pILight->GetFloatValue("lightRange.outer"));
				pElem->SetDoubleAttribute("falloff", pILight->GetFloatValue("lightRange.falloff"));
				pElem = pElem->Parent()->ToElement();
			}

			if(pILight->GetStringValue("type") == "spot" || pILight->GetStringValue("type") == "directional")
			{
				pElem = pElem->InsertEndChild(TiXmlElement("normal"))->ToElement();
				Ogre::Vector3 vNorm = pILight->GetVec3Value("normal");
				pElem->SetDoubleAttribute("x",vNorm.x);
				pElem->SetDoubleAttribute("y",vNorm.y);
				pElem->SetDoubleAttribute("z",vNorm.z);
				pElem = pElem->Parent()->ToElement();
			}

			if(pILight->GetStringValue("trackTarget.nodeName").size())
			{
				pElem = pElem->InsertEndChild(TiXmlElement("trackTarget"))->ToElement();
				pElem->SetAttribute("nodeName", pILight->GetStringValue("trackTarget.nodeName"));
				pElem = pElem->Parent()->ToElement();
			}

			// TODO: lightAttenuation: range, constant, linear, quadratic
		}
		else if(pObject->getMovableType() == "CIntermediateCamera")
		{
			CIntermediateCamera* pICamera = (CIntermediateCamera*)pObject;
			pElem = pNodeElem->InsertEndChild(TiXmlElement("camera"))->ToElement();
			pElem->SetAttribute("name", pICamera->getName());
			pElem->SetDoubleAttribute("fov", pICamera->GetFloatValue("fov"));
			pElem->SetAttribute("projectionType", pICamera->GetStringValue("projectionType"));

			pElem = pElem->InsertEndChild(TiXmlElement("clipping"))->ToElement();
			pElem->SetDoubleAttribute("near", pICamera->GetFloatValue("clipping.near"));
			pElem->SetDoubleAttribute("far", pICamera->GetFloatValue("clipping.far"));
			pElem = pElem->Parent()->ToElement();

			if(pICamera->GetStringValue("trackTarget.nodeName").size())
			{
				pElem = pElem->InsertEndChild(TiXmlElement("trackTarget"))->ToElement();
				pElem->SetAttribute("nodeName", pICamera->GetStringValue("trackTarget.nodeName"));
				pElem = pElem->Parent()->ToElement();
			}

//			Ogre::Quaternion qRot = pICamera->GetQuatValue("rotation");
//			pElem = pElem->InsertEndChild(TiXmlElement("rotation"))->ToElement();
//			pElem->SetDoubleAttribute("qw", qRot.w);
//			pElem->SetDoubleAttribute("qx", qRot.x);
//			pElem->SetDoubleAttribute("qy", qRot.y);
//			pElem->SetDoubleAttribute("qz", qRot.z);
//			pElem = pElem->Parent()->ToElement();
		}
	}

	Ogre::Node::ChildNodeIterator itChild = pNode->getChildIterator();
	while(itChild.hasMoreElements())
	{
		DotSceneAddNode((Ogre::SceneNode*)itChild.getNext(), pNodeElem);
	}
}


bool CSceneExportObject::SupportsMAXNode(INode* pMAXNode) const
{
	if(pMAXNode == NULL) return false;
	if(pMAXNode->GetHandle() == 0) return true; // MAX's scene root

	SClass_ID nClass = GetClassIDFromNode(pMAXNode);

	if(nClass == HELPER_CLASS_ID || nClass == GEOMOBJECT_CLASS_ID || nClass == CAMERA_CLASS_ID || nClass == LIGHT_CLASS_ID) return true;

	return false;
}
