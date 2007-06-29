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

#include "LexiStdAfxProxy.h"
#include "LexiMaxExportProxy.h"
#include "LexiMaxExportDescProxy.h"

UtilityObj* CExporterProxy::m_pLEXIMain = NULL;

CExporterProxy::CExporterProxy(CExporterProxyDesc* pDesc)
{
//
	//m_pLEXIMain = NULL;
}

CExporterProxy::~CExporterProxy()
{
//
}


void CExporterProxy::BeginEditParams(Interface* ip, IUtil* iu)
{

	char curDir[512];
	GetCurrentDirectory(510,curDir);

	char szAppPath[MAX_PATH] = "";
	::GetModuleFileName(NULL,szAppPath,sizeof(szAppPath) - 1);

	char szDrive[MAX_PATH];
	char szPath[MAX_PATH];
	_splitpath(szAppPath,szDrive,szPath,NULL,NULL);
	strcat(szDrive,szPath);

	strcat(szDrive,"/LEXIExporter/Dlls/");
	SetCurrentDirectory(szDrive);

//
	if(m_pLEXIMain==NULL)
	{
		m_hModuleHandle = LoadLibrary(NDS_EXPORTER_PLUGIN);
		if(m_hModuleHandle == 0)
		{
			MessageBox(NULL,NDS_EXPORTER_PLUGIN,"Error Loading Dll:",MB_ICONERROR);
			SetCurrentDirectory(curDir);
			return;
		}

		m_fnCreate = (fnCreate)GetProcAddress(m_hModuleHandle, "CreateLEXI");
		m_fnDestroy = (fnDestroy)GetProcAddress(m_hModuleHandle, "DestroyLEXI");

		if(m_fnCreate == NULL || m_fnDestroy == NULL)
		{
			MessageBox(NULL,NDS_EXPORTER_PLUGIN,"Error Loading Dll:",MB_ICONERROR);
			FreeLibrary(m_hModuleHandle);
			SetCurrentDirectory(curDir);
			return;
		}

		m_pLEXIMain = m_fnCreate();
	}
	
	m_pLEXIMain->BeginEditParams(ip,iu);
	SetCurrentDirectory(curDir);
}

void CExporterProxy::EndEditParams(Interface* ip, IUtil* iu)
{
//	
	if(m_pLEXIMain==NULL)
		return;

	m_pLEXIMain->EndEditParams(ip,iu);

#if(NDS_EXPORTER_AUTOUNLOAD)	
	m_fnDestroy(m_pLEXIMain);
	FreeLibrary(m_hModuleHandle);
	m_pLEXIMain = NULL;
#endif
}

void CExporterProxy::DeleteThis()
{
	delete this;
}