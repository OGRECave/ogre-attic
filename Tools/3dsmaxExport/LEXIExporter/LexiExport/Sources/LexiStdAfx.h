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

#ifndef __stdafx_h__
#define __stdafx_h__

//

#define _CRT_SECURE_NO_DEPRECATE

#ifndef WINVER
#define WINVER			0x0500
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x0500
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS	0x0500
#endif

#ifndef _WIN32_IE
#define _WIN32_IE		0x0600
#endif

//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <commdlg.h>

//

#include <Ogre.h>
#include <OgreNoMemoryMacros.h>

#ifdef _DEBUG
 #pragma comment(lib, "OgreMain_d.lib")	
#else
 #pragma comment(lib, "OgreMain.lib")	
#endif

//

#include "SharedUtilities.h"
#include "GDIMetaLib.h"
#include "LexiIncludes.h"
#include "LexiOgreCore.h"
#include "LexiExportObject.h"

//

#define NDS_EXPORTER_TITLE			"LEXIExporter"



//

extern HINSTANCE GetCurrentInstance();
extern void RemoveIllegalChars(std::string& sString);

//

#endif // __stdafx_h__
