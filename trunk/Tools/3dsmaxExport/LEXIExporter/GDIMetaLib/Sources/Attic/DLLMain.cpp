/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

#include "StdAfx.h"

static HINSTANCE g_hInstance=0;

// To be able to provide a pointer to binary data, we have to have a static DataStream object
static CDataStream *m_pStorage=new CDataStream();

HBRUSH	GetGDISysColorBrush(int iIndex)
{
	return ::GetSysColorBrush(iIndex);
}
DWORD	GetGDISysColor(int iIndex)
{
	return ::GetSysColor(iIndex);
}

// We keep instance handle from DLLhookup
HINSTANCE GetCurrentInstance()
{
	return g_hInstance;
}

// Called from C# when base control is created to create a underlying 
// meta control class and attach it.
extern "C" __declspec( dllexport ) void CreateWin32Class(HWND hWnd)
{
//	LOGSTATIC LOG_DEBUG, "Attached MetaControl class to window: %X", hWnd);
	GDI::MetaControl *pNew=new GDI::MetaControl;
	pNew->Attach(hWnd);
}

// Called from C# to setup new meta DDObject on previously created control
extern "C" __declspec( dllexport ) void CreateFromMeta(HWND hWnd, void *pMeta, int iMetaSize)
{
	// Find meta control from window handle
	GDI::MetaControl* pMetaCtrl=(GDI::MetaControl*)GDI::MetaControl::GetMapping(hWnd);
	if(!pMeta) 
	{
		LOGSTATIC LOG_ERROR, "Failed to find control: %X", hWnd);
		return;
	}

	// Create DDObject from data stream
	CDataStream MetaStream(pMeta, iMetaSize, 0);
	CDDObject *ddMeta=new CDDObject;
	ddMeta->FromDataStream(&MetaStream);

	// Create controls
	pMetaCtrl->CreateFromMeta(ddMeta);
	ddMeta->Release();
}

// Called from C# to setup new meta DDObject on previously created control
extern "C" __declspec( dllexport ) void CreateFromMetaData(HWND hWnd, void *pMeta, int iMetaSize, void *pData, int iDataSize)
{
	// Find meta control from window handle
	GDI::MetaControl* pMetaCtrl=(GDI::MetaControl*)GDI::MetaControl::GetMapping(hWnd);
	if(!pMeta) 
	{
		LOGSTATIC LOG_ERROR, "Failed to find control: %X", hWnd);
		return;
	}

	LOGSTATIC LOG_WARNING, "Create meta");

	// Create DDObject from data stream
	CDataStream MetaStream(pMeta, iMetaSize, 0);
	CDDObject *ddMeta=new CDDObject;
	ddMeta->FromDataStream(&MetaStream);

	LOGSTATIC LOG_WARNING, "Create data");

	// Create DDObject from data stream
	CDataStream DataStream(pData, iDataSize, 0);
	CDDObject *ddData=new CDDObject;
	ddData->FromDataStream(&DataStream);

	LOGSTATIC LOG_WARNING, "->MetaControl");

	// Create controls
	pMetaCtrl->CreateFromMetaData(ddMeta, ddData);
	ddMeta->Release();
	ddData->Release();
}

// Called from C# to retrieve data from control
extern "C" __declspec( dllexport ) const void* GetDataFromControl(HWND hWnd, int *iSize)
{
	// Find meta control from window handle
	GDI::MetaControl* pMeta=(GDI::MetaControl*)GDI::MetaControl::GetMapping(hWnd);
	if(!pMeta) 
	{
		LOGSTATIC LOG_ERROR, "Failed to find control: %X", hWnd);
		return NULL;
	}

	m_pStorage->SetPosition(0);
	CDDObject *pDDData=pMeta->GetData();
	pDDData->ToDataStream(m_pStorage);
	*iSize=m_pStorage->GetPosition();
	return m_pStorage->GetBaseData();
}

// Called from C# to update data on meta control
extern "C" __declspec( dllexport ) void SetDataToControl(HWND hWnd, void *pData, int iSize)
{
	// Find meta control from window handle
	GDI::MetaControl* pMeta=(GDI::MetaControl*)GDI::MetaControl::GetMapping(hWnd);
	if(!pMeta) 
	{
		LOGSTATIC LOG_ERROR, "Failed to find control: %X", hWnd);
		return;
	}

	// Create DDObject from data stream
	CDataStream d(pData, iSize, 0);
	CDDObject *dd=new CDDObject;
	dd->FromDataStream(&d);

	// Create controls
	pMeta->SetData(dd);
	dd->Release();
}

// Called from C# to update data on meta control
extern "C" __declspec( dllexport ) const void* CreateDefaultsFromMeta(void *pMeta, int iMetaSize, int *iOutSize)
{	
	// Create DDObject from data stream
	CDataStream d(pMeta, iMetaSize, 0);
	CDDObject *dd=new CDDObject;
	dd->FromDataStream(&d);

	CDDObject *pDefaults=new CDDObject();
	GDI::MetaControl::SetDefaultsFromMeta(dd, pDefaults);
	dd->Release();

	m_pStorage->SetPosition(0);
	pDefaults->ToDataStream(m_pStorage);
	*iOutSize=m_pStorage->GetPosition();
	pDefaults->Release();
	return m_pStorage->GetBaseData();
}

// DLL entry function
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
//			CLogSystem::Get()->AddReceiver(new CConsoleLogger());
//			CLogSystem::Get()->AddReceiver(new CFileLogger("C:\\Debuglog.txt"));
			g_hInstance=hModule;
			GDI::MetaControl::RegisterClasses();			
			break;
		case DLL_PROCESS_DETACH:
			GDI::MetaControl::UnregisterClasses();
			break;
	}	
    return TRUE;
}