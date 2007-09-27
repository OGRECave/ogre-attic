/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn
Lasse Tassing

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

CExportObject* CExportObject::Construct(CDDObject* pConfig)
{
	const char* pszType=pConfig->GetString("Type");
	ObjectMap::const_iterator it = m_ObjectMap.find(pszType);
	return it != m_ObjectMap.end() ? it->second.m_pfnConstruct(pConfig) : NULL;
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
CExportObject::CExportObject(CDDObject* pConfig)
{
	m_pDDConfig=pConfig;
	m_pDDConfig->AddRef();

	m_sType = m_pDDConfig->GetString("Type");		// Known to exist!
//	m_sName = m_pDDConfig->GetString("Name", "<unnamed>");
	
	m_pParent=NULL;	
	m_bEnabled=true;

	// Construct all child exports
	fastvector<const CDDObject*> lChildren=m_pDDConfig->GetDDList("Children");
	for(unsigned i=0;i<lChildren.size();i++)
	{
		CExportObject* pChild=Construct((CDDObject*)lChildren[i]);
		if(pChild != NULL)
			pChild->SetParent(this);
	}
}

CExportObject::~CExportObject()
{
	m_pDDConfig->Release();	

	// If we have a valid parent, we remove this instance now
	if(m_pParent) m_pParent->RemoveChild(this);	
}

CDDObject* CExportObject::GetConfig() const
{
	return m_pDDConfig;
}

void CExportObject::SetConfig(const CDDObject* pConfig)
{
	if(m_pDDConfig) m_pDDConfig->Release();
	m_pDDConfig = (CDDObject*)pConfig->Clone();
}

// Save configuration 
void CExportObject::SaveConfig(CDDObject* pOutput) const
{
	pOutput->MergeWith(m_pDDConfig);

	//m_pDDConfig->SaveASCII("C:\\RootConfig.ddconf");

	if(m_lChildren.size())
	{
		fastvector<const CDDObject*> lChildDD;

		for(unsigned i=0;i<m_lChildren.size();i++)
		{
			CDDObject* pChildConfig=new CDDObject;
			m_lChildren[i]->SaveConfig(pChildConfig);
			lChildDD.push_back(pChildConfig);
		}

		pOutput->SetDDList("Children", lChildDD, false);
	}
	else
	{
		// object has no children, so lets make sure its 
		pOutput->RemoveData("Children");
	}
}

void CExportObject::Release()
{
	// Make a copy of list of children
	// Each child will be removed from m_lChildren list when it is released
	std::vector<CExportObject*> lCopyList=m_lChildren;
	for(unsigned i=0;i<lCopyList.size();i++)
	{		
		lCopyList[i]->Release();
	}
	m_lChildren.clear();
	delete this;
}

const char* CExportObject::GetType() const
{
	return m_sType.c_str();
}

const char* CExportObject::GetTypeName() const
{
	return m_ObjectMap[GetType()].m_sTypeName.c_str();
}

const char* CExportObject::GetName() const
{
//	return m_sName.c_str();
	return m_pDDConfig->GetString("Name", "<unnamed>");
}

// Get additional description string
const char* CExportObject::GetDesc() const
{
	return m_sDesc.c_str();
}

//
// Enable/disable object during export (childs may still be exported)
void CExportObject::SetEnabled(bool bEnabled)
{
	m_bEnabled=bEnabled;
}
bool CExportObject::GetEnabled() const
{
	return m_bEnabled;
}

// Supports node class. Default implementation just returns false.
bool CExportObject::SupportsMAXNode(INode* pMAXNode) const
{
	return false;
}

// Get/Set selected MAX node. Default implementation read/writes it from
// the m_pDDConfig object on a key called "NodeID"
void CExportObject::SetMAXNodeID(unsigned int iMAXNodeID)
{
	m_pDDConfig->SetInt("NodeID", iMAXNodeID);
}
unsigned int CExportObject::GetMAXNodeID() const
{
	if(m_pDDConfig->GetKeyType("NodeID")!=DD_INT) return 0xFFFFFFFF;
	return m_pDDConfig->GetInt("NodeID");
}

// Set new parent object. This will automatically add current instance
// as child on the parent and remove it from the old parent (if available)
void CExportObject::SetParent(CExportObject* pParent)
{
	// Check if we have the same parent
	if(m_pParent==pParent) return;

	// If we have a valid parent, we remove us as child
	if(m_pParent!=NULL)
	{
		m_pParent->RemoveChild(this);
	}
	m_pParent=pParent;
	if(m_pParent!=NULL)
	{
		m_pParent->AddChild(this);
	}
}

CExportObject* CExportObject::GetParent() const
{
	return m_pParent;
}

// Add child to this instance
void CExportObject::AddChild(CExportObject* pChild)
{
	m_lChildren.push_back(pChild);
}

// Remove child from instance.
void CExportObject::RemoveChild(CExportObject* pChild)
{
	for(int i=0;i<m_lChildren.size();i++)
	{
		if(m_lChildren[i]==pChild)
		{
			m_lChildren.erase(m_lChildren.begin()+i);
			return;
		}
	}
}

// Get list of children attached to current instance
std::vector<CExportObject*> CExportObject::GetChildren() const
{
	return m_lChildren;
}
bool CExportObject::HasChildren() const
{
	return m_lChildren.size()!=0;
}

// Get number of children - optionally recurse to count all subchildren
unsigned int CExportObject::GetChildCount(bool bRecursive)
{
	unsigned int iCount = m_lChildren.size() + m_DefaultChildren.size();

	if(bRecursive)
	{
		for(unsigned int i = 0; i < m_lChildren.size(); i++)
		{
			iCount += m_lChildren[i]->GetChildCount(true);
		}
	}

	return iCount;
}

// Export object
bool CExportObject::Export(CExportProgressDlg* pProgressDlg, bool bForceAll)
{
	bool bOK = true;
	pProgressDlg->GlobalStep();

	// Iterate all children and call export on them
	for(unsigned int i = 0; i < m_lChildren.size(); i++)
	{
		// Check if user wants to abort current export
		if(pProgressDlg->CheckAbort() == true) return false;

		// Export child object
		if(!m_lChildren[i]->Export(pProgressDlg, bForceAll)) bOK = false;
	}

	// Iterate all default children and call export on them
	for(unsigned int x = 0; x < m_DefaultChildren.size(); x++)
	{
		// Check if user wants to abort current export
		if(pProgressDlg->CheckAbort() == true) return false;

		// Export default object
		if(!m_DefaultChildren[x]->Export(pProgressDlg, bForceAll)) bOK = false;
	}

	return bOK;
}

//

void CExportObject::PreExport()
{
	for(unsigned int x = 0; x < m_lChildren.size(); x++)
	{
		m_lChildren[x]->PreExport();
	}
}

void CExportObject::PostExport()
{
	for(unsigned int x = 0; x < m_lChildren.size(); x++)
	{
		m_lChildren[x]->PostExport();
	}

	//

	for(unsigned int x = 0; x < m_DefaultChildren.size(); x++)
	{
		CExportObject* pEO = m_DefaultChildren[x];
		pEO->Release();
	}
	m_DefaultChildren.clear();
}

//

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

