/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
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
#ifndef __RenderSystem_H_
#define __RenderSystem_H_

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreString.h"

#include "OgreMaterial.h"
#include "OgreCommon.h"

#include "OgreRenderOperation.h"
#include "OgreFrameListener.h"
#include "OgreConfigOptionMap.h"
#include "OgreRenderWindowMap.h"

namespace Ogre
{

    class TextureManager;
    enum TexCoordCalcMethod
    {
        /// No calculated texture coordinates
        TEXCALC_NONE,
        /// Environment map based on vertex normals
        TEXCALC_ENVIRONMENT_MAP,
        /// Environment map based on vertex positions
        TEXCALC_ENVIRONMENT_MAP_PLANAR
    };

    /** Defines the functionality of a 3D API
        @remarks
            The RenderSystem class provides a base interface
            which abstracts the general functionality of the 3D API
            e.g. Direct3D or OpenGL. Whilst a few of the general
            methods have implementations, most of this class is
            abstract, requiring a subclass based on a specific API
            to be constructed to provide the full functionality.
            Note there are 2 levels to the interface - one which
            will be used often by the caller of the Ogre library,
            and one which is at a lower level and will be used by the
            other classes provided by Ogre. These lower level
            methods are prefixed with '_' to differentiate them.
            The advanced user of the library may use these lower
            level methods to access the 3D API at a more fundamental
            level (dealing direct with render states and rendering
            primitives), but still benefitting from Ogre's abstraction
            of exactly which 3D API is in use.
        @author
            Steven Streeting
        @version
            1.0
     */
    class _OgreExport RenderSystem
    {
    public:
        /** Default Constructor.
        */
        RenderSystem();

        /** Destructor.
        */
        virtual ~RenderSystem();

        /** Returns the name of the rendering system.
        */
        virtual const String& getName(void) const = 0;

        /** Returns the details of this API's configuration options
            @remarks
                Each render system must be able to inform the world
                of what options must/can be specified for it's
                operation.
            @par
                These are passed as strings for portability, but
                grouped into a structure (_ConfigOption) which includes
                both options and current value.
            @par
                Note that the settings returned from this call are
                affected by the options that have been set so far,
                since some options are interdependent.
            @par
                This routine is called automatically by the default
                configuration dialogue produced by Root::showConfigDialog
                or may be used by the caller for custom settings dialogs
            @returns
                A 'map' of options, i.e. a list of options which is also
                indexed by option name.
         */
        virtual ConfigOptionMap& getConfigOptions(void) = 0;

        /** Sets an option for this API
            @remarks
                Used to confirm the settings (normally chosen by the user) in
                order to make the renderer able to initialise with the settings as required.
                This may be video mode, D3D driver, full screen / windowed etc.
                Called automatically by the default configuration
                dialog, and by the restoration of saved settings.
                These settings are stored and only activated when
                RenderSystem::initialise or RenderSystem::reinitialise
                are called.
            @par
                If using a custom configuration dialog, it is advised that the
                caller calls RenderSystem::getConfigOptions
                again, since some options can alter resulting from a selection.
            @param
                name The name of the option to alter.
            @param
                value The value to set the option to.
         */
        virtual void setConfigOption(const String &name, const String &value) = 0;

        /** Validates the options set for the rendering system, returning a message if there are problems.
            @note
                If the returned string is empty, there are no problems.
        */
        virtual String validateConfigOptions(void) = 0;

        /** Start up the renderer using the settings selected (Or the defaults if none have been selected).
            @remarks
                Called by Root::setRenderSystem. Shouldn't really be called
                directly, although  this can be done if the app wants to.
            @param
                autoCreateWindow If true, creates a render window
                automatically, based on settings chosen so far. This saves
                an extra call to RenderSystem::createRenderWindow
                for the main render window.
            @par
                If an application has more specific window requirements,
                however (e.g. a level design app), it should specify false
                for this parameter and do it manually.
            @returns
                A pointer to the automatically created window, if requested, otherwise null.
        */
        virtual RenderWindow* initialise(bool autoCreateWindow);

        /** Restart the renderer (normally following a change in settings).
        */
        virtual void reinitialise(void) = 0;

        /** Shutdown the renderer and cleanup resources.
        */
        virtual void shutdown(void);

        /** Registers a FrameListener which will be called back every frame.
            @remarks
                A FrameListener is a class which implements methods which
                will be called during Ogre's automatic rendering loop (started
                with RenderSystem::startRendering).
            @par
                See the FrameListener class for more details on the specifics.
                It is imperitive that the instance passed to this method is
                not destroyed before iether the rendering loop ends, or the
                class is removed from the listening list using removeFrameListener.
            @see
                FrameListener
        */
        virtual void addFrameListener(FrameListener* newListener);

        /** Removes a FrameListener from the list of listening classes.
        */
        virtual void removeFrameListener(FrameListener* oldListener);

        /** Starts / restarts the automatic rendering cycle.
            @remarks
                This method begins the automatic rendering of the scene.
                This method will NOT RETURN until the rendering
                cycle is halted.
            @par
                During rendering, any FrameListener classes registered using
                addFrameListener will be called back for each frame that is to be rendered,
                These classes can tell OGRE to halt the rendering if required,
                which will cause this method to return.
            @par
                Note - users of the OGRE library do not have to use this
                automatic rendering loop. It is there as a convenience and is most
                useful for high frame rate applications e.g. games. For applications that
                don't need to constantly refresh the rendering targets (e.g. an editor
                utility), it is better to manually refresh each render target only when
                required by calling RenderTarget::update.
            @par
                This frees up the CPU to do other things in between refreshes, since in
                this case frame rate is less important.
         */
        virtual void startRendering(void);

        /** Sets the colour & strength of the ambient (global directionless) light in the world.
        */
        virtual void setAmbientLight(float r, float g, float b) = 0;

        /** Sets the type of light shading required (default = Gouraud).
        */
        virtual void setShadingType(ShadeOptions so) = 0;

        /** Sets the type of texture filtering used when rendering
            @remarks
                This method sets the kind of texture filtering applied when rendering textures onto
                primitives. Filtering covers how the effects of minification and magnification are
                disguised by resampling.
            @param
                fo The type of filtering to apply. The options are described in
                TextureFilterOptions
         */
        virtual void setTextureFiltering(TextureFilterOptions fo) = 0;

        /** Sets whether or not dynamic lighting is enabled.
            @param
                enabled If true, dynamic lighting is performed on geometry with normals supplied, geometry without
                normals will not be displayed. If false, no lighting is applied and all geometry will be full brightness.
        */
        virtual void setLightingEnabled(bool enabled) = 0;

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
        virtual RenderWindow* createRenderWindow(const String &name, int width, int height, int colourDepth,
            bool fullScreen, int left = 0, int top = 0, bool depthBuffer = true,
            RenderWindow* parentWindowHandle = 0) = 0;

        /** Destroys a rendering window.
        */
        virtual void destroyRenderWindow(RenderWindow* pWin) = 0;

        /** Destroys a named rendering window.
        */
        virtual void destroyRenderWindow(const String &name);

        /** Retrieves a pointer to the a named render window.
        */
        virtual RenderWindow* getRenderWindow(const String &name);

        /** Returns a description of an error code.
        */
        virtual String getErrorDescription(long errorNumber) = 0;

        /** Defines whether or now fullscreen render windows wait for the vertical blank before flipping buffers.
            @remarks
                By default, all rendering windows wait for a vertical blank (when the CRT beam turns off briefly to move
                from the bottom right of the screen back to the top left) before flipping the screen buffers. This ensures
                that the image you see on the screen is steady. However it restricts the frame rate to the refresh rate of
                the monitor, and can slow the frame rate down. You can speed this up by not waiting for the blank, but
                this has the downside of introducing 'tearing' artefacts where part of the previous frame is still displayed
                as the buffers are switched. Speed vs quality, you choose.
            @note
                Has NO effect on windowed mode render targets. Only affects fullscreen mode.
            @param
                enabled If true, the system waits for vertical blanks - quality over speed. If false it doesn't - speed over quality.
        */
        void setWaitForVerticalBlank(bool enabled);

        /** Returns true if the system is synchronising frames with the monitor vertical blank.
        */
        bool getWaitForVerticalBlank(void);

        // ------------------------------------------------------------------------
        //                     Internal Rendering Access
        // All methods below here are normally only called by other OGRE classes
        // They can be called by library user if required
        // ------------------------------------------------------------------------


        /**
          Adds a light to the renderers list of active lights

          This method should not be called directly by user
          processes - this is adding a light at the rendering
          level. User processes should add lights using the
          SceneNode attachLight method
         */
        virtual void _addLight(Light *lt) = 0;
        /**
          Removes a light from the renderers list.

          As with RenderSystem::_addLight
          this method is for use internally, not by user processes.
          See SceneNode for user-level light maintenance.
         */
        virtual void _removeLight(Light *lt) = 0;
        /** Modifies a light in the renderer.
            Modifies a light which has already been added using _addLight.
        */
        virtual void _modifyLight(Light* lt) = 0;
        /**
          Clears all the lights from the renderer

          As with RenderSystem::_addLight
          this method is for use internally, not by user processes.
          See SceneManager for user-level light maintenance.
         */
        virtual void _removeAllLights(void) = 0;

        /**
          Saves the current rendering state

          Stores the current rendering state on the
          render state stack. The state may then be altered
          and restored back to it's previous state using
          RenderSystem::_popRenderState. Used internally by Ogre
          to manage changes like model/view matrices, active
          materials/textures without having to repecify them
          every time.
         */
        virtual void _pushRenderState(void) = 0;
        /** Restores the render state to a previous state. */
        virtual void _popRenderState(void) = 0;
        /** Sets the world transform matrix. */
        virtual void _setWorldMatrix(const Matrix4 &m) = 0;
        /** Sets multiple world matrices (vertex blending). */
        virtual void _setWorldMatrices(const Matrix4* m, unsigned short count);
        /** Sets the view transform matrix */
        virtual void _setViewMatrix(const Matrix4 &m) = 0;
        /** Sets the projection transform matrix */
        virtual void _setProjectionMatrix(const Matrix4 &m) = 0;
        /** Utility function for setting all the properties of a texture unit at once.
            This method is also worth using over the individual texture unit settings because it
            only sets those settings which are different from the current settings for this
            unit, thus minimising render state changes.
        */
        virtual void _setTextureUnitSettings(int texUnit, Material::TextureLayer& tl);
        /** Turns off a texture unit. */
        virtual void _disableTextureUnit(int texUnit);
        /** Sets the surface properties to be used for future rendering.

            This method sets the the properties of the surfaces of objects
            to be rendered after it. In this context these surface properties
            are the amount of each type of light the object reflects (determining
            it's colour under different types of light), whether it emits light
            itself, and how shiny it is. Textures are not dealt with here,
            see the _setTetxure method for details.
            This method is used by _setMaterial so does not need to be called
            direct if that method is being used.

            @param ambient The amount of ambient (sourceless and directionless)
            light an object reflects. Affected by the colour/amount of ambient light in the scene.
            @param diffuse The amount of light from directed sources that is
            reflected (affected by colour/amount of point, directed and spot light sources)
            @param specular The amount of specular light reflected. This is also
            affected by directed light sources but represents the colour at the
            highlights of the object.
            @param emissive The colour of light emitted from the object. Note that
            this will make an object seem brighter and not dependent on lights in
            the scene, but it will not act as a light, so will not illuminate other
            objects. Use a light attached to the same SceneNode as the object for this purpose.
            @param shininess A value which only has an effect on specular highlights (so
            specular must be non-black). The higher this value, the smaller and crisper the
            specular highlights will be, imitating a more highly polished surface.
            This value is not constrained to 0.0-1.0, in fact it is likely to
            be more (10.0 gives a modest sheen to an object).

        */
        virtual void _setSurfaceParams(ColourValue &ambient,
            ColourValue &diffuse, ColourValue &specular,
            ColourValue &emissive, Real shininess) = 0;
        /**
          Sets the status of a single texture stage.

          Sets the details of a texture stage, to be used for all primitives
          rendered afterwards. User processes would
          not normally call this direct unless rendering
          primitives themselves - the SubEntity class
          is designed to manage materials for objects.
          Note that this method is called by _setMaterial.

          @param unit The index of the texture unit to modify. Multitexturing hardware
          can support multiple units (see _getNumTextureUnits)
          @param enabled Boolean to turn the unit on/off
          @param texname The name of the texture to use - this should have
              already been loaded with TextureManager::load.
         */
        virtual void _setTexture(int unit, bool enabled, const String &texname) = 0;
        /** Returns the number of texture units the current output hardware supports.

            For use in rendering, this determines how many texture units the
            are available for multitexturing (i.e. rendering multiple textures
            in a single pass). Where a Material has multiple texture layers,
            it will try to use multitexturing where available, and where
            it is not available, will perform multipass rendering to
            achieve the same effect.
        */
        virtual unsigned short _getNumTextureUnits(void) = 0;

        /**
          Sets the texture coordinate set to use for a texture unit.

          Meant for use internally - not generally used directly by apps - the Material and TextureLayer
          classes let you manage textures far more easily.

          @param unit Texture unit as above
          @param index The index of the texture coordinate set to use.
         */
        virtual void _setTextureCoordSet(int unit, int index) = 0;

        /**
          Sets a method for automatically calculating texture coordinates for a stage.
          Should not be used by apps - for use by Ogre only.
          @param unit Texture unit as above
          @param m Calculation method to use
         */
        virtual void _setTextureCoordCalculation(int unit, TexCoordCalcMethod m) = 0;

        /** Sets the texture blend modes from a TextureLayer record.
            Meant for use internally only - apps should use the Material
            and TextureLayer classes.
            @param unit Texture unit as above
            @param bm Details of the blending mode
        */
        virtual void _setTextureBlendMode(int unit, const LayerBlendModeEx& bm) = 0;

        /** Sets the texture addressing mode for a texture unit.*/
        virtual void _setTextureAddressingMode(int unit, Material::TextureLayer::TextureAddressingMode tam) = 0;

        /** Sets the texture coordinate transformation matrix for a texture unit.
            @param unit Texture unit to affect
            @param xform The 4x4 matrix
        */
        virtual void _setTextureMatrix(int unit, const Matrix4& xform) = 0;

        /** Sets the global blending factors for combining subsequent renders with the existing frame contents.
            The result of the blending operation is:</p>
            <p align="center">final = (texture * sourceFactor) + (pixel * destFactor)</p>
            Each of the factors is specified as one of a number of options, as specified in the SceneBlendFactor
            enumerated type.
            @param sourceFactor The source factor in the above calculation, i.e. multiplied by the texture colour components.
            @param destFactor The destination factor in the above calculation, i.e. multiplied by the pixel colour components.
        */
        virtual void _setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor) = 0;

        /** Sets the global alpha rejection approach for future renders.
            By default images are rendered regardless of texture alpha. This method lets you change that.
            @param func The comparison function which must pass for a pixel to be written.
            @param val The value to compare each pixels alpha value to (recommended 0 or 128 for compatibility)
        */
        virtual void _setAlphaRejectSettings(CompareFunction func, unsigned char value) = 0;
        /**
         * Signifies the beginning of a frame, ie the start of rendering on a single viewport. Will occur
         * several times per complete frame if multiple viewports exist.
         */
        virtual void _beginFrame(void) = 0;

        /**
          Render something to the active viewport.

          Low-level rendering interface to perform rendering
          operations. Unlikely to be used directly by client
          applications, since the SceneManager and various support
          classes will be responsible for calling this method.
          Can only be called between _beginScene and _endScene

          @param op A rendering operation instance, which contains
            details of the operation to be performed.
         */
        virtual void _render(RenderOperation& op);


        /**
         * Ends rendering of a frame to the current viewport.
         */
        virtual void _endFrame(void) = 0;
        /**
          Sets the provided viewport as the active one for future
          rendering operations. This viewport is aware of it's own
          camera and render target. Must be implemented by subclass.

          @param target Pointer to the appropriate viewport.
         */
        virtual void _setViewport(Viewport *vp) = 0;
        /** Get the current active viewport for rendering. */
        virtual Viewport* _getViewport(void);

        /** Sets the culling mode for the render system based on the 'vertex winding'.
            A typical way for the rendering engine to cull triangles is based on the
            'vertex winding' of triangles. Vertex winding refers to the direction in
            which the vertices are passed or indexed to in the rendering operation as viewed
            from the camera, and will wither be clockwise or anticlockwise (that's 'counterclockwise' for
            you Americans out there ;) The default is CULL_CLOCKWISE i.e. that only triangles whose vertices
            are passed/indexed in anticlockwise order are rendered - this is a common approach and is used in 3D studio models
            for example. You can alter this culling mode if you wish but it is not advised unless you know what you are doing.
            You may wish to use the CULL_NONE option for mesh data that you cull yourself where the vertex
            winding is uncertain.
        */
        virtual void _setCullingMode(CullingMode mode) = 0;

        virtual CullingMode _getCullingMode(void);

        /** Sets the mode of operation for depth buffer tests from this point onwards.
            Sometimes you may wish to alter the behaviour of the depth buffer to achieve
            special effects. Because it's unlikely that you'll set these options for an entire frame,
            but rather use them to tweak settings between rendering objects, this is an internal
            method (indicated by the '_' prefix) which will be used by a SceneManager implementation
            rather than directly from the client application.
            If this method is never called the settings are automatically the same as the default parameters.
            @param depthTest If true, the depth buffer is tested for each pixel and the frame buffer is only updated
                if the depth function test succeeds. If false, no test is performed and pixels are always written.
            @param depthWrite If true, the depth buffer is updated with the depth of the new pixel if the depth test succeeds.
                If false, the depth buffer is left unchanged even if a new pixel is written.
            @param depthFunction Sets the function required for the depth test.
        */
        virtual void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL) = 0;

        /** Sets whether or not the depth buffer check is performed before a pixel write.
            @param enabled If true, the depth buffer is tested for each pixel and the frame buffer is only updated
                if the depth function test succeeds. If false, no test is performed and pixels are always written.
        */
        virtual void _setDepthBufferCheckEnabled(bool enabled = true) = 0;
        /** Sets whether or not the depth buffer is updated after a pixel write.
            @param enabled If true, the depth buffer is updated with the depth of the new pixel if the depth test succeeds.
                If false, the depth buffer is left unchanged even if a new pixel is written.
        */
        virtual void _setDepthBufferWriteEnabled(bool enabled = true) = 0;
        /** Sets the comparison function for the depth buffer check.
            Advanced use only - allows you to choose the function applied to compare the depth values of
            new and existing pixels in the depth buffer. Only an issue if the deoth buffer check is enabled
            (see _setDepthBufferCheckEnabled)
            @param  func The comparison between the new depth and the existing depth which must return true
             for the new pixel to be written.
        */
        virtual void _setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL) = 0;
        /** Sets the fogging mode for future geometry.
            @param mode Set up the mode of fog as described in the FogMode enum, or set to FOG_NONE to turn off.
            @param colour The colour of the fog. Either set this to the same as your viewport background colour,
                or to blend in with a skydome or skybox.
            @param expDensity The density of the fog in FOG_EXP or FOG_EXP2 mode, as a value between 0 and 1. The default is 1. i.e. completely opaque, lower values can mean
                that fog never completely obscures the scene.
            @param linearStart Distance at which linear fog starts to encroach. The distance must be passed
                as a parametric value between 0 and 1, with 0 being the near clipping plane, and 1 being the far clipping plane. Only applicable if mode is FOG_LINEAR.
            @param linearEnd Distance at which linear fog becomes completely opaque.The distance must be passed
                as a parametric value between 0 and 1, with 0 being the near clipping plane, and 1 being the far clipping plane. Only applicable if mode is FOG_LINEAR.
        */
        virtual void _setFog(FogMode mode = FOG_NONE, ColourValue colour = ColourValue::White, Real expDensity = 1.0, Real linearStart = 0.0, Real linearEnd = 1.0) = 0;


        /** The RenderSystem will keep a count of tris rendered, this resets the count. */
        virtual void _beginGeometryCount(void);
        /** Reports the number of tris rendered since the last _beginGeometryCount call. */
        virtual unsigned int _getFaceCount(void);
        /** Reports the number of vertices passed to the renderer since the last _beginGeometryCount call. */
        virtual unsigned int _getVertexCount(void);

        /** Generates a packed data version of the passed in ColourValue suitable for
            use as with this RenderSystem.
        @remarks
            Since different render systems have different colour data formats (eg
            RGBA for GL, ARGB for D3D) this method allows you to use 1 method for all.
        @param colour The colour to convert
        @param pDest Pointer to location to put the result.
        */
        virtual void convertColourValue(const ColourValue& colour, unsigned long* pDest) = 0;

        /** Returns whether or not this RenderSystem supports hardware vertex blending, ie multiple
            world matrices per vertex with blending weights.
        */
        virtual bool _isVertexBlendSupported(void);

        /** Returns the number of matrices available to hardware vertex blending for this rendering system. */
        virtual unsigned short _getNumVertexBlendMatrices(void);

        /** Builds a perspective projection matrix suitable for this render system.
        @remarks
            Because different APIs have different requirements (some incompatible) for the
            projection matrix, this method allows each to implement their own correctly and pass
            back a generic OGRE matrix for storage in the engine.
        */
        virtual void _makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, Matrix4& dest) = 0;

    protected:

        /** Set of registered frame listeners */
        std::set<FrameListener*> mFrameListeners;

        /** Internal method for raising frame started events. */
        bool fireFrameStarted(FrameEvent& evt);
        /** Internal method for raising frame ended events. */
        bool fireFrameEnded(FrameEvent& evt);
        // Stored options
        ConfigOptionMap mOptions;


        // Available rendering targets
        RenderWindowMap mRenderWindows;

        // Texture manager
        // A concrete class of this will be created and
        // made available under the TextureManager singleton,
        // managed by the RenderSystem
        TextureManager* mTextureManager;

        // Active viewport (dest for future rendering operations)
        Viewport* mActiveViewport;

        CullingMode mCullingMode;

        bool mVSync;

        // Store record of texture unit settings for efficient alterations
        Material::TextureLayer mTextureUnits[OGRE_MAX_TEXTURE_LAYERS];

        unsigned int mFaceCount;
        unsigned int mVertexCount;

        /// Saved set of world matrices
        Matrix4 mWorldMatrices[256];

        /// Temporary buffer for vertex blending in software
        std::vector<Real> mTempVertexBlendBuffer;
        std::vector<Real> mTempNormalBlendBuffer;
        /** Performs a software vertex blend on the passed in operation. 
        @remarks
            This function is supplied to calculate a vertex blend when no hardware
            support is available. The vertices contained in the passed in operation
            will be modified by the matrices supplied according to the blending weights
            also in the operation. To avoid accidentally modifying core vertex data, a
            temporary vertex buffer is used for the result, which is then used in the
            RenderOperation instead of the original passed in vertex data.
        */
        void softwareVertexBlend(RenderOperation& op, Matrix4* pMatrices);






    };




}


#endif
