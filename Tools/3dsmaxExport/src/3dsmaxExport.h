
// 3dsmaxExport.h
//
// John Martin, 2002
//
// Entry point for Ogre 3dsmax exporter. Provides three maxscript functions
// to export static, keyframe animated, and skeletal animated meshes.


#ifndef __3dsmaxExport_H__
#define __3dsmaxExport_H__


#include "windows.h"
#include "max.h"
#include "Maxscrpt/MAXScrpt.h"
#include "Maxscrpt/value.h"
#include "Maxscrpt/arrays.h"
#include "Maxscrpt/structs.h"
#include "Maxscrpt/strings.h"
#include "Maxscrpt/name.h"

// bloody discreet programmers throwing their hash-define's around
#undef PI
#include "Ogre.h"


// Standard 3dsmax exports.
__declspec(dllexport) const TCHAR *LibDescription(void) { return _T("Ogre export library."); }
__declspec(dllexport) void LibInit(void) { }
__declspec(dllexport) ULONG LibVersion(void) { return VERSION_3DSMAX; }

// Define the new primitives using macros from SDK.
#include "Maxscrpt\definsfn.h"
	def_visible_primitive(ogre_export_static_mesh, "ogreExportStaticMesh");
	def_visible_primitive(ogre_export_anim_mesh, "ogreExportAnimMesh");
	def_visible_primitive(ogre_export_skel_mesh, "ogreExportSkelMesh");

// The functions that do the work.
Value *ogre_export_static_mesh_cf(Value **arg_list, int count);
Value *ogre_export_anim_mesh_cf(Value **arg_list, int count);
Value *ogre_export_skel_mesh_cf(Value **arg_list, int count);

// Stock standard dll function.
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
    return TRUE;
}


#endif