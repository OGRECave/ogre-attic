/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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
#include <malloc.h>
#include "OgreD3D7RenderSystem.h"
#include "OgreDDDriver.h"
#include "OgreD3D7Device.h"
#include "OgreD3D7DeviceList.h"
#include "OgreDDDriverList.h"
#include "OgreDDVideoModeList.h"
#include "OgreDDVideoMode.h"
#include "OgreRenderWindow.h"
#include "OgreLogManager.h"
#include "OgreRenderSystem.h"
#include "OgreException.h"
#include "OgreSceneManager.h"
#include "OgreD3D7TextureManager.h"
#include "OgreViewport.h"
#include "OgreLight.h"
#include "OgreMatrix4.h"
#include "OgreMath.h"
#include "OgreD3D7RenderWindow.h"
#include "OgreCamera.h"
#include "OgreD3D7GpuProgramManager.h"


namespace Ogre {
    //-----------------------------------------------------------------------
    D3DRenderSystem::D3DRenderSystem(HINSTANCE hInstance)
    {
        OgreGuard( "D3DRenderSystem::D3DRenderSystem" );

        LogManager::getSingleton().logMessage(getName() + " created.");

        mlpD3DDevice = NULL;
        // Reset driver list
        mDriverList = NULL;
        mActiveDDDriver = NULL;
        mhInstance = hInstance;
        mHardwareBufferManager = NULL;
        mGpuProgramManager = NULL;

        initConfigOptions();

        // Initialise D3DX library
        D3DXInitialize();

        // set stages desc. to defaults
        for (int n = 0; n < OGRE_MAX_TEXTURE_LAYERS; n++)
        {
            mTexStageDesc[n].autoTexCoordType = TEXCALC_NONE;
            mTexStageDesc[n].coordIndex = 0;
            mTexStageDesc[n].texType = D3D_TEX_TYPE_NORMAL;
            mTexStageDesc[n].pTex = NULL;
        }

        mCurrentLights = 0;

        OgreUnguard();
    }

    //-----------------------------------------------------------------------
    D3DRenderSystem::~D3DRenderSystem()
    {
        OgreGuard( "D3DRenderSystem::~D3DRenderSystem" );

        SAFE_DELETE(mTextureManager);
        SAFE_DELETE(mDriverList);
        SAFE_DELETE(mCapabilities);
        SAFE_DELETE(mHardwareBufferManager);
        SAFE_DELETE(mGpuProgramManager);

        D3DXUninitialize();
        LogManager::getSingleton().logMessage(getName() + " destroyed.");

        OgreUnguard();
    }

    //-----------------------------------------------------------------------
    const String& D3DRenderSystem::getName(void) const
    {
        static String strName("Direct3D7 Rendering Subsystem");
        return strName;
    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::initConfigOptions(void)
    {
        OgreGuard( "D3DRenderSystem::initConfigOptions" );

        DDDriverList* ddList;
        DDDriver* dd;

        ConfigOption optDevice;
        ConfigOption optVideoMode;
        ConfigOption optFullScreen;
        ConfigOption optVSync;

        ddList = this->getDirectDrawDrivers();

        // Create option for devices
        optDevice.name = "Rendering Device";
        optDevice.currentValue = "";
        optDevice.possibleValues.clear();
        optDevice.immutable = false;

        // Option for video modes
        optVideoMode.name = "Video Mode";
        optVideoMode.currentValue = "";
        optVideoMode.immutable = false;

        // Option for full screen
        optFullScreen.name = "Full Screen";
        optFullScreen.possibleValues.push_back("Yes");
        optFullScreen.possibleValues.push_back("No");
        optFullScreen.currentValue = "Yes";


        unsigned k = ddList->count();
        // First, get DirectDraw driver options
        for( unsigned j = 0; j < ddList->count(); j++ )
        {
            dd = ddList->item(j);
            // Add to device option list
            optDevice.possibleValues.push_back( dd->DriverDescription() );

            // Make first one default
            if( j==0 )
            {
                optDevice.currentValue = dd->DriverDescription();

            }


        }

        // VSync option
        optVSync.name = "VSync";
        optVSync.immutable = false;
        optVSync.possibleValues.push_back("Yes");
        optVSync.possibleValues.push_back("No");
        optVSync.currentValue = "Yes";


        mOptions[optDevice.name] = optDevice;
        mOptions[optVideoMode.name] = optVideoMode;
        mOptions[optFullScreen.name] = optFullScreen;
        mOptions[optVSync.name] = optVSync;

        // Set default-based settings
        refreshDDSettings();

        OgreUnguard();
    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::refreshDDSettings(void)
    {
        OgreGuard( "D3DRenderSystem::refreshDDSettings" );

        DDVideoMode* vid;
        ConfigOption* optVideoMode;
        ConfigOption* optFullScreen;
        DDDriver* dd;

        // Stuffs DD-Driver specific settings

        // Find DD Driver selected in options
        ConfigOptionMap::iterator opt = mOptions.find("Rendering Device");

        if( opt != mOptions.end() )
        {
            for( unsigned j = 0; j < getDirectDrawDrivers()->count(); j++ )
            {
                dd = getDirectDrawDrivers()->item(j);
                if( dd->DriverDescription() == opt->second.currentValue )
                    break;
            }

            // Get fullScreen and Video mode options
            opt = mOptions.find("Video Mode");
            optVideoMode = &opt->second;
            opt = mOptions.find("Full Screen");
            optFullScreen = &opt->second;

            // Full screen forced?
            if (!(dd->CanRenderWindowed()))
            {
                setConfigOption("Full Screen", "Yes");
                optFullScreen->immutable = true;
            }
            else
                optFullScreen->immutable = false;

            // Get video modes for this device
            optVideoMode->possibleValues.clear();

            for( unsigned k = 0; k<dd->getVideoModeList()->count(); k++ )
            {
                vid = dd->getVideoModeList()->item(k);
                optVideoMode->possibleValues.push_back(vid->Description());
            }
        }

        OgreUnguard();
    }


    //-----------------------------------------------------------------------
    ConfigOptionMap& D3DRenderSystem::getConfigOptions(void)
    {
        // Return a COPY of the current config options
        return mOptions;

    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::setConfigOption(const String &name, const String &value)
    {
        OgreGuard( "D3DRenderSystem::setConfigOption" );

        char msg[128];

        sprintf(msg, "RenderSystem Option: %s = %s", name.c_str(), value.c_str());
        LogManager::getSingleton().logMessage(msg);

        // Find option
        ConfigOptionMap::iterator it = mOptions.find(name);

        // Update
        if( it != mOptions.end())
            it->second.currentValue = value;
        else
        {
            sprintf(msg, "Option named %s does not exist.", name.c_str());
            Except(Exception::ERR_INVALIDPARAMS,
                msg, "D3DRenderSystem::setConfigOption");
        }

        // Refresh other options if DD Driver changed
        if (name == "Rendering Device")
            refreshDDSettings();

        if (name == "Full Screen")
        {
            if (value == "No")
            {
                // Video mode is not applicable
                it = mOptions.find("Video Mode");
                it->second.currentValue = "N/A";
                it->second.immutable = true;
            }
            else
            {
                // Video mode is applicable
                it = mOptions.find("Video Mode");
                // default to 640 x 480 x 16
                it->second.currentValue = "640 x 480 @ 16-bit colour";
                it->second.immutable = false;
            }
        }

        OgreUnguard();
    }
    //-----------------------------------------------------------------------
    String D3DRenderSystem::validateConfigOptions(void)
    {
        // Check video mode specified in full screen mode
        ConfigOptionMap::iterator o = mOptions.find("Full Screen");
        if (o->second.currentValue == "Yes")
        {
            // Check video mode
            o = mOptions.find("Video Mode");
            if (o->second.currentValue == "")
            {
                return "A video mode must be selected for running in full-screen mode.";
            }
        }

        o = mOptions.find( "Rendering Device" );
        bool foundDriver = false;
        DDDriverList* driverList = getDirectDrawDrivers();
        for( ushort j=0; j < driverList->count(); j++ )
        {
            if( driverList->item(j)->DriverDescription() == o->second.currentValue )
            {
                foundDriver = true;
                break;
            }
        }
        if (!foundDriver)
        {
            // Just pick the first driver
            setConfigOption("Rendering Device", driverList->item(0)->DriverDescription());
            return "Your DirectX driver name has changed since the last time you ran OGRE; "
                "the 'Rendering Device' has been changed.";
        }
        return "";

    }
    //-----------------------------------------------------------------------
    RenderWindow* D3DRenderSystem::initialise(bool autoCreateWindow, const String& windowTitle)
    {
        RenderWindow* autoWindow = 0;


        LogManager::getSingleton().logMessage(
            "***************************************\n"
            "*** Direct3D Subsystem Initialising ***\n"
            "***************************************" );

        // ---------------------------
        // Init using current settings
        // ---------------------------

        // DirectDraw driver
        mActiveDDDriver = 0;
        ConfigOptionMap::iterator opt = mOptions.find("Rendering Device");
        for( unsigned j = 0; j<getDirectDrawDrivers()->count(); j++ )
        {
            if (getDirectDrawDrivers()->item(j)->DriverDescription() == opt->second.currentValue)
            {
                mActiveDDDriver = getDirectDrawDrivers()->item(j);
                break;
            }
        }

        if (!mActiveDDDriver)
            Except(Exception::ERR_INVALIDPARAMS, "Problems finding requested DirectDraw driver!",
            "D3DRenderSystem::initialise");


        // Sort out the creation of a new window if required
        if (autoCreateWindow)
        {
            // Full screen?
            bool fullScreen;
            opt = mOptions.find("Full Screen");
            if( opt == mOptions.end() )
                Except(999, "Can't find full screen option!",
                "D3DRenderSystem::initialise");
            if (opt->second.currentValue == "Yes")
                fullScreen = true;
            else
                fullScreen = false;

            // Get video mode
            DDVideoMode* vid;
            unsigned int height, width, colourDepth;

            vid = 0;
            String temp;
            if (fullScreen)
            {
                opt = mOptions.find("Video Mode");

                for( unsigned j=0; j<mActiveDDDriver->getVideoModeList()->count(); j++ )
                {
                    temp = mActiveDDDriver->getVideoModeList()->item(j)->Description();
                    if (temp == opt->second.currentValue)
                    {
                        vid = mActiveDDDriver->getVideoModeList()->item(j);
                        break;
                    }
                }

                if (!vid)
                    Except(9999, "Can't find requested video mode.",
                    "D3DRenderSystem::initilise");
                width = vid->mWidth;
                height = vid->mHeight;
                colourDepth = vid->mColourDepth;

            }
            else
            {
                // Notional height / width
                width = 800;
                height = 600;
                colourDepth = 0; // colour depth based on desktop
            }

            // Create myself a window
            autoWindow = this->createRenderWindow(windowTitle, width, height, colourDepth, fullScreen);



        }

        opt = mOptions.find("VSync");
        if (opt!=mOptions.end())
        {
            if (opt->second.currentValue == "Yes")
            {
                setWaitForVerticalBlank(true);
            }
            else
            {
                setWaitForVerticalBlank(false);
            }
        }


        LogManager::getSingleton().logMessage("*****************************************");
        LogManager::getSingleton().logMessage("*** Direct3D Subsystem Initialised Ok ***");
        LogManager::getSingleton().logMessage("*****************************************");

        // call superclass method
        RenderSystem::initialise(autoCreateWindow);

        // Create buffer manager
        mHardwareBufferManager = new D3D7HardwareBufferManager();
        // Create dummy gpu manager
        mGpuProgramManager = new D3D7GpuProgramManager();


        return autoWindow;

    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::reinitialise(void)
    {
        this->shutdown();
        this->initialise(true);
    }


    //-----------------------------------------------------------------------
    void D3DRenderSystem::shutdown(void)
    {
        RenderSystem::shutdown();

        // Cleanup
        // Release all DirectX resources
        //D3DTextr_InvalidateAllTextures();
        if (mActiveDDDriver)
        {
            mActiveDDDriver->Cleanup();
        }

        // Delete system objects
        SAFE_DELETE(mDriverList);

        mActiveDDDriver = NULL;



        // Write termination message
        LogManager::getSingleton().logMessage("*-*-* Direct3D Subsystem shutting down cleanly.");


    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::setAmbientLight(float r, float g, float b)
    {
        // Call D3D
        D3DCOLOR col = D3DRGB(r,g,b);

        HRESULT hr = __SetRenderState(D3DRENDERSTATE_AMBIENT, col);
        if (FAILED(hr))
            Except(hr, "Error setting ambient light.", "D3DRenderSystem::setAmbientLight");
    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::setShadingType(ShadeOptions so)
    {
        D3DSHADEMODE d3dMode;
        switch(so)
        {
        case SO_FLAT:
            d3dMode = D3DSHADE_FLAT;
            break;
        case SO_GOURAUD:
            d3dMode = D3DSHADE_GOURAUD;
            break;
        case SO_PHONG:
            d3dMode = D3DSHADE_PHONG;
            break;

        }

        HRESULT hr = __SetRenderState(D3DRENDERSTATE_SHADEMODE, d3dMode);
        if (FAILED(hr))
            Except(hr, "Error setting shading mode.", "D3DRenderSystem::setShadingType");

    }


    //-----------------------------------------------------------------------
    RenderWindow* D3DRenderSystem::createRenderWindow(const String &name, unsigned int width, unsigned int height, unsigned int colourDepth,
        bool fullScreen, int left, int top, bool depthBuffer, RenderWindow* parentWindowHandle)
    {
        static bool firstWindow = true;
        OgreGuard( "D3DRenderSystem::createRenderWindow" );

        String msg;

        // Make sure we don't already have a render target of the
        // same name as the one supplied
        if( mRenderTargets.find( name ) != mRenderTargets.end() )
        {
            msg = msg + "A render target of the same name (" + name + ") already "
                "exists. You cannot create a new window with this name.";
            Except(999,msg,"D3DRenderSystem::createRenderWindow");
        }

        RenderWindow* win = new D3D7RenderWindow();
        // Create window, supplying DD interface & hInstance
        win->create(name, width, height, colourDepth, fullScreen,
            left, top, depthBuffer, &mhInstance, mActiveDDDriver, parentWindowHandle);

        attachRenderTarget( *win );

        // If this is the first window, get the D3D device
        //  and create the texture manager, setup caps
        if (firstWindow)
        {
            win->getCustomAttribute("D3DDEVICE", &mlpD3DDevice);
            // Get caps
            mlpD3DDevice->GetCaps(&mD3DDeviceDesc);
            // Create my texture manager for use by others
            // Note this is a Singleton; pointer is held static by superclass
            mTextureManager = new D3DTextureManager(mlpD3DDevice);

            // Check for hardware stencil support
            // Get render target, then depth buffer and check format
            LPDIRECTDRAWSURFACE7 lpTarget;
            win->getCustomAttribute("DDBACKBUFFER", &lpTarget);
            DDSCAPS2 ddscaps;
            ZeroMemory(&ddscaps, sizeof(DDSCAPS2));
            ddscaps.dwCaps = DDSCAPS_ZBUFFER;
            lpTarget->GetAttachedSurface(&ddscaps, &lpTarget);
            lpTarget->Release();
            DDSURFACEDESC2 ddsd;
            ddsd.dwSize = sizeof(DDSURFACEDESC2);
            lpTarget->GetSurfaceDesc(&ddsd);
            DWORD stencil =  ddsd.ddpfPixelFormat.dwStencilBitDepth;
            if(stencil > 0)
            {
                mCapabilities->setCapability(RSC_HWSTENCIL);
                mCapabilities->setStencilBufferBitDepth(stencil);
                if ((mD3DDeviceDesc.dwStencilCaps & D3DSTENCILCAPS_INCR) && 
                    (mD3DDeviceDesc.dwStencilCaps & D3DSTENCILCAPS_DECR))
                    mCapabilities->setCapability(RSC_STENCIL_WRAP);
            }

            // Anisotropy?
            if (mD3DDeviceDesc.dwMaxAnisotropy > 1)
                mCapabilities->setCapability(RSC_ANISOTROPY);
            // Blending between stages supported
            mCapabilities->setCapability(RSC_BLENDING);
            // Cubemapping
            if (mD3DDeviceDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_CUBEMAP)
                mCapabilities->setCapability(RSC_CUBEMAPPING);

            // DOT3
            if (mD3DDeviceDesc.dwTextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)
                mCapabilities->setCapability(RSC_DOT3);

            // Set the number of texture units based on details from current device
            mCapabilities->setNumTextureUnits(mD3DDeviceDesc.wMaxSimultaneousTextures);

            mCapabilities->log(LogManager::getSingleton().getDefaultLog());

            firstWindow = false;
        }

        OgreUnguardRet( win );
    }

    RenderTexture * D3DRenderSystem::createRenderTexture( const String & name, unsigned int width, unsigned int height )
    {
        RenderTexture * rt = new D3D7RenderTexture( name, width, height );
        attachRenderTarget( *rt );
        return rt;
    }

    //-----------------------------------------------------------------------
    // Low-level overridden members
    //-----------------------------------------------------------------------
	//---------------------------------------------------------------------
    void D3DRenderSystem::_useLights(const LightList& lights, unsigned short limit)
    {
        LightList::const_iterator i, iend;
        iend = lights.end();
        unsigned short num = 0;
        for (i = lights.begin(); i != iend && num < limit; ++i, ++num)
        {
            setD3DLight(num, *i);
        }
        // Disable extra lights
        for (; num < mCurrentLights; ++num)
        {
            setD3DLight(num, NULL);
        }
        mCurrentLights = std::min(limit, static_cast<unsigned short>(lights.size()));

    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::setD3DLight(size_t index, Light* lt)
    {
        // Add to D3D
        HRESULT hr;
        D3DLIGHT7 d3dLight;

        if (!lt)
        {
            hr = mlpD3DDevice->LightEnable(index, FALSE);
            if (FAILED(hr))
                Except(hr, "Unable to disable light.", "D3DRenderSystem::setD3DLight");
        }
        else
        {
            switch (lt->getType())
            {
            case Light::LT_POINT:
                d3dLight.dltType = D3DLIGHT_POINT;
                break;
            case Light::LT_DIRECTIONAL:
                d3dLight.dltType = D3DLIGHT_DIRECTIONAL;
                break;
            case Light::LT_SPOTLIGHT:
                d3dLight.dltType = D3DLIGHT_SPOT;
                d3dLight.dvFalloff = lt->getSpotlightFalloff();
                d3dLight.dvTheta = Math::AngleUnitsToRadians(lt->getSpotlightInnerAngle());
                d3dLight.dvPhi = Math::AngleUnitsToRadians(lt->getSpotlightOuterAngle());
                break;
            }

            // Colours
            ColourValue col;
            col = lt->getDiffuseColour();
            d3dLight.dcvDiffuse.r = col.r;
            d3dLight.dcvDiffuse.g = col.g;
            d3dLight.dcvDiffuse.b = col.b;
            d3dLight.dcvDiffuse.a = col.a;

            col = lt->getSpecularColour();
            d3dLight.dcvSpecular.r = col.r;
            d3dLight.dcvSpecular.g = col.g;
            d3dLight.dcvSpecular.b = col.b;
            d3dLight.dcvSpecular.a = col.a;

            // Never use ambient for a movable light
            d3dLight.dcvAmbient.r = 0.0;
            d3dLight.dcvAmbient.g = 0.0;
            d3dLight.dcvAmbient.b = 0.0;
            d3dLight.dcvAmbient.a = 0.0;

            // Position (Irrelevant for directional)
            Vector3 vec;
            if (lt->getType() != Light::LT_DIRECTIONAL)
            {
                vec = lt->getDerivedPosition();

                d3dLight.dvPosition.x = vec.x;
                d3dLight.dvPosition.y = vec.y;
                d3dLight.dvPosition.z = vec.z;
            }
            // Direction (Irrelevant for point lights)
            if (lt->getType() != Light::LT_POINT)
            {
                vec = lt->getDerivedDirection();
                d3dLight.dvDirection.x = vec.x;
                d3dLight.dvDirection.y = vec.y;
                d3dLight.dvDirection.z = vec.z;
            }
            // Attenuation parameters
            d3dLight.dvRange = lt->getAttenuationRange();
            d3dLight.dvAttenuation0 = lt->getAttenuationConstant();
            d3dLight.dvAttenuation1 = lt->getAttenuationLinear();
            d3dLight.dvAttenuation2 = lt->getAttenuationQuadric();



            // Set light state
            hr = mlpD3DDevice->SetLight(index, &d3dLight);

            if (FAILED(hr))
                Except(hr, "Unable to set light details", "D3DRenderSystem::setD3DLight");

            hr = mlpD3DDevice->LightEnable(index, TRUE);
            if (FAILED(hr))
                Except(hr, "Unable to enable light.", "D3DRenderSystem::setD3DLight");
        }


    }
    //-----------------------------------------------------------------------
    D3DMATRIX D3DRenderSystem::makeD3DMatrix(const Matrix4& mat)
    {
        // Transpose matrix
        // D3D uses row vectors i.e. V*M
        // Ogre, OpenGL and everything else uses coloumn vectors i.e. M*V
        D3DMATRIX d3dMat;

        d3dMat.m[0][0] = mat[0][0];
        d3dMat.m[0][1] = mat[1][0];
        d3dMat.m[0][2] = mat[2][0];
        d3dMat.m[0][3] = mat[3][0];

        d3dMat.m[1][0] = mat[0][1];
        d3dMat.m[1][1] = mat[1][1];
        d3dMat.m[1][2] = mat[2][1];
        d3dMat.m[1][3] = mat[3][1];

        d3dMat.m[2][0] = mat[0][2];
        d3dMat.m[2][1] = mat[1][2];
        d3dMat.m[2][2] = mat[2][2];
        d3dMat.m[2][3] = mat[3][2];

        d3dMat.m[3][0] = mat[0][3];
        d3dMat.m[3][1] = mat[1][3];
        d3dMat.m[3][2] = mat[2][3];
        d3dMat.m[3][3] = mat[3][3];

        return d3dMat;
    }

    Matrix4 D3DRenderSystem::convertD3DMatrix(const D3DMATRIX& d3dmat)
    {
        // The reverse of makeD3DMatrix
        // Transpose matrix
        // D3D uses row vectors i.e. V*M
        // Ogre, OpenGL and everything else uses coloumn vectors i.e. M*V
        Matrix4 mat;
        for (unsigned row = 0; row < 4; ++row)
            for (unsigned col = 0; col < 4; ++col)
                mat[col][row] = d3dmat.m[row][col];

        return mat;


    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setWorldMatrix(const Matrix4 &m)
    {
        D3DMATRIX d3dmat;

        d3dmat = makeD3DMatrix(m);

        HRESULT hr = mlpD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &d3dmat);

        if (FAILED(hr))
            Except(hr, "Cannot set D3D world matrix",
            "D3DRenderSystem::_setWorldMatrix");
    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setViewMatrix(const Matrix4 &m)
    {
        D3DMATRIX d3dmat = makeD3DMatrix(m);

        // Flip the Z to compensate for D3D's left-handed co-ordinate system
        d3dmat.m[0][2] = -d3dmat.m[0][2];
        d3dmat.m[1][2] = -d3dmat.m[1][2];
        d3dmat.m[2][2] = -d3dmat.m[2][2];
        d3dmat.m[3][2] = -d3dmat.m[3][2];

        HRESULT hr = mlpD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &d3dmat);

        if (FAILED(hr))
            Except(hr, "Cannot set D3D view matrix",
            "D3DRenderSystem::_setViewMatrix");

    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setProjectionMatrix(const Matrix4 &m)
    {
        D3DMATRIX d3dmat = makeD3DMatrix(m);

        if( mActiveRenderTarget->requiresTextureFlipping() )
        {
            d3dmat._22 = - d3dmat._22;
        }

        HRESULT hr = mlpD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &d3dmat);

        if (FAILED(hr))
            Except(hr, "Cannot set D3D projection matrix",
            "D3DRenderSystem::_setProjectionMatrix");
    }


    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setSurfaceParams(const ColourValue &ambient,
        const ColourValue &diffuse, const ColourValue &specular,
        const ColourValue &emissive, const Real shininess)
    {
        // Remember last call
        static ColourValue lastAmbient = ColourValue::Black;
        static ColourValue lastDiffuse = ColourValue::Black;
        static ColourValue lastSpecular = ColourValue::Black;
        static ColourValue lastEmissive = ColourValue::Black;
        static Real lastShininess = 0.0;

        // Only update if changed
        if (ambient != lastAmbient || diffuse != lastDiffuse ||
            specular != lastSpecular || emissive != lastEmissive ||
            shininess != lastShininess)
        {

            // Convert to D3D
            D3DMATERIAL7 d3dMat;

            d3dMat.dcvDiffuse.r = diffuse.r;
            d3dMat.dcvDiffuse.g = diffuse.g;
            d3dMat.dcvDiffuse.b = diffuse.b;
            d3dMat.dcvDiffuse.a = diffuse.a;

            d3dMat.dcvAmbient.r = ambient.r;
            d3dMat.dcvAmbient.g = ambient.g;
            d3dMat.dcvAmbient.b = ambient.b;
            d3dMat.dcvAmbient.a = ambient.a;

            d3dMat.dcvSpecular.r = specular.r;
            d3dMat.dcvSpecular.g = specular.g;
            d3dMat.dcvSpecular.b = specular.b;
            d3dMat.dcvSpecular.a = specular.a;

            d3dMat.dcvEmissive.r = emissive.r;
            d3dMat.dcvEmissive.g = emissive.g;
            d3dMat.dcvEmissive.b = emissive.b;
            d3dMat.dcvEmissive.a = emissive.a;

            d3dMat.dvPower = shininess;

            HRESULT hr = mlpD3DDevice->SetMaterial(&d3dMat);
            if (FAILED(hr))
                Except(hr, "Error setting D3D material.", "D3DRenderSystem::_setSurfaceParams");

        }
    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setTexture(size_t stage, bool enabled, const String &texname)
    {
        HRESULT hr;
        D3DTexture* dt = static_cast< D3DTexture* >(TextureManager::getSingleton().getByName(texname));
        if (enabled && dt)
        {
            LPDIRECTDRAWSURFACE7 pTex = dt->getDDSurface();
            if (pTex != mTexStageDesc[stage].pTex)
            {
                hr = mlpD3DDevice->SetTexture(stage, pTex );
                if (FAILED(hr))
                    Except(hr, "Unable to set texture in D3D.", "D3DRenderSystem::_setTexture");

                // set stage desc.
                mTexStageDesc[stage].texType = _ogreTexTypeToD3DTexType(dt->getTextureType());
                mTexStageDesc[stage].pTex = pTex;
            }
        }
        else
        {
            hr = mlpD3DDevice->SetTexture(stage, 0);
            if (FAILED(hr))
                Except(hr, "Unable to disable texture in D3D.", "D3DRenderSystem::_setTexture");
            hr = __SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_DISABLE );
            if (FAILED(hr))
                Except(hr, "Unable to disable texture in D3D.", "D3DRenderSystem::_setTexture");

            // set stage desc. to defaults
            mTexStageDesc[stage].autoTexCoordType = TEXCALC_NONE;
            mTexStageDesc[stage].coordIndex = 0;
            mTexStageDesc[stage].texType = D3D_TEX_TYPE_NORMAL;
            mTexStageDesc[stage].pTex = NULL;
        }
    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setTextureCoordCalculation(size_t stage, TexCoordCalcMethod m)
    {
        HRESULT hr = S_OK;
        // record the stage state
        mTexStageDesc[stage].autoTexCoordType = m;

        switch( m )
        {
        case TEXCALC_NONE:
		    // if no calc we've already set index through D3D9RenderSystem::_setTextureCoordSet
            break;
        case TEXCALC_ENVIRONMENT_MAP: 
            // D3D7 does not support spherical reflection
            hr = __SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
            break;
        case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
            hr = __SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
            break;
        case TEXCALC_ENVIRONMENT_MAP_PLANAR:
            hr = __SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );
            break;
        case TEXCALC_ENVIRONMENT_MAP_NORMAL:
            hr = __SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
            break;
        }
        if( FAILED( hr ) )
            Except( hr, "Error setting texture coord calculation", "D3DRenderSystem::_setTextureCoordCalculation" );

    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setTextureMatrix(size_t stage, const Matrix4& xForm)
    {
		HRESULT hr;
		D3DMATRIX d3dMat; // the matrix we'll maybe apply
		Matrix4 newMat = xForm; // the matrix we'll apply after conv. to D3D format
        

		/* If envmap is applied, since D3D7 doesn't support spheremap,
		then we have to use texture transform to make the camera space normal
		reference the envmap properly. This isn't exactly the same as spheremap
		(it looks nasty on flat areas because the camera space normals are the same)
		but it's the best approximation we have in the absence of a proper spheremap */
		if (mTexStageDesc[stage].autoTexCoordType == TEXCALC_ENVIRONMENT_MAP)
		{
            Matrix4 ogreMatEnvMap = Matrix4::IDENTITY;
			// set env_map values
			ogreMatEnvMap[0][0] = 0.5f;
			ogreMatEnvMap[0][3] = 0.5f;
			ogreMatEnvMap[1][1] = -0.5f;
			ogreMatEnvMap[1][3] = 0.5f;
			// concatenate with the xForm
			newMat = newMat.concatenate(ogreMatEnvMap);
		}

        // If this is a cubic reflection, we need to modify using the view matrix
        if (mTexStageDesc[stage].autoTexCoordType == TEXCALC_ENVIRONMENT_MAP_REFLECTION)
        {
            D3DMATRIX viewMatrix; 

            // Get view matrix
            mlpD3DDevice->GetTransform(D3DTRANSFORMSTATE_VIEW, &viewMatrix);
            // Get transposed 3x3, ie since D3D is transposed just copy
            // We want to transpose since that will invert an orthonormal matrix ie rotation
            Matrix4 ogreViewTransposed;
            ogreViewTransposed[0][0] = viewMatrix.m[0][0];
            ogreViewTransposed[0][1] = viewMatrix.m[0][1];
            ogreViewTransposed[0][2] = viewMatrix.m[0][2];
            ogreViewTransposed[0][3] = 0.0f;

            ogreViewTransposed[1][0] = viewMatrix.m[1][0];
            ogreViewTransposed[1][1] = viewMatrix.m[1][1];
            ogreViewTransposed[1][2] = viewMatrix.m[1][2];
            ogreViewTransposed[1][3] = 0.0f;

            ogreViewTransposed[2][0] = viewMatrix.m[2][0];
            ogreViewTransposed[2][1] = viewMatrix.m[2][1];
            ogreViewTransposed[2][2] = viewMatrix.m[2][2];
            ogreViewTransposed[2][3] = 0.0f;

            ogreViewTransposed[3][0] = 0.0f;
            ogreViewTransposed[3][1] = 0.0f;
            ogreViewTransposed[3][2] = 0.0f;
            ogreViewTransposed[3][3] = 1.0f;
            
            newMat = newMat.concatenate(ogreViewTransposed);
        }

		// convert our matrix to D3D format
		d3dMat = makeD3DMatrix(newMat);

		// need this if texture is a cube map, to invert D3D's z coord
		if (mTexStageDesc[stage].autoTexCoordType != TEXCALC_NONE)
		{
			d3dMat._13 = -d3dMat._13;
			d3dMat._23 = -d3dMat._23;
			d3dMat._33 = -d3dMat._33;
			d3dMat._43 = -d3dMat._43;
		}

		// set the matrix if it's not the identity
        if (!(newMat == Matrix4::IDENTITY))
		{
			// tell D3D the dimension of tex. coord.
			int texCoordDim;
			switch (mTexStageDesc[stage].texType)
			{
			case D3D_TEX_TYPE_NORMAL:
				texCoordDim = 2;
				break;
			case D3D_TEX_TYPE_CUBE:
			case D3D_TEX_TYPE_VOLUME:
				texCoordDim = 3;
			}

			hr = __SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, texCoordDim );
			if (FAILED(hr))
				Except( hr, "Unable to set texture coord. dimension", "D3D9RenderSystem::_setTextureMatrix" );

			hr = mlpD3DDevice->SetTransform( 
                (D3DTRANSFORMSTATETYPE)(D3DTRANSFORMSTATE_TEXTURE0 + stage), &d3dMat );
			if (FAILED(hr))
				Except( hr, "Unable to set texture matrix", "D3D9RenderSystem::_setTextureMatrix" );
		}
		else
		{
			// disable all of this
			hr = __SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
			if( FAILED( hr ) )
				Except( hr, "Error setting texture matrix", "D3D9RenderSystem::_setTextureMatrix" );

			// set the identity matrix
			D3DUtil_SetIdentityMatrix( d3dMat );
			hr = mlpD3DDevice->SetTransform( 
                (D3DTRANSFORMSTATETYPE)(D3DTRANSFORMSTATE_TEXTURE0 + stage), &d3dMat );
			if( FAILED( hr ) )
				Except( hr, "Error setting texture matrix", "D3D9RenderSystem::_setTextureMatrix" );
		}
    }
	//---------------------------------------------------------------------
	void D3DRenderSystem::_setTextureCoordSet( size_t stage, size_t index )
	{
		HRESULT hr;
		hr = __SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, index );
		if( FAILED( hr ) )
			Except( hr, "Unable to set texture coord. set index", "D3DRenderSystem::_setTextureCoordSet" );
        // Record settings
        mTexStageDesc[stage].coordIndex = index;
	}
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm)
    {
        HRESULT hr;
        D3DTEXTURESTAGESTATETYPE tss;
        DWORD value;

        if (bm.blendType == LBT_COLOUR)
        {
            tss = D3DTSS_COLOROP;
        }
        else if (bm.blendType == LBT_ALPHA)
        {
            tss= D3DTSS_ALPHAOP;
        }

        switch (bm.operation)
        {
        case LBX_SOURCE1:
            value = D3DTOP_SELECTARG1;
            break;
        case LBX_SOURCE2:
            value = D3DTOP_SELECTARG2;
            break;
        case LBX_MODULATE:
            value = D3DTOP_MODULATE;
            break;
        case LBX_MODULATE_X2:
            value = D3DTOP_MODULATE2X;
            break;
        case LBX_MODULATE_X4:
            value = D3DTOP_MODULATE4X;
            break;
        case LBX_ADD:
            value = D3DTOP_ADD;
            break;
        case LBX_ADD_SIGNED:
            value = D3DTOP_ADDSIGNED;
            break;
        case LBX_ADD_SMOOTH:
            value = D3DTOP_ADDSMOOTH;
            break;
        case LBX_SUBTRACT:
            value = D3DTOP_SUBTRACT;
            break;
        case LBX_BLEND_DIFFUSE_ALPHA:
            value = D3DTOP_BLENDDIFFUSEALPHA;
            break;
        case LBX_BLEND_TEXTURE_ALPHA:
            value = D3DTOP_BLENDTEXTUREALPHA;
            break;
        case LBX_BLEND_CURRENT_ALPHA:
            value = D3DTOP_BLENDCURRENTALPHA;
            break;
        case LBX_BLEND_MANUAL:
            value = D3DTOP_BLENDFACTORALPHA;
            // Set factor in render state
            hr = __SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR,
                D3DRGBA(0,0,0,bm.factor));
            break;
        case LBX_DOTPRODUCT:
            if (mD3DDeviceDesc.dwTextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)
                value = D3DTOP_DOTPRODUCT3;
            else
                value = D3DTOP_MODULATE;
            break;
        }

        // Make call to set operation
        hr = __SetTextureStageState(stage, tss, value);

        // Now set up sources
        D3DCOLOR manualD3D;
        if (bm.blendType == LBT_COLOUR)
        {
            tss = D3DTSS_COLORARG1;
            manualD3D = D3DRGBA(bm.colourArg1.r,bm.colourArg1.g,bm.colourArg1.b,1.0);
        }
        else if (bm.blendType == LBT_ALPHA)
        {
            tss = D3DTSS_ALPHAARG1;
            manualD3D = D3DRGBA(0,0,0,bm.alphaArg1);
        }
        LayerBlendSource bs = bm.source1;
        for (int i = 0; i < 2; ++i)
        {
            switch (bs)
            {
            case LBS_CURRENT:
                value = D3DTA_CURRENT;
                break;
            case LBS_TEXTURE:
                value = D3DTA_TEXTURE;
                break;
            case LBS_DIFFUSE:
                value = D3DTA_DIFFUSE;
                break;
            case LBS_SPECULAR:
                value = D3DTA_SPECULAR;
                break;
            case LBS_MANUAL:
                value = D3DTA_TFACTOR;
                // Set factor in render state
                hr = __SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR,    manualD3D);
                break;
            }

            // Set source
            hr = __SetTextureStageState(stage, tss, value);

            // Source2
            bs = bm.source2;
            if (bm.blendType == LBT_COLOUR)
            {
                tss = D3DTSS_COLORARG2;
                manualD3D = D3DRGBA(bm.colourArg2.r,bm.colourArg2.g,bm.colourArg2.b,1.0);
            }
            else if (bm.blendType == LBT_ALPHA)
            {
                tss = D3DTSS_ALPHAARG2;
                manualD3D = D3DRGBA(0,0,0,bm.alphaArg2);
            }
        }
    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setTextureAddressingMode(size_t stage, TextureUnitState::TextureAddressingMode tam)
    {
        HRESULT hr;
        D3DTEXTUREADDRESS d3dType;

        switch(tam)
        {
        case TextureUnitState::TAM_WRAP:
            d3dType = D3DTADDRESS_WRAP;
            break;
        case TextureUnitState::TAM_MIRROR:
            d3dType = D3DTADDRESS_MIRROR;
            break;
        case TextureUnitState::TAM_CLAMP:
            d3dType = D3DTADDRESS_CLAMP;
            break;
        }


        hr = __SetTextureStageState(stage, D3DTSS_ADDRESS, d3dType);
    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor)
    {
        HRESULT hr;
        D3DBLEND d3dSrcBlend, d3dDestBlend;

        D3DBLEND* pBlend = &d3dSrcBlend;
        SceneBlendFactor ogreBlend = sourceFactor;

        for (int i = 0 ; i < 2; ++i)
        {
            switch(ogreBlend)
            {
            case SBF_ONE:
                *pBlend = D3DBLEND_ONE;
                break;
            case SBF_ZERO:
                *pBlend = D3DBLEND_ZERO;
                break;
            case SBF_DEST_COLOUR:
                *pBlend = D3DBLEND_DESTCOLOR;
                break;
            case SBF_SOURCE_COLOUR:
                *pBlend = D3DBLEND_SRCCOLOR;
                break;
            case SBF_ONE_MINUS_DEST_COLOUR:
                *pBlend = D3DBLEND_INVDESTCOLOR;
                break;
            case SBF_ONE_MINUS_SOURCE_COLOUR:
                *pBlend = D3DBLEND_INVSRCCOLOR;
                break;
            case SBF_DEST_ALPHA:
                *pBlend = D3DBLEND_DESTALPHA;
                break;
            case SBF_SOURCE_ALPHA:
                *pBlend = D3DBLEND_SRCALPHA;
                break;
            case SBF_ONE_MINUS_DEST_ALPHA:
                *pBlend = D3DBLEND_INVDESTALPHA;
                break;
            case SBF_ONE_MINUS_SOURCE_ALPHA:
                *pBlend = D3DBLEND_INVSRCALPHA;
                break;
            }
            ogreBlend = destFactor;
            pBlend = &d3dDestBlend;

        }

        hr = __SetRenderState(D3DRENDERSTATE_SRCBLEND, d3dSrcBlend);

        hr = __SetRenderState(D3DRENDERSTATE_DESTBLEND, d3dDestBlend);


    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value)
    {
        HRESULT hr;
        if (func != CMPF_ALWAYS_PASS)
        {
            if( FAILED( hr = __SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE,  TRUE ) ) )
                Except( hr, "Failed to enable alpha testing", 
                "D3DRenderSystem::_setAlphaRejectSettings" );
        }
        else
        {
            if( FAILED( hr = __SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE,  FALSE ) ) )
                Except( hr, "Failed to disable alpha testing", 
                "D3DRenderSystem::_setAlphaRejectSettings" );
        }

        // Set always just be sure
        hr = __SetRenderState(D3DRENDERSTATE_ALPHAFUNC, 
            convertCompareFunction(func));

        hr = __SetRenderState(D3DRENDERSTATE_ALPHAREF, value);    
    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setViewport(Viewport *vp)
    {
        // Check if viewport is different
        if (vp != mActiveViewport || vp->_isUpdated())
        {
            mActiveViewport = vp;
            mActiveRenderTarget = vp->getTarget();
            // Ok, it's different. Time to set render target (maybe)
            //  and viewport params.
            D3DVIEWPORT7 d3dvp;
            HRESULT hr;

            // Set render target
            // TODO - maybe only set when required?
            RenderTarget* target;
            target = vp->getTarget();
            // Get DD Back buffer
            LPDIRECTDRAWSURFACE7 pBack;
            target->getCustomAttribute("DDBACKBUFFER", &pBack);

            hr = mlpD3DDevice->SetRenderTarget( pBack, 0 );

            _setCullingMode( mCullingMode );

            // Set viewport dimensions
            d3dvp.dwX = vp->getActualLeft();
            d3dvp.dwY = vp->getActualTop();
            d3dvp.dwWidth = vp->getActualWidth();
            d3dvp.dwHeight = vp->getActualHeight();

            // Z-values from 0.0 to 1.0 (TODO - standardise with OpenGL?)
            d3dvp.dvMinZ = 0.0f;
            d3dvp.dvMaxZ = 1.0f;

            hr = mlpD3DDevice->SetViewport(&d3dvp);

            if (FAILED(hr))
                Except(hr, "Error setting D3D viewport.",
                "D3DRenderSystem::_setViewport");

            vp->_clearUpdatedFlag();

        }
    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::_beginFrame(void)
    {
        OgreGuard( "D3DRenderSystem::_beginFrame" );

        HRESULT hr;

        if (!mActiveViewport)
            Except(999, "Cannot begin frame - no viewport selected.",
            "D3DRenderSystem::_beginFrame");

        // Clear the viewport if required
        if (mActiveViewport->getClearEveryFrame())
        {
            clearFrameBuffer(FBT_COLOUR | FBT_DEPTH, 
                mActiveViewport->getBackgroundColour());
        }

        hr = mlpD3DDevice->BeginScene();
        if (FAILED(hr))
            Except(hr, "Error beginning frame.",
            "D3DRenderSystem::_beginFrame");

        // Moved here from _render, no point checking every rendering call
        static bool firstTime = true;
        if (firstTime)
        {
            // First-time setup
            // Set up some defaults

            // Allow alpha blending
            hr = __SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
            if (FAILED(hr))
                Except(hr, "Error enabling alpha blending option.",
                "D3DRenderSystem::_beginFrame");

            // Allow specular effects
            hr = __SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);
            if (FAILED(hr))
                Except(hr, "Error enabling specular option.",
                "D3DRenderSystem::_beginFrame");

            firstTime = false;
        }

        OgreUnguard();
    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::_render(const RenderOperation& op)
    {
        OgreGuard( "D3DRenderSystem::_render" );
        HRESULT hr;

        // Exit immediately if there is nothing to render
        // This caused a problem on FireGL 8800
        if (op.vertexData->vertexCount == 0)
            return;

        // call superclass
        RenderSystem::_render(op);
        // Set up vertex flags
        DWORD d3dVertexFormat = 0;
        unsigned int numTexCoords = 0;

        // Assume no more than 10 buffers!
        static unsigned char* pBufPtrs[10];

        // Lock all the buffers first
        // They're system memory buffers anyway
        const VertexBufferBinding::VertexBufferBindingMap binds = 
            op.vertexData->vertexBufferBinding->getBindings();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator bindi, bindend;
        bindend = binds.end();
        for (bindi = binds.begin(); bindi != bindend; ++bindi)
        {
            // lock from vertex start only
            pBufPtrs[bindi->first] = static_cast<unsigned char*>(
                bindi->second->lock(
                    op.vertexData->vertexStart * bindi->second->getVertexSize(),
                    op.vertexData->vertexCount * bindi->second->getVertexSize(),
                    HardwareBuffer::HBL_READ_ONLY)
                    );

        }

        // Determine vertex format

        // Struct for data pointers
        D3DDRAWPRIMITIVESTRIDEDDATA strideData;

        // Iterate over elements
        VertexDeclaration::VertexElementList::const_iterator elemi, elemend;
        const VertexDeclaration::VertexElementList elems = 
            op.vertexData->vertexDeclaration->getElements();
        elemend = elems.end();
        for (elemi = elems.begin(); elemi != elemend; ++elemi)
        {
            // Get a few basic details
            const VertexElement& elem = *elemi;
            unsigned short source = elem.getSource();
            size_t vertexSize = op.vertexData->vertexDeclaration->getVertexSize(source);
            size_t offset = elem.getOffset();
            // semantic-specific stuff
            switch (elem.getSemantic())
            {
            case VES_POSITION:
                d3dVertexFormat |= D3DFVF_XYZ; // Untransformed  
                strideData.position.lpvData = pBufPtrs[source] + offset;
                strideData.position.dwStride = static_cast<DWORD>(vertexSize);
                // Set up pointer
                break;
            case VES_NORMAL:
                d3dVertexFormat |= D3DFVF_NORMAL; 
                strideData.normal.lpvData = pBufPtrs[source] + offset;
                strideData.normal.dwStride = static_cast<DWORD>(vertexSize);
                break;
            case VES_DIFFUSE:
                d3dVertexFormat |= D3DFVF_DIFFUSE; 
                strideData.diffuse.lpvData = pBufPtrs[source] + offset;
                strideData.diffuse.dwStride = static_cast<DWORD>(vertexSize);
                break;
            case VES_SPECULAR:
                d3dVertexFormat |= D3DFVF_SPECULAR; 
                strideData.specular.lpvData = pBufPtrs[source] + offset;
                strideData.specular.dwStride = static_cast<DWORD>(vertexSize);
                break;
            case VES_TEXTURE_COORDINATES:
                // texcoords must go in order
                if (elem.getIndex() != numTexCoords)
                {
                    Except(Exception::ERR_INVALIDPARAMS, "Invalid vertex format, texture coordinates"
                        " must be in order wih no gaps.", "D3DRenderSystem::_render");
                }
                // Don't add D3DFVF_TEXn flag here, wait until we know how many total
                // However, specify size
                switch (elem.getType())
                {
                case VET_FLOAT1:
                    d3dVertexFormat |= D3DFVF_TEXCOORDSIZE1(numTexCoords);
                    break;
                case VET_FLOAT2:
                    d3dVertexFormat |= D3DFVF_TEXCOORDSIZE2(numTexCoords);
                    break;
                case VET_FLOAT3:
                    d3dVertexFormat |= D3DFVF_TEXCOORDSIZE3(numTexCoords);
                    break;
                case VET_FLOAT4:
                    d3dVertexFormat |= D3DFVF_TEXCOORDSIZE4(numTexCoords);
                    break;
                }

                strideData.textureCoords[numTexCoords].lpvData = pBufPtrs[source] + offset;
                strideData.textureCoords[numTexCoords].dwStride = static_cast<DWORD>(vertexSize);

                // Increment number of coords
                ++numTexCoords;
            }

        }
        // Add combined texture flag
        switch(numTexCoords)
        {
            case 0: 
                // do nothing
                break;
            case 1:
                d3dVertexFormat |= D3DFVF_TEX1;
                break;
            case 2:
                d3dVertexFormat |= D3DFVF_TEX2;
                break;
            case 3:
                d3dVertexFormat |= D3DFVF_TEX3;
                break;
            case 4:
                d3dVertexFormat |= D3DFVF_TEX4;
                break;
            case 5:
                d3dVertexFormat |= D3DFVF_TEX5;
                break;
            case 6:
                d3dVertexFormat |= D3DFVF_TEX6;
                break;
            case 7:
                d3dVertexFormat |= D3DFVF_TEX7;
                break;
            case 8:
                d3dVertexFormat |= D3DFVF_TEX8;
                break;
        }

        // Determine rendering operation
        D3DPRIMITIVETYPE primType;
        switch (op.operationType)
        {
        case RenderOperation::OT_POINT_LIST:
            primType = D3DPT_POINTLIST;
            break;
        case RenderOperation::OT_LINE_LIST:
            primType = D3DPT_LINELIST;
            break;
        case RenderOperation::OT_LINE_STRIP:
            primType = D3DPT_LINESTRIP;
            break;
        case RenderOperation::OT_TRIANGLE_LIST:
            primType = D3DPT_TRIANGLELIST;
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
            primType = D3DPT_TRIANGLESTRIP;
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
            primType = D3DPT_TRIANGLEFAN;
            break;
        }


        if (op.useIndexes)
        {
            // Get pointer to index buffer
            // D3D7 only allows 16-bit indexes, this is enforced in buffer manager
            unsigned short* pIdx = static_cast<unsigned short*>(
                op.indexData->indexBuffer->lock(
                    op.indexData->indexStart,
                    op.indexData->indexCount * sizeof(unsigned short),
                    HardwareBuffer::HBL_READ_ONLY) );

            hr = mlpD3DDevice->DrawIndexedPrimitiveStrided(primType,
                d3dVertexFormat, 
                &strideData, 
                static_cast<DWORD>(op.vertexData->vertexCount),
                pIdx, 
                static_cast<DWORD>(op.indexData->indexCount)
                , 0);
        }
        else
        {
            hr = mlpD3DDevice->DrawPrimitiveStrided(primType,
                d3dVertexFormat, 
                &strideData, 
                static_cast<DWORD>(op.vertexData->vertexCount),
                0);
        }

        // unlock buffers
        for (bindi = binds.begin(); bindi != bindend; ++bindi)
        {
            bindi->second->unlock();
        }

        if (FAILED(hr))
        {
            char szBuffer[512];
            D3DXGetErrorString( hr, 512, szBuffer );
            Except( hr, szBuffer, "D3DRenderSystem::_render");
        }




        OgreUnguard();
    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::_endFrame(void)
    {
        OgreGuard( "D3DRenderSystem::_endFrame" );

        HRESULT hr;
        hr = mlpD3DDevice->EndScene();

        if (FAILED(hr))
            Except(hr, "Error ending frame.",
            "D3DRenderSystem::_endFrame");

        OgreUnguard();
    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setCullingMode(CullingMode mode)
    {
        HRESULT hr;
        DWORD d3dMode;

        mCullingMode = mode;

        if (mode == CULL_NONE)
        {
            d3dMode = D3DCULL_NONE;
        }
        else if( mode == CULL_CLOCKWISE )
        {
            if( (mActiveRenderTarget->requiresTextureFlipping() && !mInvertVertexWinding) ||
                (!mActiveRenderTarget->requiresTextureFlipping() && mInvertVertexWinding))
                d3dMode = D3DCULL_CCW;
            else
                d3dMode = D3DCULL_CW;
        }
        else if (mode == CULL_ANTICLOCKWISE)
        {
            if( (mActiveRenderTarget->requiresTextureFlipping() && !mInvertVertexWinding) ||
                (!mActiveRenderTarget->requiresTextureFlipping() && mInvertVertexWinding))
                d3dMode = D3DCULL_CW;
            else
                d3dMode = D3DCULL_CCW;
        }

        hr = __SetRenderState(D3DRENDERSTATE_CULLMODE, d3dMode);
        if (FAILED(hr))
            Except(hr, "Unable to set D3D culling mode.",
            "D3DRenderSystem::_setCullingMode");

    }

    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction)
    {
        _setDepthBufferCheckEnabled(depthTest);
        _setDepthBufferWriteEnabled(depthWrite);
        _setDepthBufferFunction(depthFunction);

    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setDepthBufferCheckEnabled(bool enabled)
    {
        HRESULT hr;

        if (enabled)
        {
            // Use w-buffer if available
            if (mD3DDeviceDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_WBUFFER)
                hr = __SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_USEW);
            else
                hr = __SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
            if (FAILED(hr))
                Except(hr, "Error setting depth buffer test state.",
                "D3DRenderSystem::_setDepthBufferCheckEnabled");
        }
        else
        {
            hr = __SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
            if (FAILED(hr))
                Except(hr, "Error setting depth buffer test state.",
                "D3DRenderSystem::_setDepthBufferCheckEnabled");
        }
    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setDepthBufferWriteEnabled(bool enabled)
    {
        HRESULT hr;

        hr = __SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, enabled);
        if (FAILED(hr))
            Except(hr, "Error setting depth buffer write state.",
            "D3DRenderSystem::_setDepthBufferWriteEnabled");
    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setDepthBufferFunction(CompareFunction func)
    {
        HRESULT hr = __SetRenderState(D3DRENDERSTATE_ZFUNC, 
            convertCompareFunction(func));
        if (FAILED(hr))
            Except(hr, "Error setting depth buffer test function.",
            "D3DRenderSystem::_setDepthBufferFunction");
    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setDepthBias(ushort bias)
    {
        HRESULT hr = __SetRenderState(D3DRENDERSTATE_ZBIAS, 
            bias);
        if (FAILED(hr))
            Except(hr, "Error setting depth bias.",
            "D3DRenderSystem::_setDepthBias");
    }
    //-----------------------------------------------------------------------
    String D3DRenderSystem::getErrorDescription(long errCode)
    {
        char* errDesc;
        DWORD i;

        errDesc = new char[255];
        // Try windows errors first
        i = FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            DDERR_SURFACELOST,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR) errDesc,
            255,
            NULL
            );


        if (i == 0)
        {
            // Not found in windows message descriptions
            switch (errCode)
            {
            case DD_OK:
                strcpy(errDesc, "DD_OK: The request completed successfully.");
                break;
            case DDERR_ALREADYINITIALIZED:
                strcpy(errDesc, "DDERR_ALREADYINITIALIZED: The object has already been initialized.");
                break;
            case DDERR_BLTFASTCANTCLIP :
                strcpy(errDesc, "DDERR_BLTFASTCANTCLIP: A DirectDrawClipper object is attached to a source surface that has passed into a call to the IDirectDrawSurface7::BltFast method.");
                break;
            case DDERR_CANNOTATTACHSURFACE:
                strcpy(errDesc, "DDERR_CANNOTATTACHSURFACE: A surface cannot be attached to another requested surface.");
                break;
            case DDERR_CANNOTDETACHSURFACE:
                strcpy(errDesc, "DDERR_CANNOTATTACHSURFACE: A surface cannot be detached from another requested surface.");
                break;
            case DDERR_CANTCREATEDC:
                strcpy(errDesc, "DDERR_CANTCREATEDC: Windows can not create any more device contexts (DCs), or a DC was requested for a palette-indexed surface when the surface had no palette and the display mode was not palette-indexed.");
                break;
            case DDERR_CANTDUPLICATE:
                strcpy(errDesc, "DDERR_CANTDUPLICATE: Primary and 3-D surfaces, or surfaces that are implicitly created, cannot be duplicated.");
                break;
            case DDERR_CANTLOCKSURFACE:
                strcpy(errDesc, "DDERR_CANTLOCKSURFACE: Access to this surface is refused because an attempt was made to lock the primary surface without DCI support.");
                break;
            case DDERR_CANTPAGELOCK:
                strcpy(errDesc, "DDERR_CANTPAGELOCK: An attempt to page lock a surface failed. Page lock will not work on a display-memory surface or an emulated primary surface.");
                break;
            case DDERR_CANTPAGEUNLOCK:
                strcpy(errDesc, "DDERR_CANTPAGEUNLOCK: An attempt to page unlock a surface failed. Page unlock will not work on a display-memory surface or an emulated primary surface.");
                break;
            case DDERR_CLIPPERISUSINGHWND:
                strcpy(errDesc, "DDERR_CLIPPERISUSINGHWND: An attempt was made to set a clip list for a DirectDrawClipper object that is already monitoring a window handle.");
                break;
            case DDERR_COLORKEYNOTSET:
                strcpy(errDesc, "DDERR_COLORKEYNOTSET: No source color key is specified for this operation.");
                break;
            case DDERR_CURRENTLYNOTAVAIL:
                strcpy(errDesc, "DDERR_CURRENTLYNOTAVAIL: No support is currently available.");
                break;
            case DDERR_DCALREADYCREATED:
                strcpy(errDesc, "DDERR_DCALREADYCREATED: A device context (DC) has already been returned for this surface. Only one DC can be retrieved for each surface.");
                break;
            case DDERR_DEVICEDOESNTOWNSURFACE:
                strcpy(errDesc, "DDERR_DEVICEDOESNTOWNSURFACE: Surfaces created by one DirectDraw device cannot be used directly by another DirectDraw device.");
                break;
            case DDERR_DIRECTDRAWALREADYCREATED:
                strcpy(errDesc, "DDERR_DIRECTDRAWALREADYCREATED: A DirectDraw object representing this driver has already been created for this process.");
                break;
            case DDERR_EXCEPTION:
                strcpy(errDesc, "DDERR_EXCEPTION: An exception was encountered while performing the requested operation.");
                break;
            case DDERR_EXCLUSIVEMODEALREADYSET:
                strcpy(errDesc, "DDERR_EXCLUSIVEMODEALREADYSET: An attempt was made to set the cooperative level when it was already set to exclusive.");
                break;
            case DDERR_EXPIRED:
                strcpy(errDesc, "DDERR_EXPIRED: The data has expired and is therefore no longer valid.");
                break;
            case DDERR_GENERIC:
                strcpy(errDesc, "DDERR_GENERIC: There is an undefined error condition.");
                break;
            case DDERR_HEIGHTALIGN:
                strcpy(errDesc, "DDERR_HEIGHTALIGN: The height of the provided rectangle is not a multiple of the required alignment.");
                break;
            case DDERR_HWNDALREADYSET:
                strcpy(errDesc, "DDERR_HWNDALREADYSET: The DirectDraw cooperative level window handle has already been set. It cannot be reset while the process has surfaces or palettes created.");
                break;
            case DDERR_HWNDSUBCLASSED:
                strcpy(errDesc, "DDERR_HWNDSUBCLASSED: DirectDraw is prevented from restoring state because the DirectDraw cooperative level window handle has been subclassed.");
                break;
            case DDERR_IMPLICITLYCREATED:
                strcpy(errDesc, "DDERR_IMPLICITLYCREATED: The surface cannot be restored because it is an implicitly created surface.");
                break;
            case DDERR_INCOMPATIBLEPRIMARY:
                strcpy(errDesc, "DDERR_INCOMPATIBLEPRIMARY: The primary surface creation request does not match with the existing primary surface.");
                break;
            case DDERR_INVALIDCAPS:
                strcpy(errDesc, "DDERR_INVALIDCAPS: One or more of the capability bits passed to the callback function are incorrect.");
                break;
            case DDERR_INVALIDCLIPLIST:
                strcpy(errDesc, "DDERR_INVALIDCLIPLIST: DirectDraw does not support the provided clip list.");
                break;
            case DDERR_INVALIDDIRECTDRAWGUID:
                strcpy(errDesc, "DDERR_INVALIDDIRECTDRAWGUID: The globally unique identifier (GUID) passed to the DirectDrawCreate function is not a valid DirectDraw driver identifier.");
                break;
            case DDERR_INVALIDMODE:
                strcpy(errDesc, "DDERR_INVALIDMODE: DirectDraw does not support the requested mode.");
                break;
            case DDERR_INVALIDOBJECT:
                strcpy(errDesc, "DDERR_INVALIDOBJECT: DirectDraw received a pointer that was an invalid DirectDraw object.");
                break;
            case DDERR_INVALIDPARAMS:
                strcpy(errDesc, "DDERR_INVALIDPARAMS: One or more of the parameters passed to the method are incorrect.");
                break;
            case DDERR_INVALIDPIXELFORMAT:
                strcpy(errDesc, "DDERR_INVALIDPIXELFORMAT: The pixel format was invalid as specified.");
                break;
            case DDERR_INVALIDPOSITION:
                strcpy(errDesc, "DDERR_INVALIDPOSITION: The position of the overlay on the destination is no longer legal.");
                break;
            case DDERR_INVALIDRECT:
                strcpy(errDesc, "DDERR_INVALIDRECT: The provided rectangle was invalid.");
                break;
            case DDERR_INVALIDSTREAM:
                strcpy(errDesc, "DDERR_INVALIDSTREAM: The specified stream contains invalid data.");
                break;
            case DDERR_INVALIDSURFACETYPE:
                strcpy(errDesc, "DDERR_INVALIDSURFACETYPE: The requested operation could not be performed because the surface was of the wrong type.");
                break;
            case DDERR_LOCKEDSURFACES:
                strcpy(errDesc, "DDERR_LOCKEDSURFACES: One or more surfaces are locked, causing the failure of the requested operation.");
                break;
            case DDERR_MOREDATA:
                strcpy(errDesc, "DDERR_MOREDATA: There is more data available than the specified buffer size can hold.");
                break;
            case DDERR_NO3D:
                strcpy(errDesc, "DDERR_NO3D: No 3-D hardware or emulation is present.");
                break;
            case DDERR_NOALPHAHW:
                strcpy(errDesc, "DDERR_NOALPHAHW: No alpha acceleration hardware is present or available, causing the failure of the requested operation.");
                break;
            case DDERR_NOBLTHW:
                strcpy(errDesc, "DDERR_NOBLTHW: No blitter hardware is present.");
                break;
            case DDERR_NOCLIPLIST:
                strcpy(errDesc, "DDERR_NOCLIPLIST: No clip list is available.");
                break;
            case DDERR_NOCLIPPERATTACHED:
                strcpy(errDesc, "DDERR_NOCLIPPERATTACHED: No DirectDrawClipper object is attached to the surface object.");
                break;
            case DDERR_NOCOLORCONVHW:
                strcpy(errDesc, "DDERR_NOCOLORCONVHW: The operation cannot be carried out because no color-conversion hardware is present or available.");
                break;
            case DDERR_NOCOLORKEY:
                strcpy(errDesc, "DDERR_NOCOLORKEY: The surface does not currently have a color key.");
                break;
            case DDERR_NOCOLORKEYHW:
                strcpy(errDesc, "DDERR_NOCOLORKEYHW: The operation cannot be carried out because there is no hardware support for the destination color key.");
                break;
            case DDERR_NOCOOPERATIVELEVELSET:
                strcpy(errDesc, "DDERR_NOCOOPERATIVELEVELSET: A create function is called without the IDirectDraw7::SetCooperativeLevel method being called.");
                break;
            case DDERR_NODC:
                strcpy(errDesc, "DDERR_NODC: No DC has ever been created for this surface.");
                break;
            case DDERR_NODDROPSHW:
                strcpy(errDesc, "DDERR_NODDROPSHW: No DirectDraw raster operation (ROP) hardware is available.");
                break;
            case DDERR_NODIRECTDRAWHW:
                strcpy(errDesc, "DDERR_NODIRECTDRAWHW: Hardware-only DirectDraw object creation is not possible; the driver does not support any hardware.");
                break;
            case DDERR_NODIRECTDRAWSUPPORT:
                strcpy(errDesc, "DDERR_NODIRECTDRAWSUPPORT: DirectDraw support is not possible with the current display driver.");
                break;
            case DDERR_NOEMULATION:
                strcpy(errDesc, "DDERR_NOEMULATION: Software emulation is not available.");
                break;
            case DDERR_NOEXCLUSIVEMODE:
                strcpy(errDesc, "DDERR_NOEXCLUSIVEMODE: The operation requires the application to have exclusive mode, but the application does not have exclusive mode.");
                break;
            case DDERR_NOFLIPHW:
                strcpy(errDesc, "DDERR_NOFLIPHW: Flipping visible surfaces is not supported.");
                break;
            case DDERR_NOFOCUSWINDOW:
                strcpy(errDesc, "DDERR_NOFOCUSWINDOW: An attempt was made to create or set a device window without first setting the focus window.");
                break;
            case DDERR_NOGDI:
                strcpy(errDesc, "DDERR_NOGDI: No GDI is present.");
                break;
            case DDERR_NOHWND:
                strcpy(errDesc, "DDERR_NOHWND: Clipper notification requires a window handle, or no window handle has been previously set as the cooperative level window handle.");
                break;
            case DDERR_NOMIPMAPHW:
                strcpy(errDesc, "DDERR_NOMIPMAPHW: The operation cannot be carried out because no mipmap capable texture mapping hardware is present or available.");
                break;
            case DDERR_NOMIRRORHW:
                strcpy(errDesc, "DDERR_NOMIRRORHW: The operation cannot be carried out because no mirroring hardware is present or available.");
                break;
            case DDERR_NONONLOCALVIDMEM:
                strcpy(errDesc, "DDERR_NONONLOCALVIDMEM: An attempt was made to allocate non-local video memory from a device that does not support non-local video memory.");
                break;
            case DDERR_NOOPTIMIZEHW:
                strcpy(errDesc, "DDERR_NOOPTIMIZEHW: The device does not support optimized surfaces.");
                break;
            case DDERR_NOOVERLAYDEST:
                strcpy(errDesc, "DDERR_NOOVERLAYDEST: The IDirectDrawSurface4::GetOverlayPosition method is called on an overlay that the IDirectDrawSurface7::UpdateOverlay method has not been called on to establish a destination.");
                break;
            case DDERR_NOOVERLAYHW:
                strcpy(errDesc, "DDERR_NOOVERLAYHW: The operation cannot be carried out because no overlay hardware is present or available.");
                break;
            case DDERR_NOPALETTEATTACHED:
                strcpy(errDesc, "DDERR_NOPALETTEATTACHED: No palette object is attached to this surface.");
                break;
            case DDERR_NOPALETTEHW:
                strcpy(errDesc, "DDERR_NOPALETTEHW: There is no hardware support for 16- or 256-color palettes.");
                break;
            case DDERR_NORASTEROPHW:
                strcpy(errDesc, "DDERR_NORASTEROPHW: The operation cannot be carried out because no appropriate raster operation hardware is present or available.");
                break;
            case DDERR_NOROTATIONHW:
                strcpy(errDesc, "DDERR_NOROTATIONHW: The operation cannot be carried out because no rotation hardware is present or available.");
                break;
            case DDERR_NOSTRETCHHW:
                strcpy(errDesc, "DDERR_NOSTRETCHHW: The operation cannot be carried out because there is no hardware support for stretching.");
                break;
            case DDERR_NOT4BITCOLOR:
                strcpy(errDesc, "DDERR_NOT4BITCOLOR: The DirectDrawSurface object is not using a 4-bit color palette and the requested operation requires a 4-bit color palette.");
                break;
            case DDERR_NOT4BITCOLORINDEX:
                strcpy(errDesc, "DDERR_NOT4BITCOLORINDEX: The DirectDrawSurface object is not using a 4-bit color index palette and the requested operation requires a 4-bit color index palette.");
                break;
            case DDERR_NOT8BITCOLOR:
                strcpy(errDesc, "DDERR_NOT8BITCOLOR: The DirectDrawSurface object is not using an 8-bit color palette and the requested operation requires an 8-bit color palette.");
                break;
            case DDERR_NOTAOVERLAYSURFACE:
                strcpy(errDesc, "DDERR_NOTAOVERLAYSURFACE: An overlay component is called for a non-overlay surface.");
                break;
            case DDERR_NOTEXTUREHW:
                strcpy(errDesc, "DDERR_NOTEXTUREHW: The operation cannot be carried out because no texture-mapping hardware is present or available.");
                break;
            case DDERR_NOTFLIPPABLE:
                strcpy(errDesc, "DDERR_NOTFLIPPABLE: An attempt has been made to flip a surface that cannot be flipped.");
                break;
            case DDERR_NOTFOUND:
                strcpy(errDesc, "DDERR_NOTFOUND: The requested item was not found.");
                break;
            case DDERR_NOTINITIALIZED:
                strcpy(errDesc, "DDERR_NOTINITIALIZED: An attempt was made to call an interface method of a DirectDraw object created by CoCreateInstance before the object was initialized.");
                break;
            case DDERR_NOTLOADED:
                strcpy(errDesc, "DDERR_NOTLOADED: The surface is an optimized surface, but it has not yet been allocated any memory.");
                break;
            case DDERR_NOTLOCKED:
                strcpy(errDesc, "DDERR_NOTLOCKED: An attempt is made to unlock a surface that was not locked.");
                break;
            case DDERR_NOTPAGELOCKED:
                strcpy(errDesc, "DDERR_NOTPAGELOCKED: An attempt is made to page unlock a surface with no outstanding page locks.");
                break;
            case DDERR_NOTPALETTIZED:
                strcpy(errDesc, "DDERR_NOTPALETTIZED: The surface being used is not a palette-based surface.");
                break;
            case DDERR_NOVSYNCHW:
                strcpy(errDesc, "DDERR_NOVSYNCHW: The operation cannot be carried out because there is no hardware support for vertical blank synchronized operations.");
                break;
            case DDERR_NOZBUFFERHW:
                strcpy(errDesc, "DDERR_NOZBUFFERHW: The operation to create a z-buffer in display memory or to perform a blit using a z-buffer cannot be carried out because there is no hardware support for z-buffers.");
                break;
            case DDERR_NOZOVERLAYHW:
                strcpy(errDesc, "DDERR_NOZOVERLAYHW: The overlay surfaces cannot be z-layered based on the z-order because the hardware does not support z-ordering of overlays.");
                break;
            case DDERR_OUTOFCAPS:
                strcpy(errDesc, "DDERR_OUTOFCAPS: The hardware needed for the requested operation has already been allocated.");
                break;
            case DDERR_OUTOFMEMORY:
                strcpy(errDesc, "DDERR_OUTOFMEMORY: DirectDraw does not have enough memory to perform the operation.");
                break;
            case DDERR_OUTOFVIDEOMEMORY:
                strcpy(errDesc, "DDERR_OUTOFVIDEOMEMORY: DirectDraw does not have enough display memory to perform the operation.");
                break;
            case DDERR_OVERLAPPINGRECTS:
                strcpy(errDesc, "DDERR_OVERLAPPINGRECTS: Operation could not be carried out because the source and destination rectangles are on the same surface and overlap each other.");
                break;
            case DDERR_OVERLAYCANTCLIP:
                strcpy(errDesc, "DDERR_OVERLAYCANTCLIP: The hardware does not support clipped overlays.");
                break;
            case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
                strcpy(errDesc, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE: An attempt was made to have more than one color key active on an overlay.");
                break;
            case DDERR_OVERLAYNOTVISIBLE:
                strcpy(errDesc, "DDERR_OVERLAYNOTVISIBLE: The IDirectDrawSurface7::GetOverlayPosition method is called on a hidden overlay.");
                break;
            case DDERR_PALETTEBUSY:
                strcpy(errDesc, "DDERR_PALETTEBUSY: Access to this palette is refused because the palette is locked by another thread.");
                break;
            case DDERR_PRIMARYSURFACEALREADYEXISTS:
                strcpy(errDesc, "DDERR_PRIMARYSURFACEALREADYEXISTS: This process has already created a primary surface.");
                break;
            case DDERR_REGIONTOOSMALL:
                strcpy(errDesc, "DDERR_REGIONTOOSMALL: The region passed to the IDirectDrawClipper::GetClipList method is too small.");
                break;
            case DDERR_SURFACEALREADYATTACHED:
                strcpy(errDesc, "DDERR_SURFACEALREADYATTACHED: An attempt was made to attach a surface to another surface to which it is already attached.");
                break;
            case DDERR_SURFACEALREADYDEPENDENT:
                strcpy(errDesc, "DDERR_SURFACEALREADYDEPENDENT: An attempt was made to make a surface a dependency of another surface to which it is already dependent.");
                break;
            case DDERR_SURFACEBUSY:
                strcpy(errDesc, "DDERR_SURFACEBUSY: Access to the surface is refused because the surface is locked by another thread.");
                break;
            case DDERR_SURFACEISOBSCURED:
                strcpy(errDesc, "DDERR_SURFACEISOBSCURED: Access to the surface is refused because the surface is obscured.");
                break;
            case DDERR_SURFACELOST:
                strcpy(errDesc, "DDERR_SURFACELOST: Access to the surface is refused because the surface memory is gone. Call the IDirectDrawSurface7::Restore method on this surface to restore the memory associated with it.");
                break;
            case DDERR_SURFACENOTATTACHED:
                strcpy(errDesc, "DDERR_SURFACENOTATTACHED: The requested surface is not attached.");
                break;
            case DDERR_TOOBIGHEIGHT:
                strcpy(errDesc, "DDERR_TOOBIGHEIGHT: The height requested by DirectDraw is too large.");
                break;
            case DDERR_TOOBIGSIZE:
                strcpy(errDesc, "DDERR_TOOBIGSIZE: The size requested by DirectDraw is too large. However, the individual height and width are valid sizes.");
                break;
            case DDERR_TOOBIGWIDTH:
                strcpy(errDesc, "DDERR_TOOBIGWIDTH: The width requested by DirectDraw is too large.");
                break;
            case DDERR_UNSUPPORTED:
                strcpy(errDesc, "DDERR_UNSUPPORTED: The operation is not supported.");
                break;
            case DDERR_UNSUPPORTEDFORMAT:
                strcpy(errDesc, "DDERR_UNSUPPORTEDFORMAT: The FourCC format requested is not supported by DirectDraw.");
                break;
            case DDERR_UNSUPPORTEDMASK:
                strcpy(errDesc, "DDERR_UNSUPPORTEDMASK: The bitmask in the pixel format requested is not supported by DirectDraw.");
                break;
            case DDERR_UNSUPPORTEDMODE:
                strcpy(errDesc, "DDERR_UNSUPPORTEDMODE: The display is currently in an unsupported mode.");
                break;
            case DDERR_VERTICALBLANKINPROGRESS:
                strcpy(errDesc, "DDERR_VERTICALBLANKINPROGRESS: A vertical blank is in progress.");
                break;
            case DDERR_VIDEONOTACTIVE:
                strcpy(errDesc, "DDERR_VIDEONOTACTIVE: The video port is not active.");
                break;
            case DDERR_WASSTILLDRAWING:
                strcpy(errDesc, "DDERR_WASSTILLDRAWING: The previous blit operation that is transferring information to or from this surface is incomplete.");
                break;
            case DDERR_WRONGMODE:
                strcpy(errDesc, "DDERR_WRONGMODE: This surface cannot be restored because it was created in a different mode.");
                break;
            case DDERR_XALIGN:
                strcpy(errDesc, "DDERR_XALIGN: The provided rectangle was not horizontally aligned on a required boundary.");
                break;
            default:
                strcpy(errDesc, "Unknown error code.");
            }
        }
        return errDesc;
    }

    /*
    D3D_OK
    D3DERR_BADMAJORVERSION
    D3DERR_BADMINORVERSION
    D3DERR_COLORKEYATTACHED
    D3DERR_CONFLICTINGTEXTUREFILTER
    D3DERR_CONFLICTINGTEXTUREPALETTE
    D3DERR_CONFLICTINGRENDERSTATE
    D3DERR_DEVICEAGGREGATED (new for DirectX 5.0)
    D3DERR_EXECUTE_CLIPPED_FAILED
    D3DERR_EXECUTE_CREATE_FAILED
    D3DERR_EXECUTE_DESTROY_FAILED
    D3DERR_EXECUTE_FAILED
    D3DERR_EXECUTE_LOCK_FAILED
    D3DERR_EXECUTE_LOCKED
    D3DERR_EXECUTE_NOT_LOCKED
    D3DERR_EXECUTE_UNLOCK_FAILED
    D3DERR_INITFAILED (new for DirectX 5.0)
    D3DERR_INBEGIN (new for DirectX 5.0)
    D3DERR_INVALID_DEVICE (new for DirectX 5.0)
    D3DERR_INVALIDCURRENTVIEWPORT (new for DirectX 5.0)
    D3DERR_INVALIDMATRIX
    D3DERR_INVALIDPALETTE(new for DirectX 5.0)
    D3DERR_INVALIDPRIMITIVETYPE (new for DirectX 5.0)
    D3DERR_INVALIDRAMPTEXTURE (new for DirectX 5.0)
    D3DERR_INVALIDVERTEXFORMAT (new for DirectX 6.0)
    D3DERR_INVALIDVERTEXTYPE (new for DirectX 5.0)
    D3DERR_LIGHT_SET_FAILED
    D3DERR_LIGHTHASVIEWPORT (new for DirectX 5.0)
    D3DERR_LIGHTNOTINTHISVIEWPORT (new for DirectX 5.0)
    D3DERR_MATERIAL_CREATE_FAILED
    D3DERR_MATERIAL_DESTROY_FAILED
    D3DERR_MATERIAL_GETDATA_FAILED
    D3DERR_MATERIAL_SETDATA_FAILED
    D3DERR_MATRIX_CREATE_FAILED
    D3DERR_MATRIX_DESTROY_FAILED
    D3DERR_MATRIX_GETDATA_FAILED
    D3DERR_MATRIX_SETDATA_FAILED
    D3DERR_NOCURRENTVIEWPORT (new for DirectX 5.0)
    D3DERR_NOTINBEGIN (new for DirectX 5.0)
    D3DERR_NOVIEWPORTS (new for DirectX 5.0)
    D3DERR_SCENE_BEGIN_FAILED
    D3DERR_SCENE_END_FAILED
    D3DERR_SCENE_IN_SCENE
    D3DERR_SCENE_NOT_IN_SCENE
    D3DERR_SETVIEWPORTDATA_FAILED
    D3DERR_STENCILBUFFER_NOTPRESENT
    D3DERR_SURFACENOTINVIDMEM (new for DirectX 5.0)
    D3DERR_TEXTURE_BADSIZE (new for DirectX 5.0)
    D3DERR_TEXTURE_CREATE_FAILED
    D3DERR_TEXTURE_DESTROY_FAILED
    D3DERR_TEXTURE_GETSURF_FAILED
    D3DERR_TEXTURE_LOAD_FAILED
    D3DERR_TEXTURE_LOCK_FAILED
    D3DERR_TEXTURE_LOCKED
    D3DERR_TEXTURE_NO_SUPPORT
    D3DERR_TEXTURE_NOT_LOCKED
    D3DERR_TEXTURE_SWAP_FAILED
    D3DERR_TEXTURE_UNLOCK_FAILED
    D3DERR_TOOMANYOPERATIONS
    D3DERR_TOOMANYPRIMITIVES
    D3DERR_UNSUPPORTEDALPHAARG
    D3DERR_UNSUPPORTEDALPHAOPERATION
    D3DERR_UNSUPPORTEDCOLORARG
    D3DERR_UNSUPPORTEDCOLOROPERATION
    D3DERR_UNSUPPORTEDFACTORVALUE
    D3DERR_UNSUPPORTEDTEXTUREFILTER
    D3DERR_VBUF_CREATE_FAILED
    D3DERR_VERTEXBUFFERLOCKED
    D3DERR_VERTEXBUFFEROPTIMIZED
    D3DERR_VIEWPORTDATANOTSET (new for DirectX 5.0)
    D3DERR_VIEWPORTHASNODEVICE (new for DirectX 5.0)
    D3DERR_WRONGTEXTUREFORMAT
    D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY (new for DirectX 5.0)
    D3DERR_ZBUFF_NEEDS_VIDEOMEMORY (new for DirectX 5.0)
    D3DERR_ZBUFFER_NOTPRESENT
    */

    //-----------------------------------------------------------------------
    DDDriverList* D3DRenderSystem::getDirectDrawDrivers(void)
    {
        if (!mDriverList)
        {
            mDriverList = new DDDriverList;
        }

        return mDriverList;
    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::setLightingEnabled(bool enabled)
    {
        // Call D3D
        HRESULT hr = __SetRenderState(D3DRENDERSTATE_LIGHTING, enabled);
        if (FAILED(hr))
            Except(hr, "Error lighting status.", "D3DRenderSystem::setLightingEnabled");
    }
    //-----------------------------------------------------------------------
    void D3DRenderSystem::_setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
    {
        HRESULT hr;

        D3DRENDERSTATETYPE fogType, fogTypeNot;

        if (mD3DDeviceDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE)
        {
            fogType = D3DRENDERSTATE_FOGTABLEMODE;
            fogTypeNot = D3DRENDERSTATE_FOGVERTEXMODE;
        }
        else
        {
            fogType = D3DRENDERSTATE_FOGVERTEXMODE;
            fogTypeNot = D3DRENDERSTATE_FOGTABLEMODE;
        }

        if( mode == FOG_NONE)
        {
            // just disable
            hr = __SetRenderState(fogType, D3DFOG_NONE );
            hr = __SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
        }
        else
        {
            // Allow fog
            hr = __SetRenderState( D3DRENDERSTATE_FOGENABLE, TRUE );
            hr = __SetRenderState( fogTypeNot, D3DFOG_NONE );
            switch(mode)
            {
            case FOG_EXP:
                hr = __SetRenderState( fogType, D3DFOG_EXP);
                break;
            case FOG_EXP2:
                hr = __SetRenderState( fogType, D3DFOG_EXP2);
                break;
            case FOG_LINEAR:
                hr = __SetRenderState( fogType, D3DFOG_LINEAR);
                break;

            }

            hr = __SetRenderState( D3DRENDERSTATE_FOGCOLOR, colour.getAsLongARGB() );
            hr = __SetRenderState( D3DRENDERSTATE_FOGSTART, *((LPDWORD)(&start)) );
            hr = __SetRenderState( D3DRENDERSTATE_FOGEND, *((LPDWORD)(&end)) );
            hr = __SetRenderState( D3DRENDERSTATE_FOGDENSITY, *((LPDWORD)(&density)) );
        }


    }

    //---------------------------------------------------------------------
    void D3DRenderSystem::convertColourValue(const ColourValue& colour, unsigned long* pDest)
    {
        *pDest = colour.getAsLongARGB();
    }
    //---------------------------------------------------------------------
    void D3DRenderSystem::_makeProjectionMatrix(Real fovy, Real aspect, 
        Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram)
    {
        Real theta = Math::AngleUnitsToRadians(fovy * 0.5);
        Real h = 1 / Math::Tan(theta);
        Real w = h / aspect;
        Real Q = farPlane / ( farPlane - nearPlane );

        dest = Matrix4::ZERO;
        dest[0][0] = w;
        dest[1][1] = h;
        dest[2][2] = Q;
        dest[3][2] = 1.0f;
        dest[2][3] = -Q * nearPlane;

    }
    //---------------------------------------------------------------------
    void D3DRenderSystem::_makeOrthoMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, 
        Matrix4& dest, bool forGpuProgram )
    {
        Real thetaY = Math::AngleUnitsToRadians(fovy / 2.0f);
        Real sinThetaY = Math::Sin(thetaY);
        Real thetaX = thetaY * aspect;
        Real sinThetaX = Math::Sin(thetaX);
        Real w = 1.0 / (sinThetaX * nearPlane);
        Real h = 1.0 / (sinThetaY * nearPlane);
        Real q = 1.0 / (farPlane - nearPlane);


        dest = Matrix4::ZERO;
        dest[0][0] = w;
        dest[1][1] = h;
        dest[2][2] = q;
        dest[3][3] = 1;	
    }
    //---------------------------------------------------------------------
    void D3DRenderSystem::_setRasterisationMode(SceneDetailLevel level)
    {
        D3DFILLMODE d3dmode;

        switch(level)
        {
        case SDL_POINTS:
            d3dmode = D3DFILL_POINT;
            break;
        case SDL_WIREFRAME:
            d3dmode = D3DFILL_WIREFRAME;
            break;
        case SDL_SOLID:
            d3dmode = D3DFILL_SOLID;
            break;

        }

        HRESULT hr = __SetRenderState(D3DRENDERSTATE_FILLMODE, d3dmode);
        if (FAILED(hr))
        {
            Except(hr, "Error setting rasterisation mode.", 
                "D3DRenderSystem::setRasterisationMode");
        }


    }
    //---------------------------------------------------------------------
    void D3DRenderSystem::setStencilCheckEnabled(bool enabled)
    {
        // Allow stencilling
        HRESULT hr = __SetRenderState(D3DRENDERSTATE_STENCILENABLE, enabled);
        if (FAILED(hr))
            Except(hr, "Error enabling / disabling stencilling.",
            "D3DRenderSystem::setStencilCheckEnabled");

    }
    //---------------------------------------------------------------------
    void D3DRenderSystem::setStencilBufferParams(CompareFunction func, ulong refValue, 
        ulong mask, StencilOperation stencilFailOp, 
        StencilOperation depthFailOp, StencilOperation passOp, 
        bool twoSidedOperation)
    {
        HRESULT hr;

        // D3D7 does not support 2-sided stencil operations
        if (twoSidedOperation)
            Except(Exception::ERR_INVALIDPARAMS, "Direct3D7 does not support 2-sided stencil ops",
               "D3DRenderSystem::setStencilBufferParams");

        // Function
        hr = __SetRenderState(D3DRENDERSTATE_STENCILFUNC, 
            convertCompareFunction(func));

        if (FAILED(hr))
            Except(hr, "Error setting stencil buffer test function.",
            "D3DRenderSystem::setStencilBufferParams");

        // reference value
        hr = __SetRenderState(D3DRENDERSTATE_STENCILREF, refValue);
        if (FAILED(hr))
            Except(hr, "Error setting stencil buffer reference value.",
            "D3DRenderSystem::setStencilBufferParams");

        // mask
        hr = __SetRenderState(D3DRENDERSTATE_STENCILMASK, mask);
        if (FAILED(hr))
            Except(hr, "Error setting stencil buffer mask.",
            "D3DRenderSystem::setStencilBufferParams");

        // fail op
        hr = __SetRenderState(D3DRENDERSTATE_STENCILFAIL, 
            convertStencilOp(stencilFailOp));
        if (FAILED(hr))
            Except(hr, "Error setting stencil fail operation.",
            "D3DRenderSystem::setStencilBufferParams");

        // depth fail op
        hr = __SetRenderState(D3DRENDERSTATE_STENCILZFAIL, 
            convertStencilOp(depthFailOp));
        if (FAILED(hr))
            Except(hr, "Error setting stencil depth fail operation.",
            "D3DRenderSystem::setStencilBufferParams");

        // pass op
        hr = __SetRenderState(D3DRENDERSTATE_STENCILPASS, 
            convertStencilOp(passOp));
        if (FAILED(hr))
            Except(hr, "Error setting stencil pass operation.",
            "D3DRenderSystem::setStencilBufferParams");
    }
    //---------------------------------------------------------------------
    D3DCMPFUNC D3DRenderSystem::convertCompareFunction(CompareFunction func)
    {
        switch(func)
        {
        case CMPF_ALWAYS_FAIL:
            return D3DCMP_NEVER;
        case CMPF_ALWAYS_PASS:
            return D3DCMP_ALWAYS;
        case CMPF_LESS:
            return D3DCMP_LESS;
        case CMPF_LESS_EQUAL:
            return D3DCMP_LESSEQUAL;
        case CMPF_EQUAL:
            return D3DCMP_EQUAL;
        case CMPF_NOT_EQUAL:
            return D3DCMP_NOTEQUAL;
        case CMPF_GREATER_EQUAL:
            return D3DCMP_GREATEREQUAL;
        case CMPF_GREATER:
            return D3DCMP_GREATER;
        };
        // to shut the compiler up
        return D3DCMP_ALWAYS;
    }
    //---------------------------------------------------------------------
    D3DSTENCILOP D3DRenderSystem::convertStencilOp(StencilOperation op)
    {
        switch(op)
        {
        case SOP_KEEP:
            return D3DSTENCILOP_KEEP;
        case SOP_ZERO:
            return D3DSTENCILOP_ZERO;
        case SOP_REPLACE:
            return D3DSTENCILOP_REPLACE;
        case SOP_INCREMENT:
            return D3DSTENCILOP_INCRSAT;
        case SOP_DECREMENT:
            return D3DSTENCILOP_DECRSAT;
        case SOP_INCREMENT_WRAP:
            return D3DSTENCILOP_INCR;
        case SOP_DECREMENT_WRAP:
            return D3DSTENCILOP_DECR;
        case SOP_INVERT:
            return D3DSTENCILOP_INVERT;
        };
        // To shut the compiler up
        return D3DSTENCILOP_KEEP;
    }

    DWORD D3DRenderSystem::_getCurrentAnisotropy(size_t unit)
    {
        DWORD oldVal;
        mlpD3DDevice->GetTextureStageState(unit, D3DTSS_MAXANISOTROPY, &oldVal);
        return oldVal;
    }

    
    void D3DRenderSystem::_setTextureUnitFiltering(size_t unit, 
        FilterType ftype, FilterOptions filter)
    {
        __SetTextureStageState(unit, _getFilterCode(ftype), _getFilter(ftype, filter));
    }

    void D3DRenderSystem::_setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy)
    {
        if ((DWORD)maxAnisotropy > mD3DDeviceDesc.dwMaxAnisotropy)
            maxAnisotropy = mD3DDeviceDesc.dwMaxAnisotropy;

        if (_getCurrentAnisotropy(unit) != maxAnisotropy)
            __SetTextureStageState( unit, D3DTSS_MAXANISOTROPY, maxAnisotropy );
    }

    void D3DRenderSystem::setVertexDeclaration(VertexDeclaration* decl)
    {
        // TODO
    }

    void D3DRenderSystem::setVertexBufferBinding(VertexBufferBinding* binding)
    {
        // TODO
    }
    //-----------------------------------------------------------------------
    D3DTEXTURESTAGESTATETYPE D3DRenderSystem::_getFilterCode(FilterType ft)
    {
        switch (ft)
        {
        case FT_MIN:
            return D3DTSS_MINFILTER;
            break;
        case FT_MAG:
            return D3DTSS_MAGFILTER;
            break;
        case FT_MIP:
            return D3DTSS_MIPFILTER;
            break;
        }

        // to keep compiler happy
        return D3DTSS_MINFILTER;
    }
    //-----------------------------------------------------------------------
    DWORD D3DRenderSystem::_getFilter(FilterType ft, FilterOptions fo)
    {
        switch (ft)
        {
        case FT_MIN:
            switch( fo )
            {
                // NOTE: Fall through if device doesn't support requested type
            case FO_ANISOTROPIC:
                if( mD3DDeviceDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC )
                {
                    return D3DTFN_ANISOTROPIC;
                    break;
                }
            case FO_LINEAR:
                if( mD3DDeviceDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR )
                {
                    return D3DTFN_LINEAR;
                    break;
                }
            case FO_POINT:
            case TFO_NONE:
                return D3DTFN_POINT;
                break;
            }
            break;
        case FT_MAG:
            switch( fo )
            {
            // NOTE: Fall through if device doesn't support requested type
            case FO_ANISOTROPIC:
                if( mD3DDeviceDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC )
                {
                    return D3DTFG_ANISOTROPIC;
                    break;
                }
            case FO_LINEAR:
                if( mD3DDeviceDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR )
                {
                    return D3DTFG_LINEAR;
                    break;
                }
            case FO_POINT:
            case FO_NONE:
                return D3DTFG_POINT;
                break;
            }
            break;
        case FT_MIP:
            switch( fo )
            {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                if( mD3DDeviceDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR )
                {
                    return D3DTFP_LINEAR;
                    break;
                }
            case FO_POINT:
                if( mD3DDeviceDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR )
                {
                    return D3DTFP_POINT;
                    break;
                }
            case TFO_NONE:
                return D3DTFP_NONE;
                break;
            }
            break;
        }

        // should never get here
        return 0;
    }
    //---------------------------------------------------------------------
    void D3DRenderSystem::setNormaliseNormals(bool normalise)
    {
        __SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, 
            normalise ? TRUE : FALSE);
    }
    //---------------------------------------------------------------------
    HRESULT D3DRenderSystem::__SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
    {
        HRESULT hr;
        DWORD oldVal;

        if ( FAILED( hr = mlpD3DDevice->GetRenderState(state, &oldVal) ) )
            return hr;
        if ( oldVal == value )
            return D3D_OK;
        else
            return mlpD3DDevice->SetRenderState(state, value);
    }
    //---------------------------------------------------------------------
    HRESULT D3DRenderSystem::__SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
    {
        HRESULT hr;
        DWORD oldVal;

        if ( FAILED( hr = mlpD3DDevice->GetTextureStageState(stage, type, &oldVal) ) )
            return hr;
        if ( oldVal == value )
            return D3D_OK;
        else
            return mlpD3DDevice->SetTextureStageState(stage, type, value);
    }

    //---------------------------------------------------------------------
    void D3DRenderSystem::clearFrameBuffer(unsigned int buffers, 
        const ColourValue& colour, Real depth, unsigned short stencil)
    {
        DWORD flags = 0;
        if (buffers & FBT_COLOUR)
        {
            flags |= D3DCLEAR_TARGET;
        }
        if (buffers & FBT_DEPTH)
        {
            flags |= D3DCLEAR_ZBUFFER;
        }
        // Only try to clear the stencil if supported, otherwise it will fail
        if (buffers & FBT_STENCIL && mCapabilities->hasCapability(RSC_HWSTENCIL))
        {
            flags |= D3DCLEAR_STENCIL;
        }
        HRESULT hr;
        if( FAILED( hr = mlpD3DDevice->Clear( 
            0, 
            NULL, 
            flags,
            colour.getAsLongARGB(), 
            depth, 
            stencil ) ) )
        {
            String msg = getErrorDescription(hr);
            Except( hr, "Error clearing frame buffer : " 
                + msg, "D3DRenderSystem::clearFrameBuffer" );
        }
    }
    //---------------------------------------------------------------------
    void D3DRenderSystem::_makeProjectionMatrix(Real left, Real right, 
        Real bottom, Real top, Real nearPlane, Real farPlane, Matrix4& dest,
        bool forGpuProgram)
    {
        Real width = right - left;
        Real height = top - bottom;
        Real Q = farPlane / ( farPlane - nearPlane );
        dest = Matrix4::ZERO;
        dest[0][0] = 2 * nearPlane / width;
        dest[0][2] = (right+left) / width;
        dest[1][1] = 2 * nearPlane / height;
        dest[1][2] = (top+bottom) / height;
        dest[2][2] = Q;
        dest[3][2] = 1.0f;
        dest[2][3] = -Q * nearPlane;
    }

    //---------------------------------------------------------------------
    void D3DRenderSystem::setClipPlanes(const PlaneList& clipPlanes)
    {
        size_t i;
        size_t numClipPlanes;
        D3DVALUE dx7ClipPlane[4];
        DWORD mask = 0;
        HRESULT hr;
        numClipPlanes = clipPlanes.size();

        for (i = 0; i < numClipPlanes; ++i)
        {
            const Plane& plane = clipPlanes[i];

            dx7ClipPlane[0] = plane.normal.x;
            dx7ClipPlane[1] = plane.normal.y;
            dx7ClipPlane[2] = plane.normal.z;
            dx7ClipPlane[3] = -plane.d;

            hr = mlpD3DDevice->SetClipPlane(i, dx7ClipPlane);
            if (FAILED(hr))
            {
                Except(hr, "Unable to set clip plane", 
                    "D3D7RenderSystem::setClipPlanes");
            }

            mask |= (1 << i);
        }

        hr = mlpD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE, mask);
        if (FAILED(hr))
        {
            Except(hr, "Unable to set render state for clip planes", 
                "D3D7RenderSystem::setClipPlanes");
        }
    }

    // ------------------------------------------------------------------
    void D3DRenderSystem::setClipPlane (ushort index, Real A, Real B, Real C, Real D)
    {
        float plane[4] = { A, B, C, D };
        mlpD3DDevice->SetClipPlane (index, plane);
    }

    // ------------------------------------------------------------------
    void D3DRenderSystem::enableClipPlane (ushort index, bool enable)
    {
        DWORD prev;
        mlpD3DDevice->GetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE, &prev);
        __SetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE, prev | (1 << index));
    }
    //---------------------------------------------------------------------
    HardwareOcclusionQuery* D3DRenderSystem::createHardwareOcclusionQuery(void)
    {
        return (HardwareOcclusionQuery*) 0;	// Hardware occlusion is not supported when DirectX7 is used
    }

}
