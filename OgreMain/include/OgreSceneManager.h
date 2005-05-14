/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreAnimationState.h"
#include "OgreSceneQuery.h"
#include "OgreAutoParamDataSource.h"
#include "OgreAnimationState.h"
#include "OgreRenderQueue.h"
#include "OgreRenderQueueSortingGrouping.h"
#include "OgreRectangle2D.h"

namespace Ogre {

    /** Structure for holding a position & orientation pair. */
    struct ViewPoint
    {
        Vector3 position;
        Quaternion orientation;
    };

	// Forward declarations
	class DefaultIntersectionSceneQuery;
	class DefaultRaySceneQuery;
	class DefaultSphereSceneQuery;
	class DefaultAxisAlignedBoxSceneQuery;
	class EntityFactory;

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
        /// Query mask which will be used for world geometry @see SceneQuery
        static unsigned long WORLD_GEOMETRY_QUERY_MASK;
        /** Comparator for material map, for sorting materials into render order (e.g. transparent last).
        */
        struct materialLess
        {
            _OgreExport bool operator()(const Material* x, const Material* y) const;
        };
        /// Comparator for sorting lights relative to a point
        struct lightLess
        {
            _OgreExport bool operator()(const Light* a, const Light* b) const;
        };

        /// Describes the stage of rendering when performing complex illumination
        enum IlluminationRenderStage
        {
            /// No special illumination stage
            IRS_NONE,
            /// Ambient stage, when background light is added
            IRS_AMBIENT,
            /// Diffuse / specular stage, when individual light contributions are added
            IRS_PER_LIGHT,
            /// Decal stage, when texture detail is added to the lit base
            IRS_DECAL,
            /// Render to texture stage, used for texture based shadows
            IRS_RENDER_TO_TEXTURE,
            /// Modulative render from shadow texture stage
            IRS_RENDER_MODULATIVE_PASS
        };

		/** Enumeration of the possible modes allowed for processing the special case
		render queue list.
		@see SceneManager::setSpecialCaseRenderQueueMode
		*/
		enum SpecialCaseRenderQueueMode
		{
			/// Render only the queues in the special case list
			SCRQM_INCLUDE,
			/// Render all except the queues in the special case list
			SCRQM_EXCLUDE
		};
    protected:

        /// Queue of objects for rendering
        RenderQueue* mRenderQueue;

        /// Current ambient light, cached for RenderSystem
        ColourValue mAmbientLight;

        /// The rendering system to send the scene to
        RenderSystem *mDestRenderSystem;

        typedef std::map<String, Camera* > CameraList;

        /** Central list of cameras - for easy memory management and lookup.
        */
        CameraList mCameras;

        typedef std::map<String, Light* > SceneLightList;

        /** Central list of lights - for easy memory management and lookup.
        */
        SceneLightList mLights;

        typedef std::map<String, BillboardSet* > BillboardSetList;

        /** Central list of billboard sets - for easy memory management and lookup.
        */
        BillboardSetList mBillboardSets;

		typedef std::map<String, StaticGeometry* > StaticGeometryList;
		StaticGeometryList mStaticGeometryList;

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
        /// Current Viewport
        Viewport* mCurrentViewport;

        /// Root scene node
        SceneNode* mSceneRoot;

        /// Autotracking scene nodes
        typedef std::set<SceneNode*> AutoTrackingSceneNodes;
        AutoTrackingSceneNodes mAutoTrackingSceneNodes;

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

		typedef std::set<RenderQueueGroupID> SpecialCaseRenderQueueList;
		SpecialCaseRenderQueueList mSpecialCaseQueueList;
		SpecialCaseRenderQueueMode mSpecialCaseQueueMode;
		RenderQueueGroupID mWorldGeometryRenderQueue;

        /** Internal method for initialising the render queue.
        @remarks
            Subclasses can use this to install their own RenderQueue implementation.
        */
        virtual void initRenderQueue(void);
        /** Retrieves the internal render queue. */
        virtual RenderQueue* getRenderQueue(void);
        /** Internal method for setting up the renderstate for a rendering pass.
            @param
                pass The Pass details to set.
            @returns
                A Pass object that was used instead of the one passed in, can
                happen when rendering shadow passes
        */
        virtual Pass* setPass(Pass* pass);
        /// A pass designed to let us render shadow colour on white for texture shadows
        Pass* mShadowCasterPlainBlackPass;
        /// A pass designed to let us render shadow receivers for texture shadows
        Pass* mShadowReceiverPass;
        /** Internal method for turning a regular pass into a shadow caster pass.
        @remarks
            This is only used for texture shadows, basically we're trying to
            ensure that objects are rendered solid black.
            This method will usually return the standard solid black pass for
            all fixed function passes, but will merge in a vertex program
            and fudge the AutpoParamDataSource to set black lighting for
            passes with vertex programs. 
        */
        Pass* deriveShadowCasterPass(Pass* pass);
        /** Internal method for turning a regular pass into a shadow receiver pass.
        @remarks
        This is only used for texture shadows, basically we're trying to
        ensure that objects are rendered with a projective texture.
        This method will usually return a standard single-texture pass for
        all fixed function passes, but will merge in a vertex program
        for passes with vertex programs. 
        */
        Pass* deriveShadowReceiverPass(Pass* pass);
    
        /** Internal method to validate whether a Pass should be allowed to render.
        @remarks
            Called just before a pass is about to be used for rendering a group to
            allow the SceneManager to omit it if required. A return value of false
            skips this pass. 
        */
        bool validatePassForRendering(Pass* pass);

        /** Internal method to validate whether a Renderable should be allowed to render.
        @remarks
        Called just before a pass is about to be used for rendering a Renderable to
        allow the SceneManager to omit it if required. A return value of false
        skips it. 
        */
        bool validateRenderableForRendering(Pass* pass, Renderable* rend);

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
        MeshPtr createSkyboxPlane(
            BoxPlane bp,
            Real distance,
            const Quaternion& orientation,
            const String& groupName);

        /* Internal utility method for creating the planes of a skydome.
        */
        MeshPtr createSkydomePlane(
            BoxPlane bp,
            Real curvature, Real tiling, Real distance,
            const Quaternion& orientation,
            int xsegments, int ysegments, int ySegmentsToKeep, 
            const String& groupName);

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

        typedef std::vector<RenderQueueListener*> RenderQueueListenerList;
        RenderQueueListenerList mRenderQueueListeners;

        /// Internal method for firing the queue start event, returns true if queue is to be skipped
        bool fireRenderQueueStarted(RenderQueueGroupID id);
        /// Internal method for firing the queue end event, returns true if queue is to be repeated
        bool fireRenderQueueEnded(RenderQueueGroupID id);

        /** Internal method for setting the destination viewport for the next render. */
        virtual void setViewport(Viewport *vp);

		/** Flag that indicates if all of the scene node's bounding boxes should be shown as a wireframe. */
		bool mShowBoundingBoxes;       

        /** Internal utility method for rendering a single object. 
        @remarks
            Assumes that the pass has already been set up.
        @param rend The renderable to issue to the pipeline
        @param pass The pass which is being used
        @param doLightIteration If true, this method will issue the renderable to
            the pipeline possibly multiple times, if the pass indicates it should be
            done once per light
        @param manualLightList Only applicable if doLightIteration is false, this
            method allows you to pass in a previously determined set of lights
            which will be used for a single render of this object.
        */
        virtual void renderSingleObject(Renderable* rend, Pass* pass, bool doLightIteration, 
            const LightList* manualLightList = 0);

        /// Utility class for calculating automatic parameters for gpu programs
        AutoParamDataSource mAutoParamDataSource;

        ShadowTechnique mShadowTechnique;
        bool mDebugShadows;
        ColourValue mShadowColour;
        Pass* mShadowDebugPass;
        Pass* mShadowStencilPass;
        Pass* mShadowModulativePass;
		bool mShadowMaterialInitDone;
        LightList mLightsAffectingFrustum;
        HardwareIndexBufferSharedPtr mShadowIndexBuffer;
		size_t mShadowIndexBufferSize;
        Rectangle2D* mFullScreenQuad;
        Real mShadowDirLightExtrudeDist;
        IlluminationRenderStage mIlluminationStage;
        unsigned short mShadowTextureSize;
        unsigned short mShadowTextureCount;
		PixelFormat mShadowTextureFormat;
        typedef std::vector<RenderTexture*> ShadowTextureList;
        ShadowTextureList mShadowTextures;
        RenderTexture* mCurrentShadowTexture;
		bool mShadowUseInfiniteFarPlane;
        /** Internal method for locating a list of lights which could be affecting the frustum. 
        @remarks
            Custom scene managers are encouraged to override this method to make use of their
            scene partitioning scheme to more efficiently locate lights, and to eliminate lights
            which may be occluded by word geometry.
        */
        virtual void findLightsAffectingFrustum(const Camera* camera);
        /// Internal method for setting up materials for shadows
        virtual void initShadowVolumeMaterials(void);
        /// Internal method for creating shadow textures (texture-based shadows)
        virtual void createShadowTextures(unsigned short size, unsigned short count, 
			PixelFormat fmt);
        /// Internal method for preparing shadow textures ready for use in a regular render
        virtual void prepareShadowTextures(Camera* cam, Viewport* vp);

        /** Internal method for rendering all the objects for a given light into the 
            stencil buffer.
        @param light The light source
        @param cam The camera being viewed from
        */
        virtual void renderShadowVolumesToStencil(const Light* light, const Camera* cam);
        /** Internal utility method for setting stencil state for rendering shadow volumes. 
        @param secondpass Is this the second pass?
        @param zfail Should we be using the zfail method?
        @param twosided Should we use a 2-sided stencil?
        */
        virtual void setShadowVolumeStencilState(bool secondpass, bool zfail, bool twosided);
        /** Render a set of shadow renderables. */
        void renderShadowVolumeObjects(ShadowCaster::ShadowRenderableListIterator iShadowRenderables,
            Pass* pass, const LightList *manualLightList, unsigned long flags,
            bool secondpass, bool zfail, bool twosided);
        typedef std::vector<ShadowCaster*> ShadowCasterList;
        ShadowCasterList mShadowCasterList;
        SphereSceneQuery* mShadowCasterSphereQuery;
        AxisAlignedBoxSceneQuery* mShadowCasterAABBQuery;
        Real mShadowFarDist;
        Real mShadowFarDistSquared;
        Real mShadowTextureOffset; // proportion of texture offset in view direction e.g. 0.4
        Real mShadowTextureFadeStart; // as a proportion e.g. 0.6
        Real mShadowTextureFadeEnd; // as a proportion e.g. 0.9
		bool mShadowTextureSelfShadow;
		Pass* mShadowTextureCustomCasterPass;
		Pass* mShadowTextureCustomReceiverPass;
		String mShadowTextureCustomCasterVertexProgram;
		String mShadowTextureCustomReceiverVertexProgram;
		GpuProgramParametersSharedPtr mShadowTextureCustomCasterVPParams;
		GpuProgramParametersSharedPtr mShadowTextureCustomReceiverVPParams;
		bool mShadowTextureCasterVPDirty;
		bool mShadowTextureReceiverVPDirty;


        GpuProgramParametersSharedPtr mInfiniteExtrusionParams;
        GpuProgramParametersSharedPtr mFiniteExtrusionParams;

        /// Inner class to use as callback for shadow caster scene query
        class _OgreExport ShadowCasterSceneQueryListener : public SceneQueryListener
        {
        protected:
			SceneManager* mSceneMgr;
            ShadowCasterList* mCasterList;
            bool mIsLightInFrustum;
            const PlaneBoundedVolumeList* mLightClipVolumeList;
            const Camera* mCamera;
            const Light* mLight;
            Real mFarDistSquared;
        public:
            ShadowCasterSceneQueryListener(SceneManager* sm) : mSceneMgr(sm),
				mCasterList(0), mIsLightInFrustum(false), mLightClipVolumeList(0), 
                mCamera(0) {}
            // Prepare the listener for use with a set of parameters  
            void prepare(bool lightInFrustum, 
                const PlaneBoundedVolumeList* lightClipVolumes, 
                const Light* light, const Camera* cam, ShadowCasterList* casterList, 
                Real farDistSquared) 
            {
                mCasterList = casterList;
                mIsLightInFrustum = lightInFrustum;
                mLightClipVolumeList = lightClipVolumes;
                mCamera = cam;
                mLight = light;
                mFarDistSquared = farDistSquared;
            }
            bool queryResult(MovableObject* object);
            bool queryResult(SceneQuery::WorldFragment* fragment);
        };

        ShadowCasterSceneQueryListener* mShadowCasterQueryListener;

		typedef std::map<String, MovableObjectFactory*> MovableObjectFactoryMap;
		MovableObjectFactoryMap mMovableObjectFactoryMap;
		typedef std::map<String, MovableObject*> MovableObjectMap;
		typedef std::map<String, MovableObjectMap*> MovableObjectCollectionMap;
		MovableObjectCollectionMap mMovableObjectCollectionMap;
		// stock factories
		EntityFactory* mEntityFactory;

        /** Internal method for locating a list of shadow casters which 
            could be affecting the frustum for a given light. 
        @remarks
            Custom scene managers are encouraged to override this method to add optimisations, 
            and to add their own custom shadow casters (perhaps for world geometry)
        */
        virtual const ShadowCasterList& findShadowCastersForLight(const Light* light, 
            const Camera* camera);
		/** Render the objects in a given queue group 
		*/
		virtual void renderQueueGroupObjects(RenderQueueGroup* group);
        /** Render a group in the ordinary way */
        virtual void renderBasicQueueGroupObjects(RenderQueueGroup* pGroup);
		/** Render a group with the added complexity of additive stencil shadows. */
		virtual void renderAdditiveStencilShadowedQueueGroupObjects(RenderQueueGroup* group);
		/** Render a group with the added complexity of additive stencil shadows. */
		virtual void renderModulativeStencilShadowedQueueGroupObjects(RenderQueueGroup* group);
        /** Render a group rendering only shadow casters. */
        virtual void renderTextureShadowCasterQueueGroupObjects(RenderQueueGroup* group);
        /** Render a group rendering only shadow receivers. */
        virtual void renderTextureShadowReceiverQueueGroupObjects(RenderQueueGroup* group);
        /** Render a group with the added complexity of additive stencil shadows. */
        virtual void renderModulativeTextureShadowedQueueGroupObjects(RenderQueueGroup* group);
		/** Render a set of objects, see renderSingleObject for param definitions */
		virtual void renderObjects(const RenderPriorityGroup::SolidRenderablePassMap& objs, 
            bool doLightIteration, const LightList* manualLightList = 0);
        /** Render a set of objects, see renderSingleObject for param definitions */
		virtual void renderObjects(const RenderPriorityGroup::TransparentRenderablePassList& objs, 
            bool doLightIteration, const LightList* manualLightList = 0);
		/** Render those objects in the transparent pass list which have shadow casting forced on
		@remarks
			This function is intended to be used to render the shadows of transparent objects which have
			transparency_casts_shadows set to 'on' in their material
		*/
		virtual void renderTransparentShadowCasterObjects(const RenderPriorityGroup::TransparentRenderablePassList& objs, 
			bool doLightIteration, const LightList* manualLightList = 0);

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

        /** Populate a light list with an ordered set of the lights which are closest
        to the position specified.
        @remarks
            Note that since directional lights have no position, they are always considered
            closer than any point lights and as such will always take precedence. 
        @par
            Subclasses of the default SceneManager may wish to take into account other issues
            such as possible visibility of the light if that information is included in their
            data structures. This basic scenemanager simply orders by distance, eliminating 
            those lights which are out of range.
        @par
            The number of items in the list max exceed the maximum number of lights supported
            by the renderer, but the extraneous ones will never be used. In fact the limit will
            be imposed by Pass::getMaxSimultaneousLights.
        @param position The position at which to evaluate the list of lights
        @param radius The bounding radius to test
        @param destList List to be populated with ordered set of lights; will be cleared by 
            this method before population.
        */
        virtual void _populateLightList(const Vector3& position, Real radius, LightList& destList);


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

        /** Empties the entire scene, inluding all SceneNodes, Entities, Lights, 
            BillboardSets etc. Cameras are not deleted at this stage since
            they are still referenced by viewports, which are not destroyed during
            this process.
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
        void setAmbientLight(const ColourValue& colour);

        /** Returns the ambient light level to be used for the scene.
        */
        const ColourValue& getAmbientLight(void) const;

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

        /** Estimate the number of loading stages required to load the named
            world geometry. 
        @remarks
            This method should be overridden by SceneManagers that provide
            custom world geometry that can take some time to load. They should
            return from this method a count of the number of stages of progress
            they can report on whilst loading. During real loading (setWorldGeomtry),
            they should call ResourceGroupManager::_notifyWorldGeometryProgress exactly
            that number of times when loading the geometry for real.
        @note 
            The default is to return 0, ie to not report progress. 
        */
        virtual size_t estimateWorldGeometry(const String& filename) { return 0; }

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
        virtual bool hasOption( const String& strKey ) const { return false; }
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
                On failure, false is returned.
        */
        virtual bool getOptionValues( const String& strKey, StringVector& refValueList ) { return false; }

        /** Method for getting all the implementation-specific options of the scene manager.
            @param
                refKeys A reference to a list that will be filled with all the available options.
            @return
                On success, true is returned. On failiure, false is returned.
        */
        virtual bool getOptionKeys( StringVector& refKeys ) { return false; }

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
        virtual void _findVisibleObjects(Camera* cam, bool onlyShadowCasters);

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
                should use the TextureUnitState class methods.
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
                the TextureUnitState texture coordinate transformation methods.
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
			@param
				bow If zero, the plane will be completely flat (like previous
				versions.  If above zero, the plane will be curved, allowing
				the sky to appear below camera level.  Curved sky planes are 
				simular to skydomes, but are more compatable with fog.
            @param xsegments, ysegments
                Determines the number of segments the plane will have to it. This
                is most important when you are bowing the plane, but may also be useful
                if you need tesselation on the plane to perform per-vertex effects.
            @param groupName
                The name of the resource group to which to assign the plane mesh.
        */
        virtual void setSkyPlane(
            bool enable,
            const Plane& plane, const String& materialName, Real scale = 1000,
            Real tiling = 10, bool drawFirst = true, Real bow = 0, 
            int xsegments = 1, int ysegments = 1, 
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

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
                TextureUnitState class for more information.
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
            @param groupName
                The name of the resource group to which to assign the plane mesh.
        */
        virtual void setSkyBox(
            bool enable, const String& materialName, Real distance = 5000,
            bool drawFirst = true, const Quaternion& orientation = Quaternion::IDENTITY,
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

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
            @param groupName
                The name of the resource group to which to assign the plane mesh.
        */
        virtual void setSkyDome(
            bool enable, const String& materialName, Real curvature = 10,
            Real tiling = 8, Real distance = 4000, bool drawFirst = true,
            const Quaternion& orientation = Quaternion::IDENTITY,
            int xsegments = 16, int ysegments = 16, int ysegments_keep = -1,
            const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

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
            FogMode mode = FOG_NONE, const ColourValue& colour = ColourValue::White,
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

        /** Removes & destroys all BillboardSets.
        @warning
        Again, use caution since no BillboardSet must be referred to
        elsewhere e.g. attached to a SceneNode otherwise a crash
        is likely. Use clearScene if you are unsure (it clears SceneNode
        entries too.)
        @see
        SceneManager::clearScene
        */
        virtual void removeAllBillboardSets(void);

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
            Because this is an instant rendering method, timing is important. The best 
            time to call it is from a RenderTargetListener event handler.
        @par
            Don't call this method a lot, it's designed for rare (1 or 2 times per frame) use. 
            Calling it regularly per frame will cause frame rate drops!
        @param rend A RenderOperation object describing the rendering op
        @param pass The Pass to use for this render
        @param vp Pointer to the viewport to render to
        @param worldMatrix The transform to apply from object to world space
        @param viewMatrix The transform to apply from world to view space
        @param projMatrix The transform to apply from view to screen space
        @param doBeginEndFrame If true, beginFrame() and endFrame() are called, 
            otherwise not. You should leave this as false if you are calling
            this within the main render loop.
        */
        virtual void manualRender(RenderOperation* rend, Pass* pass, Viewport* vp, 
            const Matrix4& worldMatrix, const Matrix4& viewMatrix, const Matrix4& projMatrix, 
            bool doBeginEndFrame = false) ;

        /** Registers a new RenderQueueListener which will be notified when render queues
            are processed.
        */
        virtual void addRenderQueueListener(RenderQueueListener* newListener);

        /** Removes a listener previously added with addRenderQueueListener. */
        virtual void removeRenderQueueListener(RenderQueueListener* delListener);

		/** Adds an item to the 'special case' render queue list.
		@remarks
			Normally all render queues are rendered, in their usual sequence, 
			only varying if a RenderQueueListener nominates for the queue to be 
			repeated or skipped. This method allows you to add a render queue to 
			a 'special case' list, which varies the behaviour. The effect of this
			list depends on the 'mode' in which this list is in, which might be
			to exclude these render queues, or to include them alone (excluding
			all other queues). This allows you to perform broad selective
			rendering without requiring a RenderQueueListener.
		@param qid The identifier of the queue which should be added to the
			special case list. Nothing happens if the queue is already in the list.
		*/
		virtual void addSpecialCaseRenderQueue(RenderQueueGroupID qid);
		/** Removes an item to the 'special case' render queue list.
		@see SceneManager::addSpecialCaseRenderQueue
		@param qid The identifier of the queue which should be removed from the
			special case list. Nothing happens if the queue is not in the list.
		*/
		virtual void removeSpecialCaseRenderQueue(RenderQueueGroupID qid);
		/** Clears the 'special case' render queue list.
		@see SceneManager::addSpecialCaseRenderQueue
		*/
		virtual void clearSpecialCaseRenderQueues(void);
		/** Sets the way the special case render queue list is processed.
		@see SceneManager::addSpecialCaseRenderQueue
		@param mode The mode of processing
		*/
		virtual void setSpecialCaseRenderQueueMode(SpecialCaseRenderQueueMode mode);
		/** Gets the way the special case render queue list is processed. */
		virtual SpecialCaseRenderQueueMode getSpecialCaseRenderQueueMode(void);
		/** Returns whether or not the named queue will be rendered based on the
			current 'special case' render queue list and mode.
		@see SceneManager::addSpecialCaseRenderQueue
		@param qid The identifier of the queue which should be tested
		@returns true if the queue will be rendered, false otherwise
		*/
		virtual bool isRenderQueueToBeProcessed(RenderQueueGroupID qid);

		/** Sets the render queue that the world geometry (if any) this SceneManager
			renders will be associated with.
		@remarks
			SceneManagers which provide 'world geometry' should place it in a 
			specialised render queue in order to make it possible to enable / 
			disable it easily using the addSpecialCaseRenderQueue method. Even 
			if the SceneManager does not use the render queues to render the 
			world geometry, it should still pick a queue to represent it's manual
			rendering, and check isRenderQueueToBeProcessed before rendering.
		@note
			Setting this may not affect the actual ordering of rendering the
			world geometry, if the world geometry is being rendered manually
			by the SceneManager. If the SceneManager feeds world geometry into
			the queues, however, the ordering will be affected. 
		*/
		virtual void setWorldGeometryRenderQueue(RenderQueueGroupID qid);
		/** Gets the render queue that the world geometry (if any) this SceneManager
			renders will be associated with.
		@remarks
			SceneManagers which provide 'world geometry' should place it in a 
			specialised render queue in order to make it possible to enable / 
			disable it easily using the addSpecialCaseRenderQueue method. Even 
			if the SceneManager does not use the render queues to render the 
			world geometry, it should still pick a queue to represent it's manual
			rendering, and check isRenderQueueToBeProcessed before rendering.
		*/
		virtual RenderQueueGroupID getWorldGeometryRenderQueue(void);

		/** Allows all bounding boxes of scene nodes to be displayed. */
		virtual void showBoundingBoxes(bool bShow);

		/** Returns if all bounding boxes of scene nodes are to be displayed */
		virtual bool getShowBoundingBoxes() const;

        /** Internal method for notifying the manager that a SceneNode is autotracking. */
        virtual void _notifyAutotrackingSceneNode(SceneNode* node, bool autoTrack);

        
        /** Creates an AxisAlignedBoxSceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for this scene manager, 
            for an axis aligned box region. See SceneQuery and AxisAlignedBoxSceneQuery 
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param box Details of the box which describes the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        virtual AxisAlignedBoxSceneQuery* 
            createAABBQuery(const AxisAlignedBox& box, unsigned long mask = 0xFFFFFFFF);
        /** Creates a SphereSceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for this scene manager, 
            for a spherical region. See SceneQuery and SphereSceneQuery 
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param sphere Details of the sphere which describes the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        virtual SphereSceneQuery* 
            createSphereQuery(const Sphere& sphere, unsigned long mask = 0xFFFFFFFF);
        /** Creates a PlaneBoundedVolumeListSceneQuery for this scene manager. 
        @remarks
        This method creates a new instance of a query object for this scene manager, 
        for a region enclosed by a set of planes (normals pointing inwards). 
        See SceneQuery and PlaneBoundedVolumeListSceneQuery for full details.
        @par
        The instance returned from this method must be destroyed by calling
        SceneManager::destroyQuery when it is no longer required.
        @param volumes Details of the volumes which describe the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
        certain objects; see SceneQuery for details.
        */
        virtual PlaneBoundedVolumeListSceneQuery* 
            createPlaneBoundedVolumeQuery(const PlaneBoundedVolumeList& volumes, unsigned long mask = 0xFFFFFFFF);


        /** Creates a RaySceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for this scene manager, 
            looking for objects which fall along a ray. See SceneQuery and RaySceneQuery 
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param ray Details of the ray which describes the region for this query.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        virtual RaySceneQuery* 
            createRayQuery(const Ray& ray, unsigned long mask = 0xFFFFFFFF);
        //PyramidSceneQuery* createPyramidQuery(const Pyramid& p, unsigned long mask = 0xFFFFFFFF);
        /** Creates an IntersectionSceneQuery for this scene manager. 
        @remarks
            This method creates a new instance of a query object for locating
            intersecting objects. See SceneQuery and IntersectionSceneQuery
            for full details.
        @par
            The instance returned from this method must be destroyed by calling
            SceneManager::destroyQuery when it is no longer required.
        @param mask The query mask to apply to this query; can be used to filter out
            certain objects; see SceneQuery for details.
        */
        virtual IntersectionSceneQuery* 
            createIntersectionQuery(unsigned long mask = 0xFFFFFFFF);

        /** Destroys a scene query of any type. */
        virtual void destroyQuery(SceneQuery* query);

        typedef MapIterator<SceneLightList> LightIterator;
        typedef MapIterator<CameraList> CameraIterator;
        typedef MapIterator<BillboardSetList> BillboardSetIterator;
        typedef MapIterator<AnimationList> AnimationIterator;

        /** Returns a specialised MapIterator over all lights in the scene. */
        LightIterator getLightIterator(void) {
            return LightIterator(mLights.begin(), mLights.end());
        }
        /** Returns a specialised MapIterator over all cameras in the scene. */
        CameraIterator getCameraIterator(void) {
            return CameraIterator(mCameras.begin(), mCameras.end());
        }
        /** Returns a specialised MapIterator over all BillboardSets in the scene. */
        BillboardSetIterator getBillboardSetIterator(void) {
            return BillboardSetIterator(mBillboardSets.begin(), mBillboardSets.end());
        }
        /** Returns a specialised MapIterator over all animations in the scene. */
        AnimationIterator getAnimationIterator(void) {
            return AnimationIterator(mAnimationsList.begin(), mAnimationsList.end());
        }
        /** Returns a specialised MapIterator over all animation states in the scene. */
        AnimationStateIterator getAnimationStateIterator(void) {
            return AnimationStateIterator(mAnimationStates.begin(), mAnimationStates.end());
        }

        /** Sets the general shadow technique to be used in this scene.
        @remarks   
            There are multiple ways to generate shadows in a scene, and each has 
            strengths and weaknesses. 
            <ul><li>Stencil-based approaches can be used to 
            draw very long, extreme shadows without loss of precision and the 'additive'
            version can correctly show the shadowing of complex effects like bump mapping
            because they physically exclude the light from those areas. However, the edges
            are very sharp and stencils cannot handle transparency, and they involve a 
            fair amount of CPU work in order to calculate the shadow volumes, especially
            when animated objects are involved.</li>
            <li>Texture-based approaches are good for handling transparency (they can, for
            example, correctly shadow a mesh which uses alpha to represent holes), and they
            require little CPU overhead, and can happily shadow geometry which is deformed
            by a vertex program, unlike stencil shadows. However, they have a fixed precision 
            which can introduce 'jaggies' at long range and have fillrate issues of their own.</li>
            </ul>
        @par
            We support 2 kinds of stencil shadows, and 2 kinds of texture-based shadows, and one
            simple decal approach. The 2 stencil approaches differ in the amount of multipass work 
            that is required - the modulative approach simply 'darkens' areas in shadow after the 
            main render, which is the least expensive, whilst the additive approach has to perform 
            a render per light and adds the cumulative effect, whcih is more expensive but more 
            accurate. The texture based shadows both work in roughly the same way, the only difference is
            that the shadowmap approach is slightly more accurate, but requires a more recent
            graphics card.
        @par
            Note that because mixing many shadow techniques can cause problems, only one technique
            is supported at once. Also, you should call this method at the start of the 
            scene setup. 
        @param technique The shadowing technique to use for the scene.
        */
        virtual void setShadowTechnique(ShadowTechnique technique);
        
        /** Gets the current shadow technique. */
        virtual ShadowTechnique getShadowTechnique(void) const { return mShadowTechnique; }

        /** Enables / disables the rendering of debug information for shadows. */
        virtual void setShowDebugShadows(bool debug) { mDebugShadows = debug; }
        /** Are debug shadows shown? */
        virtual bool getShowDebugShadows(void ) const { return mDebugShadows; }

        /** Set the colour used to modulate areas in shadow. 
        @remarks This is only applicable for shadow techniques which involve 
            darkening the area in shadow, as opposed to masking out the light. 
            This colour provided is used as a modulative value to darken the
            areas.
        */
        virtual void setShadowColour(const ColourValue& colour);
        /** Get the colour used to modulate areas in shadow. 
        @remarks This is only applicable for shadow techniques which involve 
        darkening the area in shadow, as opposed to masking out the light. 
        This colour provided is used as a modulative value to darken the
        areas.
        */
        virtual const ColourValue& getShadowColour(void) const;
        /** Sets the distance a shadow volume is extruded for a directional light.
        @remarks
            Although directional lights are essentially infinite, there are many
            reasons to limit the shadow extrusion distance to a finite number, 
            not least of which is compatibility with older cards (which do not
            support infinite positions), and shadow caster elimination.
        @par
            The default value is 10,000 world units. This does not apply to
            point lights or spotlights, since they extrude up to their 
            attenuation range.
        */
        virtual void setShadowDirectionalLightExtrusionDistance(Real dist); 
        /** Gets the distance a shadow volume is extruded for a directional light.
        */
        virtual Real getShadowDirectionalLightExtrusionDistance(void) const;
        /** Sets the maximum distance away from the camera that shadows
        will be visible.
        @remarks
        Shadow techniques can be expensive, therefore it is a good idea
        to limit them to being rendered close to the camera if possible,
        and to skip the expense of rendering shadows for distance objects.
        This method allows you to set the distance at which shadows will no
        longer be rendered.
        @note
        Each shadow technique can interpret this subtely differently.
        For example, one technique may use this to eliminate casters,
        another might use it to attenuate the shadows themselves.
        You should tweak this value to suit your chosen shadow technique
        and scene setup.
        */
        virtual void setShadowFarDistance(Real distance);
        /** Gets the maximum distance away from the camera that shadows
        will be visible.
        */
        virtual Real getShadowFarDistance(void) const
        { return mShadowFarDist; }

		/** Sets the maximum size of the index buffer used to render shadow
		 	primitives.
		@remarks
			This method allows you to tweak the size of the index buffer used
			to render shadow primitives (including stencil shadow volumes). The
			default size is 51,200 entries, which is 100k of GPU memory, or
			enough to render approximately 17,000 triangles. You can reduce this
			as long as you do not have any models / world geometry chunks which 
			could require more than the amount you set.
		@par
			The maximum number of triangles required to render a single shadow 
			volume (including light and dark caps when needed) will be 3x the 
			number of edges on the light silhouette, plus the number of 
			light-facing triangles.	On average, half the 
			triangles will be facing toward the light, but the number of 
			triangles in the silhouette entirely depends on the mesh - 
			angular meshes will have a higher silhouette tris/mesh tris
			ratio than a smooth mesh. You can estimate the requirements for
			your particular mesh by rendering it alone in a scene with shadows
			enabled and a single light - rotate it or the light and make a note
			of how high the triangle count goes (remembering to subtract the 
			mesh triangle count)
		@param size The number of indexes; divide this by 3 to determine the
			number of triangles.
		*/
		virtual void setShadowIndexBufferSize(size_t size);
        /// Get the size of the shadow index buffer
		virtual size_t getShadowIndexBufferSize(void) const
		{ return mShadowIndexBufferSize; }
        /** Set the size of the texture used for texture-based shadows.
        @remarks
            The larger the shadow texture, the better the detail on 
            texture based shadows, but obviously this takes more memory.
            The default size is 512. Sizes must be a power of 2.
        */
        virtual void setShadowTextureSize(unsigned short size);
        /// Get the size of the texture used for texture based shadows
        unsigned short getShadowTextureSize(void) const {return mShadowTextureSize; }
        /** Set the pixel format of the textures used for texture-based shadows.
        @remarks
			By default, a colour texture is used (PF_X8R8G8B8) for texture shadows,
			but if you want to use more advanced texture shadow types you can 
			alter this. If you do, you will have to also call
			setShadowTextureCasterMaterial and setShadowTextureReceiverMaterial
			to provide shader-based materials to use these customised shadow
			texture formats.
        */
        virtual void setShadowTexturePixelFormat(PixelFormat fmt);
        /// Get the format of the textures used for texture based shadows
        PixelFormat getShadowTexturePixelFormat(void) const {return mShadowTextureFormat; }
        /** Set the number of textures allocated for texture-based shadows.
        @remarks
            The default number of textures assigned to deal with texture based
            shadows is 1; however this means you can only have one light casting
            shadows at the same time. You can increase this number in order to 
            make this more flexible, but be aware of the texture memory it will use.
        */
        virtual void setShadowTextureCount(unsigned short count);
        /// Get the number of the textures allocated for texture based shadows
        unsigned short getShadowTextureCount(void) const {return mShadowTextureCount; }
        /** Sets the size and count of textures used in texture-based shadows. 
        @remarks
            @see setShadowTextureSize and setShadowTextureCount for details, this
            method just allows you to change both at once, which can save on 
            reallocation if the textures have already been created.
        */
        virtual void setShadowTextureSettings(unsigned short size, unsigned short count, 
			PixelFormat fmt = PF_X8R8G8B8);
        /** Sets the proportional distance which a texture shadow which is generated from a
            directional light will be offset into the camera view to make best use of texture space.
        @remarks
            When generating a shadow texture from a directional light, an approximation is used
            since it is not possible to render the entire scene to one texture. 
            The texture is projected onto an area centred on the camera, and is
            the shadow far distance * 2 in length (it is square). This wastes
            a lot of texture space outside the frustum though, so this offset allows
            you to move the texture in front of the camera more. However, be aware
            that this can cause a little shadow 'jittering' during rotation, and
            that if you move it too far then you'll start to get artefacts close 
            to the camera. The value is represented as a proportion of the shadow
            far distance, and the default is 0.6.
        */
        virtual void setShadowDirLightTextureOffset(Real offset) { mShadowTextureOffset = offset;}
        /** Sets the proportional distance at which texture shadows begin to fade out.
        @remarks
            To hide the edges where texture shadows end (in directional lights)
            Ogre will fade out the shadow in the distance. This value is a proportional
            distance of the entire shadow visibility distance at which the shadow
            begins to fade out. The default is 0.7
        */
        virtual void setShadowTextureFadeStart(Real fadeStart) 
        { mShadowTextureFadeStart = fadeStart; }
        /** Sets the proportional distance at which texture shadows finish to fading out.
        @remarks
        To hide the edges where texture shadows end (in directional lights)
        Ogre will fade out the shadow in the distance. This value is a proportional
        distance of the entire shadow visibility distance at which the shadow
        is completely invisible. The default is 0.9.
        */
        virtual void setShadowTextureFadeEnd(Real fadeEnd) 
        { mShadowTextureFadeEnd = fadeEnd; }

		/** Sets whether or not texture shadows should attempt to self-shadow.
		@remarks
			The default implementation of texture shadows uses a fixed-function 
			colour texture projection approach for maximum compatibility, and 
			as such cannot support self-shadowing. However, if you decide to 
			implement a more complex shadowing technique using the 
			setShadowTextureCasterMaterial and setShadowTextureReceiverMaterial 
			there is a possibility you may be able to support 
			self-shadowing (e.g by implementing a shader-based shadow map). In 
			this case you might want to enable this option.
		@param selfShadow Whether to attempt self-shadowing with texture shadows
		*/
		virtual void setShadowTextureSelfShadow(bool selfShadow) 
		{ mShadowTextureSelfShadow = selfShadow; }
		/// Gets whether or not texture shadows attempt to self-shadow.
		virtual bool getShadowTextureSelfShadow(void) const 
		{ return mShadowTextureSelfShadow; }
		/** Sets the default material to use for rendering shadow casters.
		@remarks
			By default shadow casters are rendered into the shadow texture using
			an automatically generated fixed-function pass. This allows basic
			projective texture shadows, but it's possible to use more advanced
			shadow techniques by overriding the caster and receiver materials, for
			example providing vertex and fragment programs to implement shadow
			maps.
		@par
			You can rely on the ambient light in the scene being set to the 
			requested texture shadow colour, if that's useful. 
		@note
			Individual objects may also override the vertex program in
			your default material if their materials include 
			shadow_caster_vertex_program_ref shadow_receiver_vertex_program_ref
			entries, so if you use both make sure they are compatible.
		@note
			Only a single pass is allowed in your material, although multiple
			techniques may be used for hardware fallback.
		*/
		virtual void setShadowTextureCasterMaterial(const String& name);
		/** Sets the default material to use for rendering shadow receivers.
		@remarks
			By default shadow receivers are rendered as a post-pass using basic
			modulation. This allows basic projective texture shadows, but it's 
			possible to use more advanced shadow techniques by overriding the 
			caster and receiver materials, for example providing vertex and 
			fragment programs to implement shadow maps.
		@par
			You can rely on texture unit 0 containing the shadow texture, and 
			for the unit to be set to use projective texturing from the light 
			(only useful if you're using fixed-function, which is unlikely; 
			otherwise you should rely on the texture_viewproj_matrix auto binding)
		@note
			Individual objects may also override the vertex program in
			your default material if their materials include 
			shadow_caster_vertex_program_ref shadow_receiver_vertex_program_ref
			entries, so if you use both make sure they are compatible.
		@note
			Only a single pass is allowed in your material, although multiple
			techniques may be used for hardware fallback.
		*/
		virtual void setShadowTextureReceiverMaterial(const String& name);

		/** Sets whether we should use an inifinite camera far plane
			when rendering stencil shadows.
		@remarks
			Stencil shadow coherency is very reliant on the shadow volume
			not being clipped by the far plane. If this clipping happens, you
			get a kind of 'negative' shadow effect. The best way to achieve
			coherency is to move the far plane of the camera out to infinity,
			thus preventing the far plane from clipping the shadow volumes.
			When combined with vertex program extrusion of the volume to 
			infinity, which	Ogre does when available, this results in very
			robust shadow volumes. For this reason, when you enable stencil 
			shadows, Ogre automatically changes your camera settings to 
			project to infinity if the card supports it. You can disable this
			behaviour if you like by calling this method; although you can 
			never enable infinite projection if the card does not support it.
		@par	
			If you disable infinite projection, or it is not available, 
			you need to be far more careful with your light attenuation /
			directional light extrusion distances to avoid clipping artefacts
			at the far plane.
		@note
			Recent cards will generally support infinite far plane projection.
			However, we have found some cases where they do not, especially
			on Direct3D. There is no standard capability we can check to 
			validate this, so we use some heuristics based on experience:
			<UL>
			<LI>OpenGL always seems to support it no matter what the card</LI>
			<LI>Direct3D on non-vertex program capable systems (including 
			vertex program capable cards on Direct3D7) does not
			support it</LI>
			<LI>Direct3D on GeForce3 and GeForce4 Ti does not seem to support
			infinite projection<LI>
			</UL>
			Therefore in the RenderSystem implementation, we may veto the use
			of an infinite far plane based on these heuristics. 
		*/
        virtual void setShadowUseInfiniteFarPlane(bool enable) {
            mShadowUseInfiniteFarPlane = enable; }

		/** Creates a StaticGeometry instance suitable for use with this
			SceneManager.
		@remarks
			StaticGeometry is a way of batching up geometry into a more 
			efficient form at the expense of being able to move it. Please 
			read the StaticGeometry class documentation for full information.
		@param name The name to give the new object
		@returns The new StaticGeometry instance
		*/
		virtual StaticGeometry* createStaticGeometry(const String& name);
		/** Retrieve a previously created StaticGeometry instance. */
		virtual StaticGeometry* getStaticGeometry(const String& name) const;
		/** Remove & destroy a StaticGeometry instance. */
		virtual void removeStaticGeometry(StaticGeometry* geom);
		/** Remove & destroy a StaticGeometry instance. */
		virtual void removeStaticGeometry(const String& name);
		/** Remove & destroy all StaticGeometry instances. */
		virtual void removeAllStaticGeometry(void);


		/** Register a new MovableObjectFactory which will create new MovableObject
			instances of a particular type, as identified by the getType() method.
		@remarks
			Plugin creators can create subclasses of MovableObjectFactory which 
			construct custom subclasses of MovableObject for insertion in the 
			scene. This is the primary way that plugins can make custom objects
			available.
		*/
		virtual void addMovableObjectFactory(MovableObjectFactory* fact);
		/** Removes a previously registered MovableObjectFactory.
		@remarks
			All instances of objects created by this factory will be destroyed
			before removing the factory (by calling back the factories 
			'destroyInstance' method). The plugin writer is responsible for actually
			destroying the factory.
		*/
		virtual void removeMovableObjectFactory(MovableObjectFactory* fact);

		typedef ConstMapIterator<MovableObjectFactoryMap> MovableObjectFactoryIterator;
		/** Return an iterator over all the MovableObjectFactory instances currently
			registered.
		*/
		virtual MovableObjectFactoryIterator getMovableObjectFactoryIterator(void) const;

		/** Create a movable object of the type specified.
		@remarks
			This is the generalised form of MovableObject creation where you can
			create a MovableObject of any specialised type generically, including
			any new types registered using plugins.
		@param name The name to give the object. Must be unique within type.
		@param typeName The type of object to create
		@param params Optional name/value pair list to give extra parameters to
			the created object.
		*/
		virtual MovableObject* createMovableObject(const String& name, 
			const String& typeName, const NameValuePairList* params = 0);
		/** Destroys a MovableObject with the name specified, of the type specified.
		@remarks
			The MovableObject will automatically detach itself from any nodes
			on destruction.
		*/
		virtual void destroyMovableObject(const String& name, const String& typeName);
		/** Destroy all MovableObjects of a given type. */
		virtual void destroyAllMovableObjectsByType(const String& typeName);
		/** Destroy all MovableObjects. */
		virtual void destroyAllMovableObjects(void);
		/** Get a reference to a previously created MovableObject. */
		virtual MovableObject* getMovableObject(const String& name, const String& typeName);
		typedef MapIterator<MovableObjectMap> MovableObjectIterator;
		/** Get an iterator over all MovableObect instances of a given type. */
		MovableObjectIterator getMovableObjectIterator(const String& typeName);
		
    };

    /** Default implementation of IntersectionSceneQuery. */
    class _OgreExport DefaultIntersectionSceneQuery : 
        public IntersectionSceneQuery
    {
    public:
        DefaultIntersectionSceneQuery(SceneManager* creator);
        ~DefaultIntersectionSceneQuery();

        /** See IntersectionSceneQuery. */
        void execute(IntersectionSceneQueryListener* listener);
    };

    /** Default implementation of RaySceneQuery. */
	class _OgreExport DefaultRaySceneQuery : public RaySceneQuery
    {
    public:
        DefaultRaySceneQuery(SceneManager* creator);
        ~DefaultRaySceneQuery();

        /** See RayScenQuery. */
        void execute(RaySceneQueryListener* listener);
    };
    /** Default implementation of SphereSceneQuery. */
	class _OgreExport DefaultSphereSceneQuery : public SphereSceneQuery
    {
    public:
        DefaultSphereSceneQuery(SceneManager* creator);
        ~DefaultSphereSceneQuery();

        /** See SceneQuery. */
        void execute(SceneQueryListener* listener);
    };
    /** Default implementation of PlaneBoundedVolumeListSceneQuery. */
    class _OgreExport DefaultPlaneBoundedVolumeListSceneQuery : public PlaneBoundedVolumeListSceneQuery
    {
    public:
        DefaultPlaneBoundedVolumeListSceneQuery(SceneManager* creator);
        ~DefaultPlaneBoundedVolumeListSceneQuery();

        /** See SceneQuery. */
        void execute(SceneQueryListener* listener);
    };
    /** Default implementation of AxisAlignedBoxSceneQuery. */
	class _OgreExport DefaultAxisAlignedBoxSceneQuery : public AxisAlignedBoxSceneQuery
    {
    public:
        DefaultAxisAlignedBoxSceneQuery(SceneManager* creator);
        ~DefaultAxisAlignedBoxSceneQuery();

        /** See RayScenQuery. */
        void execute(SceneQueryListener* listener);
    };
    


} // Namespace



#endif
