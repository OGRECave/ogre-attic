/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

#ifndef __NDS_LexiExporter_ViewLogDialog__
#define __NDS_LexiExporter_ViewLogDialog__

#include <richedit.h>

//

class CExportViewLogDlg : public GDI::Dialog
{
public:
	CExportViewLogDlg(Window* pParent, CMemoryLog *pLog);
	~CExportViewLogDlg();

protected:

	virtual void OnInitDialog();
	void	OnSize(void);
	void	OnShowDebug(void);
	void	OnShowOGRE(void);
	void	OnAutoClose(void);

private:	
	static INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	void	Repopulate(void);
	void	AddMessage(CMemoryLog::SMessage *pMessage);

	RECT	m_OrgClientRect;
	CMemoryLog *m_pLog;

	GDI::TreeCtrl	m_LogTree;
	GDI::Button		m_ShowDebug;
	GDI::Button		m_ShowOGRE;
	bool			m_bShowDebug;
	bool			m_bShowOGRE;	
};

//

#endif // __NDS_LexiExporter_ExportProgressDialog__