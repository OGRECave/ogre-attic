/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2003 The OGRE Teameeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#ifndef __GLGpuProgramManager_H__
#define __GLGpuProgramManager_H__

#include "OgreGpuProgramManager.h"

namespace Ogre {

class GLGpuProgramManager : public GpuProgramManager
{
public:
    GLGpuProgramManager() {}
    ~GLGpuProgramManager() {}
    GpuProgramParametersSharedPtr createParameters(void);
    GpuProgram* create(const String& name, GpuProgramType gptype, const String& syntaxCode);
};

}; //namespace Ogre

#endif //__GLGpuProgramManager_H__
