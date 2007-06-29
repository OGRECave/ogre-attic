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
#include "LexiMaxExportDescProxy.h"

//

extern "C" __declspec(dllexport) const TCHAR* LibDescription()
{
	return NDS_EXPORTER_TITLE;
}

extern "C" __declspec(dllexport) int LibNumberClasses()
{
	return 1;
}

extern "C" __declspec(dllexport) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

extern "C" __declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
	static CExporterProxyDesc Descriptor;
	return i ? NULL : &Descriptor;
}

extern "C" __declspec(dllexport) ULONG CanAutoDefer()
{
	return 1;
}

/////////////////////////////////////////////////

int CExporterProxyDesc::IsPublic()
{
	return 1;
}

void* CExporterProxyDesc::Create(BOOL loading)
{
	return new CExporterProxy(this);
}

const TCHAR* CExporterProxyDesc::ClassName()
{
	return NDS_EXPORTER_TITLE;
}

SClass_ID CExporterProxyDesc::SuperClassID()
{
	return UTILITY_CLASS_ID;
}

Class_ID CExporterProxyDesc::ClassID()
{
	return Class_ID(0x41e590e, 0x44af686b);
}

const TCHAR* CExporterProxyDesc::Category()
{
	return "";
}

//

