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
#include "LexiDialogAnimProperties.h"
#include "LexiExportObjectBoneAnim.h"
#include "LexiOgreMeshCompiler.h"
#include "LexiOgreMaterialCompiler.h"
#include "LexiIntermediateBuilder.h"
#include "LexiIntermediateMesh.h"

#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
//

CAnimPropertiesDlg *CBoneAnimExportObject::m_pEditDlg=NULL;
CDDObject* CBoneAnimExportObject::m_pDDMetaDesc=NULL;

CBoneAnimExportObject::CBoneAnimExportObject(CDDObject *pConfig) : CExportObject(pConfig)
{
	REGISTER_MODULE("Bone Animation Export Object")
	//m_pDDMetaDesc = BuildMetaDesc();
	//m_pDDMetaDesc->SaveASCII("C:\\METADESC.ddconf");
}

CBoneAnimExportObject::~CBoneAnimExportObject()
{
	UNREGISTER_MODULE
}

// Get window for editing ExportObject properties
GDI::Window* CBoneAnimExportObject::GetEditWindow(GDI::Window *pParent)
{
	if(m_pEditDlg==NULL)
	{
		m_pEditDlg=new CAnimPropertiesDlg(pParent);
		m_pEditDlg->Create();		
		m_pDDMetaDesc=BuildMetaDesc();
		m_pEditDlg->Init(m_pDDMetaDesc, ".Skeleton");
	}

	m_pEditDlg->SetInstance(m_pDDConfig, this);
	return m_pEditDlg;
}

// End edit
void CBoneAnimExportObject::CloseEditWindow()
{
	if(m_pEditDlg!=NULL)
	{
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
bool CBoneAnimExportObject::OnCreate(CExporterPropertiesDlg *pPropDialog)
{
	CExportObject *pParent=GetParent();
	if(pParent==NULL) return false;
	std::string sName=pParent->GetName();
	sName+=" Animation";
	m_pDDConfig->SetString("Name", sName.c_str());
	return true;
}

// Check if ExportObject supports a given ExportObject instance as parent
bool CBoneAnimExportObject::SupportsParentType(const CExportObject *pParent) const
{	
	const IExportObjectSkinSupport* pMeshSupport;
	try {
		pMeshSupport = dynamic_cast<const IExportObjectSkinSupport*>(pParent);
		if(pMeshSupport == NULL)
			return false;
	} catch (...) 
	{
	  LOGERROR "Error while attempting to check compatible parent type.");
	}

	return true;

	// Bone animation requires a mesh parent
//	if(pParent==NULL || stricmp(pParent->GetType(), CSkinnedMeshExportObjectHelper::Type())!=0) return false;
//	return true;
}

//
CDDObject* CBoneAnimExportObject::BuildMetaDesc( void )
{
	int iStartFrame = GetAnimStart()/GetTicksPerFrame();
	int iEndFrame = GetAnimEnd()/GetTicksPerFrame();

	CDDObject* AnimContainer = new CDDObject();

	fastvector< const CDDObject* > lAnimSettings;
	CDDObject* pDDAnimElement;

	// --- General Animation ---

/*	// Animation Name
	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID","AnimationNameID");
	pDDAnimElement->SetString("Type","string");
	pDDAnimElement->SetString("Group","Animation");
	pDDAnimElement->SetString("Caption","Name");
	pDDAnimElement->SetString("Help","The Name of the Animation.");
	pDDAnimElement->SetBool("Default", "AnimName");
	lAnimSettings.push_back(pDDAnimElement);*/

	// Animation Type
	//pDDAnimElement = new CDDObject();
	//pDDAnimElement->SetString("ID","AnimationTypeID");
	//pDDAnimElement->SetString("Type","selection");
	//pDDAnimElement->SetString("Group","Animation");
	//vector< faststring > lTypes;
	//lTypes.push_back("Bone");	//0
	////lTypes.push_back("Morph");//1
	////lTypes.push_back("Pose");	//2
	//pDDAnimElement->SetStringList("Strings",lTypes);
	//pDDAnimElement->SetString("Caption","Type");
	//pDDAnimElement->SetString("Help","The Type of the Animation (Bone, Morph or Pose).");
	//pDDAnimElement->SetInt("Default", 0);
	//lAnimSettings.push_back(pDDAnimElement);

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
	pDDAnimElement->SetString("Group","Bone");
	pDDAnimElement->SetString("Caption","Start Frame");
	pDDAnimElement->SetString("Help","Frame which the animation begins");
	pDDAnimElement->SetString("Condition", "$AnimationTypeID=0");
	pDDAnimElement->SetInt("Default", iStartFrame);
	lAnimSettings.push_back(pDDAnimElement);

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID","AnimationEndID");
	pDDAnimElement->SetString("Type","Int");
	pDDAnimElement->SetBool("EnableSlider", false);
	pDDAnimElement->SetString("Group","Bone");
	pDDAnimElement->SetString("Caption","End Frame");
	pDDAnimElement->SetString("Help","Frame which the animation stops");
	pDDAnimElement->SetString("Condition", "$AnimationTypeID=0");
	pDDAnimElement->SetInt("Default", iEndFrame);
	lAnimSettings.push_back(pDDAnimElement);

	pDDAnimElement = new CDDObject();
	pDDAnimElement->SetString("ID","AnimationSampleRateID");
	pDDAnimElement->SetString("Type","Float");
	pDDAnimElement->SetBool("EnableSlider", false);
	pDDAnimElement->SetString("Group","Bone");
	pDDAnimElement->SetString("Caption","Samplerate");
	pDDAnimElement->SetString("Help","Number of samples per second");
	pDDAnimElement->SetString("Condition", "$AnimationTypeID=0");
	pDDAnimElement->SetFloat("Default", (float)GetFrameRate());
	lAnimSettings.push_back(pDDAnimElement);

	AnimContainer->SetDDList("MetaList", lAnimSettings, false);
	return AnimContainer;
}

bool CBoneAnimExportObject::Export(CExportProgressDlg *pProgressDlg, bool bForceAll)
{	

	if(m_bEnabled || bForceAll)
	{
		// Do bone export
		CIntermediateBuilder::Get()->GetSkeletonBuilder()->CreateAnimation( m_pDDConfig,pProgressDlg );

	}
	return CExportObject::Export(pProgressDlg, bForceAll);	
}

//
/*
const char* CBoneAnimExportObject::GetDefaultFileExt() const
{
	return "skeleton";
}*/

//

