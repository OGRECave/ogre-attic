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

#include "OgrePNGCodec.h"
#include "OgreJPEGCodec.h"
#include "OgreTGACodec.h"

#include "OgreFontManager.h"

#include "OgreOverlay.h"

#if OGRE_PLATFORM == PLATFORM_WIN32

#   define WIN32_LEAN_AND_MEAN
#   include <direct.h>

#endif


namespace Ogre {    

    typedef void (*DLL_START_PLUGIN)(void);
    typedef void (*DLL_STOP_PLUGIN)(void);


    //-----------------------------------------------------------------------
    // Termination handler
    extern "C" _OgreExport void handleTerminate(void)
    {
        Root& r = Root::getSingleton();

        r.shutdown();

        ErrorDialog* dlg;

        dlg = PlatformManager::getSingleton().createErrorDialog();

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
    Root::Root()
    {
        // First create new exception handler
        SET_TERM_HANDLER;

        // superclass will do singleton checking
        String msg;

        // Init
        mActiveRenderer = 0;
        mVersion = "0.99e";

        // Create log manager and default log file
        mLogManager = new LogManager();
        mLogManager->createLog("Ogre.log", true, true);

        // Dynamic library manager
        mDynLibManager = new DynLibManager();

        mArchiveManager = new ArchiveManager();

        // Create SceneManager enumerator (note - will be managed by singleton)
        mSceneManagerEnum = new SceneManagerEnumerator();

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

        // Overlay manager
        mOverlayManager = new OverlayManager();
        // Gui Manager
        mGuiManager = new GuiManager();
        // Font manager
        mFontManager = new FontManager();

        mZipArchiveFactory = new ZipArchiveFactory();
        ArchiveManager::getSingleton().addArchiveFactory( mZipArchiveFactory );

        mPNGCodec = new PNGCodec;
        Codec::registerCodec( mPNGCodec );
        mJPEGCodec = new JPEGCodec;
        Codec::registerCodec( mJPEGCodec );
        mTGACodec = new TGACodec;
        Codec::registerCodec( mTGACodec );
        mJPGCodec = new JPGCodec;
        Codec::registerCodec( mJPGCodec );

        // Load plugins
        loadPlugins();        

        mLogManager->logMessage("*-*-* OGRE Initialising");
        msg = "*-*-* Version " + mVersion;
        mLogManager->logMessage(msg);

        // Create new Math object (will be managed by singleton)
        mMath = new Math();


        // Can't create controller manager until initialised
        mControllerManager = 0;

        // Always add the local folder as first resource search path for all resources
        addResourceLocation("./", "FileSystem");

        // Seed random number generator for future use
        srand((unsigned)time(0));

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

        


    }

    //-----------------------------------------------------------------------
    Root::~Root()
    {
        shutdown();
        delete mSceneManagerEnum;

        delete mTGACodec;
        delete mJPGCodec;
        delete mJPEGCodec;
        delete mPNGCodec;
        delete mZipArchiveFactory;
        delete mGuiManager;
        delete mFontManager;
        delete mArchiveManager;
        delete mSkeletonManager;
        delete mMeshManager;
        delete mMaterialManager;        
        delete mMath;
        delete mParticleManager;
        if( mControllerManager )
            delete mControllerManager;
        delete mPlatformManager;

        unloadPlugins();

        delete mDynLibManager;
        delete mLogManager;
    }

    //-----------------------------------------------------------------------
    void Root::saveConfig(void)
    {
        ::FILE *fp;
        char rec[100];

        fp = fopen("ogre.cfg", "w");
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
        ::FILE *fp;
        char rec[100], *ret;
        String optName, optVal;
        RenderSystemList::iterator pRend;

        fp = fopen("ogre.cfg", "r");
        if (!fp)
            // no config file
            return false;

        // Get render system
        ret = fgets(rec, 100, fp);
        // Tokenise on tab
        optName = strtok(rec, "\t");
        optVal = strtok(NULL, "\n");

        if (optName == "Render System")
        {
            pRend = getAvailableRenderers()->begin();
            while (pRend != getAvailableRenderers()->end())
            {
                String rName = (*pRend)->getName();
                if (rName == optVal)
                    break;
                pRend++;
            }

            if (pRend == getAvailableRenderers()->end())
            {
                // Unrecognised render system
                fclose(fp);
                return false;
            }

            setRenderSystem(*pRend);

            // Load render system options
            ret = fgets(rec, 100, fp);
            while (ret != NULL)
            {
                // Tokenise on tab
                optName = strtok(rec, "\t");
                optVal = strtok(NULL, "\n");
                mActiveRenderer->setConfigOption(optName, optVal);
                ret = fgets(rec, 100, fp);
            }
        }
        else
        {
            // Render system should be first entry - error
            fclose(fp);
            return false;

        }
        // Successful load
        fclose(fp);
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
    RenderWindow* Root::initialise(bool autoCreateWindow)
    {
        if (!mActiveRenderer)
            Except(Exception::ERR_NO_RENDERSYSTEM_SELECTED,
            "Cannot initialise - no render "
            "system has been selected.", "Root::initialise");

        mControllerManager = new ControllerManager();

        // Parse all material scripts
        mMaterialManager->parseAllSources();



        RenderWindow *retWin =  mActiveRenderer->initialise(autoCreateWindow);

        if (autoCreateWindow)
        {
            // Init particle systems manager
            mParticleManager->_initialise();
            // parse all font scripts
            mFontManager->parseAllSources();
            // parse all overlay scripts
            mOverlayManager->parseAllSources();
        }

        return retWin;

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
        assert(mActiveRenderer != 0);
        mActiveRenderer->addFrameListener(newListener);

    }

    //-----------------------------------------------------------------------
    void Root::removeFrameListener(FrameListener* oldListener)
    {
        assert(mActiveRenderer != 0);
        mActiveRenderer->removeFrameListener(oldListener);
    }
    //-----------------------------------------------------------------------
    void Root::startRendering(void)
    {
        assert(mActiveRenderer != 0);
        mActiveRenderer->startRendering();
    }
    //-----------------------------------------------------------------------
    void Root::shutdown(void)
    {
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
        DynLib* lib;
        cfg.load( pluginsfile );

        pluginDir = cfg.getSetting("PluginFolder");
        pluginList = cfg.getMultiSetting("Plugin");

        if (pluginDir == "")
        {
            char cwd[255];
            getcwd(cwd, 254);
            Except(Exception::ERR_FILE_NOT_FOUND, "Unable to determine plugins folder, plugins.cfg not found in " + String(cwd),
                "Root::loadPlugins");
        }

        char szBuffer[260];
        getcwd( szBuffer, 259 );
        chdir( pluginDir.c_str() );

        for( StringVector::iterator it = pluginList.begin(); it != pluginList.end(); ++it )
        {
            // Load plugin library
            lib = DynLibManager::getSingleton().load( *it );
            // Store for later unload
            mPluginLibs.push_back(lib);

            // Call startup function
            DLL_START_PLUGIN pFunc = (DLL_START_PLUGIN)lib->getSymbol("dllStartPlugin");

            if (!pFunc)
                Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot find symbol dllStartPlugin in library " + *it,
                    "Root::loadPlugins");
            pFunc();
        }

        chdir( szBuffer );
    }
    //-----------------------------------------------------------------------
    void Root::unloadPlugins(void)
    {
        std::vector<DynLib*>::iterator i;

        for (i = mPluginLibs.begin(); i != mPluginLibs.end(); ++i)
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
    RenderWindow* Root::createRenderWindow(const String &name, int width, int height, int colourDepth,
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
        static bool firstOne = true;
        if (firstOne)
        {
            // Init particle systems manager
            mParticleManager->_initialise();
            // parse all font scripts
            mFontManager->parseAllSources();
            // init overlays
            mOverlayManager->parseAllSources();
            firstOne = false;
        }

        return ret;

    }
    //-----------------------------------------------------------------------
    void Root::destroyRenderWindow(RenderWindow* pWin)
    {
        if (!mActiveRenderer)
        {
            Except(Exception::ERR_NO_RENDERSYSTEM_SELECTED,
            "Cannot create window - no render "
            "system has been selected.", "Root::destroyRenderWindow");
        }

        mActiveRenderer->destroyRenderWindow(pWin);
    }
    //-----------------------------------------------------------------------
    void Root::destroyRenderWindow(const String &name)
    {
        if (!mActiveRenderer)
        {
            Except(Exception::ERR_NO_RENDERSYSTEM_SELECTED,
            "Cannot create window - no render "
            "system has been selected.", "Root::destroyRenderWindow");
        }
        mActiveRenderer->destroyRenderWindow(name);
    }
    //-----------------------------------------------------------------------
    RenderWindow* Root::getRenderWindow(const String &name)
    {
        if (!mActiveRenderer)
        {
            Except(Exception::ERR_NO_RENDERSYSTEM_SELECTED,
            "Cannot create window - no render "
            "system has been selected.", "Root::getRenderWindow");
        }

        return mActiveRenderer->getRenderWindow(name);
    }
    //-----------------------------------------------------------------------
    void Root::showDebugOverlay(bool show)
    {
        Overlay* o = (Overlay*)OverlayManager::getSingleton().getByName("Core/DebugOverlay");
        if (show)
        {
            o->show();
        }
        else
        {
            o->hide();
        }

        
    }

}
