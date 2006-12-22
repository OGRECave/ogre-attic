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

//

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		CExporter::m_hInstance = hInstance;

		InitCustomControls(hInstance);

		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = 0x7fff;
		InitCommonControlsEx(&icex);

		LoadLibrary("RICHED32.DLL");

		GDI::MetaControl::RegisterClasses();
	}
	else if(dwReason == DLL_PROCESS_DETACH)
	{
		GDI::MetaControl::UnregisterClasses();

		CExporter::m_hInstance = NULL;
	}

    return TRUE;
}

DWORD GetGDISysColor(int iIndex)
{
	return GetCustSysColor(iIndex);
}

HBRUSH GetGDISysColorBrush(int iIndex)
{
	return GetCustSysColorBrush(iIndex);
}
//

HINSTANCE GetCurrentInstance()
{
	return CExporter::m_hInstance;
}

//

