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

#ifndef __NDS_LexiExporter__
#define __NDS_LexiExporter__

//

class CExporterDesc;

class CExporter : public UtilityObj 
{
public:
	// Constructor/Destructor
	CExporter(CExporterDesc* pDesc);
	~CExporter();

	// From UtilityObj
	void	BeginEditParams(Interface* ip,IUtil* iu);
	void	EndEditParams(Interface* ip,IUtil* iu);
	void	DeleteThis();

	// Get Max interface
	static Interface* GetMax();

	// Module Instance (loading icons, etc.)
	static HINSTANCE m_hInstance;

	// Singleton
	static CExporter* Get();

	// Global settings
	CDDObject* GetGlobalSettings();

	//
	void	ShowLog();

	//
	void	OnPanelButtonProperties();
	void	OnPanelButtonExport();	

	void	RefreshButtons();

	// Load per-scene configuration
	void	LoadConfig();
	
	// Save per-scene configuration
	void	SaveConfig();		

	// Export items
	void	ExportItems(bool bForceAll);

private:	
	CExporterDesc* m_pDesc;
	static Interface* m_pMax;
	IUtil* m_pMaxUtil;

	HWND m_hPanel;

	//
	CMemoryLog			*m_pMemoryLog;
	bool				m_bMemoryLogOnOGRE;

	// Export root object
	CExportObjectRoot	*m_pExportRoot;

	// Per machine settings
	CDDObject* m_pGlobalSettings;	

	//	
	void	FreeConfig();
	void	LoadGlobalSettings();
	void	SaveGlobalSettings();

	//
	static INT_PTR CALLBACK ConfigDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static CExporter* m_pThis;
};

//

#define NDS_PROPERTY_NAME		L"LEXIExportSettings"

//

#endif // __NDS_LexiExporter__