/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
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

#ifndef __SHARED_UTILITIES_HEADER__
#define __SHARED_UTILITIES_HEADER__

#include <vector>
using namespace std;

//

#include <math.h>
#include <float.h>

//

#define _CRT_SECURE_NO_DEPRECATE

namespace SharedUtilities
{
// Fast template STL style utilities.
#include "fastvector.h"
#include "fastmap.h"
#include "fastintmap.h"
#include "faststring.h"

// Reference counting
#include "RefCount.h"

// Logging
#include "LogSystem.h"
#include "ConsoleLogger.h"
#include "FileLogger.h"

// Datastream
#include "DataStream.h"

// Math
#include "MathDefines.h"
#include "MathVector2.h"
#include "MathVector3.h"
#include "MathVector4.h"
#include "MathMatrix4x4.h"

// DDObject
#include "DDNotify.h"
#include "DDBase.h"
#include "DDObject.h"
#include "DDTypes.h"
#include "DDParse.h"

// Commandline Parser
extern CDDObject *GetCommandLineParms(void);
}

using namespace SharedUtilities;
/*
#ifdef _DEBUG
	#pragma comment(linker, "/DEFAULTLIB:SharedUtilitiesd.lib")
#else
	#pragma comment(linker, "/DEFAULTLIB:SharedUtilities.lib")
#endif
*/
#endif
