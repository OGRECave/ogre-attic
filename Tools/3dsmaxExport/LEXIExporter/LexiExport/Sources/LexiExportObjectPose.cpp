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
#include "LexiMaxExport.h"
#include "LexiDialogPoseProperties.h"
#include "LexiExportObjectPose.h"
#include "LexiOgreMeshCompiler.h"
#include "LexiOgreMaterialCompiler.h"
#include "LexiIntermediateBuilder.h"
#include "LexiIntermediateMesh.h"
#include "LexiDialogSelectNode.h"

#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")

CPosePropertiesDlg* CPoseMeshExportObject::m_pEditDlg=NULL;
CDDObject* CPoseMeshExportObject::m_pDDMetaDesc=NULL;
int CPoseMeshExportObject::m_iPoseCount=0;
//

CPoseMeshExportObject::CPoseMeshExportObject(CDDObject *pConfig) : CExportObject(pConfig)
{
	REGISTER_MODULE("Pose Mesh Export Object")

	//	m_pMetaCtrl = NULL;
}

CPoseMeshExportObject::~CPoseMeshExportObject()
{
	UNREGISTER_MODULE
}

// Check if ExportObject supports a given ExportObject instance as parent
bool CPoseMeshExportObject::SupportsParentType(const CExportObject *pParent) const
{	
	const IExportObjectMeshSupport* pMeshSupport;
	try {
		pMeshSupport = dynamic_cast<const IExportObjectMeshSupport*>(pParent);
		if(pMeshSupport == NULL)
			return false;
	} catch (...) 
	{
	  LOGERROR "Error while attempting to check compatible parent type. Parent must atleast support a mesh type.");
	}

	return true;
}

//
bool CPoseMeshExportObject::SupportsMAXNode(INode *pMAXNode) const
{
	// Actually we retrieve the parent node and evaluates different frames.
	//if(pMAXNode==NULL) return false;
	//SClass_ID nClass = GetClassIDFromNode(pMAXNode);
	//if(nClass == GEOMOBJECT_CLASS_ID) return true;
	return false;
}

// Get window for editing ExportObject properties
GDI::Window* CPoseMeshExportObject::GetEditWindow(GDI::Window *pParent)
{	
	if(m_pEditDlg==NULL)
	{
		m_pEditDlg=new CPosePropertiesDlg(pParent);
		m_pEditDlg->Create();
		m_pDDMetaDesc=BuildMetaDesc();
		m_pEditDlg->Init(m_pDDMetaDesc, ".mesh");	
	}

	m_pEditDlg->SetInstance(m_pDDConfig, this);
	return m_pEditDlg;
}

// End edit
void CPoseMeshExportObject::CloseEditWindow()
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
bool CPoseMeshExportObject::OnCreate(CExporterPropertiesDlg *pPropDialog)
{
	// Initialize pose name
	Ogre::String sDefaultPoseName = "Pose_"+Ogre::StringConverter::toString(m_iPoseCount++);
	m_pDDConfig->SetString("Name", sDefaultPoseName.c_str());
	return true;
}

CDDObject* CPoseMeshExportObject::BuildMetaDesc( void )
{

	CDDObject* pDDMetaDesc = new CDDObject();

	fastvector< const CDDObject* > lSettings;
	CDDObject* pDDMetaElement;

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","FrameRefID");
	pDDMetaElement->SetString("Type","int");
	pDDMetaElement->SetString("Group","Pose Settings");
	pDDMetaElement->SetString("Caption","Pose from frame:");
	pDDMetaElement->SetString("Help","Create Pose from this still frame.");
	pDDMetaElement->SetBool("EnableSlider", false);
	pDDMetaElement->SetInt("Default", 0);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","OptimizeID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Pose Settings");
	pDDMetaElement->SetString("Caption","Optimize");
	pDDMetaElement->SetString("Help","Ignore vertex data which is the same as the reference.");
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaDesc->SetDDList("MetaList", lSettings, false);

	return pDDMetaDesc;
}

//
typedef std::map< Ogre::String, CIntermediateMaterial*> MAT_LIST;

bool CPoseMeshExportObject::Export( CExportProgressDlg *pProgressDlg, bool bForceAll)
{
	//bool returnVal = false;

	if(m_bEnabled || bForceAll)
	{
		CExportObject *pBaseParent = GetParent();
		IExportObjectMeshSupport* pParent;
		try {
			pParent = dynamic_cast<IExportObjectMeshSupport*>(pBaseParent);
			if(pParent == NULL)
				return false;
		} catch (...) 
		{
		  LOGERROR "Error while attempting to check compatible parent type. Parent must atleast support a mesh type.");
		}

		CIntermediateMesh* pIMesh = pParent->GetIntermediateMesh();

		if(pIMesh->IsCollapsed())
		{
			LOGERROR "Cannot create pose data for collapsed Meshes. Pose Ignored");
			return false;
		}

		// Check vertex count before adding pose description to Intermediate Mesh
		CMeshArray* lVerts = pIMesh->GetArray("position", 0);
		int iOrigVertCount = lVerts->Size();

		int iFrame = m_pDDConfig->GetInt("FrameRefID",0);
		faststring sPoseName = m_pDDConfig->GetString("Name",0);
		bool bOptimize = m_pDDConfig->GetBool("OptimizeID",true);
		lVerts = pIMesh->GetArray("position",iFrame);
		int iPoseVertCount = lVerts->Size();

		if(iPoseVertCount != iOrigVertCount)
		{
			LOGERROR "Pose(%s) does not contain same amount of verticies as base mesh. Pose Ignored", sPoseName.c_str() );
			return false;
		}

		pIMesh->AddPose(sPoseName.c_str(), iFrame, bOptimize);

		
	}
	return CExportObject::Export(pProgressDlg, bForceAll);
}

