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
#include "LexiMaxExportDesc.h"
#include "LexiDialogProperties.h"
#include "LexiDialogProgress.h"

#include "../res/resource.h"

//

CExporter* CExporter::m_pThis = NULL;
HINSTANCE CExporter::m_hInstance = NULL;
Interface* CExporter::m_pMax = NULL;

//

static void ModifyCallback(void* param, NotifyInfo* info)
{
	switch(info->intcode)
	{
		case NOTIFY_SYSTEM_POST_RESET:
		case NOTIFY_SYSTEM_POST_NEW:
		case NOTIFY_FILE_POST_MERGE:
		case NOTIFY_FILE_POST_OPEN:
		case NOTIFY_POST_IMPORT:
			((CExporter*)param)->LoadConfig();
			break;
	}
}

//

CExporter::CExporter(CExporterDesc* pDesc)
{
	CExportObject::Initialize();

	m_pDesc = pDesc;
	m_pMax = NULL;
	m_pMaxUtil = NULL;
	m_hPanel = NULL;
	m_pThis = this;

	m_pSettings = new CDDObject;

	//

	RegisterNotification(ModifyCallback, this, NOTIFY_SYSTEM_POST_RESET);
	RegisterNotification(ModifyCallback, this, NOTIFY_SYSTEM_POST_NEW);
	RegisterNotification(ModifyCallback, this, NOTIFY_FILE_POST_OPEN);
	RegisterNotification(ModifyCallback, this, NOTIFY_FILE_POST_MERGE);
	RegisterNotification(ModifyCallback, this, NOTIFY_POST_IMPORT);
}

CExporter::~CExporter()
{
	UnRegisterNotification(ModifyCallback, this, NOTIFY_SYSTEM_POST_RESET);
	UnRegisterNotification(ModifyCallback, this, NOTIFY_SYSTEM_POST_NEW);
	UnRegisterNotification(ModifyCallback, this, NOTIFY_FILE_POST_OPEN);
	UnRegisterNotification(ModifyCallback, this, NOTIFY_FILE_POST_MERGE);
	UnRegisterNotification(ModifyCallback, this, NOTIFY_POST_IMPORT);

	//

	GDI::Window::Cleanup();

	//

	FreeConfig();
	m_pSettings->Release();
}

//

void CExporter::FreeConfig()
{
	unsigned int iNumObjects = m_Config.size();
	for(unsigned int x = 0; x < iNumObjects; x++)
	{
		CExportObject* pObj = m_Config[x];
		pObj->Release();
	}
	m_Config.clear();

	m_pSettings->Clear();

	RefreshButtons();
}

void CExporter::LoadConfig()
{
	FreeConfig();

	PROPSPEC ps;
	ps.ulKind = PRSPEC_LPWSTR;
	ps.lpwstr = NDS_PROPERTY_NAME;

	int iPropIndex = m_pMax->FindProperty(PROPSET_USERDEFINED, &ps);
	if(iPropIndex == -1) return;

	const PROPVARIANT* pProperty = m_pMax->GetPropertyVariant(PROPSET_USERDEFINED, iPropIndex);
	if(pProperty->vt != VT_BLOB) return;

	//

	const BLOB& blob = pProperty->blob;

	CDataStream stream(blob.pBlobData, blob.cbSize, NULL);
	stream.SetPosition(0);
	LoadConfig(stream);

	RefreshButtons();
}

void CExporter::SaveConfig()
{
	CDataStream stream;
	SaveConfig(stream);

	//

	PROPSPEC ps;
	ps.ulKind = PRSPEC_LPWSTR;
	ps.lpwstr = NDS_PROPERTY_NAME;

	BLOB blob;
	blob.cbSize = stream.GetPosition();
	blob.pBlobData = (BYTE*)stream.GetBaseData();

	PROPVARIANT pv;
	PropVariantInit(&pv);
	pv.vt = VT_BLOB;
	pv.blob = blob;

	m_pMax->AddProperty(PROPSET_USERDEFINED, &ps, &pv);

	SetSaveRequiredFlag();

	//

	RefreshButtons();
}

//

void CExporter::LoadConfig(CDataStream& stream)
{
	stream.SetPosition(0);

	unsigned int iNumObjects = stream.GetInt();
	for(unsigned int x = 0; x < iNumObjects; x++)
	{
		const std::string& sType = stream.GetString();

		CDDObject* pDDObj = new CDDObject;
		pDDObj->FromDataStream(&stream);

		CExportObject* pObj = CExportObject::Construct(sType.c_str());
		pObj->Read(pDDObj);

		pDDObj->Release();

		m_Config.push_back(pObj);
	}

	m_pSettings->FromDataStream(&stream);
}

void CExporter::SaveConfig(CDataStream& stream)
{
	stream.SetPosition(0);

	unsigned int iNumObjects = m_Config.size();
	stream.AddInt(iNumObjects);

	for(unsigned int x = 0; x < iNumObjects; x++)
	{
		const CExportObject* pObj = m_Config[x];

		const char* pszType = pObj->GetType();
		stream.AddString(pszType);

		CDDObject* pDDObj = new CDDObject;
		pObj->Write(pDDObj);

		pDDObj->ToDataStream(&stream);
		pDDObj->Release();
	}

	m_pSettings->ToDataStream(&stream);
}

//

void CExporter::RefreshButtons()
{
	if(!m_hPanel) return;

	PROPSPEC ps;
	ps.ulKind = PRSPEC_LPWSTR;
	ps.lpwstr = NDS_PROPERTY_NAME;
	bool bGotConfig = (m_pMax->FindProperty(PROPSET_USERDEFINED, &ps) != -1) ? true : false;

	EnableWindow(GetDlgItem(m_hPanel, IDC_EXPORT_BUTTON), bGotConfig && m_Config.size() ? TRUE : FALSE);
}

//

void CExporter::BeginEditParams(Interface* ip, IUtil* iu)
{

	try
	{
		m_pMax = ip;
		m_pMaxUtil = iu;

		m_hPanel = ip->AddRollupPage(m_hInstance, MAKEINTRESOURCE(IDD_DIALOG_CONFIG), ConfigDlgProc, NDS_EXPORTER_TITLE, 0);


		if (COgreCore::getSingletonPtr() == NULL)
		{
			new COgreCore(m_hPanel);
		}

		LoadConfig();

	}
	catch (...)
	{
		MessageBox(NULL,"Error initializing LEXIEXporter!", "ERROR", MB_ICONERROR);
	}

}

void CExporter::EndEditParams(Interface* ip, IUtil* iu)
{
	FreeConfig();

	ip->DeleteRollupPage(m_hPanel);
	m_hPanel = NULL;

	m_pMax = NULL;
	m_pMaxUtil = NULL;
}

void CExporter::DeleteThis()
{
	delete this;
}

//

INT_PTR CALLBACK CExporter::ConfigDlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
		case WM_INITDIALOG:
			break;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_PROPERTIES_BUTTON:
					m_pThis->OnPanelButtonProperties();
					break;

				case IDC_EXPORT_BUTTON:
					m_pThis->OnPanelButtonExport();
					break;

				case IDC_CLOSE_BUTTON:
					m_pThis->m_pMaxUtil->CloseUtility();
					break;

				case IDOK:
				case IDCANCEL:
					break;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			m_pThis->m_pMax->RollupMouseMessage(hWnd, Msg, wParam, lParam); 
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

//

void CExporter::OnPanelButtonProperties()
{
	CExporterPropertiesDlg dlg(GDI::Window::FromHandle(m_pMax->GetMAXHWnd()), m_pMax, m_pMaxUtil, &m_Config, m_pSettings);
	int iRtnCode = dlg.DoModal();

	if(dlg.m_bChanges) SaveConfig();

	RefreshButtons();

	std::vector<unsigned int> selectionlist;
	if(iRtnCode == 100 || iRtnCode == 101)
	{
		if(iRtnCode == 101)
		{
			dlg.GetLastSelection(selectionlist);
		}
		else
		{
			for(unsigned int x = 0; x < m_Config.size(); x++) selectionlist.push_back(x);
		}

		ExportItems(selectionlist);
	}
}

//

void CExporter::OnPanelButtonExport()
{
	std::vector<unsigned int> selectionlist;
	for(unsigned int x = 0; x < m_Config.size(); x++) selectionlist.push_back(x);

	ExportItems(selectionlist);
}

//

void CExporter::ExportItems(const std::vector<unsigned int>& selectionlist)
{
	if(!selectionlist.size()) return;

	CExportProgressDlg dlg(NULL);
	dlg.Create(IDD_DIALOG_EXPORTPROGRESS, GDI::Window::FromHandle(m_pMax->GetMAXHWnd()));
	dlg.ShowWindow(SW_SHOW);

	char temp[16384];
	sprintf(temp, "Exporting %i item(s)", selectionlist.size());
	dlg.Output(temp, 1);

	CExportObject::m_pExportProgressDlg = &dlg;

	unsigned int iNumSelections = selectionlist.size();
	for(unsigned int x = 0; x < iNumSelections; x++)
	{
		CExportObject* pExpObj = m_Config[selectionlist[x]];

		sprintf(temp, "%s: \"%s\" ID: %i -> \"%s\"", pExpObj->GetTypeName(), pExpObj->GetName(), pExpObj->GetID(), pExpObj->GetFilename());
		dlg.Output(temp, 0);

		if(!pExpObj->Export())
		{
		}
	}

	CExportObject::m_pExportProgressDlg = NULL;

	dlg.MessageBox("Done", NDS_EXPORTER_TITLE, MB_ICONINFORMATION);
	dlg.DestroyWindow();
}

//

Interface* CExporter::GetMax()
{
	return m_pMax;
}

//

