/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __ROOT__
#define __ROOT__

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreSingleton.h"
#include "OgreString.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreMeshManager.h"
#include "OgreTextureManager.h"
#include "OgreDynLibManager.h"
#include "OgrePlatformManager.h"
#include "OgreArchiveManager.h"

#include <exception>

#if OGRE_COMPILER == COMPILER_MSVC || OGRE_COMPILER == COMPILER_BORL
#   define SET_TERM_HANDLER { set_terminate( &Ogre::Root::termHandler ); }
#else
#   define SET_TERM_HANDLER { std::set_terminate( &Ogre::Root::termHandler ); }
#endif

namespace Ogre
{
    typedef std::vector<RenderSystem*> RenderSystemList;

    /** The root class of the Ogre system.
        @remarks
            The Ogre::Root class represents a starting point for the client
            application. From here, the application can gain access to the
            fundamentals of the system, namely the rendering systems
            available, management of saved configurations, logging, and
            access to other classes in the system. Acts as a hub from which
            all other objects may be reached. An instance of Root must be
            created before any other Ogre operations are called. Once an
            instance has been created, the same instance is accessible
            throughout the life of that object by using Root::getSingleton
            (as a reference) or Root::getSingletonPtr (as a pointer).
    */
    class _OgreExport Root : public Singleton<Root>
    {
        // To allow update of active renderer if
        // RenderSystem::initialise is used directly
        friend class RenderSystem;
    private:
        RenderSystemList mRenderers;
        RenderSystem* mActiveRenderer;
        String mVersion;
				String mConfigFileName;

        // Singletons
        LogManager* mLogManager;
        ControllerManager* mControllerManager;
        SceneManagerEnumerator* mSceneManagerEnum;
        DynLibManager* mDynLibManager;
        PlatformManager* mPlatformManager;
        ArchiveManager* mArchiveManager;
        MaterialManager* mMaterialManager;
        MeshManager* mMeshManager;
        ParticleSystemManager* mParticleManager;
        SkeletonManager* mSkeletonManager;
        GuiManager* mGuiManager;
        OverlayManager* mOverlayManager;
        FontManager* mFontManager;
        ArchiveFactory *mZipArchiveFactory;
        Codec* mPNGCodec, *mJPGCodec, *mJPEGCodec, *mTGACodec, *mDDSCodec, *mBMPCodec;
        Timer* mTimer;
        RenderWindow* mAutoWindow;
        Profiler* mProfiler;
        HighLevelGpuProgramManager* mHighLevelGpuProgramManager;

        std::vector<DynLib*> mPluginLibs;
        /** Method reads a plugins configuration file and instantiates all
            plugins.
            @param
                pluginsfile The file that contains plugins information.
                Defaults to "plugins.cfg".
        */
        void loadPlugins( const String& pluginsfile = "plugins.cfg" );
        /** Unloads all loaded plugins.
        */
        void unloadPlugins();

        // Internal method for one-time tasks after first window creation
        void oneTimePostWindowInit(void);

        /** Set of registered frame listeners */
        std::set<FrameListener*> mFrameListeners;


        /** Indicates the type of event to be considered by calculateEventTime(). */
        enum FrameEventTimeType {
            FETT_ANY, FETT_STARTED, FETT_ENDED
        };

        /// Contains the times of recently fired events
        std::deque<unsigned long> mEventTimes[3];

        /** Internal method for calculating the average time between recently fired events.
        @param now The current time in ms.
        @param type The type of event to be considered.
        */
        Real calculateEventTime(unsigned long now, FrameEventTimeType type);
    public:

        static void termHandler();

        /** Constructor
            @param
                pluginFileName The file that contains plugins information.
                Defaults to "plugins.cfg".
		*/
        Root(const String& pluginFileName = "plugins.cfg", const String& configFileName = "ogre.cfg", const String& logFileName = "Ogre.log");
        ~Root();

        /** Saves the details of the current configuration
            @remarks
                Stores details of the current configuration so it may be
                restored later on.
        */
        void saveConfig(void);

        /** Checks for saved video/sound/etc settings
            @remarks
                This method checks to see if there is a valid saved configuration
                from a previous run. If there is, the state of the system will
                be restored to that configuration.

            @returns
                If a valid configuration was found, <b>true</b> is returned.
            @par
                If there is no saved configuration, or if the system failed
                with the last config settings, <b>false</b> is returned.
        */
        bool restoreConfig(void);

        /** Displays a dialog asking the user to choose system settings.
            @remarks
                This method displays the default dialog allowing the user to
                choose the renderering system, video mode etc. If there is are
                any settings saved already, they will be restored automatically
                before displaying the dialogue. When the user accepts a group of
                settings, this will automatically call Root::setRenderSystem,
                RenderSystem::setConfigOption and Root::saveConfig with the
                user's choices. This is the easiest way to get the system
                configured.
            @returns
                If the user clicked 'Ok', <b>true</b> is returned.
            @par
                If they clicked 'Cancel' (in which case the app should
                strongly consider terminating), <b>false</b> is returned.
         */
        bool showConfigDialog(void);

        /** Adds a new rendering subsystem to the list of available renderers.
            @remarks
                Intended for use by advanced users and plugin writers only!
                Calling this method with a pointer to a valid RenderSystem
                (sublcass) adds a rendering API implementation to the list of
                available ones. Typical examples would be an OpenGL
                implementation and a Direct3D implementation.
            @note
                <br>This should usually be called from the dllStartPlugin()
                function of an extension plug-in.
        */
        void addRenderSystem(RenderSystem* newRend);

        /** Sets the passed in SceneManager to be the one responsible for
            the indicated type of scene.
            @remarks
                This method is provided for application writers and plugin
                authors to use to attach their SceneManager subclasses to the
                engine. See the SceneManager class for more information.
        */
        void setSceneManager(SceneType sType, SceneManager* sm);

        /** Retrieve a list of the available render systems.
            @remarks
                Retrieves a pointer to the list of available renderers as a
                list of RenderSystem subclasses. Can be used to build a
                custom settings dialog.
        */
        RenderSystemList* getAvailableRenderers(void);

        /** Sets the rendering subsystem to be used.
            @remarks
                This method indicates to OGRE which rendering system is to be
                used (e.g. Direct3D, OpenGL etc). This is called
                automatically by the default config dialog, and when settings
                are restored from a previous configuraion. If used manually
                it could be used to set the renderer from a custom settings
                dialog. Once this has been done, the renderer can be
                initialised using Root::initialise.
            @par
                This method is also called by render systems if they are
                initialised directly.
            @param
                system Pointer to the render system to use.
            @see
                RenderSystem
        */
        void setRenderSystem(RenderSystem* system);

        /** Retrieve a pointer to the currently selected render system.
        */
        RenderSystem* getRenderSystem(void);

        /** Initialises the renderer.
            @remarks
                This method can only be called after a renderer has been
                selected with Root::setRenderSystem, and it will initialise
                the selected rendering system ready for use.
            @param
                autoCreateWindow If true, a rendering window will
                automatically be created (saving a call to
                RenderSystem::createRenderWindow). The window will be
                created based on the options currently set on the render
                system.
            @returns
                A pointer to the automatically created window, if
                requested, otherwise <b>NULL</b>.
        */
	    RenderWindow* initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render Window");

        /** Gets a reference to a SceneManager object.
            @remarks
                The SceneManager class (and any subclasses) is a key class
                which controls the contents of the scene, and is responsible
                for issuing rendering commands to the RenderSystem to draw
                it. The SceneManager is the class which an application using
                Ogre will interact with most, since controlling the contents
                of the scene is the most frequent action of an application.
            @par
                As described in the SceneManager documentation, different
                subclasses can be specialised for rendering particular types
                of scene e.g. landscapes or indoor enviroments.
            @note
                <br>This function delegates it's implementation to the
                SceneManagerEnumerator class. This class can be customised to
                include new SceneType entries and to create new subclasses of
                SceneManager if they are introduced. This is done because the
                customisation of the Root class is strongly discouraged and
                in the future it may be locked down.
            @param
                sceneType A value from the SceneType enumeration. The method
                will return a SceneManager which is most appropriate for this
                type of scene.
            @see
                SceneManager, SceneManagerEnumerator
        */
        SceneManager* getSceneManager(SceneType sceneType);

        /** Retrieves a reference to the current TextureManager.
            @remarks
                This performs the same function as
                TextureManager::getSingleton, but is provided for convenience
                particularly to scripting engines.
            @par
                Note that a TextureManager will NOT be available until the
                Ogre system has been initialised by selecting a RenderSystem,
                calling Root::initialise and a window having been created
                (this may have been done by initialise if required). This is
                because the exact runtime subclass which will be implementing
                the calls will differ depending on the rendering engine
                selected, and these typically require a window upon which to
                base texture format decisions.
        */
        TextureManager* getTextureManager(void);

        /** Retrieves a reference to the current MeshManager.
            @remarks
                This performs the same function as MeshManager::getSingleton
                and is provided for convenience to scripting engines.
        */
        MeshManager* getMeshManager(void);

        /** Utility function for getting a better description of an error
            code.
        */
        String getErrorDescription(long errorNumber);

        /** Registers a FrameListener which will be called back every frame.
            @remarks
                A FrameListener is a class which implements methods which
                will be called every frame.
            @par
                See the FrameListener class for more details on the specifics
                It is imperitive that the instance passed to this method is
                not destroyed before iether the rendering loop ends, or the
                class is removed from the listening list using
                removeFrameListener.
            @note
                <br>This method can only be called after Root::initialise has
                been called.
            @see
                FrameListener, Root::removeFrameListener
        */
        void addFrameListener(FrameListener* newListener);

        /** Removes a FrameListener from the list of listening classes.
            @see
                FrameListener, Root::addFrameListener
        */
        void removeFrameListener(FrameListener* oldListener);

        /** Starts / restarts the automatic rendering cycle.
            @remarks
                This method begins the automatic rendering of the scene. It
                will <b>NOT</b> return until the rendering cycle is halted.
            @par
                During rendering, any FrameListener classes registered using
                addFrameListener will be called back for each frame that is
                to be rendered, These classes can tell OGRE to halt the
                rendering if required, which will cause this method to
                return.
            @note
                <br>Users of the OGRE library do not have to use this
                automatic rendering loop. It is there as a convenience and is
                most useful for high frame rate applications e.g. games. For
                applications that don't need to constantly refresh the
                rendering targets (e.g. an editor utility), it is better to
                manually refresh each render target only when required by
                calling RenderTarget::update.
            @note
                This frees up the CPU to do other things in between
                refreshes, since in this case frame rate is less important.
            @note
                This method can only be called after Root::initialise has
                been called.
        */
        void startRendering(void);

        /** Shuts down the system manually.
            @remarks
                This is normally done by Ogre automatically so don't think
                you have to call this yourself. However this is here for
                convenience, especially for dealing with unexpected errors or
                for systems which need to shut down Ogre on demand.
        */
        void shutdown(void);
        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton
                implementation is in a .h file, which means it gets compiled
                into anybody who includes it. This is needed for the
                Singleton template to work, but we actually only want it
                compiled into the implementation of the class based on the
                Singleton, not all of them. If we don't change this, we get
                link errors when trying to use the Singleton-based class from
                an outside dll.
            @par
                This method just delegates to the template version anyway,
                but the implementation stays in this single compilation unit,
                preventing link errors.
        */
        static Root& getSingleton(void);

        /** Adds a location to the list of searchable locations for a
            Resource type.
            @remarks
                Resource files (textures, models etc) can be loaded from
                locations other than the current application folder. By
                calling this method, you add another search location to the
                list. Locations added first are preferred over locations
                added later, with the current application folder always being
                the most preferred (and already set up for you).
            @par
                Locations can be folders, compressed archives, even perhaps
                remote locations. Facilities for loading from different
                locations are provided by plugins which provide
                implementations of the ArchiveEx class.
            @par
                All the application user has to do is specify a 'loctype'
                string in order to indicate the type of location, which
                should map onto one of the provided plugins. Ogre comes
                configured with the 'FileSystem' (folders) and 'Zip' (archive
                compressed with the pkzip / WinZip etc utilities) types.
            @note
                <br>Because of dependencies on certain initialisation tasks,
                you should not call this method for any Resource type other
                than 'RESTYPE_ALL' unless you have already called the
                'Root::initialise' method.
            @param
                name The name of the location, e.g. './data' or
                '/compressed/gamedata.zip'
            @param
                locType A string identifying the location type, e.g.
                'FileSystem' (for folders), 'Zip' etc. Must map to a
                registered plugin which deals with this type (FileSystem and
                Zip should always be available)
            @param
                resType Type of resource which will be searched for in this
                location. Defaults to all resources, but you can specify that
                textures are loaded from one location, models from another
                etc. if you like.
            @see
                ArchiveEx
        */
        void addResourceLocation(const String& name, const String& locType, ResourceType resType = RESTYPE_ALL);

        /** Generates a packed data version of the passed in ColourValue suitable for
            use with the current RenderSystem.
        @remarks
            Since different render systems have different colour data formats (eg
            RGBA for GL, ARGB for D3D) this method allows you to use 1 method for all.
        @param colour The colour to convert
        @param pDest Pointer to location to put the result.
        */
        void convertColourValue(const ColourValue& colour, unsigned long* pDest);

        /** Retrieves a pointer to the window that was created automatically
            @remarks
                When Root is initialised an optional window is created. This
                method retreives a pointer to that window.
            @note
                returns a null pointer when Root has not been initialised with
                the option of creating a window.
        */
        RenderWindow* getAutoCreatedWindow(void);

        /** Creates a new rendering window.
            @remarks
                This method creates a new rendering window as specified
                by the paramteters. The rendering system could be
                responible for only a single window (e.g. in the case
                of a game), or could be in charge of multiple ones (in the
                case of a level editor). The option to create the window
                as a child of another is therefore given.
                This method will create an appropriate subclass of
                RenderWindow depending on the API and platform implementation.
            @param
                name The name of the window. Used in other methods
                later like setRenderTarget and getRenderWindow.
            @param
                width The width of the new window.
            @param
                height The height of the new window.
            @param
                colourDepth The colour depth in bits per pixel.
                Only applicable if fullScreen = true
            @param
                fullScreen Specify true to make the window full screen
                without borders, title bar or menu bar.
            @param
                left The x position of the new window. Only applicable
                if fullScreen is false. Units are relative to the parent window
                if applicable, otherwise they are in screen coordinates.
            @param
                top The y position of the new window.
            @param
                depthBuffer If true, a depth buffer will be assigned to this window.
            @param
                parentWindowHandle Should be null if this window is to be
                stand-alone. Otherwise, specify a pointer to a RenderWindow
                which represents the parent window.
        */
        RenderWindow* createRenderWindow(const String &name, unsigned int width, unsigned int height, unsigned int colourDepth,
            bool fullScreen, int left = 0, int top = 0, bool depthBuffer = true,
            RenderWindow* parentWindowHandle = 0);

        /** Destroys a rendering window.
        */
        void detachRenderTarget( RenderTarget* pWin );

        /** Destroys a named rendering window.
        */
        void detachRenderTarget( const String & name );

        /** Retrieves a pointer to the a named render window.
        */
        RenderTarget * getRenderTarget(const String &name);

        /** Sets whether or not the debug overlay is shown.
        @remarks
            The debug overlay displays frame rate stats and various other debug
            information. You can enable it or disable it using this method.
            Alternatively you could access the overlay directly using mSceneManager::getOverlay
            but this is simpler.
        void showDebugOverlay(bool show);
        */

		/** Manually load a plugin.
		 @remarks
		 	Plugins are loaded at startup using the plugin configuration
			file specified when you create Root (default: plugins.cfg).
			This method allows you to load plugins in code.
		@param pluginName Name of the plugin library to load
		*/
		void loadPlugin(const String& pluginName);

		/** Manually unloads a plugin.
		 @remarks
		 	Plugins are unloaded at shutdown automatically.
			This method allows you to unload plugins in code, but
			make sure their dependencies are decoupled frist.
		@param pluginName Name of the plugin library to unload
		*/
		void unloadPlugin(const String& pluginName);

        /** Gets a pointer to the central timer used for all OGRE timings */
        Timer* getTimer(void);

        /** Method for raising frame started events. 
        @remarks
            This method is only for internal use when you use OGRE's inbuilt rendering
            loop (Root::startRendering). However, if you run your own rendering loop then
            you should call this method to ensure that FrameListener objects are notified
            of frame events; processes like texture animation and particle systems rely on 
            this.
        @note
            This method takes an event object as a parameter, so you can specify the times
            yourself. If you are happy for OGRE to automatically calculate the frame time
            for you, then call the other version of this method with no parameters.
        @param evt Event object which includes all the timing information which you have 
            calculated for yourself
        @returns False if one or more frame listeners elected that the rendering loop should
            be terminated, true otherwise.
        */
        bool _fireFrameStarted(FrameEvent& evt);
        /** Method for raising frame ended events. 
        @remarks
            This method is only for internal use when you use OGRE's inbuilt rendering
            loop (Root::startRendering). However, if you run your own rendering loop then
            you should call this method to ensure that FrameListener objects are notified
            of frame events; processes like texture animation and particle systems rely on 
            this.
        @note
            This method takes an event object as a parameter, so you can specify the times
            yourself. If you are happy for OGRE to automatically calculate the frame time
            for you, then call the other version of this method with no parameters.
        @param evt Event object which includes all the timing information which you have 
            calculated for yourself
        @returns False if one or more frame listeners elected that the rendering loop should
            be terminated, true otherwise.
        */
        bool _fireFrameEnded(FrameEvent& evt);
        /** Method for raising frame started events. 
        @remarks
            This method is only for internal use when you use OGRE's inbuilt rendering
            loop (Root::startRendering). However, if you run your own rendering loop then
            you should call this method to ensure that FrameListener objects are notified
            of frame events; processes like texture animation and particle systems rely on 
            this.
        @note
            This method calculates the frame timing information for you based on the elapsed
            time. If you want to specify elapsed times yourself you should call the other 
            version of this method which takes event details as a parameter.
        @returns False if one or more frame listeners elected that the rendering loop should
            be terminated, true otherwise.
        */
        bool _fireFrameStarted();
        /** Method for raising frame ended events. 
        @remarks
            This method is only for internal use when you use OGRE's inbuilt rendering
            loop (Root::startRendering). However, if you run your own rendering loop then
            you should call this method to ensure that FrameListener objects are notified
            of frame events; processes like texture animation and particle systems rely on 
            this.
        @note
            This method calculates the frame timing information for you based on the elapsed
            time. If you want to specify elapsed times yourself you should call the other 
            version of this method which takes event details as a parameter.
        @returns False if one or more frame listeners elected that the rendering loop should
            be terminated, true otherwise.
        */
        bool _fireFrameEnded();
    };
} // Namespace Ogre
#endif
