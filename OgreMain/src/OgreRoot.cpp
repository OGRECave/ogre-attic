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
// Ogre includes
#include "OgreStableHeaders.h"

#include "OgreRoot.h"

#include "OgreRenderSystem.h"
#include "OgreException.h"
#include "OgreControllerManager.h"
#include "OgreLogManager.h"
#include "OgreMath.h"
#include "OgreDynLibManager.h"
#include "OgreDynLib.h"
#include "OgreConfigFile.h"
#include "OgreMaterialManager.h"
#include "OgreResourceManager.h"
#include "OgreMeshManager.h"
#include "OgreTextureManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreSkeletonManager.h"
#include "OgreGuiManager.h"
#include "OgreOverlayManager.h"
#include "OgreZipArchiveFactory.h"
#include "OgreProfiler.h"
#include "OgreErrorDialog.h"
#include "OgreConfigDialog.h"
#include "OgreStringConverter.h"

#include "OgrePNGCodec.h"
#include "OgreBMPCodec.h"
#include "OgreJPEGCodec.h"
#include "OgreTGACodec.h"
#include "OgreDDSCodec.h"

#include "OgreFontManager.h"
#include "OgreHardwareBufferManager.h"

#include "OgreOverlay.h"
#include "OgreHighLevelGpuProgramManager.h"

#include "OgreExternalTextureSourceManager.h"

#if OGRE_PLATFORM == PLATFORM_WIN32

#   define WIN32_LEAN_AND_MEAN
#   include <direct.h>
#   include <windows.h>

#endif


namespace Ogre {    

    typedef void (*DLL_START_PLUGIN)(void);
    typedef void (*DLL_STOP_PLUGIN)(void);


    //-----------------------------------------------------------------------
    // Termination handler
    extern "C" _OgreExport void handleTerminate(void)
    {
        LogManager::getSingleton().logMessage(LML_CRITICAL, "Termination handler: uncaught exception!");

        Root::getSingleton().shutdown();

        ErrorDialog* dlg = PlatformManager::getSingleton().createErrorDialog();

        Exception* e = Exception::getLastException();

        if (e)
            dlg->display(e->getFullDescription());
        else
            dlg->display("Unknown");

        // Abort
        exit(-1);

    }

    void Root::termHandler()
    {
        handleTerminate();
    }

    //-----------------------------------------------------------------------
    template<> Root* Singleton<Root>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    Root::Root(const String& pluginFileName, const String& configFileName, const String& logFileName)
    {
        // First create new exception handler
        SET_TERM_HANDLER;

        // superclass will do singleton checking
        String msg;

        // Init
        mActiveRenderer = 0;
        mVersion = StringConverter::toString(OGRE_VERSION_MAJOR) + "." + 
            StringConverter::toString(OGRE_VERSION_MINOR) + "." + 
            StringConverter::toString(OGRE_VERSION_PATCH);
		mConfigFileName = configFileName;

        // Create log manager and default log file
        mLogManager = new LogManager();
        mLogManager->createLog(logFileName, true, true);

        // Dynamic library manager
        mDynLibManager = new DynLibManager();

        mArchiveManager = new ArchiveManager();

        // Create SceneManager enumerator (note - will be managed by singleton)
        mSceneManagerEnum = new SceneManagerEnumerator();
        mCurrentSceneManager = NULL;

        // ..material manager
        mMaterialManager = new MaterialManager();

        // Mesh manager
        mMeshManager = new MeshManager();

        // Skeleton manager
        mSkeletonManager = new SkeletonManager();

        // ..particle system manager
        mParticleManager = new ParticleSystemManager();

        // Platform manager
        mPlatformManager = new PlatformManager();

        // Timer
        mTimer = mPlatformManager->createTimer();

        // Overlay manager
        mOverlayManager = new OverlayManager();
        // Gui Manager
        mGuiManager = new GuiManager();
        // Font manager
        mFontManager = new FontManager();

#if OGRE_PROFILING
        // Profiler
        mProfiler = new Profiler();
		Profiler::getSingleton().setTimer(mTimer);
#endif
        mZipArchiveFactory = new ZipArchiveFactory();
        ArchiveManager::getSingleton().addArchiveFactory( mZipArchiveFactory );

        mPNGCodec = new PNGCodec;
        Codec::registerCodec( mPNGCodec );
        mJPEGCodec = new JPEGCodec;
        Codec::registerCodec( mJPEGCodec );
        mTGACodec = new TGACodec;
        Codec::registerCodec( mTGACodec );
        mDDSCodec = new DDSCodec;
        Codec::registerCodec( mDDSCodec );
        mJPGCodec = new JPGCodec;
        Codec::registerCodec( mJPGCodec );
        mBMPCodec = new BMPCodec;
        Codec::registerCodec( mBMPCodec );

        mHighLevelGpuProgramManager = new HighLevelGpuProgramManager();

		mExternalTextureSourceManager = new ExternalTextureSourceManager();

        // Auto window
        mAutoWindow = 0;

        // Load plugins
        if (!pluginFileName.empty())
            loadPlugins(pluginFileName);        

        mLogManager->logMessage("*-*-* OGRE Initialising");
        msg = "*-*-* Version " + mVersion;
        mLogManager->logMessage(msg);

        // Can't create managers until initialised
        mControllerManager = 0;


        // Seed random number generator for future use
        srand((unsigned)time(0));

#if OGRE_COMPILER == COMPILER_MSVC
#   if OGRE_COMP_VER < 1300
        // Instantiate templates for spooling to a String
        // This seems to be a crappy VC6 thing, it generates link errors if we use
        //  the templates from outside OgreMain, even when 'inline'.
        String str1, str2;
        Quaternion q;
        Vector3 v;
        Matrix4 m;
        float f = 0.0;
        int i = 0;
        char c = 'A';
        str1 = "";
        str2 = "aa";
        str1 << str2 << q << v << f << i << c;
        // Stop instantiating templates
#   endif
#endif

        


    }

    //-----------------------------------------------------------------------
    Root::~Root()
    {
        shutdown();
        delete mSceneManagerEnum;

		delete mExternalTextureSourceManager;

        delete mBMPCodec;
        delete mDDSCodec;
        delete mTGACodec;
        delete mJPGCodec;
        delete mJPEGCodec;
        delete mPNGCodec;
        delete mZipArchiveFactory;
#if OGRE_PROFILING
        delete mProfiler;
#endif
        delete mGuiManager;
        delete mOverlayManager;
        delete mFontManager;
        delete mArchiveManager;
        delete mSkeletonManager;
        delete mMeshManager;
        delete mMaterialManager;        
        delete mParticleManager;

        if( mControllerManager )
            delete mControllerManager;
        if (mHighLevelGpuProgramManager)
            delete mHighLevelGpuProgramManager;


        unloadPlugins();



        mPlatformManager->destroyTimer(mTimer);
        delete mPlatformManager;
        delete mDynLibManager;
        delete mLogManager;
    }

    //-----------------------------------------------------------------------
    void Root::saveConfig(void)
    {
        ::FILE *fp;
        char rec[100];

        fp = fopen(mConfigFileName, "w");
        if (!fp)
            Except(Exception::ERR_CANNOT_WRITE_TO_FILE, "Cannot create settings file.",
            "Root::saveConfig");

        if (mActiveRenderer)
        {
            sprintf(rec, "Render System\t%s\n", mActiveRenderer->getName().c_str());
            fputs(rec, fp);

            ConfigOptionMap& opts = mActiveRenderer->getConfigOptions();
            for(  ConfigOptionMap::iterator pOpt = opts.begin(); pOpt != opts.end(); ++pOpt )
            {
                sprintf(rec, "%s\t%s\n", pOpt->first.c_str(),
                    pOpt->second.currentValue.c_str());
                fputs(rec, fp);
            }
        }
        else
        {
            strcpy(rec, "Render System\t ");
            fputs(rec, fp);
        }

        fclose(fp);

    }
    //-----------------------------------------------------------------------
    bool Root::restoreConfig(void)
    {
        // Restores configuration from saved state
        // Returns true if a valid saved configuration is
        //   available, and false if no saved config is
        //   stored, or if there has been a problem
        ConfigFile cfg;
        String renderSystem;
        RenderSystemList::iterator pRend;

        try {
            // Don't trim whitespace
            cfg.load(mConfigFileName, "\t:=", false);
        }
        catch (Exception& e)
        {
            if (e.getNumber() == Exception::ERR_FILE_NOT_FOUND)
            {
                return false;
            }
            else
            {
                throw;
            }
        }

        renderSystem = cfg.getSetting("Render System");
        if(!renderSystem)
        {
            // No render system entry - error
            return false;
        }
		
        pRend = getAvailableRenderers()->begin();
        while (pRend != getAvailableRenderers()->end())
        {
            String rName = (*pRend)->getName();
            if (rName == renderSystem)
                break;
            pRend++;
        }

        if (pRend == getAvailableRenderers()->end())
        {
            // Unrecognised render system
            return false;
        }

        setRenderSystem(*pRend);

        ConfigFile::SettingsIterator i = cfg.getSettingsIterator();

        String optName, optVal;
        while (i.hasMoreElements())
        {
            optName = i.peekNextKey();
            optVal = i.getNext();
            if(optName != "Render System")
            {
                mActiveRenderer->setConfigOption(optName, optVal);
            }
        }

        // Successful load
        return true;

    }

    //-----------------------------------------------------------------------
    bool Root::showConfigDialog(void)
    {
        // Displays the standard config dialog
        // Will use stored defaults if available
        ConfigDialog* dlg;
        bool isOk;

        dlg = mPlatformManager->createConfigDialog();

        isOk = dlg->display();

        mPlatformManager->destroyConfigDialog(dlg);

        return isOk;

    }

    //-----------------------------------------------------------------------
    RenderSystemList* Root::getAvailableRenderers(void)
    {
        // Returns a vector of renders

        return &mRenderers;

    }

    //-----------------------------------------------------------------------
    void Root::setRenderSystem(RenderSystem* system)
    {
        // Sets the active rendering system
        // Can be called direct or will be called by
        //   standard config dialog

        // Is there already an active renderer?
        // If so, disable it and init the new one
        if( mActiveRenderer && mActiveRenderer != system )
        {
            mActiveRenderer->shutdown();
        }

        mActiveRenderer = system;
        // Tell scene managers
        SceneManagerEnumerator::getSingleton().setRenderSystem(system);

    }
    //-----------------------------------------------------------------------
    void Root::addRenderSystem(RenderSystem *newRend)
    {
        mRenderers.push_back(newRend);
    }
    //-----------------------------------------------------------------------
    void Root::setSceneManager(SceneType sType, SceneManager *sm)
    {
        SceneManagerEnumerator::getSingleton().setSceneManager(sType, sm);
    }

    //-----------------------------------------------------------------------
    RenderSystem* Root::getRenderSystem(void)
    {
        // Gets the currently active renderer
        return mActiveRenderer;

    }

    //-----------------------------------------------------------------------
    RenderWindow* Root::initialise(bool autoCreateWindow, const String& windowTitle)
    {
        if (!mActiveRenderer)
            Except(Exception::ERR_NO_RENDERSYSTEM_SELECTED,
            "Cannot initialise - no render "
            "system has been selected.", "Root::initialise");

        mControllerManager = new ControllerManager();

        mAutoWindow =  mActiveRenderer->initialise(autoCreateWindow, windowTitle);

        if (autoCreateWindow)
        {
            oneTimePostWindowInit();
        }

        // Initialise timer
        mTimer->reset();
        return mAutoWindow;

    }
    //-----------------------------------------------------------------------
    String Root::getErrorDescription(long errorNumber)
    {

        // Pass to render system
        if (mActiveRenderer)
            return mActiveRenderer->getErrorDescription(errorNumber);
        else
            return "";

    }
    //-----------------------------------------------------------------------
    SceneManager* Root::getSceneManager(SceneType sceneType)
    {
        // Delegate
        return mSceneManagerEnum->getSceneManager(sceneType);
    }
    //-----------------------------------------------------------------------
    TextureManager* Root::getTextureManager(void)
    {
        return &TextureManager::getSingleton();
    }
    //-----------------------------------------------------------------------
    MeshManager* Root::getMeshManager(void)
    {
        return &MeshManager::getSingleton();
    }
    //-----------------------------------------------------------------------
    void Root::addFrameListener(FrameListener* newListener)
    {
        // Insert, unique only (set)
        mFrameListeners.insert(newListener);

    }

    //-----------------------------------------------------------------------
    void Root::removeFrameListener(FrameListener* oldListener)
    {
        // Remove, 1 only (set)
        mFrameListeners.erase(oldListener);
    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameStarted(FrameEvent& evt)
    {
        // Increment frame number
        ++mCurrentFrame;

        // Tell all listeners
        std::set<FrameListener*>::iterator i;
        for (i= mFrameListeners.begin(); i != mFrameListeners.end(); ++i)
        {
            if (!(*i)->frameStarted(evt))
                return false;
        }

        return true;

    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameEnded(FrameEvent& evt)
    {
        // Tell all listeners
        std::set<FrameListener*>::iterator i;
        for (i= mFrameListeners.begin(); i != mFrameListeners.end(); ++i)
        {
            if (!(*i)->frameEnded(evt))
                return false;
        }

        // Tell buffer manager to free temp buffers used this fram
        HardwareBufferManager::getSingleton()._releaseBufferCopies();

        return true;
    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameStarted()
    {
        unsigned long now = mTimer->getMilliseconds();
        FrameEvent evt;
        evt.timeSinceLastEvent = calculateEventTime(now, FETT_ANY);
        evt.timeSinceLastFrame = calculateEventTime(now, FETT_STARTED);

        return _fireFrameStarted(evt);
    }
    //-----------------------------------------------------------------------
    bool Root::_fireFrameEnded()
    {
        unsigned long now = mTimer->getMilliseconds();
        FrameEvent evt;
        evt.timeSinceLastEvent = calculateEventTime(now, FETT_ANY);
        evt.timeSinceLastFrame = calculateEventTime(now, FETT_ENDED);

        return _fireFrameEnded(evt);
    }
    //-----------------------------------------------------------------------
    Real Root::calculateEventTime(unsigned long now, FrameEventTimeType type)
    {
        // Calculate the average time passed between events of the given type
        // during the last 0.1 seconds.

        std::deque<unsigned long>& times = mEventTimes[type];
        times.push_back(now);

        if(times.size() == 1)
            return 0;

        // Times up to 0.1 seconds old should be kept
        unsigned long discardLimit = now - 100;

        // Find the oldest time to keep
        std::deque<unsigned long>::iterator it = times.begin(),
            end = times.end()-2; // We need at least two times
        while(it != end)
        {
            if(*it < discardLimit)
                ++it;
            else
                break;
        }

        // Remove old times
        times.erase(times.begin(), it);

        return Real(times.back() - times.front()) / ((times.size()-1) * 1000);
    }
    //-----------------------------------------------------------------------
    void Root::queueEndRendering(void)
    {
	    mQueuedEnd = true;
    }	
    //-----------------------------------------------------------------------
    void Root::startRendering(void)
    {
        assert(mActiveRenderer != 0);

        mActiveRenderer->_initRenderTargets();

        // Clear event times
        for(int i=0; i!=3; ++i)
            mEventTimes[i].clear();

        // Infinite loop, until broken out of by frame listeners
		// or break out by calling queueEndRendering()
		mQueuedEnd = false;
		while( !mQueuedEnd )
		{
#if OGRE_PLATFORM == PLATFORM_WIN32
            // Pump events on Win32
    		MSG  msg;
			while( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
#endif

            if (!renderOneFrame())
                break;
		}


    }
    //-----------------------------------------------------------------------
    bool Root::renderOneFrame(void)
    {
        if(!_fireFrameStarted())
            return false;

        mActiveRenderer->_updateAllRenderTargets();

        return _fireFrameEnded();
    }
    //-----------------------------------------------------------------------
    void Root::shutdown(void)
    {
        SceneManagerEnumerator::getSingleton().shutdownAll();

        
        if (mActiveRenderer)
        {
            mActiveRenderer->shutdown();
            mActiveRenderer = 0;
        }

        mLogManager->logMessage("*-*-* OGRE Shutdown");


    }
    //-----------------------------------------------------------------------
    Root& Root::getSingleton(void)
    {
        return Singleton<Root>::getSingleton();
    }
    //-----------------------------------------------------------------------
    void Root::loadPlugins( const String& pluginsfile )
    {
        StringVector pluginList;
        String pluginDir;
        ConfigFile cfg;

		try {
        	cfg.load( pluginsfile );
		}
		catch (Exception)
		{
			LogManager::getSingleton().logMessage(pluginsfile + " not found, automatic plugin loading disabled.");
			return;
		}

        pluginDir = cfg.getSetting("PluginFolder"); // Ignored on Mac OS X, uses Resources/ directory
        pluginList = cfg.getMultiSetting("Plugin");

        char last_char = pluginDir[pluginDir.length()-1];
        if (last_char != '/' || last_char != '\\')
        {
#if OGRE_PLATFORM == PLATFORM_WIN32
            pluginDir += "\\";
#elif OGRE_PLATFORM == PLATFORM_LINUX
            pluginDir += "/";
#endif
        }

        for( StringVector::iterator it = pluginList.begin(); it != pluginList.end(); ++it )
        {
			loadPlugin(pluginDir + (*it));
        }

    }
    //-----------------------------------------------------------------------
    void Root::unloadPlugins(void)
    {
        std::vector<DynLib*>::reverse_iterator i;

        // NB Unload plugins in reverse order to enforce dependencies
        for (i = mPluginLibs.rbegin(); i != mPluginLibs.rend(); ++i)
        {
            // Call plugin shutdown
            DLL_STOP_PLUGIN pFunc = (DLL_STOP_PLUGIN)(*i)->getSymbol("dllStopPlugin");
            pFunc();
            // Unload library & destroy
            DynLibManager::getSingleton().unload((Resource*)*i);
            delete *i;

        }

        mPluginLibs.clear();

    }
    //-----------------------------------------------------------------------
    void Root::addResourceLocation(const String& name, const String& locType, ResourceType resType)
    {
        switch(resType)
        {
        case RESTYPE_ALL:
            ResourceManager::addCommonArchiveEx(name, locType);
            break;
        case RESTYPE_TEXTURES:
            TextureManager::getSingleton().addArchiveEx(name, locType);
            break;
        case RESTYPE_MODELS:
            MeshManager::getSingleton().addArchiveEx(name, locType);
            break;

        }
    }
    //-----------------------------------------------------------------------
    void Root::convertColourValue(const ColourValue& colour, unsigned long* pDest)
    {
        assert(mActiveRenderer != 0);
        mActiveRenderer->convertColourValue(colour, pDest);
    }
    //-----------------------------------------------------------------------
    RenderWindow* Root::getAutoCreatedWindow(void)
    {
        return mAutoWindow;
    }
    //-----------------------------------------------------------------------
    RenderWindow* Root::createRenderWindow(const String &name, unsigned int width, unsigned int height, unsigned int colourDepth,
                bool fullScreen, int left, int top, bool depthBuffer,RenderWindow* parentWindowHandle)
    {
        if (!mActiveRenderer)
        {
            Except(Exception::ERR_NO_RENDERSYSTEM_SELECTED,
            "Cannot create window - no render "
            "system has been selected.", "Root::createRenderWindow");
        }
        RenderWindow* ret;
        ret = mActiveRenderer->createRenderWindow(name, width, height, colourDepth, fullScreen, left, top,
            depthBuffer, parentWindowHandle);

        // Initialisation for classes dependent on first window created
        oneTimePostWindowInit();

        return ret;

    }
    //-----------------------------------------------------------------------
    void Root::detachRenderTarget(RenderTarget* target)
    {
        if (!mActiveRenderer)
        {
            Except(Exception::ERR_NO_RENDERSYSTEM_SELECTED,
            "Cannot create window - no render "
            "system has been selected.", "Root::destroyRenderWindow");
        }

        mActiveRenderer->detachRenderTarget( target->getName() );
    }
    //-----------------------------------------------------------------------
    void Root::detachRenderTarget(const String &name)
    {
        if (!mActiveRenderer)
        {
            Except(Exception::ERR_NO_RENDERSYSTEM_SELECTED,
            "Cannot create window - no render "
            "system has been selected.", "Root::destroyRenderWindow");
        }

        mActiveRenderer->detachRenderTarget( name );
    }
    //-----------------------------------------------------------------------
    RenderTarget* Root::getRenderTarget(const String &name)
    {
        if (!mActiveRenderer)
        {
            Except(Exception::ERR_NO_RENDERSYSTEM_SELECTED,
            "Cannot create window - no render "
            "system has been selected.", "Root::getRenderWindow");
        }

        return mActiveRenderer->getRenderTarget(name);
    }
    //-----------------------------------------------------------------------
    /*
    void Root::showDebugOverlay(bool show)
    {
        Overlay* o = (Overlay*)OverlayManager::getSingleton().getByName("Core/DebugOverlay");
		if (!o)
			Except( Exception::ERR_ITEM_NOT_FOUND, "Could not find overlay Core/DebugOverlay",
				"Root::showDebugOverlay" );
        if (show)
        {
            o->show();
        }
        else
        {
            o->hide();
        }
        
    }
    */
    //-----------------------------------------------------------------------
	void Root::loadPlugin(const String& pluginName)
	{
		// Load plugin library
        DynLib* lib = DynLibManager::getSingleton().load( pluginName );
		// Store for later unload
		mPluginLibs.push_back(lib);

		// Call startup function
		DLL_START_PLUGIN pFunc = (DLL_START_PLUGIN)lib->getSymbol("dllStartPlugin");

		if (!pFunc)
			Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot find symbol dllStartPlugin in library " + pluginName,
				"Root::loadPlugins");
		pFunc();
	}
    //-----------------------------------------------------------------------
	void Root::unloadPlugin(const String& pluginName)
	{
        std::vector<DynLib*>::iterator i;

        for (i = mPluginLibs.begin(); i != mPluginLibs.end(); ++i)
        {
			if ((*i)->getName() == pluginName)
			{
				// Call plugin shutdown
				DLL_STOP_PLUGIN pFunc = (DLL_STOP_PLUGIN)(*i)->getSymbol("dllStopPlugin");
				pFunc();
				// Unload library & destroy
				DynLibManager::getSingleton().unload((Resource*)*i);
				delete *i;
				mPluginLibs.erase(i);
				return;
			}

        }
	}
    //-----------------------------------------------------------------------
    Timer* Root::getTimer(void)
    {
        return mTimer;
    }
    //-----------------------------------------------------------------------
    void Root::oneTimePostWindowInit(void)
    {
        static bool firsttime = true;
        if (firsttime)
        {
			// Initialise material manager
			mMaterialManager->initialise();
            // Init particle systems manager
            mParticleManager->_initialise();
            // parse all font scripts
            mFontManager->parseAllSources();
            // init overlays
            mOverlayManager->parseAllSources();
			// Init mesh manager
			MeshManager::getSingleton()._initialise();
        }

        firsttime = false;
    }
}
