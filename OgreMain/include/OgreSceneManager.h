/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/
#ifndef __SceneManager_H__
#define __SceneManager_H__

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreSceneNode.h"
#include "OgrePlane.h"
#include "OgreQuaternion.h"
#include "OgreColourValue.h"
#include "OgreCommon.h"
#include "OgreRenderQueue.h"
#include "OgreRenderable.h"
#include "OgreDataChunk.h"
#include "OgreBillboardSet.h"
#include "OgreAnimationState.h"

namespace Ogre {

    /** Structure for holding a position & orientation pair. */
    struct ViewPoint
    {
        Vector3 position;
        Quaternion orientation;
    };


    /** Manages the rendering of a 'scene' i.e. a collection of primitives.
        @remarks
            This class defines the basic behaviour of the 'Scene Manager' family. These classes will
            organise the objects in the scene and send them to the rendering system, a subclass of
            RenderSystem. This basic superclass does no sorting, culling or organising of any sort.
        @par
            Subclasses may use various techniques to organise the scene depending on how they are
            designed (e.g. BSPs, octrees etc). As with other classes, methods preceded with '_' are
            designed to be called by other classes in the Ogre system, not by user applications,
            although this is not forbidden.
        @author
            Steve Streeting
        @version
            1.0
     */
    class _OgreExport SceneManager
    {
    public:
        /** Comparator for material map, for sorting materials into render order (e.g. transparent last).
        */
        struct materialLess
        {
            _OgreExport bool operator()(const Material* x, const Material* y) const;
        };
    protected:

        /// Queue of objects for rendering
        RenderQueue mRenderQueue;

        /// Current ambient light, cached for RenderSystem
        ColourValue mAmbientLight;

        /// The rendering system to send the scene to
        RenderSystem *mDestRenderSystem;

        typedef std::map<std::string, Camera*, std::less<std::string> > CameraList;

        /** Central list of cameras - for easy memory management and lookup.
        */
        CameraList mCameras;

        typedef std::map<std::string, Light*, std::less<std::string> > LightList;

        /** Central list of lights - for easy memory management and lookup.
        */
        LightList mLights;


        typedef std::map<std::string, Entity*, std::less<std::string> > EntityList;

        /** Central list of entities - for easy memory management and lookup.
        */
        EntityList mEntities;

        typedef std::map<std::string, BillboardSet*, std::less<std::string> > BillboardSetList;

        /** Central list of billboard sets - for easy memory management and lookup.
        */
        BillboardSetList mBillboardSets;

        typedef std::map<String, SceneNode*> SceneNodeList;

        /** Central list of SceneNodes - for easy memory management.
            @note
                Note that this list is used only for memory management; the structure of the scene
                is held using the hierarchy of SceneNodes starting with the root node. However you
                can look up nodes this way.
        */
        SceneNodeList mSceneNodes;

        /// Camera in progress
        Camera* mCameraInProgress;
        // Viewport in progress
        Viewport* mViewportInProgress;

        /// Root scene node
        SceneNode* mSceneRoot;

        // Sky params
        // Sky plane
        Entity* mSkyPlaneEntity;
        Entity* mSkyDomeEntity[5];
        Entity* mSkyBoxEntity[6];

        SceneNode* mSkyPlaneNode;
        SceneNode* mSkyDomeNode;
        SceneNode* mSkyBoxNode;

        bool mSkyPlaneEnabled;
        bool mSkyPlaneDrawFirst;
        Plane mSkyPlane;
        // Sky box
        bool mSkyBoxEnabled;
        bool mSkyBoxDrawFirst;
        Quaternion mSkyBoxOrientation;
        // Sky dome
        bool mSkyDomeEnabled;
        bool mSkyDomeDrawFirst;
        Quaternion mSkyDomeOrientation;
        // Fog
        FogMode mFogMode;
        ColourValue mFogColour;
        Real mFogStart;
        Real mFogEnd;
        Real mFogDensity;

        /** Internal method for setting a material for subsequent rendering.
            @remarks
                If this method returns a non-zero value, it means that not all
                the remaining texture layers can be rendered in one pass, and a
                subset of them have been set up in the RenderSystem for the first
                pass - the caller should render the geometry then call this
                method again to set the remaining texture layers and re-render
                the geometry again.
            @param
                mat The Material to set.
            @param
                numLayers The top 'n' number of layers to be processed,
                will only be less than total layers if a previous call
                resulted in a multipass render being required.
            @returns
                The number of layers unprocessed because of insufficient
                available texture units in the hardware.
        */
        int setMaterial(Material* mat, int numLayers);

        enum BoxPlane
        {
            BP_FRONT = 0,
            BP_BACK = 1,
            BP_LEFT = 2,
            BP_RIGHT = 3,
            BP_UP = 4,
            BP_DOWN = 5
        };

        /* Internal utility method for creating the planes of a skybox.
        */
        Mesh* createSkyboxPlane(
            BoxPlane bp,
            Real distance,
            const Quaternion& orientation);

        /* Internal utility method for creating the planes of a skydome.
        */
        Mesh* createSkydomePlane(
            BoxPlane bp,
            Real curvature, Real tiling, Real distance,
            const Quaternion& orientation);

        // Flag indicating whether SceneNodes will be rendered as a set of 3 axes
        bool mDisplayNodes;

        /// Storage of animations, lookup by name
        typedef std::map<String, Animation*> AnimationList;
        AnimationList mAnimationsList;
        AnimationStateSet mAnimationStates;

        /** Internal method used by _renderVisibleObjects to deal with renderables
            which override the camera's own view / projection materices. */
        void useRenderableViewProjMode(Renderable* pRend);

        /// Controller flag for determining if we need to set view/proj matrices
        bool mCamChanged;

    public:
        /** Default constructor.
        */
        SceneManager();

        /** Default destructor.
        */
        virtual ~SceneManager();

        /** Creates a camera to be managed by this scene manager.
            @remarks
                This camera must be added to the scene at a later time using
                the attachObject method of the SceneNode class.
            @param
                name Name to give the new camera.
        */
        virtual Camera* createCamera(const String& name);

        /** Retrieves a pointer to the named camera.
        */
        virtual Camera* getCamera(const String& name);

        /** Removes a camera from the scene.
            @remarks
                This method removes a previously added camera from the scene.
                The camera is deleted so the caller must ensure no references
                to it's previous instance (e.g. in a SceneNode) are used.
            @param
                cam Pointer to the camera to remove
        */
        virtual void removeCamera(Camera *cam);

        /** Removes a camera from the scene.
            @remarks
                This method removes an camera from the scene based on the
                camera's name rather than a pointer.
        */
        virtual void removeCamera(const String& name);

        /** Removes (and destroys) all cameras from the scene.
        */
        virtual void removeAllCameras(void);

        /** Creates a light for use in the scene.
            @remarks
                Lights can either be in a fixed position and independent of the
                scene graph, or they can be attached to SceneNodes so they derive
                their position from the parent node. Either way, they are created
                using this method so that the SceneManager manages their
                existence.
            @param
                name The name of the new light, to identify it later.
        */
        virtual Light* createLight(const String& name);

        /** Returns a pointer to the named Light which has previously been added to the scene.
        */
        virtual Light* getLight(const String& name);

        /** Removes the named light from the scene and destroys it.
            @remarks
                Any pointers held to this light after calling this method will be invalid.
        */
        virtual void removeLight(const String& name);

        /** Removes the light from the scene and destroys it based on a pointer.
            @remarks
                Any pointers held to this light after calling this method will be invalid.
        */
        virtual void removeLight(Light* light);
        /** Removes and destroys all lights in the scene.
        */
        virtual void removeAllLights(void);


        /** Creates a new (blank) material with the specified name.
        */
        virtual Material* createMaterial(const String& name);

        /** Returns a pointer to the default Material settings.
            @remarks
                Ogre comes configured with a set of defaults for newly created
                materials. If you wish to have a different set of defaults,
                simply call this method and change the returned Material's
                settings. All materials created from then on will be configured
                with the new defaults you have specified.
            @par
                The default settings begin as:
                - ambient = ColourValue::White
                - diffuse = ColourValue::White
                - specular = ColourValue::Black
                - emmissive = ColourValue::Black
                - shininess = 0
                - No texture layers (& hence no textures)
                - SourceBlendFactor = SBF_ONE
                - DestBlendFactor = SBF_ZERO (no blend, replace with new
                  colour)
                - Depth buffer checking on
                - Depth buffer writing on
                - Depth buffer comparison function = CMPF_LESS_EQUAL
                - Culling mode = CULL_CLOCKWISE
                - Ambient lighting = ColourValue(0.5, 0.5, 0.5) (mid-grey)
                - Dynamic lighting enabled
                - Gourad shading mode
                - Bilinear texture filtering
        */
        virtual Material* getDefaultMaterialSettings(void);

        /** Adds a material created outside the SceneManager to it's internal
            list.
            @remarks
                Note that SceneManager copies the Material so there are no memory
                management issues. However note that the Material's internal
                handle will be regenerated to ensure uniqueness.
        */
        virtual void addMaterial(const Material& mat);

        /** Gets a reference to a named Material.
        */
        virtual Material* getMaterial(const String& name);

        /** Gets a reference to a material by it's numerical handle.
            @remarks
                Numerical handles are assigned on creation of a material, or
                when a copy is registered with the SceneManager using the
                addMaterial method.
        */
        virtual Material* getMaterial(int handle);

        /** Creates an instance of a SceneNode.
            @remarks
                Note that this does not add the SceneNode to the scene hierarchy.
                This method is for convenience, since it allows an instance to
                be created for which the SceneManager is responsible for
                allocating and releasing memory, which is convenient in complex
                scenes.
            @par
                To include the returned SceneNode in the scene, use the addChild
                method of the SceneNode which is to be it's parent.
            @par
                Note that this method takes no parameters, and the node created is unnamed (it is
                actually given a generated name, which you can retrieve if you want).
                If you wish to create a node with a specific name, call the alternative method
                which takes a name parameter.
        */
        virtual SceneNode* createSceneNode(void);

        /** Creates an instance of a SceneNode with a given name.
            @remarks
                Note that this does not add the SceneNode to the scene hierarchy.
                This method is for convenience, since it allows an instance to
                be created for which the SceneManager is responsible for
                allocating and releasing memory, which is convenient in complex
                scenes.
            @par
                To include the returned SceneNode in the scene, use the addChild
                method of the SceneNode which is to be it's parent.
            @par
                Note that this method takes a name parameter, which makes the node easier to
                retrieve directly again later.
        */
        virtual SceneNode* createSceneNode(const String& name);

        /** Destroys a SceneNode with a given name.
        @remarks
            This allows you to physically delete an individual SceneNode if you want to.
            Note that this is not normally recommended, it's better to allow SceneManager
            to delete the nodes when the scene is cleared.
        */
        virtual void destroySceneNode(const String& name);

        /** Gets the SceneNode at the root of the scene hierarchy.
            @remarks
                The entire scene is held as a hierarchy of nodes, which
                allows things like relative transforms, general changes in
                rendering state etc (See the SceneNode class for more info).
                In this basic SceneManager class, the application using
                Ogre is free to structure this hierarchy however it likes,
                since it has no real significance apart from making transforms
                relative to each node (more specialised subclasses will
                provide utility methods for building specific node structures
                e.g. loading a BSP tree).
            @par
                However, in all cases there is only ever one root node of
                the hierarchy, and this method returns a pointer to it.
        */
        virtual SceneNode* getRootSceneNode(void) const;

        /** Retrieves a named SceneNode from the scene graph.
        @remarks
            If you chose to name a SceneNode as you created it, or if you
            happened to make a note of the generated name, you can look it
            up wherever it is in the scene graph using this method.
        */
        virtual SceneNode* getSceneNode(const String& name) const;

        /** Create an Entity (instance of a discrete mesh).
            @param
                entityName The name to be given to the entity (must be unique).
            @param
                meshName The name of the Mesh it is to be based on (e.g. 'knot.oof'). The
                mesh will be loaded if it is not already.
        */
        virtual Entity* createEntity(const String& entityName, const String& meshName);

        /** Prefab shapes available without loading a model.
            @note
                Minimal implementation at present.
            @todo
                Add more prefabs (teapots, teapots!!!)
        */
        enum PrefabType {
            PT_PLANE
        };

        /** Create an Entity (instance of a discrete mesh) from a range of prefab shapes.
            @param
                entityName The name to be given to the entity (must be unique).
            @param
                ptype The prefab type.
        */
        virtual Entity* createEntity(const String& entityName, PrefabType ptype);
        /** Retrieves a pointer to the named Entity. */
        virtual Entity* getEntity(const String& name);

        /** Removes & destroys an Entity from the SceneManager.
            @warning
                Must only be done if the Entity is not attached
                to a SceneNode. It may be safer to wait to clear the whole
                scene if you are unsure use clearScene.
            @see
                SceneManager::clearScene
        */
        virtual void removeEntity(Entity* ent);

        /** Removes & destroys an Entity from the SceneManager by name.
            @warning
                Must only be done if the Entity is not attached
                to a SceneNode. It may be safer to wait to clear the whole
                scene if you are unsure use clearScene.
            @see
                SceneManager::clearScene
        */
        virtual void removeEntity(const String& name);

        /** Removes & destroys all Entities.
            @warning
                Again, use caution since no Entity must be referred to
                elsewhere e.g. attached to a SceneNode otherwise a crash
                is likely. Use clearScene if you are unsure (it clears SceneNode
                entries too.)
            @see
                SceneManager::clearScene
        */
        virtual void removeAllEntities(void);

        /** Empties the entire scene, inluding all SceneNodes, Cameras, Entities and Lights etc.
        */
        virtual void clearScene(void);

        /** Sets the ambient light level to be used for the scene.
            @remarks
                This sets the colour and intensity of the ambient light in the scene, i.e. the
                light which is 'sourceless' and illuminates all objects equally.
                The colour of an object is affected by a combination of the light in the scene,
                and the amount of light that object reflects (in this case based on the Material::ambient
                property).
            @remarks
                By default the ambient light in the scene is ColourValue::Black, i.e. no ambient light. This
                means that any objects rendered with a Material which has lighting enabled (see Material::setLightingEnabled)
                will not be visible unless you have some dynamic lights in your scene.
        */
        void setAmbientLight(ColourValue colour);

        /** Returns the ambient light level to be used for the scene.
        */
        ColourValue getAmbientLight(void);

        /** Sets the source of the 'world' geometry, i.e. the large, mainly static geometry
            making up the world e.g. rooms, landscape etc.
            @remarks
                Depending on the type of SceneManager (subclasses will be specialised
                for particular world geometry types) you have requested via the Root or
                SceneManagerEnumerator classes, you can pass a filename to this method and it
                will attempt to load the world-level geometry for use. If you try to load
                an inappropriate type of world data an exception will be thrown. The default
                SceneManager cannot handle any sort of world geometry and so will always
                throw an exception. However subclasses like BspSceneManager can load
                particular types of world geometry e.g. "q3dm1.bsp".
            @par
                World geometry will be loaded via the 'common' resource paths and archives set in the
                ResourceManager class.
        */
        virtual void setWorldGeometry(const String& filename);

        /** Asks the SceneManager to provide a suggested viewpoint from which the scene should be viewed.
            @remarks
                Typically this method returns the origin unless a) world geometry has been loaded using
                SceneManager::setWorldGeometry and b) that world geometry has suggested 'start' points.
                If there is more than one viewpoint which the scene manager can suggest, it will always suggest
                the first one unless the random parameter is true.
            @param
                random If true, and there is more than one possible suggestion, a random one will be used. If false
                the same one will always be suggested.
            @return
                On success, true is returned.
            @par
                On failiure, false is returned.
        */
        virtual ViewPoint getSuggestedViewpoint(bool random = false);

        /** Method for setting a specific option of the Scene Manager. These options are usually
            specific for a certain implemntation of the Scene Manager class, and may (and probably
            will) not exist across different implementations.
            @param
                strKey The name of the option to set
            @param
                pValue A pointer to the value - the size should be calculated by the scene manager
                based on the key
            @return
                On success, true is returned.
            @par
                On failiure, false is returned.
        */
        virtual bool setOption( const String& strKey, const void* pValue ) { return false; }

        /** Method for getting the value of an implementation-specific Scene Manager option.
            @param
                strKey The name of the option
            @param
                pDestValue A pointer to a memory location where the value will
                be copied. Currently, the memory will be allocated by the
                scene manager, but this may change
            @return
                On success, true is returned and pDestValue points to the value of the given
                option.
            @par
                On failiure, false is returned and pDestValue is set to NULL.
        */
        virtual bool getOption( const String& strKey, void* pDestValue ) { return false; }

        /** Method for verifying wether the scene manager has an implementation-specific
            option.
            @param
                strKey The name of the option to check for.
            @return
                If the scene manager contains the given option, true is returned.
            @remarks
                If it does not, false is returned.
        */
        virtual bool hasOption( const String& strKey ) { return false; }
        /** Method for getting all possible values for a specific option. When this list is too large
            (i.e. the option expects, for example, a float), the return value will be true, but the
            list will contain just one element whose size will be set to 0.
            Otherwise, the list will be filled with all the possible values the option can
            accept.
            @param
                strKey The name of the option to get the values for.
            @param
                refValueList A reference to a list that will be filled with the available values.
            @return
                On success (the option exists), true is returned.
            @par
                On failiure, false is returned.
        */
        virtual bool getOptionValues( const String& strKey, std::list<SDDataChunk>& refValueList ) { return false; }

        /** Method for getting all the implementation-specific options of the scene manager.
            @param
                refKeys A reference to a list that will be filled with all the available options.
            @return
                On success, true is returned. On failiure, false is returned.
        */
        virtual bool getOptionKeys( std::list<String>& refKeys ) { return false; }

        /** Internal method for updating the scene graph ie the tree of SceneNode instances managed by this class.
            @remarks
                This must be done before issuing objects to the rendering pipeline, since derived transformations from
                parent nodes are not updated until required. This SceneManager is a basic implementation which simply
                updates all nodes from the root. This ensures the scene is up to date but requires all the nodes
                to be updated even if they are not visible. Subclasses could trim this such that only potentially visible
                nodes are updated.
        */
        virtual void _updateSceneGraph(Camera* cam);

        /** Internal method which parses the scene to find visible objects to render.
            @remarks
                If you're implementing a custom scene manager, this is the most important method to
                override since it's here you can apply your custom world partitioning scheme. Once you
                have added the appropriate objects to the render queue, you can let the default
                SceneManager objects _renderVisibleObjects handle the actual rendering of the objects
                you pick.
            @par
                Any visible objects will be added to a rendering queue, which is indexed by material in order
                to ensure objects with the same material are rendered together to minimise render state changes.
        */
        virtual void _findVisibleObjects(Camera* cam);

        /** Internal method for applying animations to scene nodes.
        @remarks
            Uses the internally stored AnimationState objects to apply animation to SceneNodes.
        */
        virtual void _applySceneAnimations(void);

        /** Sends visible objects found in _findVisibleObjects to the rendering engine.
        */
        virtual void _renderVisibleObjects(void);

        /** Prompts the class to send its contents to the renderer.
            @remarks
                This method prompts the scene manager to send the
                contents of the scene it manages to the rendering
                pipeline, possibly preceded by some sorting, culling
                or other scene management tasks. Note that this method is not normally called
                directly by the user application; it is called automatically
                by the Ogre rendering loop.
            @param camera Pointer to a camera from whose viewpoint the scene is to
                be rendered.
            @param vp The target viewport
            @param includeOverlays Whether or not overlay objects should be rendered
        */
        virtual void _renderScene(Camera* camera, Viewport* vp, bool includeOverlays);

        /** Internal method for queueing the sky objects with the params as 
            previously set through setSkyBox, setSkyPlane and setSkyDome.
        */
        virtual void _queueSkiesForRendering(Camera* cam);


        /** Internal method for issuing geometry for a mesh to the RenderSystem pipeline.
            @note
                Not recommended for manual usage, leave Ogre to use this one as appropriate!
            @par
                Assumed that material and world / view / projection transforms have already been set.
        */
        virtual void _renderSubMesh(SubMesh* sm);

        /** Sends any updates to the dynamic lights in the world to the renderer.
        */
        virtual void _updateDynamicLights(void);

        /** Notifies the scene manager of its destination render system
            @remarks
                Called automatically by RenderSystem::addSceneManager
                this method simply notifies the manager of the render
                system to which its output must be directed.
            @param
                sys Pointer to the RenderSystem subclass to be used as a render target.
        */
        virtual void _setDestinationRenderSystem(RenderSystem* sys);

        /** Enables / disables a 'sky plane' i.e. a plane at constant
            distance from the camera representing the sky.
            @remarks
                You can create sky planes yourself using the standard mesh and
                entity methods, but this creates a plane which the camera can
                never get closer or further away from - it moves with the camera.
                (NB you could create this effect by creating a world plane which
                was attached to the same SceneNode as the Camera too, but this
                would only apply to a single camera whereas this plane applies to
                any camera using this scene manager).
            @note
                To apply scaling, scrolls etc to the sky texture(s) you
                should use the Material::TextureLayer class methods.
            @param
                enable True to enable the plane, false to disable it
            @param
                plane Details of the plane, i.e. it's normal and it's
                distance from the camera.
            @param
                materialName The name of the material the plane will use
            @param
                scale The scaling applied to the sky plane - higher values
                mean a bigger sky plane - you may want to tweak this
                depending on the size of plane.d and the other
                characteristics of your scene
            @param
                tiling How many times to tile the texture across the sky.
                Applies to all texture layers. If you need finer control use
                the TextureLayer texture coordinate transformation methods.
            @param
                drawFirst If true, the plane is drawn before all other
                geometry in the scene, without updating the depth buffer.
                This is the safest rendering method since all other objects
                will always appear in front of the sky. However this is not
                the most efficient way if most of the sky is often occluded
                by other objects. If this is the case, you can set this
                parameter to false meaning it draws <em>after</em> all other
                geometry which can be an optimisation - however you must
                ensure that the plane.d value is large enough that no objects
                will 'poke through' the sky plane when it is rendered.
        */
        virtual void setSkyPlane(
            bool enable,
            const Plane& plane, const String& materialName, Real scale = 1000,
            Real tiling = 10, bool drawFirst = true );

        /** Enables / disables a 'sky box' i.e. a 6-sided box at constant
            distance from the camera representing the sky.
            @remarks
                You could create a sky box yourself using the standard mesh and
                entity methods, but this creates a plane which the camera can
                never get closer or further away from - it moves with the camera.
                (NB you could create this effect by creating a world box which
                was attached to the same SceneNode as the Camera too, but this
                would only apply to a single camera whereas this skybox applies
                to any camera using this scene manager).
            @par
                The material you use for the skybox can either contain layers
                which are single textures, or they can be cubic textures, i.e.
                made up of 6 images, one for each plane of the cube. See the
                Material::TextureLayer class for more information.
            @param
                enable True to enable the skybox, false to disable it
            @param
                materialName The name of the material the box will use
            @param
                distance Distance in world coorinates from the camera to
                each plane of the box. The default is normally OK.
            @param
                drawFirst If true, the box is drawn before all other
                geometry in the scene, without updating the depth buffer.
                This is the safest rendering method since all other objects
                will always appear in front of the sky. However this is not
                the most efficient way if most of the sky is often occluded
                by other objects. If this is the case, you can set this
                parameter to false meaning it draws <em>after</em> all other
                geometry which can be an optimisation - however you must
                ensure that the distance value is large enough that no
                objects will 'poke through' the sky box when it is rendered.
            @param
                orientation Optional parameter to specify the orientation
                of the box. By default the 'top' of the box is deemed to be
                in the +y direction, and the 'front' at the -z direction.
                You can use this parameter to rotate the sky if you want.
        */
        virtual void setSkyBox(
            bool enable, const String& materialName, Real distance = 5000,
            bool drawFirst = true, const Quaternion& orientation = Quaternion::IDENTITY );

        /** Enables / disables a 'sky dome' i.e. an illusion of a curved sky.
            @remarks
                A sky dome is actually formed by 5 sides of a cube, but with
                texture coordinates generated such that the surface appears
                curved like a dome. Sky domes are appropriate where you need a
                realistic looking sky where the scene is not going to be
                'fogged', and there is always a 'floor' of some sort to prevent
                the viewer looking below the horizon (the distortion effect below
                the horizon can be pretty horrible, and there is never anyhting
                directly below the viewer). If you need a complete wrap-around
                background, use the setSkyBox method instead. You can actually
                combine a sky box and a sky dome if you want, to give a positional
                backdrop with an overlayed curved cloud layer.
            @par
                Sky domes work well with 2D repeating textures like clouds. You
                can change the apparant 'curvature' of the sky depending on how
                your scene is viewed - lower curvatures are better for 'open'
                scenes like landscapes, whilst higher curvatures are better for
                say FPS levels where you don't see a lot of the sky at once and
                the exaggerated curve looks good.
            @param
                enable True to enable the skydome, false to disable it
            @param
                materialName The name of the material the dome will use
            @param
                curvature The curvature of the dome. Good values are
                between 2 and 65. Higher values are more curved leading to
                a smoother effect, lower values are less curved meaning
                more distortion at the horizons but a better distance effect.
            @param
                tiling How many times to tile the texture(s) across the
                dome.
            @param
                distance Distance in world coorinates from the camera to
                each plane of the box the dome is rendered on. The default
                is normally OK.
            @param
                drawFirst If true, the dome is drawn before all other
                geometry in the scene, without updating the depth buffer.
                This is the safest rendering method since all other objects
                will always appear in front of the sky. However this is not
                the most efficient way if most of the sky is often occluded
                by other objects. If this is the case, you can set this
                parameter to false meaning it draws <em>after</em> all other
                geometry which can be an optimisation - however you must
                ensure that the distance value is large enough that no
                objects will 'poke through' the sky when it is rendered.
            @param
                orientation Optional parameter to specify the orientation
                of the dome. By default the 'top' of the dome is deemed to
                be in the +y direction, and the 'front' at the -z direction.
                You can use this parameter to rotate the sky if you want.
        */
        virtual void setSkyDome(
            bool enable, const String& materialName, Real curvature = 10,
            Real tiling = 8, Real distance = 4000, bool drawFirst = true,
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Sets the fogging mode applied to the scene.
            @remarks
                This method sets up the scene-wide fogging effect. These settings
                apply to all geometry rendered, UNLESS the material with which it
                is rendered has it's own fog settings (see Material::setFog).
            @param
                mode Set up the mode of fog as described in the FogMode
                enum, or set to FOG_NONE to turn off.
            @param
                colour The colour of the fog. Either set this to the same
                as your viewport background colour, or to blend in with a
                skydome or skybox.
            @param
                expDensity The density of the fog in FOG_EXP or FOG_EXP2
                mode, as a value between 0 and 1. The default is 0.001. 
            @param
                linearStart Distance in world units at which linear fog starts to
                encroach. Only applicable if mode is
                FOG_LINEAR.
            @param
                linearEnd Distance in world units at which linear fog becomes completely
                opaque. Only applicable if mode is
                FOG_LINEAR.
        */
        void setFog(
            FogMode mode = FOG_NONE, ColourValue colour = ColourValue::White,
            Real expDensity = 0.001, Real linearStart = 0.0, Real linearEnd = 1.0);

        /** Returns the fog mode for the scene.
        */
        virtual FogMode getFogMode(void) const;

        /** Returns the fog colour for the scene.
        */
        virtual const ColourValue& getFogColour(void) const;

        /** Returns the fog start distance for the scene.
        */
        virtual Real getFogStart(void) const;

        /** Returns the fog end distance for the scene.
        */
        virtual Real getFogEnd(void) const;

        /** Returns the fog density for the scene.
        */
        virtual Real getFogDensity(void) const;

        /** Experimental - do not use!
        */
        virtual void displaySplashScreen(Viewport* vp, const String& name );

        /** Creates a new BillboardSet for use with this scene manager.
            @remarks
                This method creates a new BillboardSet which is registered with
                the SceneManager. The SceneManager will destroy this object when
                it shuts down or when the SceneManager::clearScene method is
                called, so the caller does not have to worry about destroying
                this object (in fact, it definitely should not do this).
            @par
                See the BillboardSet documentations for full details of the
                returned class.
            @param
                name The name to give to this billboard set. Must be unique.
            @param
                poolSize The initial size of the pool of billboards (see BillboardSet for more information)
            @see
                BillboardSet
        */
        virtual BillboardSet* createBillboardSet(const String& name, unsigned int poolSize = 20);

        /** Retrieves a pointer to the named BillboardSet.
        */
        virtual BillboardSet* getBillboardSet(const String& name);

        /** Removes & destroys an BillboardSet from the SceneManager.
            @warning
                Must only be done if the BillboardSet is not attached
                to a SceneNode. It may be safer to wait to clear the whole
                scene. If you are unsure, use clearScene.
        */
        virtual void removeBillboardSet(BillboardSet* set);

        /** Removes & destroys an BillboardSet from the SceneManager by name.
            @warning
                Must only be done if the BillboardSet is not attached
                to a SceneNode. It may be safer to wait to clear the whole
                scene. If you are unsure, use clearScene.
        */
        virtual void removeBillboardSet(const String& name);

        /** Tells the SceneManager whether it should render the SceneNodes which 
            make up the scene as well as the objects in the scene.
        @remarks
            This method is mainly for debugging purposes. If you set this to 'true',
            each node will be rendered as a set of 3 axes to allow you to easily see
            the orientation of the nodes.
        */
        virtual void setDisplaySceneNodes(bool display);

        /** Creates an animation which can be used to animate scene nodes.
        @remarks
            An animation is a collection of 'tracks' which over time change the position / orientation
            of Node objects. In this case, the animation will likely have tracks to modify the position
            / orientation of SceneNode objects, e.g. to make objects move along a path.
        @par
            You don't need to use an Animation object to move objects around - you can do it yourself
            using the methods of the Node in your FrameListener class. However, when you need relatively
            complex scripted animation, this is the class to use since it will interpolate between
            keyframes for you and generally make the whole process easier to manage.
        @par
            A single animation can affect multiple Node objects (each AnimationTrack affects a single Node).
            In addition, through animation blending a single Node can be affected by multiple animations,
            athough this is more useful when performing skeletal animation (see Skeleton::createAnimation).
        @par
            Note that whilst it uses the same classes, the animations created here are kept separate from the
            skeletal animations of meshes (each Skeleton owns those animations).
        @param name The name of the animation, must be unique within this SceneManager.
        @param length The total length of the animation.
        */
        virtual Animation* createAnimation(const String& name, Real length);

        /** Looks up an Animation object previously created with createAnimation. */
        virtual Animation* getAnimation(const String& name) const;

        /** Destroys an Animation. 
        @remarks
            You should ensure that none of your code is referencing this animation objects since the 
            memory will be freed.
        */
        virtual void destroyAnimation(const String& name);

        /** Removes all animations created using this SceneManager. */
        virtual void destroyAllAnimations(void);

        /** Create an AnimationState object for managing application of animations.
        @remarks
            You can create Animation objects for animating SceneNode obejcts using the
            createAnimation method. However, in order to actually apply those animations
            you have to call methods on Node and Animation in a particular order (namely
            Node::resetToInitialState and Animation::apply). To make this easier and to
            help track the current time position of animations, the AnimationState object
            is provided. </p>
            So if you don't want to control animation application manually, call this method,
            update the returned object as you like every frame and let SceneManager apply 
            the animation state for you.
        @par
            Remember, AnimationState objects are disabled by default at creation time. 
            Turn them on when you want them using their setEnabled method.
        @par
            Note that any SceneNode affected by this automatic animation will have it's state
            reset to it's initial position before application of the animation. Unless specifically
            modified using Node::setInitialState the Node assumes it's initial state is at the
            origin. If you want the base state of the SceneNode to be elsewhere, make your changes
            to the node using the standard transform methods, then call setInitialState to 
            'bake' this reference position into the node.
        @param animName The name of an animation created already with createAnimation.
        */
        virtual AnimationState* createAnimationState(const String& animName);

        /** Retrieves animation state as previously created using createAnimationState */
        virtual AnimationState* getAnimationState(const String& animName);

        /** Destroys an AnimationState. 
        @remarks
            You should ensure that none of your code is referencing this animation 
            state object since the memory will be freed.
        */
        virtual void destroyAnimationState(const String& name);

        /** Removes all animation states created using this SceneManager. */
        virtual void destroyAllAnimationStates(void);

        /** Manual rendering method, for advanced users only.
        @remarks
            This method allows you to send rendering commands through the pipeline on
            demand, bypassing OGRE's normal world processing. You should only use this if you
            really know what you're doing; OGRE does lots of things for you that you really should
            let it do. However, there are times where it may be useful to have this manual interface,
            for example overlaying something on top of the scene rendered by OGRE.
        @par
            Why use this instead of calling RenderSystem direct? Well, because SceneManagers cache
            state information about the last requested operations in RenderSystem in order to minimise
            render state changes, and you need to maintain this by ensuring calls go through the 
            SceneManager. The other reason is that it's simpler: 1 call instead of many.
        @par
            Because this is an instant rendering method, timing is important. The best 
            time to call it is from a RenderTargetListener event handler.
        @par
            Don't call this method a lot, it's designed for rare (1 or 2 times per frame) use. 
            Calling it regularly per frame will cause frame rate drops!
        @param rend A RenderOperation object describing the rendering op
        @param mat The material to use
        @param vp Pointer to the viewport to render to
        @param worldMatrix The transform to apply from object to world space
        @param viewMatrix The transform to apply from world to view space
        @param projMatrix The transform to apply from view to screen space
        */
        virtual void manualRender(RenderOperation* rend, Material* mat, Viewport* vp, 
            const Matrix4& worldMatrix, const Matrix4& viewMatrix, const Matrix4& projMatrix) ;

        /** Creates a new Overlay.
        @remarks
            Overlays can be used to render heads-up-displays (HUDs), menu systems,
            cockpits and any other 2D or 3D object you need to appear above the
            rest of the scene. See the Overlay class for more information.
        @par
            NOTE: after creation, the Overlay is initially hidden. You can create
            as many overlays as you like ready to be displayed whenever. Just call
            Overlay::show to display the overlay.
        @param name The name to give the overlay, must be unique.
        @param zorder The zorder of the overlay relative to it's peers, higher zorders
            appear on top of lower ones.
        */
        Overlay* createOverlay(const String& name, ushort zorder = 100); 

        /** Gets a pointer to the named Overlay, previously created using createOverlay. */
        Overlay* getOverlay(const String& name);

        /** Destroys the named Overlay. */
        void destroyOverlay(const String& name);
        
        /** Destroys all the overlays. */
        void destroyAllOverlays(void);



    };



} // Namespace



#endif

