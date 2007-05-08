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
#include "LexiDialogViewLog.h"
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
		case NOTIFY_SYSTEM_POST_NEW:
		case NOTIFY_FILE_POST_MERGE:
		case NOTIFY_FILE_POST_OPEN:
		case NOTIFY_POST_IMPORT:
		case NOTIFY_SYSTEM_POST_RESET:
		case NOTIFY_FILE_OPEN_FAILED:
		case NOTIFY_IMPORT_FAILED:
			((CExporter*)param)->LoadConfig();
			break;
	}
}

// Singleton
CExporter* CExporter::Get()
{
	return m_pThis;
}

// Global settings
CDDObject* CExporter::GetGlobalSettings()
{
	return m_pGlobalSettings;
}

CDDObject* CExporter::GetRootConfig() const
{
	return m_pExportRoot->GetConfig();
}

//

CExporter::CExporter(CExporterDesc* pDesc)
{
	REGISTER_MODULE("Exporter")

	LoadGlobalSettings();
	CExportObject::Initialize();

	// We always write Logs to the <MAXDIR>\LEXIExporter\Logs Directory
	//--
	char szAppPath[MAX_PATH] = "";
	::GetModuleFileName(NULL,szAppPath,sizeof(szAppPath) - 1);
	Ogre::String cwd(szAppPath);
	Ogre::String fileNameTMP, filePath;
	Ogre::StringUtil::splitFilename(cwd, fileNameTMP, filePath);
	int n = filePath.find("/");
	while(n != Ogre::String::npos)
	{
		filePath.replace(n,1,"\\");
		n = filePath.find("/");
	}
	filePath+="LEXIExporter\\Logs\\LexiExport.log";
	//--

	CLogSystem::Get()->AddReceiver(new CFileLogger(filePath.c_str()));

	m_pDesc = pDesc;
	m_pMax = NULL;
	m_pMaxUtil = NULL;
	m_hPanel = NULL;
	m_pThis = this;
	m_pExportRoot=NULL;	
	m_pMemoryLog=new CMemoryLog();
	m_bMemoryLogOnOGRE=false;

	//
	RegisterNotification(ModifyCallback, this, NOTIFY_SYSTEM_POST_RESET);
	RegisterNotification(ModifyCallback, this, NOTIFY_SYSTEM_POST_NEW);
	RegisterNotification(ModifyCallback, this, NOTIFY_FILE_POST_OPEN);
	RegisterNotification(ModifyCallback, this, NOTIFY_FILE_POST_MERGE);
	RegisterNotification(ModifyCallback, this, NOTIFY_POST_IMPORT);
	RegisterNotification(ModifyCallback, this, NOTIFY_FILE_OPEN_FAILED);
	RegisterNotification(ModifyCallback, this, NOTIFY_IMPORT_FAILED);
}

CExporter::~CExporter()
{
	SaveGlobalSettings();
	UnRegisterNotification(ModifyCallback, this, NOTIFY_SYSTEM_POST_RESET);
	UnRegisterNotification(ModifyCallback, this, NOTIFY_SYSTEM_POST_NEW);
	UnRegisterNotification(ModifyCallback, this, NOTIFY_FILE_POST_OPEN);
	UnRegisterNotification(ModifyCallback, this, NOTIFY_FILE_POST_MERGE);
	UnRegisterNotification(ModifyCallback, this, NOTIFY_POST_IMPORT);
	UnRegisterNotification(ModifyCallback, this, NOTIFY_FILE_OPEN_FAILED);
	UnRegisterNotification(ModifyCallback, this, NOTIFY_IMPORT_FAILED);

	try {
		//
		GDI::Window::Cleanup();
		delete m_pMemoryLog;
		m_pMemoryLog=0;
	} catch(...)
	{
	}

	// Free ExportObjects
	FreeConfig();

	UNREGISTER_MODULE

}

void CExporter::LoadConfig()
{
	// Remove old configuration
	FreeConfig();

	// Root config
	CDDObject *pRootConfig=new CDDObject();
	pRootConfig->SetString("Type", "root");	

	try {
		PROPSPEC ps;
		ps.ulKind = PRSPEC_LPWSTR;
		ps.lpwstr = NDS_PROPERTY_NAME;

		int iPropIndex = m_pMax->FindProperty(PROPSET_USERDEFINED, &ps);
		if(iPropIndex != -1) 
		{
			const PROPVARIANT* pProperty = m_pMax->GetPropertyVariant(PROPSET_USERDEFINED, iPropIndex);
			if(pProperty->vt == VT_BLOB)
			{
				const BLOB& blob = pProperty->blob;

				CDataStream stream(blob.pBlobData, blob.cbSize, NULL);
				stream.SetPosition(0);	

				// Check Version
				unsigned int iVersionID = stream.GetInt();
				if(iVersionID == NDS_EXPORTER_CONFIG_VERSION)
					pRootConfig->FromDataStream(&stream);
			}
		}
	} catch(...)
	{	
	}
	//pRootConfig->SaveASCII("C:\\loadStream.txt");
	// Create new export root object
	m_pExportRoot=(CExportObjectRoot*)CExportObject::Construct(pRootConfig);	

	pRootConfig->Release();

	// Refresh buttonstates on MAX panel
	RefreshButtons();
}

void CExporter::SaveConfig()
{
	if(!m_pExportRoot) return;// || !m_pExportRoot->HasChildren()) return;

	CDataStream stream;
	stream.Reserve(50000);

	//Write Version ID
	stream.SetPosition(0);
	stream.AddInt( NDS_EXPORTER_CONFIG_VERSION );

	CDDObject *pConfig=new CDDObject();
	m_pExportRoot->SaveConfig(pConfig);

	//pConfig->SaveASCII("C:\\saveStream.txt");

	pConfig->ToDataStream(&stream);
	pConfig->Release();

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
void CExporter::FreeConfig()
{
	// If we had an old configuration, we release it
	if(m_pExportRoot!=NULL)
	{
		try {
			m_pExportRoot->Release();
		} catch(...)
		{
		}
	}
	m_pExportRoot=NULL;
}

//
void CExporter::RefreshButtons()
{
	if(!m_hPanel) return;

	PROPSPEC ps;
	ps.ulKind = PRSPEC_LPWSTR;
	ps.lpwstr = NDS_PROPERTY_NAME;
	bool bGotConfig = (m_pMax->FindProperty(PROPSET_USERDEFINED, &ps) != -1) ? true : false;

	EnableWindow(GetDlgItem(m_hPanel, IDC_EXPORT_BUTTON), m_pExportRoot!=NULL && m_pExportRoot->HasChildren() ? TRUE : FALSE);
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
		if(!m_bMemoryLogOnOGRE)
		{
			Ogre::LogManager::getSingleton().getDefaultLog()->addListener(m_pMemoryLog);
			m_bMemoryLogOnOGRE=true;
		}

		LoadConfig();
		Ogre::String sVersionInfo = Ogre::String("Version: ") + Ogre::String(NDS_EXPORTER_VERSION);
		Static_SetText( GetDlgItem(m_hPanel, IDC_VERSION_INFO), sVersionInfo.c_str());

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
	if(m_bMemoryLogOnOGRE)
	{
		Ogre::LogManager::getSingleton().getDefaultLog()->removeListener(m_pMemoryLog);
		m_bMemoryLogOnOGRE=false;
	}
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
	CExporterPropertiesDlg dlg(GDI::Window::FromHandle(m_pMax->GetMAXHWnd()), m_pMax, m_pMaxUtil, m_pExportRoot);
	int iRtnCode = dlg.DoModal();

	if(dlg.m_bChanges) SaveConfig();

	RefreshButtons();	
	if(iRtnCode == 100 || iRtnCode == 101)
	{
		if(iRtnCode == 101)
		{
			ExportItems(false);			
		}
		else
		{
			ExportItems(true);			
		}
	}
}

//
void CExporter::OnPanelButtonExport()
{	
	ExportItems(true);
}

//
void CExporter::ShowLog()
{
	CExportViewLogDlg dlg(GDI::Window::FromHandle(m_pMax->GetMAXHWnd()), m_pMemoryLog);
	dlg.DoModal();
}

//
void CExporter::ExportItems(bool bForceAll)
{
	if(m_pExportRoot==NULL || !m_pExportRoot->HasChildren()) return;

	if(!ValidateFilenames()) return;

	m_pMemoryLog->Flush();
	m_pMemoryLog->LogImportant(true);
	LOGINFO "Memory log flushed");

	// Create progress dialog
	CExportProgressDlg dlg(NULL);
	dlg.Create(IDD_DIALOG_PROGRESS, GDI::Window::FromHandle(m_pMax->GetMAXHWnd()));
	LOGINFO "Progress created");
	dlg.ShowWindow(SW_SHOW);
	LOGINFO "Progress show");
	unsigned int iCount=m_pExportRoot->GetChildCount(true);
	iCount++; // Inclusive Root Object
	LOGINFO "Counted: %d", iCount);
	dlg.InitGlobal(iCount);
	LOGINFO "Progress initglobal");

	LOGINFO "Exporting %i topitem(s)", m_pExportRoot->GetChildren().size());
	m_pExportRoot->Export(&dlg, bForceAll);

//	dlg.MessageBox("Done", NDS_EXPORTER_TITLE, MB_ICONINFORMATION);
//	dlg.DestroyWindow();
	dlg.ExportDone();
	if(m_pMemoryLog->LogImportant(true))
		ShowLog();
}

//

Interface* CExporter::GetMax()
{
	return m_pMax;
}

//
void CExporter::LoadGlobalSettings()
{
	char buffer[_MAX_PATH];
	GetModuleFileName(m_hInstance, buffer, _MAX_PATH);
	PathRemoveFileSpec(buffer);
	strcat(buffer, "\\LEXIExport.DDConf");
	CDDParse p;
	m_pGlobalSettings=p.ParseFromFile(buffer);
	if(m_pGlobalSettings==NULL) m_pGlobalSettings=new CDDObject();
}

//
void CExporter::SaveGlobalSettings()
{
	char buffer[_MAX_PATH];
	GetModuleFileName(m_hInstance, buffer, _MAX_PATH);
	PathRemoveFileSpec(buffer);
	strcat(buffer, "\\LEXIExport.DDConf");
	m_pGlobalSettings->SaveASCII(buffer);
}
