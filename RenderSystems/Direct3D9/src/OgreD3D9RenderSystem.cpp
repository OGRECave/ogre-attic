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
#include "OgreD3D9RenderSystem.h"
#include "OgreD3D9Prerequisites.h"
#include "OgreD3D9DriverList.h"
#include "OgreD3D9Driver.h"
#include "OgreD3D9VideoModeList.h"
#include "OgreD3D9VideoMode.h"
#include "OgreD3D9RenderWindow.h"
#include "OgreD3D9TextureManager.h"
#include "OgreD3D9Texture.h"
#include "OgreLogManager.h"
#include "OgreLight.h"
#include "OgreMath.h"
#include "OgreD3D9HardwareBufferManager.h"
#include "OgreD3D9HardwareIndexBuffer.h"
#include "OgreD3D9HardwareVertexBuffer.h"
#include "OgreD3D9VertexDeclaration.h"
#include "OgreD3D9GpuProgram.h"
#include "OgreD3D9GpuProgramManager.h"


namespace Ogre 
{
	//---------------------------------------------------------------------
	D3D9RenderSystem::D3D9RenderSystem( HINSTANCE hInstance )
	{
		OgreGuard( "D3D9RenderSystem::D3D9RenderSystem" );
		LogManager::getSingleton().logMessage( "D3D9 : " + getName() + " created." );

		// set the instance being passed 
		mhInstance = hInstance;

		// set pointers to NULL
		mpD3D = NULL;
		mpD3DDevice = NULL;
		mDriverList = NULL;
		mActiveD3DDriver = NULL;
		mExternalHandle = NULL;
        mTextureManager = NULL;
        mHardwareBufferManager = NULL;
		mGpuProgramManager = NULL;

		// init lights
		for(int i = 0; i < MAX_LIGHTS; i++ )
			mLights[i] = 0;

		// Create our Direct3D object
		if( NULL == (mpD3D = Direct3DCreate9(D3D_SDK_VERSION)) )
			Except( Exception::ERR_INTERNAL_ERROR, "Failed to create Direct3D9 object", "D3D9RenderSystem::D3D9RenderSystem" );

		// set config options defaults
		initConfigOptions();

		// fsaa options
		mFSAAType = D3DMULTISAMPLE_NONE;
		mFSAAQuality = 0;

		// set stages desc. to defaults
		for (int n = 0; n < OGRE_MAX_TEXTURE_LAYERS; n++)
		{
			mTexStageDesc[n].autoTexCoordType = TEXCALC_NONE;
			mTexStageDesc[n].coordIndex = 0;
			mTexStageDesc[n].texType = D3D9Mappings::D3D_TEX_TYPE_NORMAL;
			mTexStageDesc[n].pTex = 0;
		}

		mLastVertexSourceCount = 0;

        mForcedNormalisation = false;
        mCurrentLights = 0;


		OgreUnguard();
	}
	//---------------------------------------------------------------------
	D3D9RenderSystem::~D3D9RenderSystem()
	{
		OgreGuard( "D3D9RenderSystem::~D3D9RenderSystem" );

		// Unbind any vertex streams to avoid memory leaks
		for (unsigned int i = 0; i < mLastVertexSourceCount; ++i)
		{
            HRESULT hr = mpD3DDevice->SetStreamSource(i, NULL, 0, 0);
		}
		
		
		SAFE_DELETE( mDriverList );
		SAFE_DELETE( mTextureManager );
        SAFE_DELETE(mHardwareBufferManager);
		SAFE_DELETE(mGpuProgramManager);
		SAFE_RELEASE( mpD3D );

        if (mCapabilities)
        {
            delete mCapabilities;
            mCapabilities = NULL;
        }

		LogManager::getSingleton().logMessage( "D3D9 : " + getName() + " destroyed." );
		OgreUnguard();
	}
	//---------------------------------------------------------------------
	const String& D3D9RenderSystem::getName() const
	{
		static String strName( "Direct3D9 Rendering SubSystem");
		return strName;
	}
	//---------------------------------------------------------------------
	D3D9DriverList* D3D9RenderSystem::getDirect3DDrivers()
	{
		if( !mDriverList )
			mDriverList = new D3D9DriverList( mpD3D );

		return mDriverList;
	}
	//---------------------------------------------------------------------
	bool D3D9RenderSystem::_checkMultiSampleQuality(D3DMULTISAMPLE_TYPE type, DWORD *outQuality, D3DFORMAT format, UINT adapterNum, D3DDEVTYPE deviceType, BOOL fullScreen)
	{
		HRESULT hr;
		hr = mpD3D->CheckDeviceMultiSampleType( 
				adapterNum, 
				deviceType, 
				format, 
				fullScreen, 
				type, 
				outQuality);

		if (SUCCEEDED(hr))
			return true;
		else
			return false;
	}
	//---------------------------------------------------------------------
	D3D9RenderSystem::initConfigOptions()
	{
		OgreGuard( "D3D9RenderSystem::initConfigOptions" );

		D3D9DriverList* driverList;
		D3D9Driver* driver;

		ConfigOption optDevice;
		ConfigOption optVideoMode;
		ConfigOption optFullScreen;
		ConfigOption optVSync;
		ConfigOption optAA;

		driverList = this->getDirect3DDrivers();

		optDevice.name = "Rendering Device";
		optDevice.currentValue = "";
		optDevice.possibleValues.clear();
		optDevice.immutable = false;

		optVideoMode.name = "Video Mode";
		optVideoMode.currentValue = "800 x 600 @ 32-bit colour";
		optVideoMode.immutable = false;

		optFullScreen.name = "Full Screen";
		optFullScreen.possibleValues.push_back( "Yes" );
		optFullScreen.possibleValues.push_back( "No" );
		optFullScreen.currentValue = "Yes";
		optFullScreen.immutable = false;

		for( unsigned j=0; j < driverList->count(); j++ )
		{
			driver = driverList->item(j);
			optDevice.possibleValues.push_back( driver->DriverDescription() );
			// Make first one default
			if( j==0 )
				optDevice.currentValue = driver->DriverDescription();
		}

		optVSync.name = "VSync";
		optVSync.immutable = false;
		optVSync.possibleValues.push_back( "Yes" );
		optVSync.possibleValues.push_back( "No" );
		optVSync.currentValue = "No";

		optAA.name = "Anti aliasing";
		optAA.immutable = false;
		optAA.possibleValues.push_back( "None" );
		optAA.currentValue = "None";

		mOptions[optDevice.name] = optDevice;
		mOptions[optVideoMode.name] = optVideoMode;
		mOptions[optFullScreen.name] = optFullScreen;
		mOptions[optVSync.name] = optVSync;
		mOptions[optAA.name] = optAA;

		refreshD3DSettings();

		OgreUnguard();
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::refreshD3DSettings()
	{
		OgreGuard( "D3D9RenderSystem::refreshD3DSettings" );

		ConfigOption* optVideoMode;
		D3D9Driver* driver;
		D3D9VideoMode* videoMode;

		ConfigOptionMap::iterator opt = mOptions.find( "Rendering Device" );
		if( opt != mOptions.end() )
		{
			for( unsigned j=0; j < getDirect3DDrivers()->count(); j++ )
			{
				driver = getDirect3DDrivers()->item(j);
				if( driver->DriverDescription() == opt->second.currentValue )
					break;
			}

			opt = mOptions.find( "Video Mode" );
			optVideoMode = &opt->second;
			optVideoMode->possibleValues.clear();
			// get vide modes for this device
			for( unsigned k=0; k < driver->getVideoModeList()->count(); k++ )
			{
				videoMode = driver->getVideoModeList()->item( k );
				optVideoMode->possibleValues.push_back( videoMode->getDescription() );
			}
		}

		OgreUnguard();
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setConfigOption( const String &name, const String &value )
	{
		OgreGuard( "D3D9RenderSystem::setConfigOption" );

		char msg[128];
		sprintf( msg, "D3D9 : RenderSystem Option: %s = %s", name.c_str(), value.c_str() );
		LogManager::getSingleton().logMessage( msg );

		// Find option
		ConfigOptionMap::iterator it = mOptions.find( name );

		// Update
		if( it != mOptions.end() )
			it->second.currentValue = value;
		else
		{
			sprintf( msg, "Option named '%s' does not exist.", name.c_str() );
			Except( Exception::ERR_INVALIDPARAMS, msg, "D3D9RenderSystem::setConfigOption" );
		}

		// Refresh other options if D3DDriver changed
		if( name == "Rendering Device" )
			refreshD3DSettings();

		if( name == "Full Screen" )
		{
			// Video mode is applicable
			it = mOptions.find( "Video Mode" );
			if (it->second.currentValue == "")
				it->second.currentValue = "800 x 600 @ 32-bit colour";
		}

		if( name == "Anti aliasing" )
		{
			if (value == "None")
				_setFSAA(D3DMULTISAMPLE_NONE, 0);
			else 
			{
				D3DMULTISAMPLE_TYPE fsaa = D3DMULTISAMPLE_NONE;
				DWORD level = 0;

				if (value.find_first_of("NonMaskable") != -1)
				{
					fsaa = D3DMULTISAMPLE_NONMASKABLE;
					size_t pos = value.find_last_of(" ");
					String sNum = value.substr(pos + 1);
					level = StringConverter::parseInt(sNum);
					level -= 1;
				}
				else if (value.find_first_of("Level") != -1)
				{
					size_t pos = value.find_last_of(" ");
					String sNum = value.substr(pos + 1);
					fsaa = (D3DMULTISAMPLE_TYPE)StringConverter::parseInt(sNum);
				}

				_setFSAA(fsaa, level);
			}
		}

		if( name == "VSync" )
		{
			if (value == "Yes")
				mVSync = true;
			else
				mVSync = false;
		}

		if( name == "Video Mode" )
		{
			ConfigOption* optFSAA;
			it = mOptions.find( "Anti aliasing" );
			optFSAA = &it->second;
			optFSAA->possibleValues.clear();
			optFSAA->possibleValues.push_back("None");

			it = mOptions.find("Rendering Device");
			D3D9Driver *driver = getDirect3DDrivers()->item(it->second.currentValue);
			if (driver)
			{
				it = mOptions.find("Video Mode");
				D3D9VideoMode *videoMode = driver->getVideoModeList()->item(it->second.currentValue);
				if (videoMode)
				{
					// get non maskable FSAA for this VMODE
					DWORD numLevels = 0;
					bool bOK = this->_checkMultiSampleQuality(
						D3DMULTISAMPLE_NONMASKABLE, 
						&numLevels, 
						videoMode->getFormat(), 
						driver->getAdapterNumber(),
						D3DDEVTYPE_HAL,
						TRUE);
					if (bOK && numLevels > 0)
					{
						for (DWORD n = 0; n < numLevels; n++)
							optFSAA->possibleValues.push_back("NonMaskable " + StringConverter::toString(n + 1));
					}

					// set maskable levels supported
					for (int n = 2; n < 17; n++)
					{
						bOK = this->_checkMultiSampleQuality(
							(D3DMULTISAMPLE_TYPE)n, 
							&numLevels, 
							videoMode->getFormat(), 
							driver->getAdapterNumber(),
							D3DDEVTYPE_HAL,
							TRUE);
						if (bOK)
							optFSAA->possibleValues.push_back("Level " + StringConverter::toString(n));
					}
				}
			}
		}

		OgreUnguard();
	}
	//---------------------------------------------------------------------
	String D3D9RenderSystem::validateConfigOptions()
	{
		ConfigOptionMap::iterator it;
		
		// check if video mode is selected
		it = mOptions.find( "Video Mode" );
		if( it->second.currentValue == "" )
			return "A video mode must be selected.";

		it = mOptions.find( "Rendering Device" );
		bool foundDriver = false;
		D3D9DriverList* driverList = getDirect3DDrivers();
		for( ushort j=0; j < driverList->count(); j++ )
		{
			if( driverList->item(j)->DriverDescription() == it->second.currentValue )
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

        it = mOptions.find( "VSync" );
		if( it->second.currentValue == "Yes" )
			mVSync = true;
		else
			mVSync = false;

		return "";
	}
	//---------------------------------------------------------------------
	ConfigOptionMap& D3D9RenderSystem::getConfigOptions()
	{
		// return a COPY of the current config options
		return mOptions;
	}
	//---------------------------------------------------------------------
	RenderWindow* D3D9RenderSystem::initialise( bool autoCreateWindow )
	{
		RenderWindow* autoWindow = NULL;
		LogManager::getSingleton().logMessage( "D3D9 : Subsystem Initialising" );

		// Init using current settings
		mActiveD3DDriver = NULL;
		ConfigOptionMap::iterator opt = mOptions.find( "Rendering Device" );
		for( unsigned j=0; j < getDirect3DDrivers()->count(); j++ )
		{
			if( getDirect3DDrivers()->item(j)->DriverDescription() == opt->second.currentValue )
			{
				mActiveD3DDriver = getDirect3DDrivers()->item(j);
				break;
			}
		}

		if( !mActiveD3DDriver )
			Except( Exception::ERR_INVALIDPARAMS, "Problems finding requested Direct3D driver!", "D3D9RenderSystem::initialise" );

		if( autoCreateWindow )
		{
			bool fullScreen;
			opt = mOptions.find( "Full Screen" );
			if( opt == mOptions.end() )
				Exception( Exception::ERR_INTERNAL_ERROR, "Can't find full screen option!", "D3D9RenderSystem::initialise" );
			fullScreen = opt->second.currentValue == "Yes";

			D3D9VideoMode* videoMode = NULL;
			int width, height, colourDepth;
			String temp;

			opt = mOptions.find( "Video Mode" );
			if( opt == mOptions.end() )
				Exception( Exception::ERR_INTERNAL_ERROR, "Can't find Video Mode option!", "D3D9RenderSystem::initialise" );

			for( unsigned j=0; j < mActiveD3DDriver->getVideoModeList()->count(); j++ )
			{
				temp = mActiveD3DDriver->getVideoModeList()->item(j)->getDescription();
				if( temp == opt->second.currentValue )
				{
					videoMode = mActiveD3DDriver->getVideoModeList()->item(j);
					break;
				}
			}

			if( !videoMode )
				Except( Exception::ERR_INTERNAL_ERROR, "Can't find requested video mode.", "D3D9RenderSystem::initialise" );

			width = videoMode->getWidth();
			height = videoMode->getHeight();
			colourDepth = videoMode->getColourDepth();

			autoWindow = this->createRenderWindow( "OGRE Render Window", width, height, colourDepth, fullScreen );
		}

        LogManager::getSingleton().logMessage("***************************************");

		LogManager::getSingleton().logMessage("*** D3D9 : Subsystem Initialised OK ***");
        LogManager::getSingleton().logMessage("***************************************");

		// call superclass method
		RenderSystem::initialise( autoCreateWindow );



		return autoWindow;
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setFSAA(D3DMULTISAMPLE_TYPE type, DWORD qualityLevel)
	{
		if (!mpD3DDevice)
		{
			mFSAAType = type;
			mFSAAQuality = qualityLevel;
		}
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::reinitialise()
	{
		LogManager::getSingleton().logMessage( "D3D9 : Reinitialising" );
		this->shutdown();
		this->initialise( true );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::shutdown()
	{
		RenderSystem::shutdown();
		SAFE_DELETE( mDriverList );
		mActiveD3DDriver = NULL;
		LogManager::getSingleton().logMessage("D3D9 : Shutting down cleanly.");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::startRendering()
	{
		OgreGuard( "D3D9RenderSystem::startRendering" );

		MSG  msg;
		// Call superclass
		RenderSystem::startRendering();

		// Render this window
		PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

		while( mRenderTargets.size() )
		{
			if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else
			{
				if(!fireFrameStarted())
					return;

				// Render a frame during idle time (no messages are waiting)
				RenderTargetPriorityMap::iterator itarg, itargend;
				itargend = mPrioritisedRenderTargets.end();
				for( itarg = mPrioritisedRenderTargets.begin(); itarg != itargend; ++itarg )
				{
					if( itarg->second->isActive() )
						itarg->second->update();
				}

				if(!fireFrameEnded())
					return;
			}
		}

		OgreUnguard();
	}
	//---------------------------------------------------------------------
	RenderWindow* D3D9RenderSystem::createRenderWindow( const String &name, int width, int height, int colourDepth,
		bool fullScreen, int left, int top, bool depthBuffer, RenderWindow* parentWindowHandle)
	{
		static bool firstWindow = true;
		
		OgreGuard( "D3D9RenderSystem::createRenderWindow" );

		String msg;

		// Make sure we don't already have a render target of the 
		// sam name as the one supplied
		if( mRenderTargets.find( name ) != mRenderTargets.end() )
		{
			msg = "A render target of the same name '" + name + "' already "
				"exists.  You cannot create a new window with this name.";
			Except( Exception::ERR_INTERNAL_ERROR, msg, "D3D9RenderSystem::createRenderWindow" );
		}

		RenderWindow* win = new D3D9RenderWindow();
		if (!fullScreen && mExternalHandle)
		{
			D3D9RenderWindow *pWin32Window = (D3D9RenderWindow *)win;
	 		pWin32Window->SetExternalWindowHandle(mExternalHandle);
		}

		win->create( name, width, height, colourDepth, fullScreen, 
				left, top, depthBuffer, &mhInstance, mActiveD3DDriver, 
				parentWindowHandle, mFSAAType, mFSAAQuality, mVSync );

		attachRenderTarget( *win );

		// If this is the first window, get the D3D device and create the texture manager
		if( firstWindow )
		{
			win->getCustomAttribute( "D3DDEVICE", &mpD3DDevice );

			// Create the texture manager for use by others
			mTextureManager = new D3D9TextureManager( mpD3DDevice );
            // Also create hardware buffer manager
            mHardwareBufferManager = new D3D9HardwareBufferManager(mpD3DDevice);

			// Create the GPU program manager
			mGpuProgramManager = new D3D9GpuProgramManager(mpD3DDevice);

            // Initialise the capabilities structures
            initCapabilities();


			firstWindow = false;
			
		}

		OgreUnguardRet( win );
	}
    //---------------------------------------------------------------------
    void D3D9RenderSystem::initCapabilities(void)
    {
		// get caps
		mpD3D->GetDeviceCaps( mActiveD3DDriver->getAdapterNumber(), D3DDEVTYPE_HAL, &mCaps );

        // Check for hardware stencil support
		LPDIRECT3DSURFACE9 pSurf;
		D3DSURFACE_DESC surfDesc;
		mpD3DDevice->GetDepthStencilSurface(&pSurf);
		pSurf->GetDesc(&surfDesc);

		if (surfDesc.Format == D3DFMT_D24S8 || surfDesc.Format == D3DFMT_D24X8)
		{
			mCapabilities->setCapability(RSC_HWSTENCIL);
			// Actually, it's always 8-bit
			mCapabilities->setStencilBufferBitDepth(8);

		}

		// Set number of texture units
		mCapabilities->setNumTextureUnits(mCaps.MaxSimultaneousTextures);
        // Anisotropy?
        if (mCaps.MaxAnisotropy > 1)
            mCapabilities->setCapability(RSC_ANISOTROPY);
        // Automatic mipmap generation?
        if (mCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)
            mCapabilities->setCapability(RSC_AUTOMIPMAP);
        // Blending between stages supported
        mCapabilities->setCapability(RSC_BLENDING);
        // Dot 3
        if (mCaps.TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)
            mCapabilities->setCapability(RSC_DOT3);
        // Cube map
        if (mCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP)
            mCapabilities->setCapability(RSC_CUBEMAPPING);

        mCapabilities->setCapability(RSC_VBO);

        convertVertexShaderCaps();
        convertPixelShaderCaps();

        mCapabilities->log(LogManager::getSingleton().getDefaultLog());
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::convertVertexShaderCaps(void)
    {
        ushort major, minor;
        major = static_cast<ushort>((mCaps.VertexShaderVersion & 0x0000FF00) >> 8);
        minor = static_cast<ushort>(mCaps.VertexShaderVersion & 0x000000FF);

        // Populate max version & params
        switch (major)
        {
        case 1:
            mCapabilities->setMaxVertexProgramVersion("vs_1_1");
            // No boolean params allowed
            mCapabilities->setVertexProgramConstantBoolCount(0);
            // No integer params allowed
            mCapabilities->setVertexProgramConstantIntCount(0);
            // float params, always 4D
            mCapabilities->setVertexProgramConstantFloatCount(mCaps.MaxVertexShaderConst);
           
            break;
        case 2:
            if (minor > 0)
            {
                mCapabilities->setMaxVertexProgramVersion("vs_2_x");
            }
            else
            {
                mCapabilities->setMaxVertexProgramVersion("vs_2_0");
            }
            // 16 boolean params allowed
            mCapabilities->setVertexProgramConstantBoolCount(16);
            // 16 integer params allowed, 4D
            mCapabilities->setVertexProgramConstantIntCount(16);
            // float params, always 4D
            mCapabilities->setVertexProgramConstantFloatCount(mCaps.MaxVertexShaderConst);
            break;
        case 3:
            mCapabilities->setMaxVertexProgramVersion("vs_3_0");
            // 16 boolean params allowed
            mCapabilities->setVertexProgramConstantBoolCount(16);
            // 16 integer params allowed, 4D
            mCapabilities->setVertexProgramConstantIntCount(16);
            // float params, always 4D
            mCapabilities->setVertexProgramConstantFloatCount(mCaps.MaxVertexShaderConst);
            break;
        default:
            mCapabilities->setMaxVertexProgramVersion("");
            break;
        }

        // populate syntax codes in program manager (no breaks in this one so it falls through)
        switch(major)
        {
        case 3:
            mGpuProgramManager->_pushSyntaxCode("vs_3_0");
        case 2:
            if (major > 2 || minor > 0)
                mGpuProgramManager->_pushSyntaxCode("vs_2_x");

            mGpuProgramManager->_pushSyntaxCode("vs_2_0");
        case 1:
            mGpuProgramManager->_pushSyntaxCode("vs_1_1");
            mCapabilities->setCapability(RSC_VERTEX_PROGRAM);
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::convertPixelShaderCaps(void)
    {
        ushort major, minor;
        major = static_cast<ushort>((mCaps.PixelShaderVersion & 0x0000FF00) >> 8);
        minor = static_cast<ushort>(mCaps.PixelShaderVersion & 0x000000FF);
        switch (major)
        {
        case 1:
            switch(minor)
            {
            case 1:
                mCapabilities->setMaxFragmentProgramVersion("ps_1_1");
                break;
            case 2:
                mCapabilities->setMaxFragmentProgramVersion("ps_1_2");
                break;
            case 3:
                mCapabilities->setMaxFragmentProgramVersion("ps_1_3");
                break;
            case 4:
                mCapabilities->setMaxFragmentProgramVersion("ps_1_4");
                break;
            }
            break;
            // no boolean params allowed
            mCapabilities->setFragmentProgramConstantBoolCount(0);
            // no integer params allowed
            mCapabilities->setFragmentProgramConstantIntCount(0);
            // float params, always 4D
            // NB in ps_1_x these are actually stored as fixed point values,
            // but they are entered as floats
            mCapabilities->setFragmentProgramConstantFloatCount(8);
        case 2:
            if (minor > 0)
            {
                mCapabilities->setMaxFragmentProgramVersion("ps_2_x");
                // 16 boolean params allowed
                mCapabilities->setFragmentProgramConstantBoolCount(16);
                // 16 integer params allowed, 4D
                mCapabilities->setFragmentProgramConstantIntCount(16);
                // float params, always 4D
                mCapabilities->setFragmentProgramConstantFloatCount(224);
            }
            else
            {
                mCapabilities->setMaxFragmentProgramVersion("ps_2_0");
                // no boolean params allowed
                mCapabilities->setFragmentProgramConstantBoolCount(0);
                // no integer params allowed
                mCapabilities->setFragmentProgramConstantIntCount(0);
                // float params, always 4D
                mCapabilities->setFragmentProgramConstantFloatCount(32);
            }
            break;
        case 3:
            if (minor > 0)
            {
                mCapabilities->setMaxFragmentProgramVersion("ps_3_x");
            }
            else
            {
                mCapabilities->setMaxFragmentProgramVersion("ps_3_0");
            }
            // 16 boolean params allowed
            mCapabilities->setFragmentProgramConstantBoolCount(16);
            // 16 integer params allowed, 4D
            mCapabilities->setFragmentProgramConstantIntCount(16);
            // float params, always 4D
            mCapabilities->setFragmentProgramConstantFloatCount(224);
            break;
        default:
            mCapabilities->setMaxFragmentProgramVersion("");
            break;
        }

        // populate syntax codes in program manager (no breaks in this one so it falls through)
        switch(major)
        {
        case 3:
            if (minor > 0)
                mGpuProgramManager->_pushSyntaxCode("ps_3_x");

            mGpuProgramManager->_pushSyntaxCode("ps_3_0");
        case 2:
            if (major > 2 || minor > 0)
                mGpuProgramManager->_pushSyntaxCode("ps_2_x");

            mGpuProgramManager->_pushSyntaxCode("ps_2_0");
        case 1:
            if (major > 1 || minor >= 4)
                mGpuProgramManager->_pushSyntaxCode("ps_1_4");
            if (major > 1 || minor >= 3)
                mGpuProgramManager->_pushSyntaxCode("ps_1_3");
            if (major > 1 || minor >= 2)
                mGpuProgramManager->_pushSyntaxCode("ps_1_2");
            
            mGpuProgramManager->_pushSyntaxCode("ps_1_1");
            mCapabilities->setCapability(RSC_FRAGMENT_PROGRAM);
        }
    }
    //---------------------------------------------------------------------
	RenderTexture * D3D9RenderSystem::createRenderTexture( const String & name, int width, int height )
	{
		RenderTexture *rt = new D3D9RenderTexture( name, width, height );
		attachRenderTarget( *rt );
		return rt;
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::destroyRenderWindow( RenderWindow* pWin )
	{
		// Find it to remove from list
		RenderTargetMap::iterator i = mRenderTargets.begin();

		while( i->second != pWin && i != mRenderTargets.end() )
		{
			if( i->second == pWin )
			{
				mRenderTargets.erase(i);
				delete pWin;
				break;
			}
		}
	}
	//---------------------------------------------------------------------
	String D3D9RenderSystem::getErrorDescription( long errorNumber )
	{
		String errMsg = DXGetErrorDescription9( errorNumber );
		return errMsg;
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::convertColourValue( const ColourValue& colour, unsigned long* pDest )
	{
		*pDest = colour.getAsLongARGB();
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, Matrix4& dest)
	{

        D3DXMATRIX d3dMatrix;
        D3DXMatrixPerspectiveFovLH(&d3dMatrix,
          Math::AngleUnitsToRadians(fovy),
          aspect,
          nearPlane,
          farPlane);

        dest = D3D9Mappings::convertD3DXMatrix(d3dMatrix);

        /*
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
        */
	}
	//---------------------------------------------------------------------
	D3D9RenderSystem::ResizeRepositionWindow(HWND wich)
	{
		for (RenderTargetMap::iterator it = mRenderTargets.begin(); it != mRenderTargets.end(); ++it)
		{
			if (it->second->isActive())
			{
				D3D9RenderWindow *pWin32Window = (D3D9RenderWindow *)it->second;
				if (pWin32Window->getWindowHandle() == wich)
				{
					pWin32Window->WindowMovedOrResized();
					break;
				}
			}
		}
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setAmbientLight( float r, float g, float b )
	{
		HRESULT hr = __SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( r, g, b, 1.0f ) );
		if( FAILED( hr ) )
			Except( hr, "Failed to set render stat D3DRS_AMBIENT", "D3D9RenderSystem::setAmbientLight" );
	}
	//---------------------------------------------------------------------
    void D3D9RenderSystem::_useLights(const LightList& lights, unsigned short limit)
    {
        LightList::const_iterator i, iend;
        iend = lights.end();
        unsigned short num = 0;
        for (i = lights.begin(); i != iend && num < limit; ++i, ++num)
        {
            setD3D9Light(num, *i);
        }
        // Disable extra lights
        for (; num < mCurrentLights; ++num)
        {
            setD3D9Light(num, NULL);
        }
        mCurrentLights = std::min(limit, static_cast<unsigned short>(lights.size()));

    }
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setShadingType( ShadeOptions so )
	{
		HRESULT hr = __SetRenderState( D3DRS_SHADEMODE, D3D9Mappings::get(so) );
		if( FAILED( hr ) )
			Except( hr, "Failed to set render stat D3DRS_SHADEMODE", "D3D9RenderSystem::setShadingType" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setLightingEnabled( bool enabled )
	{
		HRESULT hr;
		if( FAILED( hr = __SetRenderState( D3DRS_LIGHTING, enabled ) ) )
			Except( hr, "Failed to set render state D3DRS_LIGHTING", "D3D9RenderSystem::setLightingEnabled" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setD3D9Light( int index, Light* lt )
	{
		HRESULT hr;

		D3DLIGHT9 d3dLight;
		ZeroMemory( &d3dLight, sizeof(d3dLight) );

        if (!lt)
        {
            if( FAILED( hr = mpD3DDevice->LightEnable( index, FALSE) ) )
			    Except( hr, "Unable to disable light", "D3D9RenderSystem::setD3D9Light" );
        }
        else
        {
			switch( lt->getType() )
			{
			case Light::LT_POINT:
				d3dLight.Type = D3DLIGHT_POINT;
				break;

			case Light::LT_DIRECTIONAL:
				d3dLight.Type = D3DLIGHT_DIRECTIONAL;
				break;

			case Light::LT_SPOTLIGHT:
				d3dLight.Type = D3DLIGHT_SPOT;
				d3dLight.Falloff = lt->getSpotlightFalloff();
				d3dLight.Theta = Math::AngleUnitsToRadians( lt->getSpotlightInnerAngle() );
				d3dLight.Phi = Math::AngleUnitsToRadians( lt->getSpotlightOuterAngle() );
				break;
			}

			ColourValue col;
			col = lt->getDiffuseColour();
			d3dLight.Diffuse = D3DXCOLOR( col.r, col.g, col.b, col.a );

			col = lt->getSpecularColour();
			d3dLight.Specular = D3DXCOLOR( col.r, col.g, col.b, col.a );

			Vector3 vec;
			if( lt->getType() != Light::LT_DIRECTIONAL )
			{
				vec = lt->getDerivedPosition();
				d3dLight.Position = D3DXVECTOR3( vec.x, vec.y, vec.z );
			}
			if( lt->getType() != Light::LT_POINT )
			{
				vec = lt->getDerivedDirection();
				d3dLight.Direction = D3DXVECTOR3( vec.x, vec.y, vec.z );
			}

			d3dLight.Range = lt->getAttenuationRange();
			d3dLight.Attenuation0 = lt->getAttenuationConstant();
			d3dLight.Attenuation1 = lt->getAttenuationLinear();
			d3dLight.Attenuation2 = lt->getAttenuationQuadric();

			if( FAILED( hr = mpD3DDevice->SetLight( index, &d3dLight ) ) )
				Except( hr, "Unable to set light details", "D3D9RenderSystem::setD3D9Light" );

            if( FAILED( hr = mpD3DDevice->LightEnable( index, TRUE ) ) )
			    Except( hr, "Unable to enable light", "D3D9RenderSystem::setD3D9Light" );
        }


	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setViewMatrix( const Matrix4 &m )
	{
        D3DXMATRIX d3dmat = D3D9Mappings::makeD3DXMatrix( m );
		d3dmat.m[0][2] = -d3dmat.m[0][2];
		d3dmat.m[1][2] = -d3dmat.m[1][2];
		d3dmat.m[2][2] = -d3dmat.m[2][2];
		d3dmat.m[3][2] = -d3dmat.m[3][2];

		HRESULT hr;
		if( FAILED( hr = mpD3DDevice->SetTransform( D3DTS_VIEW, &d3dmat ) ) )
			Except( hr, "Cannot set D3D9 view matrix", "D3D9RenderSystem::_setViewMatrix" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setProjectionMatrix( const Matrix4 &m )
	{
		D3DXMATRIX d3dMat = D3D9Mappings::makeD3DXMatrix( m );

		if( mActiveRenderTarget->requiresTextureFlipping() )
			d3dMat._22 = - d3dMat._22;

		HRESULT hr;
		if( FAILED( hr = mpD3DDevice->SetTransform( D3DTS_PROJECTION, &d3dMat ) ) )
			Except( hr, "Cannot set D3D9 projection matrix", "D3D9RenderSystem::_setProjectionMatrix" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setWorldMatrix( const Matrix4 &m )
	{
		D3DXMATRIX d3dMat = D3D9Mappings::makeD3DXMatrix( m );

		HRESULT hr;
		if( FAILED( hr = mpD3DDevice->SetTransform( D3DTS_WORLD, &d3dMat ) ) )
			Except( hr, "Cannot set D3D9 world matrix", "D3D9RenderSystem::_setWorldMatrix" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse,
		const ColourValue &specular, const ColourValue &emissive, Real shininess )
	{
		// Remember last call
		static ColourValue lastAmbient = ColourValue::Black;
		static ColourValue lastDiffuse = ColourValue::Black;
		static ColourValue lastSpecular = ColourValue::Black;
		static ColourValue lastEmissive = ColourValue::Black;
		static Real lastShininess = 0.0;

		// Only update if changed
		if( ambient != lastAmbient || diffuse != lastDiffuse ||
			specular != lastSpecular || emissive != lastEmissive ||
			shininess != lastShininess )
		{
			D3DMATERIAL9 material;
			material.Diffuse = D3DXCOLOR( diffuse.r, diffuse.g, diffuse.b, diffuse.a );
			material.Ambient = D3DXCOLOR( ambient.r, ambient.g, ambient.b, ambient.a );
			material.Specular = D3DXCOLOR( specular.r, specular.g, specular.b, specular.a );
			material.Emissive = D3DXCOLOR( emissive.r, emissive.g, emissive.b, emissive.a );
			material.Power = shininess;

			HRESULT hr = mpD3DDevice->SetMaterial( &material );
			if( FAILED( hr ) )
				Except( hr, "Error setting D3D material", "D3D9RenderSystem::_setSurfaceParams" );

			// Remember the details
			lastAmbient = ambient;
			lastDiffuse = diffuse;
			lastSpecular = specular;
			lastEmissive = emissive;
			lastShininess = shininess;
		}
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setTexture( size_t stage, bool enabled, const String &texname )
	{
		HRESULT hr;
		D3D9Texture *dt = (D3D9Texture *)TextureManager::getSingleton().getByName(texname);
		if (enabled && dt)
		{
			IDirect3DBaseTexture9 *pTex = dt->getTexture();
			if (mTexStageDesc[stage].pTex != pTex)
			{
				hr = mpD3DDevice->SetTexture(stage, pTex);
				if( hr != S_OK )
				{
					String str = "Unable to set texture '" + texname + "' in D3D9";
					Except( hr, str, "D3D9RenderSystem::_setTexture" );
				}
				
				// set stage desc.
				mTexStageDesc[stage].pTex = pTex;
				mTexStageDesc[stage].texType = D3D9Mappings::get(dt->getTextureType());
			}
		}
		else
		{
			if (mTexStageDesc[stage].pTex != 0)
			{
				hr = mpD3DDevice->SetTexture(stage, 0);
				if( hr != S_OK )
				{
					String str = "Unable to disable texture '" + texname + "' in D3D9";
					Except( hr, str, "D3D9RenderSystem::_setTexture" );
				}
			}

			hr = this->__SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
			if( hr != S_OK )
			{
				String str = "Unable to disable texture '" + texname + "' in D3D9";
				Except( hr, str, "D3D9RenderSystem::_setTexture" );
			}

			// set stage desc. to defaults
			mTexStageDesc[stage].pTex = 0;
			mTexStageDesc[stage].autoTexCoordType = TEXCALC_NONE;
			mTexStageDesc[stage].coordIndex = 0;
			mTexStageDesc[stage].texType = D3D9Mappings::D3D_TEX_TYPE_NORMAL;
		}
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setTextureCoordSet( size_t stage, size_t index )
	{
		HRESULT hr;
		hr = __SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, index );
		if( FAILED( hr ) )
			Except( hr, "Unable to set texture coord. set index", "D3D8RenderSystem::_setTextureCoordSet" );
        // Record settings
        mTexStageDesc[stage].coordIndex = index;
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setTextureCoordCalculation( size_t stage, TexCoordCalcMethod m)
	{
		HRESULT hr = S_OK;
		// record the stage state
		mTexStageDesc[stage].autoTexCoordType = m;

		// choose normalization method
		if (m == TEXCALC_ENVIRONMENT_MAP_NORMAL || m == TEXCALC_ENVIRONMENT_MAP)
        {
            mForcedNormalisation = true;
			setNormaliseNormals(true);
        }
		else
        {
            mForcedNormalisation = false;
			setNormaliseNormals(false);
        }
		if (FAILED(hr))
			Except( hr, "Unable to set auto-normalisation", "D3D9RenderSystem::_setTextureCoordCalculation" );

		// set aut.tex.coord.gen.mode if present
		// if not present we'v already set it through D3D9RenderSystem::_setTextureCoordSet
		if (m != TEXCALC_NONE)
		{
			hr = __SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, D3D9Mappings::get(m, mCaps));
			if(FAILED(hr))
				Except( hr, "Unable to set texture auto tex.coord. generation mode", "D3D8RenderSystem::_setTextureCoordCalculation" );
		}
	}
    //---------------------------------------------------------------------
	void D3D9RenderSystem::_setTextureMatrix( size_t stage, const Matrix4& xForm )
	{
		HRESULT hr;
		D3DXMATRIX d3dMatId; // ident. matrix in D3DX format
		D3DXMATRIX d3dMat; // the matrix we'll maybe apply
		Matrix4 newMat = xForm; // the matrix we'll apply after conv. to D3D format
		// make the ident. matrix in D3D format
		D3DXMatrixIdentity(&d3dMatId);

		/* If envmap is applied, but device doesn't support spheremap,
		then we have to use texture transform to make the camera space normal
		reference the envmap properly. This isn't exactly the same as spheremap
		(it looks nasty on flat areas because the camera space normals are the same)
		but it's the best approximation we have in the absence of a proper spheremap */
		if (mTexStageDesc[stage].autoTexCoordType == TEXCALC_ENVIRONMENT_MAP &&
			!(mCaps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN_SPHEREMAP))
		{
			// if so we must concatenate the current with the env_map matrix
			D3DXMATRIX d3dMatEnvMap; // the env_map matrix
			// ident. it 
			D3DXMatrixIdentity(&d3dMatEnvMap);
			// set env_map values
			d3dMatEnvMap(0,0) = 0.5f;
			d3dMatEnvMap(3,0) = 0.5f;
			d3dMatEnvMap(1,1) = -0.5f;
			d3dMatEnvMap(3,1) = 0.5f;
			// convert it to ogre format
            Matrix4 ogreMatEnvMap = D3D9Mappings::convertD3DXMatrix(d3dMatEnvMap);
			// concatenate with the xForm
			newMat = newMat.concatenate(ogreMatEnvMap);
		}

        // If this is a cubic reflection, we need to modify using the view matrix
        if (mTexStageDesc[stage].autoTexCoordType == TEXCALC_ENVIRONMENT_MAP_REFLECTION)
        {
            D3DXMATRIX viewMatrix; 

            // Get view matrix
            mpD3DDevice->GetTransform(D3DTS_VIEW, &viewMatrix);
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
		d3dMat = D3D9Mappings::makeD3DXMatrix(newMat);

		// need this if texture is a cube map, to invert D3D's z coord
		if (mTexStageDesc[stage].autoTexCoordType != TEXCALC_NONE)
		{
			d3dMat._13 = -d3dMat._13;
			d3dMat._23 = -d3dMat._23;
			d3dMat._33 = -d3dMat._33;
			d3dMat._43 = -d3dMat._43;
		}

		// set the matrix if it's not the identity
		if (d3dMat != d3dMatId)
		{
			// tell D3D the dimension of tex. coord.
			int texCoordDim;
			switch (mTexStageDesc[stage].texType)
			{
			case D3D9Mappings::D3D_TEX_TYPE_NORMAL:
				texCoordDim = 2;
				break;
			case D3D9Mappings::D3D_TEX_TYPE_CUBE:
			case D3D9Mappings::D3D_TEX_TYPE_VOLUME:
				texCoordDim = 3;
			}

			hr = __SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, texCoordDim );
			if (FAILED(hr))
				Except( hr, "Unable to set texture coord. dimension", "D3D9RenderSystem::_setTextureMatrix" );

			hr = mpD3DDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage), &d3dMat );
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
			D3DXMatrixIdentity( &d3dMat );
			hr = mpD3DDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage), &d3dMat );
			if( FAILED( hr ) )
				Except( hr, "Error setting texture matrix", "D3D9RenderSystem::_setTextureMatrix" );
		}
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setTextureAddressingMode( size_t stage, TextureUnitState::TextureAddressingMode tam )
	{
		HRESULT hr;
		if( FAILED( hr = __SetSamplerState( stage, D3DSAMP_ADDRESSU, D3D9Mappings::get(tam) ) ) )
			Except( hr, "Failed to set texture addressing mode for U", "D3D9RenderSystem::_setTextureAddressingMode" );
		if( FAILED( hr = __SetSamplerState( stage, D3DSAMP_ADDRESSV, D3D9Mappings::get(tam) ) ) )
			Except( hr, "Failed to set texture addressing mode for V", "D3D9RenderSystem::_setTextureAddressingMode" );
		if( FAILED( hr = __SetSamplerState( stage, D3DSAMP_ADDRESSW, D3D9Mappings::get(tam) ) ) )
			Except( hr, "Failed to set texture addressing mode for W", "D3D9RenderSystem::_setTextureAddressingMode" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setTextureBlendMode( size_t stage, const LayerBlendModeEx& bm )
	{
		HRESULT hr = S_OK;
		D3DTEXTURESTAGESTATETYPE tss;
		D3DCOLOR manualD3D;

		// choose type of blend.
		if( bm.blendType == LBT_COLOUR )
			tss = D3DTSS_COLOROP;
		else if( bm.blendType == LBT_ALPHA )
			tss = D3DTSS_ALPHAOP;
		// set manual factor if required by operation
		if (bm.operation == LBX_BLEND_MANUAL)
		{
			hr = __SetRenderState( D3DRS_TEXTUREFACTOR, D3DXCOLOR(0.0, 0.0, 0.0,  bm.factor) );
			if (FAILED(hr))
				Except( hr, "Failed to set manual factor", "D3D9RenderSystem::_setTextureBlendMode" );
		}
		// set operation
		hr = __SetTextureStageState( stage, tss, D3D9Mappings::get(bm.operation, mCaps) );
		if (FAILED(hr))
			Except( hr, "Failed to set operation", "D3D9RenderSystem::_setTextureBlendMode" );

		// choose source 1
		if( bm.blendType == LBT_COLOUR )
		{
			tss = D3DTSS_COLORARG1;
			manualD3D = D3DXCOLOR( bm.colourArg1.r, bm.colourArg1.g, bm.colourArg1.b, 1.0 );
		}
		else if( bm.blendType == LBT_ALPHA )
		{
			tss = D3DTSS_ALPHAARG1;
			manualD3D = D3DXCOLOR( 0.0, 0.0, 0.0, bm.alphaArg1 );
		}
		// Set manual factor if required
		if (bm.source1 == LBS_MANUAL)
		{
			hr = __SetRenderState( D3DRS_TEXTUREFACTOR, manualD3D );
			if (FAILED(hr))
				Except( hr, "Failed to set manual factor", "D3D9RenderSystem::_setTextureBlendMode" );
		}
		// set source 1
		hr = __SetTextureStageState( stage, tss, D3D9Mappings::get(bm.source1) );
		if (FAILED(hr))
			Except( hr, "Failed to set source1", "D3D9RenderSystem::_setTextureBlendMode" );
		
		// choose source 2
		if( bm.blendType == LBT_COLOUR )
		{
			tss = D3DTSS_COLORARG2;
			manualD3D = D3DXCOLOR( bm.colourArg2.r, bm.colourArg2.g, bm.colourArg2.b, 1.0 );
		}
		else if( bm.blendType == LBT_ALPHA )
		{
			tss = D3DTSS_ALPHAARG2;
			manualD3D = D3DXCOLOR( 0.0, 0.0, 0.0, bm.alphaArg2 );
		}
		// Set manual factor if required
		if (bm.source2 == LBS_MANUAL)
		{
			hr = __SetRenderState( D3DRS_TEXTUREFACTOR, manualD3D );
			if (FAILED(hr))
				Except( hr, "Failed to set manual factor", "D3D9RenderSystem::_setTextureBlendMode" );
		}
		// Now set source 2
		hr = __SetTextureStageState( stage, tss, D3D9Mappings::get(bm.source2) );
		if (FAILED(hr))
			Except( hr, "Failed to set source 2", "D3D9RenderSystem::_setTextureBlendMode" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor )
	{
		HRESULT hr;
		if( FAILED( hr = __SetRenderState( D3DRS_SRCBLEND, D3D9Mappings::get(sourceFactor) ) ) )
			Except( hr, "Failed to set source blend", "D3D9RenderSystem::_setSceneBlending" );
		if( FAILED( hr = __SetRenderState( D3DRS_DESTBLEND, D3D9Mappings::get(destFactor) ) ) )
			Except( hr, "Failed to set destination blend", "D3D9RenderSystem::_setSceneBlending" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setAlphaRejectSettings( CompareFunction func, unsigned char value )
	{
		HRESULT hr;
        if (func != CMPF_ALWAYS_PASS)
        {
            if( FAILED( hr = __SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE ) ) )
    			Except( hr, "Failed to enable alpha testing", 
                "D3D9RenderSystem::_setAlphaRejectSettings" );
        }
        else
        {
            if( FAILED( hr = __SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE ) ) )
    			Except( hr, "Failed to disable alpha testing", 
                "D3D9RenderSystem::_setAlphaRejectSettings" );
        }
        // Set always just be sure
		if( FAILED( hr = __SetRenderState( D3DRS_ALPHAFUNC, D3D9Mappings::get(func) ) ) )
			Except( hr, "Failed to set alpha reject function", "D3D9RenderSystem::_setAlphaRejectSettings" );
		if( FAILED( hr = __SetRenderState( D3DRS_ALPHAREF, value ) ) )
			Except( hr, "Failed to set render state D3DRS_ALPHAREF", "D3D9RenderSystem::_setAlphaRejectSettings" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setCullingMode( CullingMode mode )
	{
		HRESULT hr;
		if( FAILED (hr = __SetRenderState(D3DRS_CULLMODE, D3D9Mappings::get(mode, mActiveRenderTarget->requiresTextureFlipping()))) )
			Except( hr, "Failed to set culling mode", "D3D9RenderSystem::_setCullingMode" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setDepthBufferParams( bool depthTest, bool depthWrite, CompareFunction depthFunction )
	{
		_setDepthBufferCheckEnabled( depthTest );
		_setDepthBufferWriteEnabled( depthWrite );
		_setDepthBufferFunction( depthFunction );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setDepthBufferCheckEnabled( bool enabled )
	{
		HRESULT hr;

		if( enabled )
		{
			// Use w-buffer if abialable
			if( mCaps.RasterCaps & D3DPRASTERCAPS_WBUFFER )
				hr = __SetRenderState( D3DRS_ZENABLE, D3DZB_USEW );
			else
				hr = __SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
		}
		else
			hr = __SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );

		if( FAILED( hr ) )
			Except( hr, "Error setting depth buffer test state", "D3D9RenderSystem::_setDepthBufferCheckEnabled" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setDepthBufferWriteEnabled( bool enabled )
	{
		HRESULT hr;

		if( FAILED( hr = __SetRenderState( D3DRS_ZWRITEENABLE, enabled ) ) )
			Except( hr, "Error setting depth buffer write state", "D3D9RenderSystem::_setDepthBufferWriteEnabled" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setDepthBufferFunction( CompareFunction func )
	{
		HRESULT hr;
		if( FAILED( hr = __SetRenderState( D3DRS_ZFUNC, D3D9Mappings::get(func) ) ) )
			Except( hr, "Error setting depth buffer test function", "D3D9RenderSystem::_setDepthBufferFunction" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setDepthBias(ushort bias)
	{
		HRESULT hr = __SetRenderState(D3DRS_DEPTHBIAS, bias);
		if (FAILED(hr))
			Except(hr, "Error setting depth bias", "D3D9RenderSystem::_setDepthBias");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setColourBufferWriteEnabled(bool red, bool green, 
		bool blue, bool alpha)
	{
		DWORD val = 0;
		if (red) 
			val |= D3DCOLORWRITEENABLE_RED;
		if (green)
			val |= D3DCOLORWRITEENABLE_GREEN;
		if (blue)
			val |= D3DCOLORWRITEENABLE_BLUE;
		if (alpha)
			val |= D3DCOLORWRITEENABLE_ALPHA;
		HRESULT hr = __SetRenderState(D3DRS_COLORWRITEENABLE, val); 
		if (FAILED(hr))
			Except(hr, "Error setting colour write enable flags", 
			"D3D9RenderSystem::_setColourWriteEnabled");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setFog( FogMode mode, const ColourValue& colour, Real densitiy, Real start, Real end )
	{
		HRESULT hr;

		D3DRENDERSTATETYPE fogType, fogTypeNot;

		if (mCaps.RasterCaps & D3DPRASTERCAPS_FOGTABLE)
		{
			fogType = D3DRS_FOGTABLEMODE;
			fogTypeNot = D3DRS_FOGVERTEXMODE;
		}
		else
		{
			fogType = D3DRS_FOGVERTEXMODE;
			fogTypeNot = D3DRS_FOGTABLEMODE;
		}

		if( mode == FOG_NONE)
		{
			// just disable
			hr = __SetRenderState(fogType, D3DFOG_NONE );
			hr = __SetRenderState(D3DRS_FOGENABLE, FALSE);
		}
		else
		{
			// Allow fog
			hr = __SetRenderState( D3DRS_FOGENABLE, TRUE );
			hr = __SetRenderState( fogTypeNot, D3DFOG_NONE );
			hr = __SetRenderState( fogType, D3D9Mappings::get(mode) );

			hr = __SetRenderState( D3DRS_FOGCOLOR, colour.getAsLongARGB() );
			hr = __SetRenderState( D3DRS_FOGSTART, *((LPDWORD)(&start)) );
			hr = __SetRenderState( D3DRS_FOGEND, *((LPDWORD)(&end)) );
			hr = __SetRenderState( D3DRS_FOGDENSITY, *((LPDWORD)(&densitiy)) );
		}

		if( FAILED( hr ) )
			Except( hr, "Error setting render state", "D3D9RenderSystem::_setFog" );
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setRasterisationMode(SceneDetailLevel level)
	{
		HRESULT hr = __SetRenderState(D3DRS_FILLMODE, D3D9Mappings::get(level));
		if (FAILED(hr))
			Except(hr, "Error setting rasterisation mode.", "D3D9RenderSystem::setRasterisationMode");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setStencilCheckEnabled(bool enabled)
	{
		// Allow stencilling
		HRESULT hr = __SetRenderState(D3DRS_STENCILENABLE, enabled);
		if (FAILED(hr))
			Except(hr, "Error enabling / disabling stencilling.",
			"D3D9RenderSystem::setStencilCheckEnabled");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setStencilBufferFunction(CompareFunction func)
	{
		HRESULT hr = __SetRenderState(D3DRS_STENCILFUNC, D3D9Mappings::get(func));
		if (FAILED(hr))
			Except(hr, "Error setting stencil buffer test function.",
			"D3D9RenderSystem::_setStencilBufferFunction");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setStencilBufferReferenceValue(ulong refValue)
	{
		HRESULT hr = __SetRenderState(D3DRS_STENCILREF, refValue);
		if (FAILED(hr))
			Except(hr, "Error setting stencil buffer reference value.",
			"D3D9RenderSystem::setStencilBufferReferenceValue");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setStencilBufferMask(ulong mask)
	{
		HRESULT hr = __SetRenderState(D3DRS_STENCILMASK, mask);
		if (FAILED(hr))
			Except(hr, "Error setting stencil buffer mask.",
			"D3D9RenderSystem::setStencilBufferMask");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setStencilBufferFailOperation(StencilOperation op)
	{
		HRESULT hr = __SetRenderState(D3DRS_STENCILFAIL, D3D9Mappings::get(op));
		if (FAILED(hr))
			Except(hr, "Error setting stencil fail operation.",
			"D3D9RenderSystem::setStencilBufferFailOperation");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setStencilBufferDepthFailOperation(StencilOperation op)
	{
		HRESULT hr = __SetRenderState(D3DRS_STENCILZFAIL, D3D9Mappings::get(op));
		if (FAILED(hr))
			Except(hr, "Error setting stencil depth fail operation.",
			"D3D9RenderSystem::setStencilBufferDepthFailOperation");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::setStencilBufferPassOperation(StencilOperation op)
	{
		HRESULT hr = __SetRenderState(D3DRS_STENCILPASS, D3D9Mappings::get(op));
		if (FAILED(hr))
			Except(hr, "Error setting stencil pass operation.",
			"D3D9RenderSystem::setStencilBufferPassOperation");
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setTextureLayerFiltering(size_t unit, const TextureFilterOptions texLayerFilterOps)
	{
		HRESULT hr;
		D3D9Mappings::eD3DTexType texType = mTexStageDesc[unit].texType;
		// set mag. filter
		hr = __SetSamplerState( unit, D3DSAMP_MAGFILTER, D3D9Mappings::get(texLayerFilterOps, mCaps, texType, D3D9Mappings::D3D_FUSAGE_MAG) );
		if (FAILED(hr))
			Except(hr, "Failed to set MagFilter", "D3D9RenderSystem::_setTextureLayerFiltering");
		// set min. filter
		hr = __SetSamplerState( unit, D3DSAMP_MINFILTER, D3D9Mappings::get(texLayerFilterOps, mCaps, texType, D3D9Mappings::D3D_FUSAGE_MIN) );
		if (FAILED(hr))
			Except(hr, "Failed to set MinFilter", "D3D9RenderSystem::_setTextureLayerFiltering");
		// set mip filter
		hr = __SetSamplerState( unit, D3DSAMP_MIPFILTER, D3D9Mappings::get(texLayerFilterOps, mCaps, texType, D3D9Mappings::D3D_FUSAGE_MIP) );
		if (FAILED(hr))
			Except(hr, "Failed to set MipFilter", "D3D9RenderSystem::_setTextureLayerFiltering");
	}
    //---------------------------------------------------------------------
	DWORD D3D9RenderSystem::_getCurrentAnisotropy(size_t unit)
	{
		DWORD oldVal;
		mpD3DDevice->GetSamplerState(unit, D3DSAMP_MAXANISOTROPY, &oldVal);
			return oldVal;
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setTextureLayerAnisotropy(size_t unit, int maxAnisotropy)
	{
		if ((DWORD)maxAnisotropy > mCaps.MaxAnisotropy)
			maxAnisotropy = mCaps.MaxAnisotropy;

		if (_getCurrentAnisotropy(unit) != maxAnisotropy)
			__SetSamplerState( unit, D3DSAMP_MAXANISOTROPY, maxAnisotropy );
	}
	//---------------------------------------------------------------------
	HRESULT D3D9RenderSystem::__SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
	{
		HRESULT hr;
		DWORD oldVal;

		if ( FAILED( hr = mpD3DDevice->GetRenderState(state, &oldVal) ) )
			return hr;
		if ( oldVal == value )
			return D3D_OK;
		else
			return mpD3DDevice->SetRenderState(state, value);
	}
	//---------------------------------------------------------------------
	HRESULT D3D9RenderSystem::__SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
	{
		HRESULT hr;
		DWORD oldVal;

		if ( FAILED( hr = mpD3DDevice->GetSamplerState(sampler, type, &oldVal) ) )
			return hr;
		if ( oldVal == value )
			return D3D_OK;
		else
			return mpD3DDevice->SetSamplerState(sampler, type, value);
	}
	//---------------------------------------------------------------------
	HRESULT D3D9RenderSystem::__SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
	{
		HRESULT hr;
		DWORD oldVal;
		
		if ( FAILED( hr = mpD3DDevice->GetTextureStageState(stage, type, &oldVal) ) )
			return hr;
		if ( oldVal == value )
			return D3D_OK;
		else
			return mpD3DDevice->SetTextureStageState(stage, type, value);
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setViewport( Viewport *vp )
	{
		if( vp != mActiveViewport || vp->_isUpdated() )
		{
			mActiveViewport = vp;
			mActiveRenderTarget = vp->getTarget();

			// ok, it's different, time to set render target and viewport params
			D3DVIEWPORT9 d3dvp;
			HRESULT hr;

			// Set render target
			RenderTarget* target;
			target = vp->getTarget();

			LPDIRECT3DSURFACE9 pBack = NULL;
			target->getCustomAttribute( "DDBACKBUFFER", &pBack );
			if (!pBack)
				return;

			LPDIRECT3DSURFACE9 pDepth = NULL;
			target->getCustomAttribute( "D3DZBUFFER", &pDepth );
			if (!pDepth)
				return;
			
			hr = mpD3DDevice->SetRenderTarget(0, pBack);
			if (FAILED(hr))
			{
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Failed to setRenderTarget : " + msg, "D3D9RenderSystem::_setViewport" );
			}
			hr = mpD3DDevice->SetDepthStencilSurface(pDepth);
			if (FAILED(hr))
			{
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Failed to setDepthStencil : " + msg, "D3D9RenderSystem::_setViewport" );
			}

			_setCullingMode( mCullingMode );

			// set viewport dimensions
			d3dvp.X = vp->getActualLeft();
			d3dvp.Y = vp->getActualTop();
			d3dvp.Width = vp->getActualWidth();
			d3dvp.Height = vp->getActualHeight();

			// Z-values from 0.0 to 1.0 (TODO: standardise with OpenGL)
			d3dvp.MinZ = 0.0f;
			d3dvp.MaxZ = 1.0f;

			if( FAILED( hr = mpD3DDevice->SetViewport( &d3dvp ) ) )
				Except( hr, "Failed to set viewport.", "D3D9RenderSystem::_setViewport" );

			vp->_clearUpdatedFlag();
		}
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_beginFrame()
	{
		OgreGuard( "D3D9RenderSystem::_beginFrame" );

		HRESULT hr;

		if( !mActiveViewport )
			Except( Exception::ERR_INTERNAL_ERROR, "Cannot begin frame - no viewport selected.", "D3D9RenderSystem::_beginFrame" );

		// Clear the viewport if required
		if( mActiveViewport->getClearEveryFrame() )
		{
			if( FAILED( hr = mpD3DDevice->Clear( 
				0, 
				NULL, 
				D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
				mActiveViewport->getBackgroundColour().getAsLongARGB(), 
				1.0f, 0 ) ) )
			{
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Error clearing viewport : " + msg, "D3D9RenderSystem::_beginFrame" );
			}
		}

		if( FAILED( hr = mpD3DDevice->BeginScene() ) )
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Error beginning frame :" + msg, "D3D9RenderSystem::_beginFrame" );
		}

		static bool firstTime = true;
		if( firstTime )
		{
			// First-time 
			// setup some defaults
			// Allow alpha blending
			hr = __SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			if (FAILED(hr))
			{
				String msg = DXGetErrorDescription9(hr);
				Except(hr, "Error enabling alpha blending option : " + msg, "D3D9RenderSystem::_beginFrame");
			}
			// Allow specular
			hr = __SetRenderState(D3DRS_SPECULARENABLE, TRUE);
			if (FAILED(hr))
			{
				String msg = DXGetErrorDescription9(hr);
				Except(hr, "Error enabling alpha blending option : " + msg, "D3D9RenderSystem::_beginFrame");
			}
			firstTime = false;
		}

		OgreUnguard();
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_endFrame()
	{
		OgreGuard( "D3D9RenderSystem::_endFrame" );

		HRESULT hr;
		if( FAILED( hr = mpD3DDevice->EndScene() ) )
			Except( hr, "Error ending frame", "D3D9RenderSystem::_endFrame" );

		OgreUnguard();
	}
	//---------------------------------------------------------------------
	inline bool D3D9RenderSystem::compareDecls( D3DVERTEXELEMENT9* pDecl1, D3DVERTEXELEMENT9* pDecl2, int size )
	{
		for( int i=0; i < size; i++ )
		{
			if( pDecl1[i].Method != pDecl2[i].Method ||
				pDecl1[i].Offset != pDecl2[i].Offset ||
				pDecl1[i].Stream != pDecl2[i].Stream ||
				pDecl1[i].Type != pDecl2[i].Type ||
				pDecl1[i].Usage != pDecl2[i].Usage ||
				pDecl1[i].UsageIndex != pDecl2[i].UsageIndex)
			{
				return false;
			}
		}

		return true;
	}
    //---------------------------------------------------------------------
	void D3D9RenderSystem::setVertexDeclaration(VertexDeclaration* decl)
	{
		// Guard
		OgreGuard ("D3D9RenderSystem::setVertexDeclaration");
        HRESULT hr;

        D3D9VertexDeclaration* d3ddecl = 
            static_cast<D3D9VertexDeclaration*>(decl);

        static VertexDeclaration* lastDecl = 0;

        // attempt to detect duplicates
        if (!lastDecl || !(*lastDecl == *decl))
        {

            if (FAILED(hr = mpD3DDevice->SetVertexDeclaration(d3ddecl->getD3DVertexDeclaration())))
            {
                Except(hr, "Unable to set D3D9 vertex declaration", 
                    "D3D9RenderSystem::setVertexDeclaration");
            }
        }

        // UnGuard
		OgreUnguard();
	}
    //---------------------------------------------------------------------
	void D3D9RenderSystem::setVertexBufferBinding(VertexBufferBinding* binding)
	{
		// Guard
		OgreGuard ("D3D9RenderSystem::setVertexBufferBinding");

        HRESULT hr;

        // TODO: attempt to detect duplicates
        const VertexBufferBinding::VertexBufferBindingMap& binds = binding->getBindings();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator i, iend;
        iend = binds.end();
        for (i = binds.begin(); i != iend; ++i)
        {
            const D3D9HardwareVertexBuffer* d3d9buf = 
                static_cast<const D3D9HardwareVertexBuffer*>(i->second.get());
            hr = mpD3DDevice->SetStreamSource(
                static_cast<UINT>(i->first),
                d3d9buf->getD3D9VertexBuffer(),
                0, // no stream offset, this is handled in _render instead
                static_cast<UINT>(d3d9buf->getVertexSize()) // stride
                );
            if (FAILED(hr))
            {
                Except(hr, "Unable to set D3D9 stream source for buffer binding", 
                    "D3D9RenderSystem::setVertexBufferBinding");
            }


        }

		// Unbind any unused sources
		for (size_t unused = binds.size(); unused < mLastVertexSourceCount; ++unused)
		{
			
            hr = mpD3DDevice->SetStreamSource(static_cast<UINT>(unused), NULL, 0, 0);
            if (FAILED(hr))
            {
                Except(hr, "Unable to reset unused D3D9 stream source", 
                    "D3D9RenderSystem::setVertexBufferBinding");
            }
			
		}
		mLastVertexSourceCount = binds.size();
		

		
        // UnGuard
		OgreUnguard();
	}
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_render(const RenderOperation& op)
	{
		// Guard
		OgreGuard ("D3D9RenderSystem::_render");

        // Exit immediately if there is nothing to render
        // This caused a problem on FireGL 8800
        if (op.vertexData->vertexCount == 0)
            return;

        // Call super class
		RenderSystem::_render(op);

        // To think about: possibly remove setVertexDeclaration and 
        // setVertexBufferBinding from RenderSystem since the sequence is
        // a bit too D3D9-specific?
		setVertexDeclaration(op.vertexData->vertexDeclaration);
        setVertexBufferBinding(op.vertexData->vertexBufferBinding);

		// Determine rendering operation
		D3DPRIMITIVETYPE primType;
		DWORD primCount = 0;
        switch( op.operationType )
		{
        case RenderOperation::OT_POINT_LIST:
			primType = D3DPT_POINTLIST;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount);
			break;

		case RenderOperation::OT_LINE_LIST:
			primType = D3DPT_LINELIST;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2;
			break;

		case RenderOperation::OT_LINE_STRIP:
			primType = D3DPT_LINESTRIP;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 1;
			break;

		case RenderOperation::OT_TRIANGLE_LIST:
			primType = D3DPT_TRIANGLELIST;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 3;
			break;

		case RenderOperation::OT_TRIANGLE_STRIP:
			primType = D3DPT_TRIANGLESTRIP;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
			break;

		case RenderOperation::OT_TRIANGLE_FAN:
			primType = D3DPT_TRIANGLEFAN;
			primCount = (DWORD)(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
			break;
		}

        if (!primCount)
			return;

		// Issue the op
        HRESULT hr;
		if( op.useIndexes )
		{
            D3D9HardwareIndexBuffer* d3dIdxBuf = 
                static_cast<D3D9HardwareIndexBuffer*>(op.indexData->indexBuffer.get());
			hr = mpD3DDevice->SetIndices( d3dIdxBuf->getD3DIndexBuffer() );
			if (FAILED(hr))
            {
				Except( hr, "Failed to set index buffer", "D3D9RenderSystem::_render" );
            }

			// do indexed draw operation
			hr = mpD3DDevice->DrawIndexedPrimitive(
                primType, 
                static_cast<INT>(op.vertexData->vertexStart), 
                0, // Min vertex index - assume we can go right down to 0 
                static_cast<UINT>(op.vertexData->vertexCount), 
                static_cast<UINT>(op.indexData->indexStart), 
                static_cast<UINT>(primCount)
                );
		}
		else
        {
            // Unindexed, a little simpler!
			hr = mpD3DDevice->DrawPrimitive(
                primType, 
                static_cast<UINT>(op.vertexData->vertexStart), 
                static_cast<UINT>(primCount)
                ); 
        }

		if( FAILED( hr ) )
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Failed to DrawPrimitive : " + msg, "D3D9RenderSystem::_render" );
		}
        
        // UnGuard
		OgreUnguard();

	}
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setNormaliseNormals(bool normalise)
    {
        __SetRenderState(D3DRS_NORMALIZENORMALS, 
            (normalise || mForcedNormalisation) ? TRUE : FALSE);
    }
	//---------------------------------------------------------------------
    void D3D9RenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        HRESULT hr;
        switch (prg->getType())
        {
        case GPT_VERTEX_PROGRAM:
            hr = mpD3DDevice->SetVertexShader(
                static_cast<D3D9GpuVertexProgram*>(prg)->getVertexShader());
            if (FAILED(hr))
            {
                Except(hr, "Error calling SetVertexShader", "D3D9RenderSystem::bindGpuProgram");
            }
            break;
        case GPT_FRAGMENT_PROGRAM:
            hr = mpD3DDevice->SetPixelShader(
                static_cast<D3D9GpuFragmentProgram*>(prg)->getPixelShader());
            if (FAILED(hr))
            {
                Except(hr, "Error calling SetPixelShader", "D3D9RenderSystem::bindGpuProgram");
            }
            break;
        };

    }
	//---------------------------------------------------------------------
    void D3D9RenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
        HRESULT hr;
        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            hr = mpD3DDevice->SetVertexShader(NULL);
            if (FAILED(hr))
            {
                Except(hr, "Error resetting SetVertexShader to NULL", 
                    "D3D9RenderSystem::unbindGpuProgram");
            }
            break;
        case GPT_FRAGMENT_PROGRAM:
            hr = mpD3DDevice->SetPixelShader(NULL);
            if (FAILED(hr))
            {
                Except(hr, "Error resetting SetPixelShader to NULL", 
                    "D3D9RenderSystem::unbindGpuProgram");
            }
            break;
        };
    }
	//---------------------------------------------------------------------
    void D3D9RenderSystem::bindGpuProgramParameters(GpuProgramType gptype, 
        GpuProgramParametersSharedPtr params)
    {
        HRESULT hr;
        unsigned int index;
        GpuProgramParameters::IntConstantIterator intIt = params->getIntConstantIterator();
        GpuProgramParameters::RealConstantIterator realIt = params->getRealConstantIterator();

        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            // Bind floats
            if (params->hasRealConstantParams())
            {
                // Iterate over params and set the relevant ones
                index = 0;
                while (realIt.hasMoreElements())
                {
                    GpuProgramParameters::RealConstantEntry* e = realIt.peekNextPtr();
                    if (e->isSet)
                    {
                        if (FAILED(hr = mpD3DDevice->SetVertexShaderConstantF(
                            index++, e->val, 1)))
                        {
                            Except(hr, "Unable to upload shader float parameters", 
                                "D3D9RenderSystem::bindGpuProgramParameters");
                        }
                    }
                    realIt.moveNext();
                }
            }
            // Bind ints
            if (params->hasIntConstantParams())
            {
                // Iterate over params and set the relevant ones
                index = 0;
                while (intIt.hasMoreElements())
                {
                    GpuProgramParameters::IntConstantEntry* e = intIt.peekNextPtr();
                    if (e->isSet)
                    {
                        if (FAILED(hr = mpD3DDevice->SetVertexShaderConstantI(
                            index++, e->val, 1)))
                        {
                            Except(hr, "Unable to upload shader float parameters", 
                                "D3D9RenderSystem::bindGpuProgramParameters");
                        }
                    }
                    intIt.moveNext();
                }
            }
            break;
        case GPT_FRAGMENT_PROGRAM:
            // Bind floats
            if (params->hasRealConstantParams())
            {
                // Iterate over params and set the relevant ones
                index = 0;
                while (realIt.hasMoreElements())
                {
                    GpuProgramParameters::RealConstantEntry* e = realIt.peekNextPtr();
                    if (e->isSet)
                    {
                        if (FAILED(hr = mpD3DDevice->SetPixelShaderConstantF(
                            index++, e->val, 1)))
                        {
                            Except(hr, "Unable to upload shader float parameters", 
                                "D3D9RenderSystem::bindGpuProgramParameters");
                        }
                    }
                    realIt.moveNext();
                }
            }
            // Bind ints
            if (params->hasIntConstantParams())
            {
                // Iterate over params and set the relevant ones
                index = 0;
                while (intIt.hasMoreElements())
                {
                    GpuProgramParameters::IntConstantEntry* e = intIt.peekNextPtr();
                    if (e->isSet)
                    {
                        if (FAILED(hr = mpD3DDevice->SetPixelShaderConstantI(
                            index++, e->val, 1)))
                        {
                            Except(hr, "Unable to upload shader float parameters", 
                                "D3D9RenderSystem::bindGpuProgramParameters");
                        }
                    }
                    intIt.moveNext();
                }
            }
            break;
        };
    }

}
