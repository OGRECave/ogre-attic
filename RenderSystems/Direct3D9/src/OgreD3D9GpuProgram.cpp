/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

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
#include "OgreD3D9GpuProgram.h"
#include "OgreMatrix4.h"
#include "OgreException.h"
#include "OgreLogManager.h"
namespace Ogre {

    //-----------------------------------------------------------------------------
	D3D9GpuProgram::D3D9GpuProgram(const String& name, GpuProgramType gptype, 
        const String& syntaxCode, LPDIRECT3DDEVICE9 pDev) 
        : GpuProgram(name, syntaxCode, gptype), mpDevice(pDev)
    {
    }
	//-----------------------------------------------------------------------------
	void D3D9GpuProgramParameters::setConstant(size_t index, const Matrix4& m)
    {
        // TODO
    }
	//-----------------------------------------------------------------------------
    D3D9GpuVertexProgram::D3D9GpuVertexProgram(const String& name, const String& syntaxCode, LPDIRECT3DDEVICE9 pDev) 
        : D3D9GpuProgram(name, GPT_VERTEX_PROGRAM, syntaxCode, pDev), mpVertexShader(NULL)
    {
        // do nothing here, all is done in load()
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuVertexProgram::loadFromSource(void)
    {
        // Assemble source into microcode
        LPD3DXBUFFER microcode;
        LPD3DXBUFFER errors;
		LogManager::getSingleton().logMessage(mSource);
        HRESULT hr = D3DXAssembleShader(
            mSource.c_str(),
            static_cast<UINT>(mSource.length()+1),
            NULL,               // no #define support
            NULL,               // no #include support
            0,                  // standard compile options
            &microcode,
            &errors);

        if (FAILED(hr))
        {
			
			Except(hr, "Cannot assemble D3D9 vertex shader " + mName + 
				": " + DXGetErrorDescription9(hr) + "\n" + 
				(const char*)errors->GetBufferPointer(),
                "D3D9GpuVertexProgram::loadFromSource");
            
        }

        // Create the shader
        hr = mpDevice->CreateVertexShader( 
            static_cast<DWORD*>(microcode->GetBufferPointer()), 
            &mpVertexShader);

        if (FAILED(hr))
        {
            Except(hr, "Cannot create D3D9 vertex shader " + mName + " from microcode.",
                "D3D9GpuVertexProgram::loadFromSource");
            
        }




    }
	//-----------------------------------------------------------------------------
    void D3D9GpuVertexProgram::unload(void)
    {
        SAFE_RELEASE(mpVertexShader);
    }
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
    D3D9GpuFragmentProgram::D3D9GpuFragmentProgram(const String& name, const String& syntaxCode, LPDIRECT3DDEVICE9 pDev) 
        : D3D9GpuProgram(name, GPT_FRAGMENT_PROGRAM, syntaxCode, pDev), mpPixelShader(NULL)
    {
        // do nothing here, all is done in load()
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuFragmentProgram::loadFromSource(void)
    {
        // Create the shader
        // Assemble source into microcode
        LPD3DXBUFFER microcode;
        LPD3DXBUFFER errors;
        HRESULT hr = D3DXAssembleShader(
            mSource.c_str(),
            static_cast<UINT>(mSource.length()),
            NULL,               // no #define support
            NULL,               // no #include support
            0,                  // standard compile options
            &microcode,
            &errors);

        if (FAILED(hr))
        {
            Except(hr, "Cannot assemble D3D9 pixel shader " + mName,
                "D3D9GpuFragmentProgram::loadFromSource");
            
        }

        // Create the shader
        hr = mpDevice->CreatePixelShader(
            static_cast<DWORD*>(microcode->GetBufferPointer()), 
            &mpPixelShader);

        if (FAILED(hr))
        {
            Except(hr, "Cannot create D3D9 vertex shader " + mName + " from microcode.",
                "D3D9GpuFragmentProgram::loadFromSource");
            
        }

    }
	//-----------------------------------------------------------------------------
    void D3D9GpuFragmentProgram::unload(void)
    {
        SAFE_RELEASE(mpPixelShader);
    }
	//-----------------------------------------------------------------------------

}

