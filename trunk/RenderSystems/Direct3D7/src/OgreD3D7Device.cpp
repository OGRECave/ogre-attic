/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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
#include "OgreD3D7RenderSystem.h"
#include "OgreD3D7Device.h"
#include "OgreDDDriver.h"
#include "OgreDDVideoMode.h"
#include "OgreLight.h"
#include "OgreCamera.h"
#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre {

    static HRESULT CALLBACK EnumZBuffersCallback(DDPIXELFORMAT* pddpf,
            VOID* pFormats)
    {
        // Add to list of formats
        std::vector<DDPIXELFORMAT> *vec;
        vec = (std::vector<DDPIXELFORMAT>*)pFormats;

        if (pddpf->dwFlags & DDPF_ZBUFFER)
            vec->push_back(*pddpf);
        return D3DENUMRET_OK;

    }




    D3DDevice D3DDevice::operator=(const D3DDevice &orig)
    {


        mDeviceName = orig.mDeviceName;
        mDeviceDescription = orig.mDeviceDescription;
        mD3DDeviceDesc = orig.mD3DDeviceDesc;
        mIsHardwareAccelerated = orig.mIsHardwareAccelerated;
        mNeedsZBuffer = orig.mNeedsZBuffer;


        return *this;

    }

    // Default constructor
    D3DDevice::D3DDevice()
    {
        // Init pointers
        lpD3D = NULL;

    }

    // Copy Constructor
    D3DDevice::D3DDevice(const D3DDevice &ob)
    {
        lpD3D = ob.lpD3D;
        mViewport = ob.mViewport;
        mDeviceDescription = ob.mDeviceDescription;
        mDeviceName = ob.mDeviceName;
        mD3DDeviceDesc = ob.mD3DDeviceDesc;
        mIsHardwareAccelerated = ob.mIsHardwareAccelerated;
        mNeedsZBuffer = ob.mNeedsZBuffer;

    }

    // Enum constructor
    D3DDevice::D3DDevice(LPDIRECT3D7 lpDirect3D, LPSTR lpDeviceDesc, LPSTR lpDeviceName,
                    LPD3DDEVICEDESC7 lpD3DDeviceDesc)
    {
        // Init pointers
        lpD3D = NULL;

        // Copy pointer to Direct3D7 interface
        lpD3D = lpDirect3D;

        // Copy Name and Description
        mDeviceDescription = lpDeviceDesc;
        mDeviceName = lpDeviceName;

        // Is this a hardware or emulation device?
        mIsHardwareAccelerated = ( 0 != (lpD3DDeviceDesc->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) );


        // Copy device description
        // No need to differentiate between SW and HW anymore
        memcpy(&mD3DDeviceDesc, lpD3DDeviceDesc, sizeof(D3DDEVICEDESC7));

        char msg[255];
        sprintf(msg, "Detected Direct3D Device %s.", lpDeviceDesc);
        LogManager::getSingleton().logMessage(msg);
        logCaps();

        // Do we need a Z Buffer?
        mNeedsZBuffer = !(mD3DDeviceDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR);
        if (mNeedsZBuffer)
            sprintf(msg, "This device needs a Z-Buffer");
        else
            sprintf(msg, "This device does not need a Z-Buffer");

        LogManager::getSingleton().logMessage(msg);





    }

    D3DDevice::~D3DDevice()
    {
    }

    LPDIRECT3DDEVICE7 D3DDevice::createDevice(LPDIRECTDRAWSURFACE7 renderTarget)
    {
        LPDIRECT3DDEVICE7 dev;
        HRESULT hr;

        hr = lpD3D->CreateDevice(mD3DDeviceDesc.deviceGUID, renderTarget, &dev);
        if(FAILED(hr))
            throw Exception(hr, "Error creating new D3D device.",
            "D3DDevice::createDevice");

        return dev;

    }

    LPDIRECT3D7 D3DDevice::getID3D(void)
    {
        return lpD3D;
    }


    bool D3DDevice::HardwareAccelerated(void)
    {
        return mIsHardwareAccelerated;
    }

    void D3DDevice::logCaps(void)
    {
        // Sends capabilities of this driver to the log
        char msg[255];

        LogManager::getSingleton().logMessage("Direct3D Device Capabilities:");

        sprintf(msg, "  Hardware Accelerated: %i", HardwareAccelerated());
        LogManager::getSingleton().logMessage(msg);

        sprintf(msg, "  Mipmapping: %i", CanMipMap());
        LogManager::getSingleton().logMessage(msg);

        sprintf(msg, "  Bilinear Filtering: %i", CanBilinearFilter());
        LogManager::getSingleton().logMessage(msg);

        sprintf(msg, "  Trilinear Filtering: %i", CanTrilinearFilter());
        LogManager::getSingleton().logMessage(msg);

        sprintf(msg, "  Hardware Transform & Light: %i", CanHWTransformAndLight());
        LogManager::getSingleton().logMessage(msg);

        sprintf(msg, "  Max rendering colour depth: %i", RenderBitDepth());
        LogManager::getSingleton().logMessage(msg);

        sprintf(msg, "  Max single-pass texture layers: %i", MaxSinglePassTextureLayers());
        LogManager::getSingleton().logMessage(msg);

        sprintf(msg, "  Pixel fog supported: %i", ( mD3DDeviceDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE ) );
        LogManager::getSingleton().logMessage(msg);

        sprintf(msg, "  Vertex fog supported: %i", ( mD3DDeviceDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX) );
        LogManager::getSingleton().logMessage(msg);



    }


    void D3DDevice::Cleanup(void)
    {
        // Release DirectX Objects

        lpD3D = NULL;
    }


    void D3DDevice::createViewport(void)
    {
        /*
        HRESULT hr;
        char msg[255];
        sprintf(msg, "Creating Direct3D Viewport For %s.", mDeviceDescription.c_str());
        LogManager::getSingleton().logMessage(msg);

        // Create the viewport (D3D interface)
        // No need to create from device anymore

        // Set size (based on render target surface)
        setViewportSize();

        // Set current viewport for device
        hr = lpD3DDevice->SetViewport(&mViewport);
        if FAILED(hr)
            throw Exception(hr, "Error setting current viewport for device", "D3DDevice - setActive");

        LogManager::getSingleton().logMessage("Viewport Created OK");

        // Set up initial camera position
        //mCamera = new CCamera(this);
        //mCamera->setLocation(0.0f,0.0f,-10.0f);
        //mCamera->setTarget(0.0f,0.0f,0.0f);

        */
    }

    void D3DDevice::setViewportSize(void)
    {

        /*
        DDSURFACEDESC2 renderDesc;
        HRESULT hr;
        // Get surface desc of render target
        renderDesc.dwSize = sizeof(DDSURFACEDESC2);
        hr = lpRenderTarget->GetSurfaceDesc(&renderDesc);


        if FAILED(hr)
            throw Exception(hr, "Error getting render target surface description", "D3DDevice - setActive");

        // Set viewport to match the surface
        mViewport.dwX            = 0;
        mViewport.dwY            = 0;
        mViewport.dwWidth      = renderDesc.dwWidth;
        mViewport.dwHeight     = renderDesc.dwHeight;
        mViewport.dvMinZ       = 0.0f;
        mViewport.dvMaxZ       = 1.0f;



        // Set the parameters for the new viewport.

        // Store viewport sizes
        rcViewportRect.x1 = 0;
        rcViewportRect.x2 = renderDesc.dwWidth;
        rcViewportRect.y1 = 0;
        rcViewportRect.y2 = renderDesc.dwHeight;
        */
    }

    void D3DDevice::setViewMatrix(D3DMATRIX *mat)
    {
        /*
        HRESULT hr;

        hr = lpD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, mat);

        if (FAILED(hr))
            throw Exception(hr, "Error setting view matrix.",
                "D3DDevice - setViewMatrix");
        */
     }

    void D3DDevice::setProjectionMatrix(D3DMATRIX *mat)
    {
        /*
        HRESULT hr;
        hr = lpD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, mat);

        if (FAILED(hr))
            throw Exception(hr, "Error setting projection matrix.",
                "D3DDevice - setProjectionMatrix");
        */
    }

    void D3DDevice::setWorldMatrix(D3DMATRIX *mat)
    {
        /*
        HRESULT hr;
        hr = lpD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, mat);

        if (FAILED(hr))
            throw Exception(hr, "Error setting world matrix.",
                "D3DDevice - setWorldMatrix");
        */
    }

    void D3DDevice::beginScene(void)
    {
        /*
        HRESULT hr;

        // Clear viewport
        hr = lpD3DDevice->Clear(0, NULL,
            D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1L, 0L);
        if (FAILED(hr))
            throw Exception(hr, "Can't clear viewport.",
                "D3DDevice - beginScene");

        // BEGIN

        hr = lpD3DDevice->BeginScene();
        if (FAILED(hr))
            throw Exception(hr, "Can't begin scene.",
                "D3DDevice - beginScene");
        */
    }

    void D3DDevice::endScene(void)
    {
        //lpD3DDevice->EndScene();
    }


    void D3DDevice::setAmbientLight(float r, float g, float b)
    {

        // Just set white for now
        //HRESULT hr;
        //hr = lpD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff );
    }




    String D3DDevice::DeviceName(void)
    {
        return mDeviceName;
    }

    String D3DDevice::DeviceDescription(void)
    {
        return mDeviceDescription;
    }


    void D3DDevice::createDepthBuffer(LPDIRECTDRAWSURFACE7 renderTarget)
    {
        DWORD bestDepth;
        DDSURFACEDESC2 ddsd, src_ddsd;
        LPDIRECTDRAW7 lpDD7;
        LPDIRECTDRAWSURFACE7 lpZBuffer;
        HRESULT hr;

        LogManager::getSingleton().logMessage("Direct3D - Creating Z-Buffer");

        // First check we NEED a depth buffer - e.g. PowerVR doesn't need one
        if (mNeedsZBuffer)
        {
            // Get description from source surface
            ZeroMemory(&src_ddsd, sizeof(DDSURFACEDESC2));
            src_ddsd.dwSize = sizeof(DDSURFACEDESC2);
            renderTarget->GetSurfaceDesc(&src_ddsd);

            // Enumerate Depth Buffers
            mDepthBufferFormats.clear();
            lpD3D->EnumZBufferFormats(
				mD3DDeviceDesc.deviceGUID, 
				EnumZBuffersCallback, 
				(LPVOID)&mDepthBufferFormats );

            // Choose the best one
            // NB make sure Z buffer is the same depth as colour buffer (GeForce TnL problem)
            bestDepth = 0;

            std::vector<DDPIXELFORMAT>::iterator it, best_it;
            for( 
                it = mDepthBufferFormats.begin();
                it != mDepthBufferFormats.end();
                ++it )
            {
                if( (*it).dwZBufferBitDepth > bestDepth &&
                    (*it).dwZBufferBitDepth <= src_ddsd.ddpfPixelFormat.dwZBufferBitDepth )
                {
                    best_it = it;
                    bestDepth = (*it).dwZBufferBitDepth;
                }
            }

            // Setup the surface desc for the z-buffer.
            ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));

            ddsd.dwSize         = sizeof(DDSURFACEDESC2);
            ddsd.dwFlags        = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;

            ddsd.dwWidth        = src_ddsd.dwWidth;
            ddsd.dwHeight       = src_ddsd.dwHeight;

            ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;

            memcpy( &ddsd.ddpfPixelFormat, &(*best_it), sizeof(DDPIXELFORMAT) );

            // Software devices require system-memory depth buffers.
            if( mIsHardwareAccelerated )
                ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
            else
                ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

            // Create the depth-buffer.
            renderTarget->GetDDInterface( (VOID**)&lpDD7 );
            lpDD7->Release();

            if( FAILED( hr = lpDD7->CreateSurface( &ddsd, &lpZBuffer, NULL ) ) )
                Except(
                    hr, 
                    "Error creating depth buffer",
                    "D3DDevice::createDepthBuffer" );

            if( FAILED( hr = renderTarget->AddAttachedSurface(lpZBuffer) ) )
                Except(
                    hr, 
                    "Error attaching depth buffer to render target",
                    "D3DDevice::createDepthBuffer" );

            LogManager::getSingleton().logMessage( 
                LML_NORMAL, 
                "Z-Buffer created (%i-bit)", ddsd.ddpfPixelFormat.dwZBufferBitDepth );
        }
    }

    bool D3DDevice::CanMipMap(void)
    {
        return (mD3DDeviceDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPNEAREST) > 0;
    }

    bool D3DDevice::CanBilinearFilter(void)
    {
        return (mD3DDeviceDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR) > 0;
    }

    bool D3DDevice::CanTrilinearFilter(void)
    {
        return (mD3DDeviceDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR) > 0;
    }

    int D3DDevice::RenderBitDepth(void)
    {

        if (mD3DDeviceDesc.dwDeviceRenderBitDepth & DDBD_32)
            return 32;
        else if (mD3DDeviceDesc.dwDeviceRenderBitDepth & DDBD_24)
            return 24;
        else if (mD3DDeviceDesc.dwDeviceRenderBitDepth & DDBD_16)
            return 16;
        else if (mD3DDeviceDesc.dwDeviceRenderBitDepth & DDBD_8)
            return 8;
        else
            return 0;
    }

    int D3DDevice::ZBufferBitDepth(void)
    {
        switch(mD3DDeviceDesc.dwDeviceZBufferBitDepth)
        {
        case DDBD_8:
            return 8;
        case DDBD_16:
            return 16;
        case DDBD_24:
            return 24;
        case DDBD_32:
            return 32;
        }

        return 0;

    }
    bool D3DDevice::NeedsZBuffer(void)
    {
        return mNeedsZBuffer;
    }

    bool D3DDevice::CanHWTransformAndLight(void)
    {
        return (mD3DDeviceDesc.dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) > 0;
    }

    int D3DDevice::MaxSinglePassTextureLayers(void)
    {
        // The maximum number of texture layers the device can support in a singe pass

        return mD3DDeviceDesc.wMaxSimultaneousTextures;
    }


} // Namespace
