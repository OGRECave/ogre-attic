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

#include "OgreGLGpuProgram.h"
#include "OgreException.h"

using namespace Ogre;

GLGpuProgram::GLGpuProgram(const String& name, GpuProgramType gptype, const String& syntaxCode) :
    GpuProgram(name, gptype, syntaxCode)
{
    mProgramType = (gptype == GPT_VERTEX_PROGRAM) ? GL_VERTEX_PROGRAM_ARB : GL_FRAGMENT_PROGRAM_ARB;
    glGenProgramsARB_ptr(1, &mProgramID);
}

void GLGpuProgram::unload(void)
{
    glDeleteProgramsARB_ptr(1, &mProgramID);
}

void GLGpuProgram::loadFromSource(void)
{
    glBindProgramARB_ptr(mProgramType, mProgramID);
    glProgramStringARB_ptr(mProgramType, GL_PROGRAM_FORMAT_ASCII_ARB, mSource.length(), mSource.c_str());
    if (GL_INVALID_OPERATION == glGetError())
    {
        int errPos;
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
        char errPosStr[16];
        snprintf(errPosStr, 16, "%d", errPos);
        char* errStr = (char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        // XXX New exception code?
        Except(Exception::ERR_INTERNAL_ERROR, 
            "Cannot load GL vertex program " + mName + 
            ".  Line " + errPosStr + ":\n" + errStr, mName);
    }
    glBindProgramARB_ptr(mProgramType, 0);
}

