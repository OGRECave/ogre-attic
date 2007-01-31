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

#include "stdafx.h"

namespace GDI
{

fastintmap<MetaBaseCtrl*>	MetaControl::m_mControlMap;
HCURSOR	MetaControl::m_hGrab;
HCURSOR	MetaControl::m_hNormal;

MetaControl::MetaControl()
{
	m_pEditData=NULL;
	m_pMetaData=NULL;
	m_bHasScroll=true;
	m_iWidth=0;	
	m_iYOffset=0;
	m_iOrgMouseY=0;
	m_iClientHeight=0;
	m_iContentHeight=0;
	m_hWndTip=NULL;
	m_pListener=0;
}
MetaControl::~MetaControl()
{
	DestroyControls();

	if(m_hWndTip)
	{
		::DestroyWindow(m_hWndTip);
		m_hWndTip=NULL;
	}

	if(m_pMetaData)
	{
		m_pMetaData->Release();
		m_pMetaData=0;
	}
	if(m_pEditData)
	{
		m_pEditData->SetNotifier(NULL);
		m_pEditData->Release();
		m_pEditData=0;
	}
}

void MetaControl::RegisterClasses()
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_NOCLOSE|CS_GLOBALCLASS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= GET_CURRENT_INSTANCE;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= GET_SYSTEM_COLOR_BRUSH(COLOR_3DFACE);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "MetaControl";
	wcex.hIconSm		= NULL;
	if(!RegisterClassEx(&wcex))
		LOGSTATIC LOG_ERROR, "Failed to register MetaControl");

	//LOGSTATIC LOG_INFO, "Register info: %X", wcex.hInstance);

	wcex.lpfnWndProc	= MetaControl::WndChildProc;
	wcex.lpszClassName	= "MetaChildControl";
	if(!RegisterClassEx(&wcex))
		LOGSTATIC LOG_ERROR,  "Failed to register MetaChildControl");

	wcex.style			= CS_HREDRAW;
	wcex.lpfnWndProc	= MetaGroup::WndProc;
	wcex.lpszClassName	= "MetaGroup";
	if(!RegisterClassEx(&wcex))
		LOGSTATIC LOG_ERROR,  "Failed to register MetaGroup");

	m_hNormal=LoadCursor(NULL, IDC_ARROW);
	m_hGrab=LoadCursor(NULL, IDC_HAND);
//	LOGSTATIC LOG_INFO, "Controls are registered OK");
}

void MetaControl::UnregisterClasses()
{
	::UnregisterClass("MetaControl", GET_CURRENT_INSTANCE);
	::UnregisterClass("MetaChildControl", GET_CURRENT_INSTANCE);
	::UnregisterClass("MetaGroup", GET_CURRENT_INSTANCE);
}

// Get/Set data object to edit
CDDObject* MetaControl::GetData()
{
	return m_pEditData;
}
void MetaControl::SetData(CDDObject *pData)
{
	if(pData==NULL) return;

	// We we had an existing object, we release it
	if(m_pEditData) 
	{
		m_pEditData->SetNotifier(NULL);
		m_pEditData->Release();
	}

	// Setup reference to the new object
	m_pEditData=pData;
	m_pEditData->AddRef();
	m_pEditData->SetNotifier(this);

	// Iterate all groups and notify about the new data object.
	// This will also setup default values (if needed) on the object.
	fastvector<MetaGroup*> lGroups=m_mGroups.data();
	for(unsigned i=0;i<lGroups.size();i++)
	{
		lGroups[i]->UpdateData(m_pEditData);
		lGroups[i]->CheckConditions("$Global");
	}
}
// Set data notifier
void MetaControl::SetDataNotify(IDDNotify *pListener)
{
	m_pListener=pListener;
}

void MetaControl::CreateFromMeta(CDDObject *pMeta)
{
	if(!pMeta) return;

	// Release previous meta
	if(m_pMetaData)
	{
		m_pMetaData->Release();
	}

	// Keep reference
	m_pMetaData=pMeta;
	m_pMetaData->AddRef();

	// If we don't have a data object set already, 
	// we create an empty one
	if(!m_pEditData) 
	{
		m_pEditData=new CDDObject;
		m_pEditData->SetNotifier(this);
	}

	// Create controls based on the new meta data
	CreateControls();

	// Iterate and check all conditions
	fastvector<MetaGroup*> lGroups=m_mGroups.data();
	for(unsigned i=0;i<lGroups.size();i++)
	{
		lGroups[i]->CheckConditions("$Global");
	}
}

void MetaControl::CreateFromMetaData(CDDObject *pMeta, CDDObject *pData)
{
	if(!pMeta || !pData) return;

	// Release previous meta
	if(m_pMetaData)
	{
		m_pMetaData->Release();
	}

	// We we had an existing object, we release it
	if(m_pEditData) 
	{
		m_pEditData->SetNotifier(NULL);
		m_pEditData->Release();
	}

	// Setup reference to the new object
	m_pEditData=pData;
	m_pEditData->AddRef();
	m_pEditData->SetNotifier(this);

	// Keep reference
	m_pMetaData=pMeta;
	m_pMetaData->AddRef();	

	// Create controls based on the new meta data
	CreateControls();

	// Iterate and check all conditions
	fastvector<MetaGroup*> lGroups=m_mGroups.data();
	for(unsigned i=0;i<lGroups.size();i++)
	{
		lGroups[i]->CheckConditions("$Global");
	}
}

void MetaControl::AddControl(const char *pszID, const CDDObject *pMetaKey)
{
	// Check if the control is currently hidden
	if (pMetaKey->GetInt("Visible", 1) == 0) return;

	// Get type from descriptor
	const char *pszType = pMetaKey->GetString("Type");
	const char *pszGroup = pMetaKey->GetString("Group", "Generic");

	MetaBaseCtrl *pMetaCtrl = NULL;
	MetaGroup	 *pMetaGroup= AquireGroup(pszGroup);

	// Create new instance based on type identifier
	if(_stricmp(pszType, "int")==0 || _stricmp(pszType, "integer")==0)
	{
		pMetaCtrl=new MetaInt();
	} else if(_stricmp(pszType, "stringsel")==0 || _stricmp(pszType, "selection")==0)
	{
		pMetaCtrl=new MetaSelection();
	} else if(_stricmp(pszType, "string")==0)
	{
		pMetaCtrl=new MetaString();
	} else if(_stricmp(pszType, "float")==0)
	{
		pMetaCtrl=new MetaFloat();
	} else if(_stricmp(pszType, "bool")==0 || _stricmp(pszType, "boolean")==0)
	{
		pMetaCtrl=new MetaBool();
	} else if(_stricmp(pszType, "color")==0 || _stricmp(pszType, "colour")==0)
	{
		pMetaCtrl=new MetaColor();
	} else if(_stricmp(pszType, "vec3")==0 || _stricmp(pszType, "vector3")==0)
	{
		pMetaCtrl=new MetaVec3();
	}

	// Bail if a new instance was not created
	if(pMetaCtrl==NULL) return;
	
	// Create window and resources
	pMetaCtrl->Create(this, pMetaGroup, pszID, pMetaKey);	
	m_mControlMap.map((unsigned int)pMetaCtrl->m_hWnd, pMetaCtrl);

	if(m_hWndTip!=NULL)
	{
		//
		TOOLINFO tInfo;
		tInfo.cbSize=sizeof(TOOLINFO);
		tInfo.uFlags=TTF_IDISHWND|TTF_SUBCLASS;
		tInfo.hwnd=m_hWnd;
		tInfo.uId=(UINT_PTR)pMetaCtrl->m_hWnd;
		tInfo.lpszText=(LPSTR)pMetaKey->GetString("Help", "No help available for this control");
		::SendMessage(m_hWndTip, TTM_ADDTOOL, 0, (LPARAM)&tInfo);
	}
}

int metaGroupCompare( const void *arg1, const void *arg2 )
{
   /* Compare all of both strings: */
	MetaGroup* pMetaGroup1 = *(MetaGroup**)arg1;
	MetaGroup* pMetaGroup2 = *(MetaGroup**)arg2;

	return pMetaGroup1->GetTitle().compare(pMetaGroup2->GetTitle());
}

void MetaControl::CreateControls()
{
	if(m_pMetaData==NULL) return;

	if(m_hWndTip==NULL)
	{
		m_hWndTip = ::CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
                            WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            m_hWnd, NULL, GET_CURRENT_INSTANCE,
                            NULL);

		::SetWindowPos(m_hWndTip, HWND_TOPMOST,0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);	
		::SendMessage(m_hWndTip, TTM_ACTIVATE, true, 0);
	}

	// Destroy all current controls
	DestroyControls();

	// Check if we have a list of meta controls
	if (m_pMetaData->GetKeyType("MetaList") == DD_OBJLIST)
	{
		fastvector<const CDDObject*> lMetaList=m_pMetaData->GetDDList("MetaList");

		// Iterate meta description list and create controls
		for (unsigned i = 0; i < lMetaList.size(); i++)
		{
			const CDDObject *pMetaKey = lMetaList[i];
			if (pMetaKey == NULL || pMetaKey->GetKeyType("ID") != DD_STRING) continue;

			// Add new control from desc
			AddControl(pMetaKey->GetString("ID"), pMetaKey);
		}
	}

	// Get client area
	RECT rClient;
	::GetClientRect(m_hWnd, &rClient);

	// Layout all groups and controls
	int iY=0;
	m_iWidth=rClient.right;
	fastvector<MetaGroup*> lGroups=m_mGroups.data();

	lGroups.sort(metaGroupCompare);

	for(unsigned i=0;i<lGroups.size();i++)
	{
		lGroups[i]->SetWidth(m_iWidth);
		lGroups[i]->DoLayout();
		lGroups[i]->SetPosition(iY);
		iY+=lGroups[i]->GetHeight();
	}
	m_iContentHeight=iY;
	m_iClientHeight=rClient.bottom;
	UpdateScroll(m_iClientHeight, m_iContentHeight);
}

// Destroy all groups and subcontrols
void MetaControl::DestroyControls()
{
	// Remove all controls from map
	fastvector<MetaBaseCtrl*> lCtrls=m_mControlMap.data();
	for(unsigned i=0;i<lCtrls.size();i++)
		if(lCtrls[i]->GetOwner()==this)
			m_mControlMap.erase((unsigned int)lCtrls[i]->m_hWnd);

	fastvector<MetaGroup*> lGroups=m_mGroups.data();
	for(unsigned i=0;i<lGroups.size();i++)
	{				
		lGroups[i]->ClearAll();
		lGroups[i]->DestroyWindow();
		delete lGroups[i];
	}		
	m_mGroups.clear();
	m_iContentHeight=0;
	UpdateScroll(m_iClientHeight, 0);
}

// Open existing group or create new if needed
MetaGroup *MetaControl::AquireGroup(const char *pszGroup)
{
	MetaGroup *pGroup;
	if(m_mGroups.find(pszGroup, pGroup)) return pGroup;

	pGroup=new MetaGroup();
	pGroup->Create(this, pszGroup);
	m_mGroups.map(pszGroup, pGroup);
	return pGroup;
}

void MetaControl::OnSize()
{
	RECT rClient;
	::GetClientRect(m_hWnd, &rClient);
	m_iClientHeight=rClient.bottom;
	
	// If client area width changes, notify all groups
	if(rClient.right!=m_iWidth)
	{
		m_iWidth=rClient.right;
		fastvector<MetaGroup*> lGroups=m_mGroups.data();
		for(unsigned i=0;i<lGroups.size();i++)
		{
			lGroups[i]->SetWidth(m_iWidth);
		}				
	}

	// Check for changes in scroll sizes
	UpdateScroll(m_iClientHeight, m_iContentHeight);
}

void MetaControl::OnVScroll(int iCode, int iPos)
{
	int iNewPos=0;
	if(iCode==SB_THUMBPOSITION || iCode==SB_THUMBTRACK)
	{
		iNewPos=iPos;
		SCROLLINFO sInfo;
		sInfo.cbSize=sizeof(SCROLLINFO);
		sInfo.fMask=SIF_POS;
		sInfo.nPos=iPos;
		SetScrollInfo(m_hWnd, SB_VERT, &sInfo, true);
	}
	else // TODO: Handle other codes
		iNewPos=::GetScrollPos(m_hWnd, SB_VERT);

	int iDelta=m_iYOffset-iNewPos;
	if(!iDelta) return;
	::ScrollWindowEx(m_hWnd, 0, iDelta, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN|SW_INVALIDATE);
	::UpdateWindow(m_hWnd);
	m_iYOffset=iNewPos;
}

// This function is called whenever client height vs. content height changes
void MetaControl::UpdateScroll(int iNewHeight, int iContentHeight)
{
	if(iContentHeight-m_iYOffset<iNewHeight)
	{	
		int iNewOffset=iContentHeight-iNewHeight;
		if(iNewOffset<0) iNewOffset=0;
		int iDelta=m_iYOffset-iNewOffset;
		if(iDelta)
		{
			::ScrollWindowEx(m_hWnd, 0, iDelta, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN|SW_INVALIDATE);
			::UpdateWindow(m_hWnd);
			m_iYOffset=iNewOffset;
		}
	}
	SCROLLINFO sInfo;
	sInfo.cbSize=sizeof(SCROLLINFO);
	sInfo.fMask=SIF_RANGE|SIF_POS|SIF_PAGE|SIF_DISABLENOSCROLL;
	sInfo.nMin=0;
	sInfo.nPos=m_iYOffset;
	sInfo.nPage=iNewHeight;
	sInfo.nMax=iContentHeight;
	SetScrollInfo(m_hWnd, SB_VERT, &sInfo, true);
}

// Called when data object has changed
void MetaControl::OnChanged(const CDDObject *pInstance, const char *pszKey)
{
	// Iterate all groups and re-evaluate conditions
	fastvector<MetaGroup*> lGroups=m_mGroups.data();
	for(unsigned i=0;i<lGroups.size();i++)
	{
		lGroups[i]->CheckConditions(pszKey);
	}

	// Notify listener if any
	if(m_pListener) m_pListener->OnChanged(pInstance, pszKey);

	// Notify owning window about change
	::SendMessage(this->m_hWnd, WM_NOTIFY_MESSAGE_ID, 0, (LPARAM)pszKey);
}

void MetaControl::OnMouseDown(int iFlags, int iX, int iY)
{
	// If the control can be scrolled, we capture the mouse
	if(m_iClientHeight<m_iContentHeight)
	{
		::SetCapture(m_hWnd);
		m_bHasCapture=true;
		m_iOrgMouseY=iY;
		::SetCursor(m_hGrab);
	}
}

void MetaControl::OnMouseUp(int iFlags, int iX, int iY)
{
	if(m_bHasCapture)
	{
		::SetCursor(m_hNormal);
		::ReleaseCapture();
		m_bHasCapture=false;
	}
}

void MetaControl::OnMouseMove(int iFlags, int iX, int iY)
{	
	if(m_bHasCapture)
	{
		// Check if mouse button is still down
		if((iFlags&MK_LBUTTON)!=MK_LBUTTON)
		{
			::SetCursor(m_hNormal);
			::ReleaseCapture();
			m_bHasCapture=false;
			return;
		}

		// Calculate delta on Y-axis - we only scroll on changes
		int iDelta=iY-m_iOrgMouseY;
		if(iDelta)
		{
/*			POINT p;
			p.x=iX;
			p.y=m_iOrgMouseY;
			::ClientToScreen(m_hWnd, &p);
			::SetCursorPos(p.x, p.y);*/
			// Calculate new scrolling offset and clamp
			m_iOrgMouseY=iY;
			int iNewOffset=m_iYOffset-iDelta;
			if(iNewOffset<0) iNewOffset=0;
			if(iNewOffset>(m_iContentHeight-m_iClientHeight))
				iNewOffset=m_iContentHeight-m_iClientHeight;

			// Check if we need to scroll the client area
			if(iNewOffset!=m_iYOffset)
			{
				// Scroll client area and update scrollbar position
				iDelta=m_iYOffset-iNewOffset;
				::ScrollWindowEx(m_hWnd, 0, iDelta, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN|SW_INVALIDATE);
				::UpdateWindow(m_hWnd);
				m_iYOffset=iNewOffset;
				SCROLLINFO sInfo;
				sInfo.cbSize=sizeof(SCROLLINFO);
				sInfo.fMask=SIF_POS;				
				sInfo.nPos=m_iYOffset;
				SetScrollInfo(m_hWnd, SB_VERT, &sInfo, true);
			}
		}
	}	
}

//
void MetaControl::DoLayout()
{
	RECT rClient;
	::GetClientRect(m_hWnd, &rClient);

	// Layout all groups and controls
	int iY=0;
	m_iWidth=rClient.right;
	fastvector<MetaGroup*> lGroups=m_mGroups.data();
	lGroups.sort(metaGroupCompare);
	for(unsigned i=0;i<lGroups.size();i++)
	{
		lGroups[i]->SetWidth(m_iWidth);
		lGroups[i]->DoLayout();
		lGroups[i]->SetPosition(iY);
		iY+=lGroups[i]->GetHeight();
	}	

	int iDelta=m_iContentHeight-iY;
	m_iContentHeight=iY;
	m_iClientHeight=rClient.bottom;
	UpdateScroll(m_iClientHeight, m_iContentHeight);
	//InvalidateRect(m_hWnd, &rClient, true);
}

// Messages bound for MetaControls
LRESULT CALLBACK MetaControl::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Find instance from handle. Pass to default handler if not found
	MetaControl *pWnd=(MetaControl*)GetMapping(hWnd);
	if(!pWnd) return ::DefWindowProc(hWnd, message, wParam, lParam);	

	switch (message)
	{		
	case WM_VSCROLL:
		pWnd->OnVScroll(LOWORD(wParam), HIWORD(wParam));
		break;
	case WM_LBUTTONDOWN:
		pWnd->OnMouseDown(wParam, ((signed short)LOWORD(lParam)), ((signed short)HIWORD(lParam)));
		break;
	case WM_LBUTTONUP:
		pWnd->OnMouseUp(wParam, ((signed short)LOWORD(lParam)), ((signed short)HIWORD(lParam)));
		break;
	case WM_MOUSEMOVE:
		pWnd->OnMouseMove(wParam, ((signed short)LOWORD(lParam)), ((signed short)HIWORD(lParam)));	
		break;
	case WM_SIZE:
	case WM_SIZING:
		pWnd->OnSize();
		break;
	default:
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Messages bound for MetaChild controls (i.e. MetaBaseCtrl instances)
LRESULT CALLBACK MetaControl::WndChildProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MetaBaseCtrl *pCtrl;
	if(!m_mControlMap.find((unsigned int)hWnd, pCtrl))
		return ::DefWindowProc(hWnd, message, wParam, lParam);

	switch (message)
	{
	case WM_COMMAND:
		pCtrl->OnCommand((HWND)lParam, HIWORD(wParam), LOWORD(wParam));
		break;
	case WM_ERASEBKGND:
		break;
	case WM_LBUTTONDBLCLK:
		pCtrl->OnMouseDblClick(wParam, ((signed short)LOWORD(lParam)), ((signed short)HIWORD(lParam)));
		break;
	case WM_LBUTTONDOWN:
		// If a mouse message is not used by the control, pass to parent MetaControl
		// this enables the scroll anywhere functionality
		if(!pCtrl->OnMouseDown(wParam, ((signed short)LOWORD(lParam)), ((signed short)HIWORD(lParam))))
		{
			POINT p;
			p.x=((signed short)LOWORD(lParam));
			p.y=((signed short)HIWORD(lParam));
			pCtrl->ClientToScreen(p);
			pCtrl->GetOwner()->ScreenToClient(p);
			pCtrl->GetOwner()->OnMouseDown(wParam, p.x, p.y);
		}
		break;
	case WM_MOUSEMOVE:
		// If a mouse message is not used by the control, pass to parent MetaControl
		// this enables the scroll anywhere functionality
		if(!pCtrl->OnMouseMove(wParam, ((signed short)LOWORD(lParam)), ((signed short)HIWORD(lParam))))
		{
			POINT p;
			p.x=((signed short)LOWORD(lParam));
			p.y=((signed short)HIWORD(lParam));
			pCtrl->ClientToScreen(p);
			pCtrl->GetOwner()->ScreenToClient(p);
			pCtrl->GetOwner()->OnMouseMove(wParam, p.x, p.y);
		}
		break;
	case WM_PAINT:
		pCtrl->OnPaint();
		break;
	default:
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

}