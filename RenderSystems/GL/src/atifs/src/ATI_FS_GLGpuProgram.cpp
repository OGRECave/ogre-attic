/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2004 The OGRE Team
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

#include "ps_1_4.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreLogManager.h"
#include "ATI_FS_GLGpuProgram.h"

using namespace Ogre;


ATI_FS_GLGpuProgram::ATI_FS_GLGpuProgram(const String& name, GpuProgramType gptype, const String& syntaxCode) :
    GLGpuProgram(name, gptype, syntaxCode)
{
	mProgramType = GL_FRAGMENT_SHADER_ATI;
    mProgramID = glGenFragmentShadersATI(1);
}

void ATI_FS_GLGpuProgram::bindProgram(void)
{
	glEnable(mProgramType);
	glBindFragmentShaderATI(mProgramID);
}

void ATI_FS_GLGpuProgram::unbindProgram(void)
{
	glDisable(mProgramType);
}


void ATI_FS_GLGpuProgram::bindProgramParameters(GpuProgramParametersSharedPtr params)
{
	// program constants done internally by compiler for local
    
    if (params->hasRealConstantParams())
    {
        // Iterate over params and set the relevant ones
        GpuProgramParameters::RealConstantIterator realIt = 
            params->getRealConstantIterator();
        unsigned int index = 0;
		// test
        while (realIt.hasMoreElements())
        {
            GpuProgramParameters::RealConstantEntry* e = realIt.peekNextPtr();
            if (e->isSet)
            {
                glSetFragmentShaderConstantATI( GL_CON_0_ATI + index, e->val);
            }
            index++;
            realIt.moveNext();
        }
    }


}


void ATI_FS_GLGpuProgram::unload(void)
{
	glDeleteFragmentShaderATI(mProgramID);
}


void ATI_FS_GLGpuProgram::loadFromSource(void)
{

    PS_1_4 PS1_4Assembler;
	// attempt to compile the source
    bool Error = !PS1_4Assembler.compile(mSource.c_str());

    if(!Error) { 
		glBindFragmentShaderATI(mProgramID);
		glBeginFragmentShaderATI();
			// compile was successfull so send the machine instructions thru GL to GPU
			Error = !PS1_4Assembler.bindAllMachineInstToFragmentShader();
        glEndFragmentShaderATI();

		// check GL for GPU machine instruction bind erros
		if (Error)
		{
			Except(Exception::ERR_INTERNAL_ERROR, 
				"Cannot Bind ATI fragment shader :" + mName, mName); 
		}

    }
    else {
		// an error occured when compiling the ps_1_4 source code
		char buff[100];
        sprintf(buff,"error at position: %d\n%s\n", PS1_4Assembler.mCurrentLine, &mSource.c_str()[PS1_4Assembler.mCharPos] );

		LogManager::getSingleton().logMessage("Warning: atifs compiler reported the following errors:");
		LogManager::getSingleton().logMessage(buff);

		Except(Exception::ERR_INTERNAL_ERROR, 
			"Cannot Compile ATI fragment shader :" + mName + "\n" + buff, mName);// + 
    }


}

