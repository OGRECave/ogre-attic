/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreD3D9Mappings.h"
#include "OgreGpuProgramManager.h"
#include "OgreSDDataChunk.h"

namespace Ogre {

    //-----------------------------------------------------------------------------
	D3D9GpuProgram::D3D9GpuProgram(const String& name, GpuProgramType gptype, 
        const String& syntaxCode, LPDIRECT3DDEVICE9 pDev) 
        : GpuProgram(name, gptype, syntaxCode), mpDevice(pDev), mpExternalMicrocode(NULL)
    {
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuProgram::load(void)
    {
        if (mIsLoaded)
        {
            unload();
        }

        if (mpExternalMicrocode)
        {
            loadFromMicrocode(mpExternalMicrocode);
        }
        else
        {
            // Normal load-from-source approach
            if (mLoadFromFile)
            {
                // find & load source code
                SDDataChunk chunk;
                GpuProgramManager::getSingleton()._findResourceData(mFilename, chunk);
                mSource = chunk.getAsString();
            }

            // Call polymorphic load
            loadFromSource();
        }

        mIsLoaded = true;

    }
	//-----------------------------------------------------------------------------
    void D3D9GpuProgram::loadFromSource(void)
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
            Except(hr, "Cannot assemble D3D9 shader " + mName,
                "D3D9GpuProgram::loadFromSource");
            
        }

        loadFromMicrocode(microcode);

        SAFE_RELEASE(microcode);
        SAFE_RELEASE(errors);
    }
	//-----------------------------------------------------------------------------
    D3D9GpuVertexProgram::D3D9GpuVertexProgram(const String& name, const String& syntaxCode, LPDIRECT3DDEVICE9 pDev) 
        : D3D9GpuProgram(name, GPT_VERTEX_PROGRAM, syntaxCode, pDev), mpVertexShader(NULL)
    {
        // do nothing here, all is done in load()
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuVertexProgram::loadFromMicrocode(LPD3DXBUFFER microcode)
    {
        // Create the shader
        HRESULT hr = mpDevice->CreateVertexShader( 
            static_cast<DWORD*>(microcode->GetBufferPointer()), 
            &mpVertexShader);

        if (FAILED(hr))
        {
            Except(hr, "Cannot create D3D9 vertex shader " + mName + " from microcode.",
                "D3D9GpuVertexProgram::loadFromMicrocode");
            
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
    void D3D9GpuFragmentProgram::loadFromMicrocode(LPD3DXBUFFER microcode)
    {
        // Create the shader
        HRESULT hr = mpDevice->CreatePixelShader(
            static_cast<DWORD*>(microcode->GetBufferPointer()), 
            &mpPixelShader);

        if (FAILED(hr))
        {
            Except(hr, "Cannot create D3D9 pixel shader " + mName + " from microcode.",
                "D3D9GpuFragmentProgram::loadFromMicrocode");
            
        }
    }
	//-----------------------------------------------------------------------------
    void D3D9GpuFragmentProgram::unload(void)
    {
        SAFE_RELEASE(mpPixelShader);
    }
	//-----------------------------------------------------------------------------

}

