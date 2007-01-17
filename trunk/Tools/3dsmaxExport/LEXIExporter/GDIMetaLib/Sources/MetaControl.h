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

#ifndef __NDS_META_CONTROL__
#define __NDS_META_CONTROL__

//
namespace GDI 
{
class MetaBaseCtrl;
class MetaGroup;

class MetaControl : public Window, public IDDNotify
{
public:
	MetaControl();
	~MetaControl();

	// Register/unregister window classes
	static void	RegisterClasses();
	static void	UnregisterClasses();

	// Find meta control from window handle
	static MetaControl*	FindFromHWND(HWND hWnd);

	// Get/Set data object to edit
	CDDObject*	GetData();
	void	SetData(CDDObject *pData);

	void	CreateFromMeta(CDDObject *pMeta);
	void	CreateFromMetaData(CDDObject *pMeta, CDDObject *pData);

	// Notified from clients. This is enabled to make
	// scroll-on-client-area function work.
	void	OnMouseDown(int iFlags, int iX, int iY);
	void	OnMouseUp(int iFlags, int iX, int iY);
	void	OnMouseMove(int iFlags, int iX, int iY);

	//
	void	DoLayout();

private:	
	// Called internally from CreateControls()
	void	AddControl(const char *pszID, const CDDObject *pMeta);

	// Create/destroy meta controls
	void	CreateControls();
	void	DestroyControls();	

	// Open existing group or create new if needed
	MetaGroup	*AquireGroup(const char *pszGroup);

	// Messages bound for MetaControls
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Messages bound for MetaChild controls (i.e. MetaBaseCtrl instances)
	static LRESULT CALLBACK WndChildProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Called when window size changes
	void	OnSize(void);
	void	OnVScroll(int iCode, int iPos);		

	// This function is called whenever client height vs. content height changes
	void	UpdateScroll(int iNewHeight, int iContentHeight);

	// Called when data object has changed
	void	OnChanged(const CDDObject *pInstance, const char *pszKey);
	
	// Map HWND<->MetaBaseCtrl instances
	static fastintmap<MetaBaseCtrl*>	m_mControlMap;

	// Map ID<->MetaGroup instances
	fastmap<MetaGroup*>	m_mGroups;

	// Meta data reference
	const CDDObject	*m_pMetaData;

	// Reference to the data object being edited
	CDDObject	*m_pEditData;

	//
	bool	m_bHasCapture;
	int		m_iClientHeight;
	int		m_iOrgMouseY;

	//
	HWND	m_hWndTip;

	//
	static HCURSOR	m_hNormal;
	static HCURSOR	m_hGrab;

	//	
	bool	m_bHasScroll;
	int		m_iYOffset;
	int		m_iContentHeight;
	int		m_iWidth;
};

}
#endif