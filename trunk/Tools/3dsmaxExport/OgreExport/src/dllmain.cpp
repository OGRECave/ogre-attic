#include "max.h"
#include "plugapi.h"
#include "impexp.h"

#include "OgreExport.h"

static int controlsInit = FALSE;
static HINSTANCE hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		
		// jaguar controls
		InitCustomControls(hInstance);

		// initialize Chicago controls
		InitCommonControls();
		}
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL,_T("3DSIMP.DLL: DllMain"),_T("3DSIMP"),MB_OK);
			if (FAILED(CoInitialize(NULL)))
				return FALSE;
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
	}


//------------------------------------------------------
static OgreMaxExportClassDesc OgreMaxExportDescInst;

int OgreMaxExportClassDesc::IsPublic() {
	return 1; 
}

void * OgreMaxExportClassDesc::Create(BOOL loading) { 
	OgreMaxExport *inst = new OgreMaxExport(hInstance); 
	return inst;
}

const TCHAR * OgreMaxExportClassDesc::ClassName() { 
	return _T("Ogre 3DSMax Exporter"); 
}

SClass_ID OgreMaxExportClassDesc::SuperClassID() { 
	return SCENE_EXPORT_CLASS_ID; 
}

Class_ID OgreMaxExportClassDesc::ClassID() { 
	return Class_ID(0x2a961d1d, 0x8160db1); 
}

const TCHAR* OgreMaxExportClassDesc::Category() { 
	return _T("Ogre 3DSMax Exporter"); 
}

extern "C" {
//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

// what does this DLL contain?
__declspec( dllexport ) const TCHAR *
LibDescription() { 
	return _T("Ogre 3DSMax Exporter"); 
}

// how many plugin classes do we implement in this DLL?
__declspec( dllexport ) int
LibNumberClasses() { 
	return 1; 
}

// return a class descriptor class for each plugin class in this DLL; 
// 0-based, so i==0 is the first plugin in this DLL, and so on
__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: 
			return &OgreMaxExportDescInst; 
			break;
		default: 
			return 0; 
			break;
	}
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { 
	return VERSION_3DSMAX; 
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

}