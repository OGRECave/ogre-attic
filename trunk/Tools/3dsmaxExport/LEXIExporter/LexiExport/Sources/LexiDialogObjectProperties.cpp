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
#include "LexiExportObject.h"
#include "LexiDialogSelectNode.h"

#include "..\Res\resource.h"

#define _PRINT_DEBUG_INFO_
//

CObjectPropertiesDlg::CObjectPropertiesDlg(Window* pParent, CExportObject* pObj) : Dialog(IDD_DIALOG_OBJECTPROPERTIES, DlgProc, pParent)
{
	m_pObj = pObj;
}

//

INT_PTR CALLBACK CObjectPropertiesDlg::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CObjectPropertiesDlg* pThis = (CObjectPropertiesDlg*)Window::GetMapping(hWnd);

	switch(message)
	{
		case WM_INITDIALOG:
		{
			pThis = (CObjectPropertiesDlg*)lParam;
			Window::AddMapping(pThis, hWnd);

			pThis->OnInitDialog();

		}	return 0;

		case WM_CLOSE:
		{
			pThis->OnCancel();
			pThis->EndDialog(IDCANCEL);
		}	break;

		case WM_NOTIFY_MESSAGE_ID:
		{
			pThis->UpdateItemList();
		}	break;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
					pThis->OnOK();
					pThis->EndDialog(IDOK);
					break;

				case IDCANCEL:
					pThis->OnCancel();
					pThis->EndDialog(IDCANCEL);
					break;

				case IDC_BROWSENODE:
					pThis->BrowseNode();
					break;

				case IDC_BUTTON_ADD_ANIMATION:
					pThis->AddAnimation();
					break;
				case IDC_BUTTON_REMOVE_ANIMATION:
					pThis->RemoveAnimationData();
					break;
			}
		}	break;

		case WM_NOTIFY:
		{
			const NMHDR* pHdr = (const NMHDR*)lParam;
			switch(pHdr->idFrom)
			{
				case IDC_ANIMATION_LIST:
					if(pHdr->code == LVN_ITEMCHANGED)
					{
						pThis->DoAnimationSelected();
					}
					else if(pHdr->code == NM_DBLCLK)
					{
						//pThis->OnConfigButtonProperties();
					}
					else if(pHdr->code == LVN_KEYDOWN)
					{
						const NMLVKEYDOWN* pKey = (const NMLVKEYDOWN*)pHdr;
						if(pKey->wVKey == VK_DELETE)
						{
							//pThis->OnConfigButtonRemove();
						}
					}
					break;
			}		
		}	return 1;
	}

	return 0;
}

//

void CObjectPropertiesDlg::OnInitDialog()
{

	Bind(IDC_ANIMATION_LIST, m_ItemList);
	Bind(IDC_BUTTON_REMOVE_ANIMATION, m_ButtonRemove);

	SetWindowText(m_sTitle.c_str());

	GetDlgItem(IDC_GROUPBOX_TITLE)->SetWindowText(m_sGBTitle.c_str());
	GetDlgItem(IDC_NAME)->SetWindowText(m_sName.c_str());
	GetDlgItem(IDC_FILENAME)->SetWindowText(m_sFilename.c_str());

	// Ready the animation list
	m_lAnimMap.clear();
	m_lAnimNames.clear();
	
	if(m_iInitFromSelected)
	{
		m_iID = m_iInitFromSelected;
		ForceUpdateMembers();
	}
	else if(strcmp(m_sName.c_str(),"<unnamed>")==0)
	{
		BrowseNode();
	}

	GetDlgItem(IDC_NODE)->SetWindowText(GetNameFromID(m_iID));

	//

	RECT mainrc;
	GetClientRect(mainrc);
	ClientToScreen(mainrc);


	// -- Create Animation list

	RECT rc;
	m_ItemList.GetClientRect(rc);
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	int w1 = 106;
	int w2 = 115;
	int w0 = w - w1 - w2 - ::GetSystemMetrics(SM_CXVSCROLL);

	m_ItemList.SetExtendedStyles(LVS_EX_FULLROWSELECT, 0xffffffff);

	m_ItemList.InsertColumn(0, "Name", LVCFMT_LEFT, w0);
	m_ItemList.InsertColumn(1, "Type", LVCFMT_LEFT, w1);
	m_ItemList.InsertColumn(2, "Parameters", LVCFMT_LEFT, w2);



	// -- animation list created

	RECT childrc;

	GetDlgItem(IDC_OBJECT_CHILD)->GetClientRect(childrc);
	GetDlgItem(IDC_OBJECT_CHILD)->ClientToScreen(childrc);

	int iChildWidth = childrc.right - childrc.left;
	int iChildHeight = childrc.bottom - childrc.top;

	int iChildX = childrc.left - mainrc.left;
	int iChildY = childrc.top - mainrc.top;

	RECT animchildrc;

	GetDlgItem(IDC_ANIMATION_PROPERTIES)->GetClientRect(animchildrc);
	GetDlgItem(IDC_ANIMATION_PROPERTIES)->ClientToScreen(animchildrc);

	int iAnimChildWidth = animchildrc.right - animchildrc.left;
	int iAnimChildHeight = animchildrc.bottom - animchildrc.top;

	int iAnimChildX = animchildrc.left - mainrc.left;
	int iAnimChildY = animchildrc.top - mainrc.top;

	CDDObject* pMetaDesc = m_pObj->GetMetaDesc();
	CDDObject* pEditMeta = m_pObj->GetEditMeta();

	//pEditMeta->SaveASCII("C:\\EditMeta_onLoad.txt");

	if(pMetaDesc)
	{
		m_hMetaWnd = ::CreateWindow("MetaControl", NULL, WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, iChildX, iChildY, iChildWidth, iChildHeight, m_hWnd, NULL, CExporter::m_hInstance, NULL);

		m_pMetaCtrl = new GDI::MetaControl;
		m_pMetaCtrl->Attach(m_hMetaWnd);
		m_pMetaCtrl->SetData(pEditMeta);
		m_pMetaCtrl->CreateFromMeta(pMetaDesc);

		m_hAnimMetaWnd = ::CreateWindow("MetaControl", NULL, WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, iAnimChildX, iAnimChildY, iAnimChildWidth, iAnimChildHeight, m_hWnd, NULL, CExporter::m_hInstance, NULL);
		//DWORD returnVal = GetLastError();

		CDDObject* pAnimationDD = pEditMeta->GetDDObject("AnimationDataContainer");
		if(pAnimationDD == NULL)
		{
			pAnimationDD = new CDDObject();
			pEditMeta->SetDDObject("AnimationDataContainer",pAnimationDD);
		}

		// Retrieve animations
		fastvector< const CDDObject* > lAnims = pAnimationDD->GetDDList("Animations");
		LoadAnimationData(lAnims);
		//pAnimationDD->RemoveData("Animations");
		//lAnims.clear();


		m_pDummyDD = new CDDObject();

		m_pAnimMetaCtrl = new GDI::MetaControl;
		m_pAnimMetaCtrl->Attach(m_hAnimMetaWnd);
		m_pAnimMetaCtrl->SetData(m_pDummyDD);
		m_pAnimMetaCtrl->CreateFromMeta(ExtractAnimationMeta(pMetaDesc));

		//pMetaDesc->Release();
	}

	CenterWindow();
	PopulateItemList();
	//UpdateStuff();
	DoAnimationSelected();

	GotoDlgCtrl(IDC_NODE);
}

//

void CObjectPropertiesDlg::OnOK()
{
	m_sName = GetDlgItem(IDC_NAME)->GetWindowText();
	m_sFilename = GetDlgItem(IDC_FILENAME)->GetWindowText();


	//

	m_pMetaCtrl->Detach();
	::DestroyWindow(m_hMetaWnd);
	delete m_pMetaCtrl;

	m_pAnimMetaCtrl->Detach();
	::DestroyWindow(m_hAnimMetaWnd);
	delete m_pAnimMetaCtrl;

	//

	CDDObject* pAnimationDD = m_pObj->GetEditMeta()->GetDDObject("AnimationDataContainer");
	fastvector< CDDObject* > lAnims = m_lAnimMap.data();

	fastvector< const CDDObject*> lConstAnims;
	for(unsigned i=0;i<lAnims.size();i++)
	{
		CDDObject* listDD = lAnims[i];
		const CDDObject* tmpDD = new CDDObject(*listDD);
		//tmpDD->MergeWith(listDD);
		lConstAnims.push_back( tmpDD );
		//listDD->Release();
	}
	pAnimationDD->SetDDList("Animations", lConstAnims);

	//pAnimationDD->SaveASCII("C:\\OnOK.ddconf");
	//

	//CDDObject* pEditMeta = m_pObj->GetEditMeta();
	//pEditMeta->SaveASCII("C:\\EditMeta_OnOK.txt");

	m_lAnimMap.clear();
	m_lAnimNames.clear();

}

void CObjectPropertiesDlg::OnCancel()
{

	//CDDObject* pEditMeta = m_pObj->GetEditMeta();
	//pEditMeta->SaveASCII("C:\\EditMeta_OnCancel.txt");

	m_pMetaCtrl->Detach();
	::DestroyWindow(m_hMetaWnd);
	delete m_pMetaCtrl;

	m_pAnimMetaCtrl->Detach();
	::DestroyWindow(m_hAnimMetaWnd);
	delete m_pAnimMetaCtrl;

	m_lAnimMap.clear();
	m_lAnimNames.clear();
}

//

void CObjectPropertiesDlg::BrowseNode()
{
	CSelectNodeDlg dlg(this, m_pObj);
	dlg.m_iNode = m_iID;

	if(dlg.DoModal() != IDOK) return;

	m_iID = dlg.m_iNode;
	GetDlgItem(IDC_NODE)->SetWindowText(GetNameFromID(m_iID));

	ForceUpdateMembers();
}

//

void CObjectPropertiesDlg::ForceUpdateMembers()
{
	std::string sTemp = GetDlgItem(IDC_NAME)->GetWindowText();
	if(!sTemp.size() || !stricmp(sTemp.c_str(), "<unnamed>"))
	{
		sTemp = GetNameFromID(m_iID);
		RemoveIllegalChars(sTemp);
		GetDlgItem(IDC_NAME)->SetWindowText(sTemp.c_str());
	}

	sTemp = GetDlgItem(IDC_FILENAME)->GetWindowText();
	if(!sTemp.size() || !stricmp(sTemp.c_str(), "<unknown>"))
	{
		sTemp = GetNameFromID(m_iID);
		sTemp += ".";
		sTemp += m_pObj->GetDefaultFileExt();
		RemoveIllegalChars(sTemp);
		sTemp = "C:\\" + sTemp;
		GetDlgItem(IDC_FILENAME)->SetWindowText(sTemp.c_str());
	}
}


void CObjectPropertiesDlg::PopulateItemList()
{
	UpdateItemList();
}

void CObjectPropertiesDlg::UpdateItemList( void )
{
	m_ItemList.DeleteAllItems();

	std::vector< std::string > lTypes;
	lTypes.push_back("Bone");
	lTypes.push_back("Morph");
	lTypes.push_back("Pose");

	fastvector< CDDObject* > lAnims = m_lAnimMap.data();

	for(int i=0; i < m_lAnimNames.size(); i++)
	{
		CDDObject* pAnimData = FindAnimationData(m_lAnimNames[i].c_str());
	
		m_ItemList.InsertItem(i, pAnimData->GetString("AnimationNameID"));
		m_ItemList.SetItemText(i, 0, pAnimData->GetString("AnimationNameID"));
		unsigned int iAnimType = pAnimData->GetInt("AnimationTypeID");

		if( iAnimType < lTypes.size() )
			m_ItemList.SetItemText(i, 1, lTypes[iAnimType].c_str());

		m_ItemList.SetItemText(i, 2, pAnimData->GetString("AnimationNameID"));
	}
}

// Called when data object has changed
void CObjectPropertiesDlg::OnChanged(const CDDObject *pInstance, const char *pszKey)
{
	//CDDObject* pDDObj = NULL;
	//if( m_lAnimMap.find( pInstance->GetString("AnimationNameID"), pDDObj) )
	//{
		UpdateItemList();
	//}
	// Notify owning window about change
	//::SendMessage(this->m_hWnd, WM_NOTIFY_MESSAGE_ID, 0, (LPARAM)pszKey);
}

void CObjectPropertiesDlg::DoAnimationSelected()
{
	// Enable remove button
	int iNumSel = m_ItemList.GetSelectedCount();
	m_ButtonRemove.EnableWindow(iNumSel ? true : false);

	// Update animation property window
	int iSelIndex = m_ItemList.GetNextItem(-1, LVNI_SELECTED);
	CDDObject* pDDAnimData = FindAnimationDataFromIndex(iSelIndex);
	if(pDDAnimData != NULL)
	{
		m_pAnimMetaCtrl->SetData( pDDAnimData );
	}
}

CDDObject* CObjectPropertiesDlg::ExtractAnimationMeta( CDDObject* pDDObj )
{
	CDDObject* pAnimContainer = pDDObj->GetDDObject("AnimationContainer");
	return pAnimContainer;
}
//

void CObjectPropertiesDlg::AddAnimation( void )
{
	// Create DD object and add new ListControl entry.
	CDDObject* newAnimData = FindOrCreateAnimationData(m_ItemList.GetItemCount());
	UpdateItemList();
	DoAnimationSelected();
}

CDDObject* CObjectPropertiesDlg::FindAnimationDataFromIndex( int index )
{
	if( (index < 0) && (index > m_lAnimNames.size()) )
		return false;
	
	std::string animName = m_lAnimNames[index];

	CDDObject* pDDAnim = NULL;
	m_lAnimMap.find(animName.c_str(), pDDAnim);
	return pDDAnim;
}

CDDObject* CObjectPropertiesDlg::FindAnimationData( faststring animName )
{
	CDDObject* pDDObj= NULL;
	m_lAnimMap.find(animName.c_str(), pDDObj);
	return pDDObj;
}

CDDObject* CObjectPropertiesDlg::FindOrCreateAnimationData( unsigned int index )
{
	if(index < 0)
		return false;

	if(index == m_lAnimNames.size())
	{
		// append new animation
		CDDObject* newAnimData = new CDDObject();
		Ogre::String newName = "New Animation " + Ogre::StringConverter::toString( m_lAnimNames.size() );
		m_lAnimNames.push_back(newName);
		m_lAnimMap.map(newName.c_str(), newAnimData);
		return newAnimData;
	}

	std::string animName = m_lAnimNames[index];
	CDDObject* animData = NULL;
	m_lAnimMap.find(animName.c_str(), animData);
	return animData;
}

bool CObjectPropertiesDlg::RemoveAnimationData( void )
{
	int index = m_ItemList.GetNextItem(-1, LVNI_SELECTED);

	if( (index < 0) && (index > m_lAnimNames.size()) )
		return false;

	std::string animName = m_lAnimNames[index];
	if(m_lAnimMap.erase(animName.c_str()))
		m_lAnimNames.erase( m_lAnimNames.begin()+index );

	UpdateItemList();

	return true;
}

void CObjectPropertiesDlg::LoadAnimationData( fastvector< const CDDObject* > lAnimList)
{
	while(!lAnimList.empty())
	{
		const CDDObject* pAnimData = lAnimList.pop_back();
		CDDObject* pNewAnimData = new CDDObject( );
		pNewAnimData->MergeWith(pAnimData);
		pNewAnimData->SaveASCII("C:\\LoadAnimationData.ddconf");
		faststring animName = pNewAnimData->GetString("AnimationNameID");
		m_lAnimNames.push_back(animName.c_str());
		m_lAnimMap.map(animName.c_str(), pNewAnimData);
		// We override the animation list when pressing OK. We need to keep this reference if Cancel is pressed.
		//pAnimData->Release(); 
	}
}