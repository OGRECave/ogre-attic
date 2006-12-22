#define _CRT_SECURE_NO_DEPRECATE
#define WINVER			0x0500
#define _WIN32_WINNT	0x0500
#define _WIN32_WINDOWS	0x0500
#define _WIN32_IE		0x0600
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <commdlg.h>
#include <map>
#include <string>
#include "..\Include\SharedUtilities.h"
#include "..\Include\GDIMetaLib.h"

// Global Variables:
HINSTANCE hInst;	// current instance

HBRUSH GetGDISysColorBrush(int iIndex)
{
	return GetSysColorBrush(iIndex);
}
DWORD GetGDISysColor(int iIndex)
{
	return GetSysColor(iIndex);
}

HINSTANCE GetCurrentInstance(void)
{
	return hInst;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	INITCOMMONCONTROLSEX ex;
	ex.dwSize=sizeof(INITCOMMONCONTROLSEX);
	ex.dwICC=ICC_WIN95_CLASSES;
	InitCommonControlsEx(&ex);	

	GDI::MetaControl::RegisterClasses();	

	CDDParse p;
	CDDObject *pMeta=p.ParseFromFile("TestMeta.DDConf");
	HWND hMetaWnd=::CreateWindow("MetaControl", "My Test Meta", WS_VISIBLE|WS_BORDER|WS_CLIPCHILDREN|WS_SIZEBOX|WS_VSCROLL, 50, 50, 400, 400, NULL, NULL, NULL, NULL);
	GDI::MetaControl meta;
	meta.Attach(hMetaWnd);
	meta.CreateFromMeta(pMeta);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{		
		if(msg.message==WM_KEYDOWN && msg.wParam==27)
		{
			break;
		}
		if(msg.message==WM_KEYUP && msg.wParam==65)
		{
			meta.CreateFromMeta(pMeta);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);	
	}

	GDI::MetaControl::UnregisterClasses();

	return (int) msg.wParam;
}