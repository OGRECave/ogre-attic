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

#ifndef __NDS_LexiExporterProxy__
#define __NDS_LexiExporterProxy__

//

class CExporterProxyDesc;

class CExporterProxy : public UtilityObj 
{
public:
	// Constructor/Destructor
	CExporterProxy(CExporterProxyDesc* pDesc);
	~CExporterProxy();

	// From UtilityObj
	void	BeginEditParams(Interface* ip,IUtil* iu);
	void	EndEditParams(Interface* ip,IUtil* iu);
	void	DeleteThis();

private:
	HMODULE m_hModuleHandle;
	typedef UtilityObj* (*fnCreate)();
	typedef void (*fnDestroy)(UtilityObj*);

	fnCreate	m_fnCreate;
	fnDestroy	m_fnDestroy;
	static UtilityObj* m_pLEXIMain;
};

//

#endif // __NDS_LexiExporterProxy__