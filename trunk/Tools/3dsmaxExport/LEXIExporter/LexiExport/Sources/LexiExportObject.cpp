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
#include "LexiMaxExport.h"
#include "LexiDialogObjectProperties.h"
#include "LexiDialogProgress.h"

#include "..\Res\resource.h"

//

#pragma warning(disable:4073)
#pragma init_seg(lib)
#pragma warning(default:4073)

//

typedef struct {

	std::string m_sType;
	std::string m_sTypeName;
	CExportObject::Construct_FN m_pfnConstruct;
	unsigned int m_iIconResID;
	HICON m_hIcon;

} OBJECT;

typedef std::map<std::string, OBJECT> ObjectMap;
static ObjectMap m_ObjectMap;

bool CExportObject::RegisterObject(const char* pszType, const char* pszTypeName, unsigned int iIcon, Construct_FN pfnConstruct)
{
	OBJECT o;
	o.m_sType = pszType;
	o.m_sTypeName = pszTypeName;
	o.m_pfnConstruct = pfnConstruct;
	o.m_iIconResID = iIcon;

	m_ObjectMap[pszType] = o;

	return true;
}

void CExportObject::Initialize()
{
	for(ObjectMap::iterator it = m_ObjectMap.begin(); it != m_ObjectMap.end(); it++)
	{
		OBJECT& o = it->second;
		o.m_hIcon = ::LoadIcon(CExporter::m_hInstance, MAKEINTRESOURCE(o.m_iIconResID));
	}
}

CExportObject* CExportObject::Construct(const char* pszType)
{
	ObjectMap::const_iterator it = m_ObjectMap.find(pszType);
	return it != m_ObjectMap.end() ? it->second.m_pfnConstruct() : NULL;
}

void CExportObject::EnumObjects(std::vector<Desc>& objlist)
{
	for(ObjectMap::const_iterator it = m_ObjectMap.begin(); it != m_ObjectMap.end(); it++)
	{
		const OBJECT& o = it->second;

		Desc d;
		d.m_pszType = o.m_sType.c_str();
		d.m_pszTypeName = o.m_sTypeName.c_str();
		d.m_hIcon = o.m_hIcon;

		objlist.push_back(d);
	}
}

//

CExportProgressDlg* CExportObject::m_pExportProgressDlg = NULL;

//

CExportObject::CExportObject(const char* pszType)
{
	m_sType = pszType;
	m_iID = 0xffffffff;
	m_sName = "<unnamed>";
	m_sFilename = "<unknown>";
	m_pSceneNode = NULL;
	m_pDDMetaDesc = NULL;

	m_pDDEditMeta = new CDDObject();
//	m_pDDEditMeta->AddRef();
}

CExportObject::~CExportObject()
{
	if(m_pSceneNode) delete m_pSceneNode;
}

//

void CExportObject::Release()
{
	if(m_pDDEditMeta)
		m_pDDEditMeta->Release();
	delete this;
}

//

void CExportObject::Read(CDDObject* pConfig)
{
	m_iID = pConfig->GetInt("id", 0xffffffff);
	m_sName = pConfig->GetString("name", "<unnamed>");
	m_sFilename = pConfig->GetString("filename", "<unknown>");

	CDDObject* tmpDD = pConfig->GetDDObject("EditedSettings");
	if(tmpDD != NULL) 
	{
		m_pDDEditMeta->Release();
		m_pDDEditMeta = tmpDD;
		m_pDDEditMeta->AddRef();
	}
}

void CExportObject::Write(CDDObject* pConfig) const
{
	pConfig->SetInt("id", m_iID);
	pConfig->SetString("name", m_sName.c_str());
	pConfig->SetString("filename", m_sFilename.c_str());
	pConfig->SetDDObject("EditedSettings", m_pDDEditMeta);
}

//

bool CExportObject::CreateSceneNode()
{
	if(m_pSceneNode)
	{
		delete m_pSceneNode;
		m_pSceneNode = NULL;
	}

	return false;
}

//

const char* CExportObject::GetType() const
{
	return m_sType.c_str();
}

const char* CExportObject::GetTypeName() const
{
	return m_ObjectMap[GetType()].m_sTypeName.c_str();
}

//

void CExportObject::SetID(unsigned int iID)
{
	m_iID = iID;
}

unsigned int CExportObject::GetID() const
{
	return m_iID;
}

//

void CExportObject::SetName(const char* pszName)
{
	m_sName = pszName;
}

const char* CExportObject::GetName() const
{
	return m_sName.c_str();
}

//

void CExportObject::SetFilename(const char* pszFilename)
{
	m_sFilename = pszFilename;
}

const char* CExportObject::GetFilename() const
{
	return m_sFilename.c_str();
}

//

bool CExportObject::Edit(GDI::Window* pParent, const char* pszTitle, unsigned int iInitSelectedID)
{
	CObjectPropertiesDlg dlg(pParent, this);
	dlg.m_sTitle = pszTitle;

	char gbtitle[256];
	sprintf(gbtitle, "%s Properties", GetTypeName());
	dlg.m_sGBTitle = gbtitle;

	dlg.m_sName = GetName();
	dlg.m_iID = GetID();
	dlg.m_sFilename = GetFilename();
	dlg.m_iInitFromSelected = iInitSelectedID;

	if(dlg.DoModal() != IDOK) return false;

	SetName(dlg.m_sName.c_str());
	SetID(dlg.m_iID);
	SetFilename(dlg.m_sFilename.c_str());

	return true;
}

//

void CExportObject::OutputProgress(const char* pszText, unsigned int iLevel) const
{
	if(m_pExportProgressDlg) m_pExportProgressDlg->Output(pszText, iLevel);
}

//

const char* CExportObject::GetDefaultFileExt() const
{
	return "export";
}

///////////////////////////////////////////////////////////

const char* GetNameFromID(unsigned int iID)
{
	if(iID == 0) return "<SceneRoot>";

	INode* pNode = GetNodeFromID(iID);
	return pNode ? pNode->GetName() : "<invalid>";
}

//

static INode* GetNodeFromID(unsigned int iID, INode* pNode)
{
	if(pNode->GetHandle() == iID) return pNode;

	unsigned int iNumChildren = pNode->NumberOfChildren();
	for(unsigned int x = 0; x < iNumChildren; x++)
	{
		INode* pSubNode = GetNodeFromID(iID, pNode->GetChildNode(x));
		if(pSubNode) return pSubNode;
	}

	return NULL;
}

INode* GetNodeFromID(unsigned int iID)
{
	return GetNodeFromID(iID, CExporter::GetMax()->GetRootNode());
}

//

SClass_ID GetClassIDFromNodeID(unsigned int iID)
{
	INode* pNode = GetNodeFromID(iID);
	return pNode ? GetClassIDFromNode(pNode) : NULL;
}

//

SClass_ID GetClassIDFromNode(INode* pNode)
{
	const ObjectState& os = pNode->EvalWorldState(0, TRUE);
	return os.obj ? os.obj->SuperClassID() : NULL;
}

//

