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
#include "LexiDialogPoseProperties.h"
#include "LexiExportObjectPoseAnim.h"
#include "LexiOgreMeshCompiler.h"
#include "LexiOgreMaterialCompiler.h"
#include "LexiIntermediateBuilder.h"
#include "LexiIntermediateMesh.h"

#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
//

CPosePropertiesDlg *CPoseAnimExportObject::m_pEditDlg=NULL;
CDDObject* CPoseAnimExportObject::m_pDDMetaDesc=NULL;

CPoseAnimExportObject::CPoseAnimExportObject(CDDObject *pConfig) : CExportObject(pConfig)
{
	REGISTER_MODULE("Pose Animation Export Object")
}

CPoseAnimExportObject::~CPoseAnimExportObject()
{
	UNREGISTER_MODULE
}

// Get window for editing ExportObject properties
GDI::Window* CPoseAnimExportObject::GetEditWindow(GDI::Window *pParent)
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
void CPoseAnimExportObject::CloseEditWindow()
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
bool CPoseAnimExportObject::OnCreate(CExporterPropertiesDlg *pPropDialog)
{
	CExportObject *pParent=GetParent();
	if(pParent==NULL) return false;
	std::string sName=pParent->GetName();
	sName+=" Animation";
	m_pDDConfig->SetString("Name", sName.c_str());
	return true;
}

// Check if ExportObject supports a given ExportObject instance as parent
bool CPoseAnimExportObject::SupportsParentType(const CExportObject *pParent) const
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
CDDObject* CPoseAnimExportObject::BuildMetaDesc( void )
{
	int iStartFrame = GetAnimStart()/GetTicksPerFrame();
	int iEndFrame = GetAnimEnd()/GetTicksPerFrame();

	CDDObject* AnimContainer = new CDDObject();

	fastvector< const CDDObject* > lAnimSettings;
	CDDObject* pDDAnimElement;

	// --- General Animation ---

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID","AnimationOptimizeID");
	pDDAnimElement->SetString("Type","Bool");
	pDDAnimElement->SetString("Group","Animation");
	pDDAnimElement->SetString("Caption","Optimize");
	pDDAnimElement->SetString("Help","Reduces the amount of KeyFrames by removing redundant data.");
	pDDAnimElement->SetBool("Default", true);
	lAnimSettings.push_back(pDDAnimElement);

	// --- Bone Animation ---

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID","AnimationStartID");
	pDDAnimElement->SetString("Type","Int");
	pDDAnimElement->SetBool("EnableSlider", false);
	pDDAnimElement->SetString("Group","Animation");
	pDDAnimElement->SetString("Caption","Start Frame");
	pDDAnimElement->SetString("Help","Frame which the animation begins");
	pDDAnimElement->SetInt("Default", iStartFrame);
	lAnimSettings.push_back(pDDAnimElement);

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID","AnimationEndID");
	pDDAnimElement->SetString("Type","Int");
	pDDAnimElement->SetBool("EnableSlider", false);
	pDDAnimElement->SetString("Group","Animation");
	pDDAnimElement->SetString("Caption","End Frame");
	pDDAnimElement->SetString("Help","Frame which the animation stops");
	pDDAnimElement->SetInt("Default", iEndFrame);
	lAnimSettings.push_back(pDDAnimElement);

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID","AnimationSampleRateID");
	pDDAnimElement->SetString("Type","Float");
	pDDAnimElement->SetBool("EnableSlider", false);
	pDDAnimElement->SetString("Group","Animation");
	pDDAnimElement->SetString("Caption","Samplerate");
	pDDAnimElement->SetString("Help","Number of samples per second");
	pDDAnimElement->SetFloat("Default", (float)GetFrameRate());
	lAnimSettings.push_back(pDDAnimElement);

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID","OptimizeID");
	pDDAnimElement->SetString("Type","Bool");
	pDDAnimElement->SetString("Group","Animation");
	pDDAnimElement->SetString("Caption","Optimize");
	pDDAnimElement->SetString("Help","Rate at which samples should be done. e.g 2 yields every second frame in max.");
	pDDAnimElement->SetBool("Default", true);
	lAnimSettings.push_back(pDDAnimElement);
	

	AnimContainer->SetDDList("MetaList", lAnimSettings, false);
	return AnimContainer;
}

bool CPoseAnimExportObject::Export(CExportProgressDlg *pProgressDlg, bool bForceAll)
{	

	if(m_bEnabled || bForceAll)
	{
		// Do pose animation export

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

		//// Check vertex count before adding pose description to Intermediate Mesh
		//CMeshArray* lVerts = pIMesh->GetArray("position", 0);
		//int iOrigVertCount = lVerts->Size();

		int iStartFrame = m_pDDConfig->GetInt("AnimationStartID",0);
		int iEndFrame = m_pDDConfig->GetInt("AnimationEndID",0);
		float fRate = m_pDDConfig->GetFloat("AnimationSampleRateID",(float)GetFrameRate());
		if(fRate < 0.0001f) fRate = 1.0f;
		fRate = ((float)GetFrameRate()) / fRate;

		faststring sPoseAnimName = m_pDDConfig->GetString("Name");
		bool bOptimize = m_pDDConfig->GetBool("OptimizeID",true);


		//lVerts = pIMesh->GetArray("position",iFrame);
		//int iPoseVertCount = lVerts->Size();
		//if(iPoseVertCount != iOrigVertCount)
		//{
		//	LOGERROR "Pose(%s) does not contain same amount of verticies as base mesh. Pose Ignored", sPoseAnimName.c_str() );
		//	return false;
		//}

		pIMesh->AddPoseAnimation(sPoseAnimName.c_str(), iStartFrame, iEndFrame, fRate, bOptimize);

	}

	return CExportObject::Export(pProgressDlg, bForceAll);	
}
