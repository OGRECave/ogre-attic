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
#include "OgreStableHeaders.h"

#include "OgreSceneManager.h"

#include "OgreCamera.h"
#include "OgreRenderSystem.h"
#include "OgreMeshManager.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreLight.h"
#include "OgreMath.h"
#include "OgreControllerManager.h"
#include "OgreMaterialManager.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreRenderQueueSortingGrouping.h"
#include "OgreOverlay.h"
#include "OgreOverlayManager.h"
#include "OgreStringConverter.h"
#include "OgreRenderQueueListener.h"
#include "OgreBillboardSet.h"
#include "OgrePass.h"
#include "OgreTechnique.h"
#include "OgreTextureUnitState.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreRoot.h"
#include "OgreSpotShadowFadePng.h"
#include "OgreGpuProgramManager.h"
#include "OgreGpuProgram.h"
#include "OgreShadowVolumeExtrudeProgram.h"

// This class implements the most basic scene manager

#include <cstdio>

namespace Ogre {

    SceneManager::SceneManager()
    {
        // Root scene node
        mSceneRoot = new SceneNode(this, "root node");
        mRenderQueue = 0;

        // No sky by default
        mSkyPlaneEnabled = false;
        mSkyBoxEnabled = false;
        mSkyDomeEnabled = false;

        // init sky
        mSkyPlaneEntity = 0;
        size_t i;
        for (i = 0; i < 6; ++i)
        {
            mSkyBoxEntity[i] = 0;
        }
        for (i = 0; i < 5; ++i)
        {
            mSkyDomeEntity[i] = 0;
        }
        mSkyPlaneNode = 0;
        mSkyDomeNode = 0;
        mSkyBoxNode = 0;


        // No fog
        mFogMode = FOG_NONE;

        mDisplayNodes = false;

	    mShowBoundingBoxes = false;
        mShadowTechnique = SHADOWTYPE_NONE;
        mDebugShadows = false;
        mShadowDebugPass = 0;
        mShadowStencilPass = 0;
        mShadowModulativePass = 0;
        mShadowCasterPlainBlackPass = 0;
        mShadowReceiverPass = 0;
        mFullScreenQuad = 0;
        mShadowCasterSphereQuery = 0;
        mShadowCasterAABBQuery = 0;
        mShadowDirLightExtrudeDist = 10000;
        mIlluminationStage = IRS_NONE;
        mShadowFarDist = 0;
        mShadowFarDistSquared = 0;
		mShadowIndexBufferSize = 51200;
        mShadowTextureSize = 512;
        mShadowTextureCount = 1;
        mShadowColour = ColourValue(0.25, 0.25, 0.25);
        mShadowTextureOffset = 0.6; 
        mShadowTextureFadeStart = 0.7; 
        mShadowTextureFadeEnd = 0.9; 




    }

    SceneManager::~SceneManager()
    {
        clearScene();
        removeAllCameras();
        delete mSceneRoot;
        delete mFullScreenQuad;
        delete mShadowCasterSphereQuery;
        delete mShadowCasterAABBQuery;
        delete mRenderQueue;
    }
    //-----------------------------------------------------------------------
    RenderQueue* SceneManager::getRenderQueue(void)
    {
        if (!mRenderQueue)
        {
            initRenderQueue();
        }
        return mRenderQueue;
    }
    //-----------------------------------------------------------------------
    void SceneManager::initRenderQueue(void)
    {
        mRenderQueue = new RenderQueue();
        // init render queues that do not need shadows
        mRenderQueue->getQueueGroup(RENDER_QUEUE_BACKGROUND)->setShadowsEnabled(false);
        mRenderQueue->getQueueGroup(RENDER_QUEUE_OVERLAY)->setShadowsEnabled(false);
        mRenderQueue->getQueueGroup(RENDER_QUEUE_SKIES_EARLY)->setShadowsEnabled(false);
        mRenderQueue->getQueueGroup(RENDER_QUEUE_SKIES_LATE)->setShadowsEnabled(false);
    }
    //-----------------------------------------------------------------------
    Camera* SceneManager::createCamera(const String& name)
    {
        Camera *c = new Camera(name, this);
        mCameras.insert(CameraList::value_type(name, c));


        return c;
    }

    //-----------------------------------------------------------------------
    Camera* SceneManager::getCamera(const String& name)
    {
        CameraList::iterator i = mCameras.find(name);
        if (i == mCameras.end())
        {
            return 0;
        }
        else
        {
            return i->second;
        }
    }

    //-----------------------------------------------------------------------
    void SceneManager::removeCamera(Camera *cam)
    {
        // Find in list
        CameraList::iterator i = mCameras.begin();
        for (; i != mCameras.end(); ++i)
        {
            if (i->second == cam)
            {
                mCameras.erase(i);
                // notify render targets
                mDestRenderSystem->_notifyCameraRemoved(cam);
                delete cam;
                break;
            }
        }

    }

    //-----------------------------------------------------------------------
    void SceneManager::removeCamera(const String& name)
    {
        // Find in list
        CameraList::iterator i = mCameras.find(name);
        if (i != mCameras.end())
        {
            // Notify render system
            mDestRenderSystem->_notifyCameraRemoved(i->second);
            delete i->second;
            mCameras.erase(i);
        }

    }

    //-----------------------------------------------------------------------
    void SceneManager::removeAllCameras(void)
    {

        CameraList::iterator i = mCameras.begin();
        for (; i != mCameras.end(); ++i)
        {
            // Notify render system
            mDestRenderSystem->_notifyCameraRemoved(i->second);
            delete i->second;
        }
        mCameras.clear();
    }

    //-----------------------------------------------------------------------
    Light* SceneManager::createLight(const String& name)
    {
        Light *l = new Light(name);
        mLights.insert(SceneLightList::value_type(name, l));
        return l;
    }

    //-----------------------------------------------------------------------
    Light* SceneManager::getLight(const String& name)
    {
        SceneLightList::iterator i = mLights.find(name);
        if (i == mLights.end())
        {
            return 0;
        }
        else
        {
            return i->second;
        }
    }

    //-----------------------------------------------------------------------
    void SceneManager::removeLight(Light *l)
    {
        // Find in list
        SceneLightList::iterator i = mLights.begin();
        for (; i != mLights.end(); ++i)
        {
            if (i->second == l)
            {
                mLights.erase(i);
                delete l;
                break;
            }
        }

    }

    //-----------------------------------------------------------------------
    void SceneManager::removeLight(const String& name)
    {
        // Find in list
        SceneLightList::iterator i = mLights.find(name);
        if (i != mLights.end())
        {
            delete i->second;
            mLights.erase(i);
        }

    }

    //-----------------------------------------------------------------------
    void SceneManager::removeAllLights(void)
    {

        SceneLightList::iterator i = mLights.begin();
        for (; i != mLights.end(); ++i)
        {
            delete i->second;
        }
        mLights.clear();
    }
    //-----------------------------------------------------------------------
    bool SceneManager::lightLess::operator()(const Light* a, const Light* b) const
    {
        return a->tempSquareDist < b->tempSquareDist;
    }
    //-----------------------------------------------------------------------
    void SceneManager::_populateLightList(const Vector3& position, LightList& destList)
    {
        // Really basic trawl of the lights, then sort
        destList.clear();

        SceneLightList::iterator i, iend;
        iend = mLights.end();
        for (i = mLights.begin(); i != iend; ++i)
        {
            Light* lt = i->second;
            if (lt->isVisible())
            {
                if (lt->getType() == Light::LT_DIRECTIONAL)
                {
                    // No distance
                    lt->tempSquareDist = 0.0f;
                    destList.push_back(lt);
                }
                else
                {
                    // Calc squared distance
                    lt->tempSquareDist = (lt->getDerivedPosition() - position).squaredLength();
                    // only add in-range lights
                    Real range = lt->getAttenuationRange();
                    if (lt->tempSquareDist <= (range * range))
                    {
                        destList.push_back(lt);
                    }
                }
            }
        }

        // Sort
        std::sort(destList.begin(), destList.end(), lightLess());


    }
    //-----------------------------------------------------------------------
    Entity* SceneManager::createEntity(const String& entityName, PrefabType ptype)
    {
        switch (ptype)
        {
        case PT_PLANE:
            return createEntity(entityName, "Prefab_Plane");

            break;
        }

        return 0;
    }

    //-----------------------------------------------------------------------
    Entity* SceneManager::createEntity(
        const String& entityName,
        const String& meshName )
    {
        // Check name not used
        EntityList::iterator it = mEntities.find( entityName );
        if( it != mEntities.end() )
        {
            Except(
                Exception::ERR_DUPLICATE_ITEM,
                "An entity with the name " + entityName + " already exists",
                "SceneManager::createEntity" );
        }

        // Get mesh (load if required)
        Mesh* pMesh = MeshManager::getSingleton().load( meshName );

        // Create entity
        Entity* e = new Entity( entityName, pMesh, this );

        // Add to internal list
        mEntities[entityName] = e; //.insert(EntityList::value_type(entityName, e));

        return e;
    }

    //-----------------------------------------------------------------------
    Entity* SceneManager::getEntity(const String& name)
    {
        EntityList::iterator i = mEntities.find(name);
        if (i == mEntities.end())
        {
            return 0;
        }
        else
        {
            return i->second;
        }
    }

    //-----------------------------------------------------------------------
    void SceneManager::removeEntity(Entity *cam)
    {
        // Find in list
        EntityList::iterator i = mEntities.begin();
        for (; i != mEntities.end(); ++i)
        {
            if (i->second == cam)
            {
                mEntities.erase(i);
                delete cam;
                break;
            }
        }

    }

    //-----------------------------------------------------------------------
    void SceneManager::removeEntity(const String& name)
    {
        // Find in list
        EntityList::iterator i = mEntities.find(name);
        if (i != mEntities.end())
        {
            delete i->second;
            mEntities.erase(i);
        }

    }

    //-----------------------------------------------------------------------
    void SceneManager::removeAllEntities(void)
    {

        EntityList::iterator i = mEntities.begin();
        for (; i != mEntities.end(); ++i)
        {
            delete i->second;
        }
        mEntities.clear();
    }

    //-----------------------------------------------------------------------
    void SceneManager::removeAllBillboardSets(void)
    {
        // Delete all BillboardSets
        for (BillboardSetList::iterator bi = mBillboardSets.begin();
            bi != mBillboardSets.end(); ++bi)
        {
            delete bi->second;
        }
        mBillboardSets.clear();
    }
    //-----------------------------------------------------------------------
    void SceneManager::clearScene(void)
    {
        // Delete all SceneNodes, except root that is
        for (SceneNodeList::iterator i = mSceneNodes.begin();
            i != mSceneNodes.end(); ++i)
        {
            delete i->second;
        }
        mSceneNodes.clear();
        mAutoTrackingSceneNodes.clear();

        // Clear root node of all children
        mSceneRoot->removeAllChildren();
        mSceneRoot->detachAllObjects();

        removeAllEntities();
        removeAllBillboardSets();
        removeAllLights();

        // Clear animations
        destroyAllAnimations();

        // Remove sky nodes since they've been deleted
        mSkyBoxNode = mSkyPlaneNode = mSkyDomeNode = 0;
        mSkyBoxEnabled = mSkyPlaneEnabled = mSkyDomeEnabled = false; 

    }

    //-----------------------------------------------------------------------
    Material* SceneManager::createMaterial(const String& name)
    {
        // Create using MaterialManager
        Material* m = (Material*)MaterialManager::getSingleton().create(name);


        return m;
    }
    //-----------------------------------------------------------------------
    Material* SceneManager::getDefaultMaterialSettings(void)
    {
        return Material::mDefaultSettings;
    }
    //-----------------------------------------------------------------------
    Material* SceneManager::getMaterial(const String& name)
    {
        return (Material*)MaterialManager::getSingleton().getByName(name);
    }

    //-----------------------------------------------------------------------
    Material* SceneManager::getMaterial(int handle)
    {
        return static_cast<Material*>(
            MaterialManager::getSingleton().getByHandle(handle));
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneManager::createSceneNode(void)
    {
        SceneNode* sn = new SceneNode(this);
        mSceneNodes[sn->getName()] = sn;
        return sn;
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneManager::createSceneNode(const String& name)
    {
        SceneNode* sn = new SceneNode(this, name);
        mSceneNodes[sn->getName()] = sn;
        return sn;
    }
    //-----------------------------------------------------------------------
    void SceneManager::destroySceneNode(const String& name)
    {
        SceneNodeList::iterator i = mSceneNodes.find(name);

        if (i == mSceneNodes.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "SceneNode '" + name + "' not found.",
                "SceneManager::destroySceneNode");
        }

        // Find any scene nodes which are tracking this node, and turn them off
        AutoTrackingSceneNodes::iterator ai, aiend;
        aiend = mAutoTrackingSceneNodes.end();
        for (ai = mAutoTrackingSceneNodes.begin(); ai != aiend; ++ai)
        {
            SceneNode* n = *ai;
            // Tracking this node
            if (n->getAutoTrackTarget() == i->second)
            {
                // turn off, this will notify SceneManager to remove
                n->setAutoTracking(false);
                // no need to reset iterator since set erase does not invalidate
            }
            // node is itself a tracker
            else if (n == i->second)
            {
                mAutoTrackingSceneNodes.erase(ai);
            }
        }

        delete i->second;
        mSceneNodes.erase(i);
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneManager::getRootSceneNode(void) const
    {
        return mSceneRoot;
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneManager::getSceneNode(const String& name) const
    {
        SceneNodeList::const_iterator i = mSceneNodes.find(name);

        if (i == mSceneNodes.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "SceneNode '" + name + "' not found.",
                "SceneManager::getSceneNode");
        }

        return i->second;
        
    }
    //-----------------------------------------------------------------------
    Pass* SceneManager::setPass(Pass* pass)
    {
		static bool lastUsedVertexProgram = false;
		static bool lastUsedFragmentProgram = false;

        if (mIlluminationStage == IRS_RENDER_TO_TEXTURE)
        {
            // Derive a special shadow caster pass from this one
            pass = deriveShadowCasterPass(pass);
        }
        else if (mIlluminationStage == IRS_RENDER_MODULATIVE_PASS)
        {
            pass = deriveShadowReceiverPass(pass);
        }

        // TEST
        /*
        LogManager::getSingleton().logMessage("BEGIN PASS " + StringConverter::toString(pass->getIndex()) + 
            " of " + pass->getParent()->getParent()->getName());
        */

        if (pass->hasVertexProgram())
        {
            mDestRenderSystem->bindGpuProgram(pass->getVertexProgram()->_getBindingDelegate());
            // bind parameters later since they can be per-object
            lastUsedVertexProgram = true;
        }
        else
        {
			// Unbind program?
			if (lastUsedVertexProgram)
			{
				mDestRenderSystem->unbindGpuProgram(GPT_VERTEX_PROGRAM);
                lastUsedVertexProgram = false;
            }
            // Set fixed-function vertex parameters

            // Set surface reflectance properties, only valid if lighting is enabled
            if (pass->getLightingEnabled())
            {
                mDestRenderSystem->_setSurfaceParams( 
                    pass->getAmbient(), 
                    pass->getDiffuse(), 
                    pass->getSpecular(), 
                    pass->getSelfIllumination(), 
                    pass->getShininess() );
            }

            // Dynamic lighting enabled?
            mDestRenderSystem->setLightingEnabled(pass->getLightingEnabled());
        }

        // Using a fragment program?
        if (pass->hasFragmentProgram())
        {
            mDestRenderSystem->bindGpuProgram(
                pass->getFragmentProgram()->_getBindingDelegate());
            // bind parameters later since they can be per-object
			lastUsedFragmentProgram = true;
        }
        else
        {
			// Unbind program?
			if (lastUsedFragmentProgram)
			{
				mDestRenderSystem->unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
				lastUsedFragmentProgram = false;
			}

            // Set fixed-function fragment settings

            // Fog (assumes we want pixel fog which is the usual)
            // New fog params can either be from scene or from material
            FogMode newFogMode;
            ColourValue newFogColour;
            Real newFogStart, newFogEnd, newFogDensity;
            if (pass->getFogOverride())
            {
                // New fog params from material
                newFogMode = pass->getFogMode();
                newFogColour = pass->getFogColour();
                newFogStart = pass->getFogStart();
                newFogEnd = pass->getFogEnd();
                newFogDensity = pass->getFogDensity();
            }
            else
            {
                // New fog params from scene
                newFogMode = mFogMode;
                newFogColour = mFogColour;
                newFogStart = mFogStart;
                newFogEnd = mFogEnd;
                newFogDensity = mFogDensity;
            }
            mDestRenderSystem->_setFog(
                newFogMode, newFogColour, newFogDensity, newFogStart, newFogEnd);

        }

        // The rest of the settings are the same no matter whether we use programs or not

        // Set scene blending
        mDestRenderSystem->_setSceneBlending(
            pass->getSourceBlendFactor(), pass->getDestBlendFactor());


        // Texture unit settings
        
        Pass::TextureUnitStateIterator texIter =  pass->getTextureUnitStateIterator();
        size_t unit = 0;
        while(texIter.hasMoreElements())
        {
            TextureUnitState* pTex = texIter.getNext();
            mDestRenderSystem->_setTextureUnitSettings(unit, *pTex);
            ++unit;
        }
        // Disable remaining texture units
        mDestRenderSystem->_disableTextureUnitsFrom(pass->getNumTextureUnitStates());

        // Set up non-texture related material settings
        // Depth buffer settings
        mDestRenderSystem->_setDepthBufferFunction(pass->getDepthFunction());
        mDestRenderSystem->_setDepthBufferCheckEnabled(pass->getDepthCheckEnabled());
        mDestRenderSystem->_setDepthBufferWriteEnabled(pass->getDepthWriteEnabled());
        mDestRenderSystem->_setDepthBias(pass->getDepthBias());
        // Set colour write mode
        // Right now we only use on/off, not per-channel
        bool colWrite = pass->getColourWriteEnabled();
        mDestRenderSystem->_setColourBufferWriteEnabled(colWrite, colWrite, colWrite, colWrite);
        // Culling mode
        mDestRenderSystem->_setCullingMode(pass->getCullingMode());
        // Shading
        mDestRenderSystem->setShadingType(pass->getShadingMode());

        return pass;
    }
    //-----------------------------------------------------------------------
    void SceneManager::_renderScene(Camera* camera, Viewport* vp, bool includeOverlays)
    {
        Root::getSingleton()._setCurrentSceneManager(this);
		// Prep Pass for use in debug shadows
		initShadowVolumeMaterials();
        // Perform a quick pre-check to see whether we should override far distance
        // When using stencil volumes we have to use infinite far distance
        // to prevent dark caps getting clipped
        if ((mShadowTechnique == SHADOWTYPE_STENCIL_ADDITIVE ||
            mShadowTechnique == SHADOWTYPE_STENCIL_MODULATIVE) && 
            camera->getFarClipDistance() != 0)
        {
            // infinite far distance
            camera->setFarClipDistance(0);
        }

        mCameraInProgress = camera;
        mCamChanged = true;


        // Update the scene
        _applySceneAnimations();
        _updateSceneGraph(camera);

        // Auto-track nodes
        AutoTrackingSceneNodes::iterator atsni, atsniend;
        atsniend = mAutoTrackingSceneNodes.end();
        for (atsni = mAutoTrackingSceneNodes.begin(); atsni != atsniend; ++atsni)
        {
            (*atsni)->_autoTrack();
        }
        // Auto-track camera if required
        camera->_autoTrack();

        // Are we using any shadows at all?
        if (mShadowTechnique != SHADOWTYPE_NONE && 
            mIlluminationStage != IRS_RENDER_TO_TEXTURE)
        {
            // Locate any lights which could be affecting the frustum
            findLightsAffectingFrustum(camera);
            if (mShadowTechnique == SHADOWTYPE_TEXTURE_MODULATIVE 
                /* || mShadowTechnique == SHADOWTYPE_TEXTURE_SHADOWMAP */)
            {
                // *******
                // WARNING
                // *******
                // This call will result in re-entrant calls to this method
                // therefore anything which comes before this is NOT 
                // guaranteed persistent. Make sure that anything which 
                // MUST be specific to this camera / target is done 
                // AFTER THIS POINT
                prepareShadowTextures(camera, vp);
                // reset the cameras because of the re-entrant call
                mCameraInProgress = camera;
                mCamChanged = true;
            }
        }
        
        // Invert vertex winding?
        if (camera->isReflected())
        {
            mDestRenderSystem->setInvertVertexWinding(true);
        }
        else
        {
            mDestRenderSystem->setInvertVertexWinding(false);
        }

        // Set the viewport
        setViewport(vp);

        // Tell params about camera
        mAutoParamDataSource.setCurrentCamera(camera);

        // Tell params about current ambient light
        mAutoParamDataSource.setAmbientLightColour(mAmbientLight);

        // Tell params about render target
        mAutoParamDataSource.setCurrentRenderTarget(vp->getTarget());


        // Set camera window clipping planes (if any)
		if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_USER_CLIP_PLANES))
		{
			if (camera->isWindowSet())  
			{
				const std::vector<Plane>& planeList = 
					camera->getWindowPlanes();
				for (ushort i = 0; i < 4; ++i)
				{
					mDestRenderSystem->enableClipPlane(i, true);
					mDestRenderSystem->setClipPlane(i, planeList[i]);
				}
			}
			else
			{
				for (ushort i = 0; i < 4; ++i)
				{
					mDestRenderSystem->enableClipPlane(i, false);
				}
			}
		}

        // Clear the render queue
        getRenderQueue()->clear();

        // Parse the scene and tag visibles
        _findVisibleObjects(camera, 
            mIlluminationStage == IRS_RENDER_TO_TEXTURE? true : false);
        // Add overlays, if viewport deems it
        if (vp->getOverlaysEnabled())
        {
            OverlayManager::getSingleton()._queueOverlaysForRendering(camera, getRenderQueue(), vp);
        }
        // Queue skies
        _queueSkiesForRendering(camera);


        // Don't do view / proj here anymore
        // Checked per renderable now, although only changed when required
        //mDestRenderSystem->_setViewMatrix(camera->getViewMatrix());
        //mDestRenderSystem->_setProjectionMatrix(camera->getProjectionMatrix());

        mDestRenderSystem->_beginGeometryCount();
        // Begin the frame
        mDestRenderSystem->_beginFrame();

        // Set rasterisation mode
        mDestRenderSystem->_setRasterisationMode(camera->getDetailLevel());

        // Update controllers (after begineFrame since some are frameTime dependent)
        ControllerManager::getSingleton().updateAllControllers();

        // Render scene content 
        _renderVisibleObjects();

        // End frame
        mDestRenderSystem->_endFrame();

        // Notify camera or vis faces
        camera->_notifyRenderedFaces(mDestRenderSystem->_getFaceCount());



    }


    //-----------------------------------------------------------------------
    void SceneManager::_setDestinationRenderSystem(RenderSystem* sys)
    {
        mDestRenderSystem = sys;
    }


    //-----------------------------------------------------------------------
    void SceneManager::setWorldGeometry(const String& filename)
    {
        // This default implementation cannot handle world geometry
        Except(Exception::ERR_INVALIDPARAMS,
            "World geometry is not supported by the generic SceneManager.",
            "SceneManager::setWorldGeometry");
    }

    //-----------------------------------------------------------------------
    bool SceneManager::materialLess::operator() (const Material* x, const Material* y) const
    {
        // If x transparent and y not, x > y (since x has to overlap y)
        if (x->isTransparent() && !y->isTransparent())
        {
            return false;
        }
        // If y is transparent and x not, x < y
        else if (!x->isTransparent() && y->isTransparent())
        {
            return true;
        }
        else
        {
            // Otherwise don't care (both transparent or both solid)
            // Just arbitrarily use pointer
            return x < y;
        }

    }

    //-----------------------------------------------------------------------
    void SceneManager::setSkyPlane(
        bool enable,
        const Plane& plane,
        const String& materialName,
        Real gscale,
        Real tiling,
        bool drawFirst,
		Real bow)
    {
        mSkyPlaneEnabled = enable;
        if (enable)
        {
            String meshName = "SkyPlane";
            mSkyPlane = plane;

            Material* m = getMaterial(materialName);
            if (!m)
            {
                Except(Exception::ERR_INVALIDPARAMS, 
					"Sky plane material '" + materialName + "' not found.",
					"SceneManager::setSkyPlane");
            }
            // Make sure the material doesn't update the depth buffer
            m->setDepthWriteEnabled(false);
            // Ensure loaded
            m->load();

            mSkyPlaneDrawFirst = drawFirst;

            // Set up the plane
            Mesh* planeMesh = (Mesh*)MeshManager::getSingleton().getByName(meshName);
            if (planeMesh)
            {
                // Destroy the old one
                MeshManager::getSingleton().unload(planeMesh);
                delete planeMesh;
            }

            // Create up vector
            Vector3 up = plane.normal.crossProduct(Vector3::UNIT_X);
            if (up == Vector3::ZERO)
                up = plane.normal.crossProduct(-Vector3::UNIT_Z);

            // Create skyplane
			if( bow > 0 )
			{
				// Build a curved skyplane
				planeMesh = MeshManager::getSingleton().createCurvedPlane(meshName, plane, gscale * 100, gscale * 100, gscale * bow * 100, 6, 6, false, 1, tiling, tiling, up);
			}
			else
			{
				planeMesh = MeshManager::getSingleton().createPlane(meshName, plane, gscale * 100, gscale * 100, 1, 1, false, 1, tiling, tiling, up);
			}

            // Create entity 
            if (mSkyPlaneEntity)
            {
                // destroy old one, do it by name for speed
                removeEntity(meshName);
            }
            // Create, use the same name for mesh and entity
            mSkyPlaneEntity = createEntity(meshName, meshName);
            mSkyPlaneEntity->setMaterialName(materialName);
            mSkyPlaneEntity->setCastShadows(false);

            // Create node and attach
            if (!mSkyPlaneNode)
            {
                mSkyPlaneNode = createSceneNode(meshName + "Node");
            }
            else
            {
                mSkyPlaneNode->detachAllObjects();
            }
            mSkyPlaneNode->attachObject(mSkyPlaneEntity);

        }
    }
    //-----------------------------------------------------------------------
    void SceneManager::setSkyBox(
        bool enable,
        const String& materialName,
        Real distance,
        bool drawFirst,
        const Quaternion& orientation )
    {
        mSkyBoxEnabled = enable;
        if (enable)
        {
            Material* m = getMaterial(materialName);
            if (!m)
            {
                Except(Exception::ERR_INVALIDPARAMS, 
					"Sky box material '" + materialName + " not found.",
					"SceneManager::setSkyBox");
            }
            // Make sure the material doesn't update the depth buffer
            m->setDepthWriteEnabled(false);
            // Ensure loaded
            m->load();
            // Also clamp texture, don't wrap (otherwise edges can get filtered)
            m->getBestTechnique()->getPass(0)->getTextureUnitState(0)->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);


            mSkyBoxDrawFirst = drawFirst;

            // Create node 
            if (!mSkyBoxNode)
            {
                mSkyBoxNode = createSceneNode("SkyBoxNode");
            }
            else
            {
                mSkyBoxNode->detachAllObjects();
            }

            MaterialManager& matMgr = MaterialManager::getSingleton();
            // Set up the box (6 planes)
            for (int i = 0; i < 6; ++i)
            {
                Mesh* planeMesh = createSkyboxPlane((BoxPlane)i, distance, orientation);
                String entName = "SkyBoxPlane" + StringConverter::toString(i);

                // Create entity 
                if (mSkyBoxEntity[i])
                {
                    // destroy old one, do it by name for speed
                    removeEntity(entName);
                }
                mSkyBoxEntity[i] = createEntity(entName, planeMesh->getName());
                mSkyBoxEntity[i]->setCastShadows(false);
                // Have to create 6 materials, one for each frame
                // Used to use combined material but now we're using queue we can't split to change frame
                // This doesn't use much memory because textures aren't duplicated
                Material* boxMat = (Material*)matMgr.getByName(entName);
                if (!boxMat)
                {
                    // Create new by clone
                    boxMat = m->clone(entName);
                    boxMat->load();
                }
                else
                {
                    // Copy over existing
                    m->copyDetailsTo(boxMat);
                    boxMat->load();
                }
                // Set active frame
                boxMat->getBestTechnique()->getPass(0)->getTextureUnitState(0)
                    ->setCurrentFrame(i);

                mSkyBoxEntity[i]->setMaterialName(boxMat->getName());

                // Attach to node
                mSkyBoxNode->attachObject(mSkyBoxEntity[i]);
            } // for each plane

        }

    }
    //-----------------------------------------------------------------------
    void SceneManager::setSkyDome(
        bool enable,
        const String& materialName,
        Real curvature,
        Real tiling,
        Real distance,
        bool drawFirst,
        const Quaternion& orientation )
    {
        mSkyDomeEnabled = enable;
        if (enable)
        {
            Material* m = getMaterial(materialName);
            if (!m)
            {
                Except(Exception::ERR_INVALIDPARAMS, 
					"Sky dome material '" + materialName + " not found.",
					"SceneManager::setSkyDome");
            }
            // Make sure the material doesn't update the depth buffer
            m->setDepthWriteEnabled(false);
            // Ensure loaded
            m->load();

            mSkyDomeDrawFirst = drawFirst;

            // Create node 
            if (!mSkyDomeNode)
            {
                mSkyDomeNode = createSceneNode("SkyDomeNode");
            }
            else
            {
                mSkyDomeNode->detachAllObjects();
            }

            // Set up the dome (5 planes)
            for (int i = 0; i < 5; ++i)
            {
                Mesh* planeMesh = createSkydomePlane((BoxPlane)i, curvature, tiling, distance, orientation);

                String entName = "SkyDomePlane" + StringConverter::toString(i);

                // Create entity 
                if (mSkyDomeEntity[i])
                {
                    // destroy old one, do it by name for speed
                    removeEntity(entName);
                }
                mSkyDomeEntity[i] = createEntity(entName, planeMesh->getName());
                mSkyDomeEntity[i]->setMaterialName(m->getName());
                mSkyDomeEntity[i]->setCastShadows(false);

                // Attach to node
                mSkyDomeNode->attachObject(mSkyDomeEntity[i]);
            } // for each plane

        }
    }
    //-----------------------------------------------------------------------
    Mesh* SceneManager::createSkyboxPlane(
        BoxPlane bp,
        Real distance,
        const Quaternion& orientation )
    {
        Plane plane;
        String meshName;
        Vector3 up;

        meshName = "SkyBoxPlane_";
        // Set up plane equation
        plane.d = distance;
        switch(bp)
        {
        case BP_FRONT:
            plane.normal = Vector3::UNIT_Z;
            up = Vector3::UNIT_Y;
            meshName += "Front";
            break;
        case BP_BACK:
            plane.normal = -Vector3::UNIT_Z;
            up = Vector3::UNIT_Y;
            meshName += "Back";
            break;
        case BP_LEFT:
            plane.normal = Vector3::UNIT_X;
            up = Vector3::UNIT_Y;
            meshName += "Left";
            break;
        case BP_RIGHT:
            plane.normal = -Vector3::UNIT_X;
            up = Vector3::UNIT_Y;
            meshName += "Right";
            break;
        case BP_UP:
            plane.normal = -Vector3::UNIT_Y;
            up = Vector3::UNIT_Z;
            meshName += "Up";
            break;
        case BP_DOWN:
            plane.normal = Vector3::UNIT_Y;
            up = -Vector3::UNIT_Z;
            meshName += "Down";
            break;
        }
        // Modify by orientation
        plane.normal = orientation * plane.normal;
        up = orientation * up;


        // Check to see if existing plane
        MeshManager& mm = MeshManager::getSingleton();
        Mesh* planeMesh = (Mesh*)mm.getByName(meshName);
        if(planeMesh)
        {
            // destroy existing
            mm.unload(planeMesh);
            delete planeMesh;
        }
        // Create new
        Real planeSize = distance * 2;
        const int BOX_SEGMENTS = 1;
        planeMesh = mm.createPlane(meshName, plane, planeSize, planeSize, BOX_SEGMENTS, BOX_SEGMENTS, false, 1, 1, 1, up);

        //planeMesh->_dumpContents(meshName);

        return planeMesh;

    }
    //-----------------------------------------------------------------------
    Mesh* SceneManager::createSkydomePlane(
        BoxPlane bp,
        Real curvature,
        Real tiling,
        Real distance,
        const Quaternion& orientation )
    {

        Plane plane;
        String meshName;
        Vector3 up;

        meshName = "SkyDomePlane_";
        // Set up plane equation
        plane.d = distance;
        switch(bp)
        {
        case BP_FRONT:
            plane.normal = Vector3::UNIT_Z;
            up = Vector3::UNIT_Y;
            meshName += "Front";
            break;
        case BP_BACK:
            plane.normal = -Vector3::UNIT_Z;
            up = Vector3::UNIT_Y;
            meshName += "Back";
            break;
        case BP_LEFT:
            plane.normal = Vector3::UNIT_X;
            up = Vector3::UNIT_Y;
            meshName += "Left";
            break;
        case BP_RIGHT:
            plane.normal = -Vector3::UNIT_X;
            up = Vector3::UNIT_Y;
            meshName += "Right";
            break;
        case BP_UP:
            plane.normal = -Vector3::UNIT_Y;
            up = Vector3::UNIT_Z;
            meshName += "Up";
            break;
        case BP_DOWN:
            // no down
            return 0;
        }
        // Modify by orientation
        plane.normal = orientation * plane.normal;
        up = orientation * up;

        // Check to see if existing plane
        MeshManager& mm = MeshManager::getSingleton();
        Mesh* planeMesh = (Mesh*)mm.getByName(meshName);
        if(planeMesh)
        {
            // destroy existing
            mm.unload(planeMesh);
            delete planeMesh;
        }
        // Create new
        Real planeSize = distance * 2;
        const int BOX_SEGMENTS = 16;
        planeMesh = mm.createCurvedIllusionPlane(meshName, plane, planeSize, planeSize, curvature, 
			BOX_SEGMENTS, BOX_SEGMENTS, false, 1, tiling, tiling, up, orientation, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
			false, false);

        //planeMesh->_dumpContents(meshName);

        return planeMesh;

    }


    //-----------------------------------------------------------------------
    void SceneManager::_updateSceneGraph(Camera* cam)
    {
        // Cascade down the graph updating transforms & world bounds
        // In this implementation, just update from the root
        // Smarter SceneManager subclasses may choose to update only
        //   certain scene graph branches
        mSceneRoot->_update(true, false);


    }
    //-----------------------------------------------------------------------
    void SceneManager::_findVisibleObjects(Camera* cam, bool onlyShadowCasters)
    {
        // Tell nodes to find, cascade down all nodes
        mSceneRoot->_findVisibleObjects(cam, getRenderQueue(), true, 
            mDisplayNodes, onlyShadowCasters);

    }
    //-----------------------------------------------------------------------
    void SceneManager::_renderVisibleObjects(void)
    {
        // Render each separate queue
        RenderQueue::QueueGroupIterator queueIt = getRenderQueue()->_getQueueGroupIterator();

        // NB only queues which have been created are rendered, no time is wasted
        //   parsing through non-existent queues (even though there are 10 available)

        while (queueIt.hasMoreElements())
        {
            // Get queue group id
            RenderQueueGroupID qId = queueIt.peekNextKey();
            RenderQueueGroup* pGroup = queueIt.getNext();


            bool repeatQueue = false;
            do // for repeating queues
            {
                // Fire queue started event
                if (fireRenderQueueStarted(qId))
                {
                    // Someone requested we skip this queue
                    continue;
                }

				renderQueueGroupObjects(pGroup);
            
                // Fire queue ended event
                if (fireRenderQueueEnded(qId))
                {
                    // Someone requested we repeat this queue
                    repeatQueue = true;
                }
                else
                {
                    repeatQueue = false;
                }
            } while (repeatQueue);

        } // for each queue group

    }
    //-----------------------------------------------------------------------
	void SceneManager::renderAdditiveStencilShadowedQueueGroupObjects(RenderQueueGroup* pGroup)
	{
        RenderQueueGroup::PriorityMapIterator groupIt = pGroup->getIterator();
        LightList lightList;

        while (groupIt.hasMoreElements())
        {
            RenderPriorityGroup* pPriorityGrp = groupIt.getNext();

            // Sort the queue first
            pPriorityGrp->sort(mCameraInProgress);

            // Clear light list
            lightList.clear();

            // Render all the ambient passes first, no light iteration, no lights
            mIlluminationStage = IRS_AMBIENT;
            renderObjects(pPriorityGrp->_getSolidPasses(), false, &lightList);
            // Also render any objects which have receive shadows disabled
            renderObjects(pPriorityGrp->_getSolidPassesNoShadow(), true);


            // Now iterate per light
            mIlluminationStage = IRS_PER_LIGHT;

            // Iterate over lights, render all volumes to stencil
            LightList::const_iterator li, liend;
            liend = mLightsAffectingFrustum.end();

            for (li = mLightsAffectingFrustum.begin(); li != liend; ++li)
            {
                Light* l = *li;
                // Set light state

                if (l->getCastShadows())
                {
                    // Clear stencil
                    mDestRenderSystem->clearFrameBuffer(FBT_STENCIL);
                    renderShadowVolumesToStencil(l, mCameraInProgress);
                    // turn stencil check on
                    mDestRenderSystem->setStencilCheckEnabled(true);
                    // NB we render where the stencil is equal to zero to render lit areas
                    mDestRenderSystem->setStencilBufferParams(CMPF_EQUAL, 0);
                }

                // render lighting passes for this light
                if (lightList.empty())
                    lightList.push_back(l);
                else
                    lightList[0] = l;
                renderObjects(pPriorityGrp->_getSolidPassesDiffuseSpecular(), false, &lightList);

                // Reset stencil params
                mDestRenderSystem->setStencilBufferParams();
                mDestRenderSystem->setStencilCheckEnabled(false);
                mDestRenderSystem->_setDepthBufferParams();

            }// for each light


            // Now render decal passes, no need to set lights as lighting will be disabled
            mIlluminationStage = IRS_DECAL;
            renderObjects(pPriorityGrp->_getSolidPassesDecal(), false);


        }// for each priority

        // reset lighting stage
        mIlluminationStage = IRS_NONE;

        // Iterate again - variable name changed to appease gcc.
        RenderQueueGroup::PriorityMapIterator groupIt2 = pGroup->getIterator();
        while (groupIt2.hasMoreElements())
        {
            RenderPriorityGroup* pPriorityGrp = groupIt2.getNext();

            // Do transparents
            renderObjects(pPriorityGrp->_getTransparentPasses(), true);

        }// for each priority


	}
	//-----------------------------------------------------------------------
	void SceneManager::renderModulativeStencilShadowedQueueGroupObjects(RenderQueueGroup* pGroup)
	{
		/* For each light, we need to render all the solids from each group, 
		then do the modulative shadows, then render the transparents from
		each group.
		Now, this means we are going to reorder things more, but that it required
		if the shadows are to look correct. The overall order is preserved anyway,
		it's just that all the transparents are at the end instead of them being
		interleaved as in the normal rendering loop. 
		*/
		// Iterate through priorities
		RenderQueueGroup::PriorityMapIterator groupIt = pGroup->getIterator();

		while (groupIt.hasMoreElements())
		{
			RenderPriorityGroup* pPriorityGrp = groupIt.getNext();

			// Sort the queue first
			pPriorityGrp->sort(mCameraInProgress);

			// Do (shadowable) solids
			renderObjects(pPriorityGrp->_getSolidPasses(), true);
		}


		// Iterate over lights, render all volumes to stencil
		LightList::const_iterator li, liend;
		liend = mLightsAffectingFrustum.end();

		for (li = mLightsAffectingFrustum.begin(); li != liend; ++li)
		{
			Light* l = *li;
            if (l->getCastShadows())
            {
                // Clear stencil
                mDestRenderSystem->clearFrameBuffer(FBT_STENCIL);
                renderShadowVolumesToStencil(l, mCameraInProgress);
                // render full-screen shadow modulator for all lights
                setPass(mShadowModulativePass);
                // turn stencil check on
                mDestRenderSystem->setStencilCheckEnabled(true);
                // NB we render where the stencil is not equal to zero to render shadows, not lit areas
                mDestRenderSystem->setStencilBufferParams(CMPF_NOT_EQUAL, 0);
                renderSingleObject(mFullScreenQuad, mShadowModulativePass, false);
                // Reset stencil params
                mDestRenderSystem->setStencilBufferParams();
                mDestRenderSystem->setStencilCheckEnabled(false);
                mDestRenderSystem->_setDepthBufferParams();
            }

		}// for each light

        // Iterate again - variable name changed to appease gcc.
        RenderQueueGroup::PriorityMapIterator groupIt2 = pGroup->getIterator();
        while (groupIt2.hasMoreElements())
        {
            RenderPriorityGroup* pPriorityGrp = groupIt2.getNext();

            // Do non-shadowable solids
            renderObjects(pPriorityGrp->_getSolidPassesNoShadow(), true);

        }// for each priority


		// Iterate again - variable name changed to appease gcc.
        RenderQueueGroup::PriorityMapIterator groupIt3 = pGroup->getIterator();
		while (groupIt3.hasMoreElements())
		{
			RenderPriorityGroup* pPriorityGrp = groupIt3.getNext();

			// Do transparents
			renderObjects(pPriorityGrp->_getTransparentPasses(), true);

		}// for each priority

	}
    //-----------------------------------------------------------------------
    void SceneManager::renderTextureShadowCasterQueueGroupObjects(RenderQueueGroup* pGroup)
    {
        static LightList nullLightList;
        // This is like the basic group render, except we skip all transparents
        // and we also render any non-shadowed objects
        // Note that non-shadow casters will have already been eliminated during
        // _findVisibleObjects

        // Iterate through priorities
        RenderQueueGroup::PriorityMapIterator groupIt = pGroup->getIterator();

        // Override auto param ambient to force vertex programs and fixed function to 
        // use shadow colour
        mAutoParamDataSource.setAmbientLightColour(mShadowColour);
        mDestRenderSystem->setAmbientLight(mShadowColour.r, mShadowColour.g, mShadowColour.b);

        while (groupIt.hasMoreElements())
        {
            RenderPriorityGroup* pPriorityGrp = groupIt.getNext();

            // Sort the queue first
            pPriorityGrp->sort(mCameraInProgress);

            // Do solids, override light list incase any vertex programs use them
            renderObjects(pPriorityGrp->_getSolidPasses(), false, &nullLightList);
            renderObjects(pPriorityGrp->_getSolidPassesNoShadow(), false, &nullLightList);

        }// for each priority
        
        // reset ambient light
        mAutoParamDataSource.setAmbientLightColour(mAmbientLight);
        mDestRenderSystem->setAmbientLight(mAmbientLight.r, mAmbientLight.g, mAmbientLight.b);
    }
    //-----------------------------------------------------------------------
    void SceneManager::renderModulativeTextureShadowedQueueGroupObjects(RenderQueueGroup* pGroup)
    {
        /* For each light, we need to render all the solids from each group, 
        then do the modulative shadows, then render the transparents from
        each group.
        Now, this means we are going to reorder things more, but that it required
        if the shadows are to look correct. The overall order is preserved anyway,
        it's just that all the transparents are at the end instead of them being
        interleaved as in the normal rendering loop. 
        */
        // Iterate through priorities
        RenderQueueGroup::PriorityMapIterator groupIt = pGroup->getIterator();

        while (groupIt.hasMoreElements())
        {
            RenderPriorityGroup* pPriorityGrp = groupIt.getNext();

            // Sort the queue first
            pPriorityGrp->sort(mCameraInProgress);

            // Do solids
            renderObjects(pPriorityGrp->_getSolidPasses(), true);
            renderObjects(pPriorityGrp->_getSolidPassesNoShadow(), true);
        }


        // Iterate over lights, render received shadows
        // only perform this if we're in the 'normal' render stage, to avoid
        // doing it during the render to texture
        if (mIlluminationStage == IRS_NONE)
        {
            mIlluminationStage = IRS_RENDER_MODULATIVE_PASS;

            LightList::iterator i, iend;
            ShadowTextureList::iterator si, siend;
            iend = mLightsAffectingFrustum.end();
            siend = mShadowTextures.end();
            for (i = mLightsAffectingFrustum.begin(), si = mShadowTextures.begin();
                i != iend && si != siend; ++i)
            {
                Light* l = *i;
                mCurrentShadowTexture = *si;
                // Hook up receiver texture
                mShadowReceiverPass->getTextureUnitState(0)->setTextureName(
                    mCurrentShadowTexture->getName());
                // Hook up projection frustum
                mShadowReceiverPass->getTextureUnitState(0)->setProjectiveTexturing(
                    true, mCurrentShadowTexture->getViewport(0)->getCamera());
                mAutoParamDataSource.setTextureProjector(
                    mCurrentShadowTexture->getViewport(0)->getCamera());
                // if this light is a spotlight, we need to add the spot fader layer
                if (l->getType() == Light::LT_SPOTLIGHT)
                {
                    // Add spot fader if not present already
                    if (mShadowReceiverPass->getNumTextureUnitStates() == 1)
                    {
                        TextureUnitState* t = 
                            mShadowReceiverPass->createTextureUnitState("spot_shadow_fade.png");
                        t->setProjectiveTexturing(
                            true, mCurrentShadowTexture->getViewport(0)->getCamera());
                        t->setColourOperation(LBO_ADD);
                        t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
                    }
                }
                else if (mShadowReceiverPass->getNumTextureUnitStates() > 1)
                {
                    // remove spot fader layer
                    mShadowReceiverPass->removeTextureUnitState(1);

                }
                mShadowReceiverPass->_load();

                if (l->getCastShadows() && pGroup->getShadowsEnabled())
                {
                    renderTextureShadowReceiverQueueGroupObjects(pGroup);
                }

            }// for each light

            mIlluminationStage = IRS_NONE;

        }

        // Iterate again - variable name changed to appease gcc.
        RenderQueueGroup::PriorityMapIterator groupIt3 = pGroup->getIterator();
        while (groupIt3.hasMoreElements())
        {
            RenderPriorityGroup* pPriorityGrp = groupIt3.getNext();

            // Do transparents
            renderObjects(pPriorityGrp->_getTransparentPasses(), true);

        }// for each priority

    }
    //-----------------------------------------------------------------------
    void SceneManager::renderTextureShadowReceiverQueueGroupObjects(RenderQueueGroup* pGroup)
    {
        static LightList nullLightList;

        // Iterate through priorities
        RenderQueueGroup::PriorityMapIterator groupIt = pGroup->getIterator();

        // Override auto param ambient to force vertex programs to go full-bright
        mAutoParamDataSource.setAmbientLightColour(ColourValue::White);
        mDestRenderSystem->setAmbientLight(1, 1, 1);

        while (groupIt.hasMoreElements())
        {
            RenderPriorityGroup* pPriorityGrp = groupIt.getNext();

            // Do solids, override light list incase any vertex programs use them
            renderObjects(pPriorityGrp->_getSolidPasses(), false, &nullLightList);

            // Don't render transparents or passes which have shadow receipt disabled

        }// for each priority

        // reset ambient
        mAutoParamDataSource.setAmbientLightColour(mAmbientLight);
        mDestRenderSystem->setAmbientLight(mAmbientLight.r, mAmbientLight.g, mAmbientLight.b);

    }
    //-----------------------------------------------------------------------
    bool SceneManager::validatePassForRendering(Pass* pass)
    {
        // Bypass if we're doing a texture shadow render and 
        // this pass is after the first (only 1 pass needed for shadow texture)
        if ((mIlluminationStage == IRS_RENDER_TO_TEXTURE ||
            mIlluminationStage == IRS_RENDER_MODULATIVE_PASS) && 
            pass->getIndex() > 0)
        {
            return false;
        }

        return true;
    }
    //-----------------------------------------------------------------------
    bool SceneManager::validateRenderableForRendering(Pass* pass, Renderable* rend)
    {
        // Skip this renderable if we're doing texture shadows, it casts shadows
        // and we're doing the render receivers pass
        if (mShadowTechnique == SHADOWTYPE_TEXTURE_MODULATIVE && 
            mIlluminationStage == IRS_RENDER_MODULATIVE_PASS && 
            rend->getCastsShadows())
        {
            return false;
        }

        return true;

    }
	//-----------------------------------------------------------------------
	void SceneManager::renderObjects(
        const RenderPriorityGroup::SolidRenderablePassMap& objs, bool doLightIteration, 
        const LightList* manualLightList)
	{
		// ----- SOLIDS LOOP -----
		RenderPriorityGroup::SolidRenderablePassMap::const_iterator ipass, ipassend;
		ipassend = objs.end();
		for (ipass = objs.begin(); ipass != ipassend; ++ipass)
		{
            // Fast bypass if this group is now empty
            if (ipass->second->empty()) continue;

            // Give SM a chance to eliminate this pass
            if (!validatePassForRendering(ipass->first))
                continue;

            // For solids, we try to do each pass in turn
			Pass* usedPass = setPass(ipass->first);
			RenderPriorityGroup::RenderableList* rendList = ipass->second;
			RenderPriorityGroup::RenderableList::const_iterator irend, irendend;
			irendend = rendList->end();
			for (irend = rendList->begin(); irend != irendend; ++irend)
			{
                // Give SM a chance to eliminate
                if (!validateRenderableForRendering(ipass->first, *irend))
                    continue;
				// Render a single object, this will set up auto params if required
				renderSingleObject(*irend, usedPass, doLightIteration, manualLightList);
			}
		} 
	}
	//-----------------------------------------------------------------------
	void SceneManager::renderObjects(
        const RenderPriorityGroup::TransparentRenderablePassList& objs, bool doLightIteration,
        const LightList* manualLightList)
	{
		// ----- TRANSPARENT LOOP -----
		// This time we render by Z, not by pass
		// The mTransparentObjects set needs to be ordered first
		// Render each non-transparent entity in turn, grouped by material
		RenderPriorityGroup::TransparentRenderablePassList::const_iterator itrans, itransend;

		itransend = objs.end();
		for (itrans = objs.begin(); 
			itrans != itransend; ++itrans)
		{
			// For transparents, we have to accept that we can't sort entirely by pass
			setPass(itrans->pass);
			renderSingleObject(itrans->renderable, itrans->pass, doLightIteration, 
                manualLightList);
		}

	}
	//-----------------------------------------------------------------------
	void SceneManager::renderQueueGroupObjects(RenderQueueGroup* pGroup)
	{
		if (pGroup->getShadowsEnabled() && 
			mShadowTechnique == SHADOWTYPE_STENCIL_ADDITIVE)
		{
            // Additive stencil shadows in use
			renderAdditiveStencilShadowedQueueGroupObjects(pGroup);
		}
		else if (pGroup->getShadowsEnabled() && 
			mShadowTechnique == SHADOWTYPE_STENCIL_MODULATIVE)
		{
            // Modulative stencil shadows in use
			renderModulativeStencilShadowedQueueGroupObjects(pGroup);
		}
        else if (mShadowTechnique == SHADOWTYPE_TEXTURE_MODULATIVE)
        {
            // Modulative texture shadows in use
            if (mIlluminationStage == IRS_RENDER_TO_TEXTURE)
            {
                // Shadow caster pass
                if (pGroup->getShadowsEnabled())
                    renderTextureShadowCasterQueueGroupObjects(pGroup);
            }
            else
            {
                // Ordinary pass
                renderModulativeTextureShadowedQueueGroupObjects(pGroup);
            }
        }
		else
		{
            // No shadows, ordinary pass
            renderBasicQueueGroupObjects(pGroup);
        }


	}
    //-----------------------------------------------------------------------
    void SceneManager::renderBasicQueueGroupObjects(RenderQueueGroup* pGroup)
    {
        // Basic render loop
        // Iterate through priorities
        RenderQueueGroup::PriorityMapIterator groupIt = pGroup->getIterator();

        while (groupIt.hasMoreElements())
        {
            RenderPriorityGroup* pPriorityGrp = groupIt.getNext();

            // Sort the queue first
            pPriorityGrp->sort(mCameraInProgress);

            // Do solids
            renderObjects(pPriorityGrp->_getSolidPasses(), true);
            // Do transparents
            renderObjects(pPriorityGrp->_getTransparentPasses(), true);


        }// for each priority
    }
    //-----------------------------------------------------------------------
    void SceneManager::renderSingleObject(Renderable* rend, Pass* pass, 
        bool doLightIteration, const LightList* manualLightList)
    {
        static Matrix4 xform[256];
        unsigned short numMatrices;
        static bool normalisedNormals = false;
        static SceneDetailLevel camDetailLevel = mCameraInProgress->getDetailLevel();
        static SceneDetailLevel lastDetailLevel = camDetailLevel;
        static RenderOperation ro;
		static LightList localLightList;

        if (pass->isProgrammable())
        {
            // Tell auto params object about the renderable change
            mAutoParamDataSource.setCurrentRenderable(rend);
			pass->_updateAutoParamsNoLights(mAutoParamDataSource);
        }
        
        // Set world transformation
        rend->getWorldTransforms(xform);
        numMatrices = rend->getNumWorldTransforms();
        if (numMatrices > 1)
        {
            mDestRenderSystem->_setWorldMatrices(xform, numMatrices);
        }
        else
        {
            mDestRenderSystem->_setWorldMatrix(*xform);
        }

        // Issue view / projection changes if any
        useRenderableViewProjMode(rend);

        // Reissue any texture gen settings which are dependent on view matrix
        Pass::TextureUnitStateIterator texIter =  pass->getTextureUnitStateIterator();
        size_t unit = 0;
        while(texIter.hasMoreElements())
        {
            TextureUnitState* pTex = texIter.getNext();
            if (pTex->hasViewRelativeTextureCoordinateGeneration())
            {
                mDestRenderSystem->_setTextureUnitSettings(unit, *pTex);
            }
            ++unit;
        }


        // Sort out normalisation
        bool thisNormalise = rend->getNormaliseNormals();
        if (thisNormalise != normalisedNormals)
        {
            mDestRenderSystem->setNormaliseNormals(thisNormalise);
            normalisedNormals = thisNormalise;
        }

        // Set up the solid / wireframe override
        SceneDetailLevel reqDetail = rend->getRenderDetail();
        if (reqDetail != lastDetailLevel || reqDetail != camDetailLevel)
        {
            if (reqDetail > camDetailLevel)
            {
                // only downgrade detail; if cam says wireframe we don't go up to solid
                reqDetail = camDetailLevel;
            }
            mDestRenderSystem->_setRasterisationMode(reqDetail);
            lastDetailLevel = reqDetail;

        }

        mDestRenderSystem->setClipPlanes(rend->getClipPlanes());

        // Set up rendering operation
        rend->getRenderOperation(ro);
		ro.srcRenderable = rend;

		if (doLightIteration)
        {
            // Here's where we issue the rendering operation to the render system
		    // Note that we may do this once per light, therefore it's in a loop
		    // and the light parameters are updated once per traversal through the
		    // loop
		    const LightList& rendLightList = rend->getLights();
		    bool iteratePerLight = pass->getRunOncePerLight();
		    size_t numIterations = iteratePerLight ? rendLightList.size() : 1;
		    const LightList* pLightListToUse;
		    for (size_t i = 0; i < numIterations; ++i)
		    {
			    // Determine light list to use
			    if (iteratePerLight)
			    {
				    // Change the only element of local light list to be
				    // the light at index i
				    localLightList.clear();
                    // Check whether we need to filter this one out
                    if (pass->getRunOnlyForOneLightType() && 
                        pass->getOnlyLightType() != rendLightList[i]->getType())
                    {
                        // Skip
                        continue;
                    }

				    localLightList.push_back(rendLightList[i]);
				    pLightListToUse = &localLightList;
			    }
			    else
			    {
				    // Use complete light list
				    pLightListToUse = &rendLightList;
			    }

			    // Do we need to update GPU program parameters?
			    if (pass->isProgrammable())
			    {
				    // Update any automatic gpu params for lights
				    // Other bits of information will have to be looked up
                    mAutoParamDataSource.setCurrentLightList(pLightListToUse);
				    pass->_updateAutoParamsLightsOnly(mAutoParamDataSource);
				    // NOTE: We MUST bind parameters AFTER updating the autos
				    // TEST
				    if (pass->hasVertexProgram())
				    {
					    mDestRenderSystem->bindGpuProgramParameters(GPT_VERTEX_PROGRAM, 
						    pass->getVertexProgramParameters());
				    }
				    if (pass->hasFragmentProgram())
				    {
					    mDestRenderSystem->bindGpuProgramParameters(GPT_FRAGMENT_PROGRAM, 
						    pass->getFragmentProgramParameters());
				    }
			    }
			    // Do we need to update light states? 
			    // Only do this if fixed-function vertex lighting applies
			    if (pass->getLightingEnabled() && !pass->hasVertexProgram())
			    {
				    mDestRenderSystem->_useLights(*pLightListToUse, pass->getMaxSimultaneousLights());
			    }
			    // issue the render op		
			    mDestRenderSystem->_render(ro);
		    } // possibly iterate per light
        }
        else // no automatic light processing
        {
            // Do we need to update GPU program parameters?
            if (pass->isProgrammable())
            {
                // Do we have a manual light list?
                if (manualLightList)
                {
                    // Update any automatic gpu params for lights
                    mAutoParamDataSource.setCurrentLightList(manualLightList);
                    pass->_updateAutoParamsLightsOnly(mAutoParamDataSource);
                }

                if (pass->hasVertexProgram())
                {
                    mDestRenderSystem->bindGpuProgramParameters(GPT_VERTEX_PROGRAM, 
                        pass->getVertexProgramParameters());
                }
                if (pass->hasFragmentProgram())
                {
                    mDestRenderSystem->bindGpuProgramParameters(GPT_FRAGMENT_PROGRAM, 
                        pass->getFragmentProgramParameters());
                }
            }

            // Use manual lights if present, and not using vertex programs
            if (manualLightList && 
                pass->getLightingEnabled() && !pass->hasVertexProgram())
            {
                mDestRenderSystem->_useLights(*manualLightList, pass->getMaxSimultaneousLights());
            }
            // issue the render op		
            mDestRenderSystem->_render(ro);
        }
    }
    //-----------------------------------------------------------------------
    void SceneManager::setAmbientLight(const ColourValue& colour)
    {
        mAmbientLight = colour;
        mDestRenderSystem->setAmbientLight(colour.r, colour.g, colour.b);
    }
    //-----------------------------------------------------------------------
    const ColourValue& SceneManager::getAmbientLight(void) const
    {
        return mAmbientLight;
    }
    //-----------------------------------------------------------------------
    ViewPoint SceneManager::getSuggestedViewpoint(bool random)
    {
        // By default return the origin
        ViewPoint vp;
        vp.position = Vector3::ZERO;
        vp.orientation = Quaternion::IDENTITY;
        return vp;
    }
    //-----------------------------------------------------------------------
    void SceneManager::setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
    {
        mFogMode = mode;
        mFogColour = colour;
        mFogStart = start;
        mFogEnd = end;
        mFogDensity = density;
    }
    //-----------------------------------------------------------------------
    FogMode SceneManager::getFogMode(void) const
    {
        return mFogMode;
    }
    //-----------------------------------------------------------------------
    const ColourValue& SceneManager::getFogColour(void) const
    {
        return mFogColour;
    }
    //-----------------------------------------------------------------------
    Real SceneManager::getFogStart(void) const
    {
        return mFogStart;
    }
    //-----------------------------------------------------------------------
    Real SceneManager::getFogEnd(void) const
    {
        return mFogEnd;
    }
    //-----------------------------------------------------------------------
    Real SceneManager::getFogDensity(void) const
    {
        return mFogDensity;
    }
    //-----------------------------------------------------------------------
    BillboardSet* SceneManager::createBillboardSet(const String& name, unsigned int poolSize)
    {
        BillboardSet* set = new BillboardSet( name, poolSize );
        mBillboardSets[name] = set;//.insert(BillboardSetList::value_type(name, set));

        return set;
    }
    //-----------------------------------------------------------------------
    BillboardSet* SceneManager::getBillboardSet(const String& name)
    {
        BillboardSetList::iterator i = mBillboardSets.find(name);
        if (i == mBillboardSets.end())
        {
            return 0;
        }
        else
        {
            return i->second;
        }
    }
    //-----------------------------------------------------------------------
    void SceneManager::removeBillboardSet(BillboardSet* set)
    {
        // Find in list
        BillboardSetList::iterator i = mBillboardSets.begin();
        for (; i != mBillboardSets.end(); ++i)
        {
            if (i->second == set)
            {
                mBillboardSets.erase(i);
                delete set;
                break;
            }
        }

    }
    //-----------------------------------------------------------------------
    void SceneManager::removeBillboardSet(const String& name)
    {
        // Find in list
        BillboardSetList::iterator i = mBillboardSets.find(name);
        if (i != mBillboardSets.end())
        {
            delete i->second;
            mBillboardSets.erase(i);
        }
    }
    //-----------------------------------------------------------------------
    void SceneManager::setDisplaySceneNodes(bool display)
    {
        mDisplayNodes = display;
    }
    //-----------------------------------------------------------------------
    Animation* SceneManager::createAnimation(const String& name, Real length)
    {
        Animation* pAnim = new Animation(name, length);
        mAnimationsList[name] = pAnim;
        return pAnim;
    }
    //-----------------------------------------------------------------------
    Animation* SceneManager::getAnimation(const String& name) const
    {
        AnimationList::const_iterator i = mAnimationsList.find(name);
        if (i == mAnimationsList.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find animation with name " + name, 
                "SceneManager::getAnimation");
        }
        return i->second;
    }
    //-----------------------------------------------------------------------
    void SceneManager::destroyAnimation(const String& name)
    {
        // Also destroy any animation states referencing this animation
        AnimationStateSet::iterator si, siend;
        siend = mAnimationStates.end();
        for (si = mAnimationStates.begin(); si != siend; )
        {
            if (si->second.getAnimationName() == name)
            {
                // erase, post increment to avoid the invalidated iterator
                mAnimationStates.erase(si++);
            }
            else
            {
                ++si;
            }
        }

        AnimationList::iterator i = mAnimationsList.find(name);
        if (i == mAnimationsList.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find animation with name " + name, 
                "SceneManager::getAnimation");
        }

        // Free memory
        delete i->second;

        mAnimationsList.erase(i);


    }
    //-----------------------------------------------------------------------
    void SceneManager::destroyAllAnimations(void)
    {
        // Destroy all states too, since they cannot reference destroyed animations
        destroyAllAnimationStates();

        AnimationList::iterator i;
        for (i = mAnimationsList.begin(); i != mAnimationsList.end(); ++i)
        {
            // destroy
            delete i->second;
        }
        mAnimationsList.clear();
    }
    //-----------------------------------------------------------------------
    AnimationState* SceneManager::createAnimationState(const String& animName)
    {
        if (mAnimationStates.find(animName) != mAnimationStates.end())
        {
            Except(Exception::ERR_DUPLICATE_ITEM, 
                "Cannot create, AnimationState already exists: "+animName, 
                "SceneManager::createAnimationState");
        }

        // Get animation, this will throw an exception if not found
        Animation* anim = getAnimation(animName);

        // Create new state
        AnimationState newState(animName, 0, anim->getLength());

        // Record it
        std::pair<AnimationStateSet::iterator, bool> retPair = 
            mAnimationStates.insert(AnimationStateSet::value_type(animName, newState));

        // Check boolean return
        if (retPair.second)
        {
            // insert was OK
            // Get pointer from iterator in pair
            return &(retPair.first->second);
        }
        else
        {
            // Problem
            // Not because of duplicate item, that's checked for above
            Except(Exception::ERR_INTERNAL_ERROR, "Unexpected error creating new animation state.",
                "SceneManager::createAnimationState");
        }


    }
    //-----------------------------------------------------------------------
    AnimationState* SceneManager::getAnimationState(const String& animName) 
    {
        AnimationStateSet::iterator i = mAnimationStates.find(animName);

        if (i == mAnimationStates.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot locate animation state for animation " + animName,
                "SceneManager::getAnimationState");
        }

        return &(i->second);

    }
    //-----------------------------------------------------------------------
    void SceneManager::destroyAnimationState(const String& name)
    {
        AnimationStateSet::iterator i = mAnimationStates.find(name);

        if (i == mAnimationStates.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot locate animation state for animation " + name,
                "SceneManager::destroyAnimationState");
        }

        mAnimationStates.erase(i);


    }
    //-----------------------------------------------------------------------
    void SceneManager::destroyAllAnimationStates(void)
    {
        mAnimationStates.clear();
    }
    //-----------------------------------------------------------------------
    void SceneManager::_applySceneAnimations(void)
    {
        AnimationStateSet::const_iterator i, iend;

        i = mAnimationStates.begin();
        iend = mAnimationStates.end();
    
        for (;i != iend; ++i)
        {
			if (i->second.getEnabled())
			{
				Animation* anim = getAnimation(i->second.getAnimationName());

				// Reset any nodes involved
				// NB this excludes blended animations
				const Animation::TrackList& trackList = anim->_getTrackList();
				Animation::TrackList::const_iterator ti, tend;
				ti = trackList.begin();
				tend = trackList.end();
				for (;ti != tend; ++ti)
				{
					Node* nd = ti->second->getAssociatedNode();
					nd->resetToInitialState();
				}


				// Apply the animation
				anim->apply(i->second.getTimePosition(), i->second.getWeight());
			}
        }


    }
    //---------------------------------------------------------------------
    void SceneManager::manualRender(RenderOperation* rend, 
        Pass* pass, Viewport* vp, const Matrix4& worldMatrix, 
        const Matrix4& viewMatrix, const Matrix4& projMatrix, 
        bool doBeginEndFrame) 
    {
        mDestRenderSystem->_setViewport(vp);
        mDestRenderSystem->_setWorldMatrix(worldMatrix);
        mDestRenderSystem->_setViewMatrix(viewMatrix);
        mDestRenderSystem->_setProjectionMatrix(projMatrix);

        if (doBeginEndFrame)
            mDestRenderSystem->_beginFrame();

        setPass(pass);
        mDestRenderSystem->_render(*rend);

        if (doBeginEndFrame)
            mDestRenderSystem->_endFrame();
        
    }
    //---------------------------------------------------------------------
    Overlay* SceneManager::createOverlay(const String& name, ushort zorder)
    {
        /*
        // check not existing
        OverlayList::iterator i = mOverlays.find(name);
        if (i != mOverlays.end())
        {
            Except(Exception::ERR_DUPLICATE_ITEM, 
                "An overlay named " + name + " already exists.",
                "SceneManager::createOverlay");
        }
        Overlay *newOverlay = new Overlay(name, zorder);

        mOverlays.insert(OverlayList::value_type(name, newOverlay));
        return newOverlay;
        */

        Overlay* newOverlay = (Overlay*)OverlayManager::getSingleton().create(name);
        newOverlay->setZOrder(zorder);
        return newOverlay;



    }
    //---------------------------------------------------------------------
    Overlay* SceneManager::getOverlay(const String& name)
    {
        /*
        OverlayList::iterator i = mOverlays.find(name);
        if (i == mOverlays.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "An overlay named " + name + " cannot be found.",
                "SceneManager::getOverlay");
        }

        return i->second;
        */
        Overlay* ret = (Overlay*)OverlayManager::getSingleton().getByName(name);
        if (!ret)
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "An overlay named " + name + " cannot be found.",
                "SceneManager::getOverlay");
        }

        return ret;

    }
    //---------------------------------------------------------------------
    void SceneManager::destroyOverlay(const String& name)
    {
        /*
        OverlayList::iterator i = mOverlays.find(name);
        if (i == mOverlays.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "An overlay named " + name + " cannot be found.",
                "SceneManager::destroyOverlay");
        }

        delete i->second;
        mOverlays.erase(i);
        */
        Overlay* pOver = (Overlay*)OverlayManager::getSingleton().getByName(name);
        if (!pOver)
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "An overlay named " + name + " cannot be found.",
                "SceneManager::destroyOverlay");
        }
        OverlayManager::getSingleton().unload(pOver);
        delete pOver;

    }
    //---------------------------------------------------------------------
    void SceneManager::destroyAllOverlays(void)
    {
        /*
        OverlayList::iterator i, iend;
        iend = mOverlays.end();
        for (i = mOverlays.begin(); i != iend; ++i)
        {
            delete i->second;
        }
        mOverlays.clear();
        */
        OverlayManager::getSingleton().unloadAndDestroyAll();


    }
    //---------------------------------------------------------------------
    void SceneManager::useRenderableViewProjMode(Renderable* pRend)
    {
        // Check view matrix
        static bool lastViewWasIdentity = false;
        bool useIdentityView = pRend->useIdentityView();
        if (useIdentityView && (mCamChanged || !lastViewWasIdentity))
        {
            // Using identity view now, change it
            mDestRenderSystem->_setViewMatrix(Matrix4::IDENTITY);
            lastViewWasIdentity = true;
        }
        else if (!useIdentityView && (mCamChanged || lastViewWasIdentity))
        {
            // Coming back to normal from identity view
            mDestRenderSystem->_setViewMatrix(mCameraInProgress->getViewMatrix());
            lastViewWasIdentity = false;
        }
        
        static bool lastProjWasIdentity = false;
        bool useIdentityProj = pRend->useIdentityProjection();

        if (useIdentityProj && (mCamChanged || !lastProjWasIdentity))
        {
            mDestRenderSystem->_setProjectionMatrix(Matrix4::IDENTITY);

            lastProjWasIdentity = true;
        }
        else if (!useIdentityProj && (mCamChanged || lastProjWasIdentity))
        {
            // Coming back from flat projection
            mDestRenderSystem->_setProjectionMatrix(mCameraInProgress->getProjectionMatrix());
            lastProjWasIdentity = false;
        }

        mCamChanged = false;

    }

    //---------------------------------------------------------------------
    void SceneManager::_queueSkiesForRendering(Camera* cam)
    {
        // Update nodes
        // Translate the box by the camera position (constant distance)
        if (mSkyPlaneNode)
        {
            // The plane position relative to the camera has already been set up
            mSkyPlaneNode->setPosition(cam->getDerivedPosition());
        }

        if (mSkyBoxNode)
        {
            mSkyBoxNode->setPosition(cam->getDerivedPosition());
        }

        if (mSkyDomeNode)
        {
            mSkyDomeNode->setPosition(cam->getDerivedPosition());
        }

        RenderQueueGroupID qid;
        if (mSkyPlaneEnabled)
        {
            qid = mSkyPlaneDrawFirst? 
                        RENDER_QUEUE_SKIES_EARLY : RENDER_QUEUE_SKIES_LATE;
            getRenderQueue()->addRenderable(mSkyPlaneEntity->getSubEntity(0), qid, RENDERABLE_DEFAULT_PRIORITY);
        }

        uint plane;
        if (mSkyBoxEnabled)
        {
            qid = mSkyBoxDrawFirst? 
                        RENDER_QUEUE_SKIES_EARLY : RENDER_QUEUE_SKIES_LATE;

            for (plane = 0; plane < 6; ++plane)
            {
                getRenderQueue()->addRenderable(
                    mSkyBoxEntity[plane]->getSubEntity(0), qid, RENDERABLE_DEFAULT_PRIORITY);
            }
        }

        if (mSkyDomeEnabled)
        {
            qid = mSkyDomeDrawFirst? 
                        RENDER_QUEUE_SKIES_EARLY : RENDER_QUEUE_SKIES_LATE;

            for (plane = 0; plane < 5; ++plane)
            {
                getRenderQueue()->addRenderable(
                    mSkyDomeEntity[plane]->getSubEntity(0), qid, RENDERABLE_DEFAULT_PRIORITY);
            }
        }
    }
    //---------------------------------------------------------------------
    void SceneManager::addRenderQueueListener(RenderQueueListener* newListener)
    {
        mRenderQueueListeners.push_back(newListener);
    }
    //---------------------------------------------------------------------
    void SceneManager::removeRenderQueueListener(RenderQueueListener* delListener)
    {
        RenderQueueListenerList::iterator i, iend;
        iend = mRenderQueueListeners.end();
        for (i = mRenderQueueListeners.begin(); i != iend; ++i)
        {
            if (*i == delListener)
            {
                mRenderQueueListeners.erase(i);
                break;
            }
        }

    }
    //---------------------------------------------------------------------
    bool SceneManager::fireRenderQueueStarted(RenderQueueGroupID id)
    {
        RenderQueueListenerList::iterator i, iend;
        bool skip = false;

        iend = mRenderQueueListeners.end();
        for (i = mRenderQueueListeners.begin(); i != iend; ++i)
        {
            (*i)->renderQueueStarted(id, skip);
        }
        return skip;
    }
    //---------------------------------------------------------------------
    bool SceneManager::fireRenderQueueEnded(RenderQueueGroupID id)
    {
        RenderQueueListenerList::iterator i, iend;
        bool repeat = false;

        iend = mRenderQueueListeners.end();
        for (i = mRenderQueueListeners.begin(); i != iend; ++i)
        {
            (*i)->renderQueueEnded(id, repeat);
        }
        return repeat;
    }
    //---------------------------------------------------------------------
    void SceneManager::setViewport(Viewport* vp)
    {
        mCurrentViewport = vp;
        // Set viewport in render system
        mDestRenderSystem->_setViewport(vp);
    }
	//---------------------------------------------------------------------
	void SceneManager::showBoundingBoxes(bool bShow) 
	{
		mShowBoundingBoxes = bShow;
	}
	//---------------------------------------------------------------------
	bool SceneManager::getShowBoundingBoxes() const
	{
		return mShowBoundingBoxes;
	}
    //---------------------------------------------------------------------
    void SceneManager::_notifyAutotrackingSceneNode(SceneNode* node, bool autoTrack)
    {
        if (autoTrack)
        {
            mAutoTrackingSceneNodes.insert(node);
        }
        else
        {
            mAutoTrackingSceneNodes.erase(node);
        }
    }
	//---------------------------------------------------------------------
    void SceneManager::setShadowTechnique(ShadowTechnique technique)
    {
        mShadowTechnique = technique;
        if (technique == SHADOWTYPE_STENCIL_ADDITIVE || 
            technique == SHADOWTYPE_STENCIL_MODULATIVE)
        {
            // Firstly check that we  have a stencil
            // Otherwise forget it
            if (!mDestRenderSystem->getCapabilities()->hasCapability(RSC_HWSTENCIL))
            {
                LogManager::getSingleton().logMessage(
                    "WARNING: Stencil shadows were requested, but this device does not "
                    "have a hardware stencil. Shadows disabled.");
                mShadowTechnique = SHADOWTYPE_NONE;
            }
            else
            {
                // Create an estimated sized shadow index buffer
                mShadowIndexBuffer = HardwareBufferManager::getSingleton().
                    createIndexBuffer(HardwareIndexBuffer::IT_16BIT, 
					    mShadowIndexBufferSize, 
                	    HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
					    false);
                // tell all meshes to prepare shadow volumes
                MeshManager::getSingleton().setPrepareAllMeshesForShadowVolumes(true);
            }
        }

        if (mShadowTechnique == SHADOWTYPE_STENCIL_ADDITIVE)
        {
            // Additive stencil, we need to split everything by illumination stage
            getRenderQueue()->setSplitPassesByLightingType(true);
        }
        else
        {
            getRenderQueue()->setSplitPassesByLightingType(false);
        }

        if (mShadowTechnique != SHADOWTYPE_NONE)
        {
            // Tell render queue to split off non-shadowable materials
            getRenderQueue()->setSplitNoShadowPasses(true);
        }
        else
        {
            getRenderQueue()->setSplitNoShadowPasses(false);
        }

        if (mShadowTechnique == SHADOWTYPE_TEXTURE_MODULATIVE 
            /* || mShadowTechnique == SHADOWTYPE_TEXTURE_SHADOWMAP */)
        {
            createShadowTextures(mShadowTextureSize, mShadowTextureCount);
        }

    }
	//---------------------------------------------------------------------
    void SceneManager::findLightsAffectingFrustum(const Camera* camera)
    {
        // Basic iteration for this SM
        mLightsAffectingFrustum.clear();
        SceneLightList::iterator i, iend;
        iend = mLights.end();
        Sphere sphere;
        for (i = mLights.begin(); i != iend; ++i)
        {
            Light* l = i->second;
            if (l->getType() == Light::LT_DIRECTIONAL)
            {
                // Always visible
                mLightsAffectingFrustum.push_back(l);
            }
            else
            {
                // NB treating spotlight as point for simplicity
                // Just see if the lights attenuation range is within the frustum
                sphere.setCenter(l->getDerivedPosition());
                sphere.setRadius(l->getAttenuationRange());
                if (camera->isVisible(sphere))
                {
                    mLightsAffectingFrustum.push_back(l);
                }
                
            }
        }

    }
	//---------------------------------------------------------------------
    bool SceneManager::ShadowCasterSceneQueryListener::queryResult(
        MovableObject* object)
    {
        if (object->getCastShadows() && object->isVisible())
        {
            if (mFarDistSquared)
            {
                // Check object is within the shadow far distance
                Vector3 toObj = object->getParentNode()->_getDerivedPosition() 
                    - mCamera->getDerivedPosition();
                Real radius = object->getWorldBoundingSphere().getRadius();
                Real dist =  toObj.squaredLength();               
                if (dist - (radius * radius) > mFarDistSquared)
                {
                    // skip, beyond max range
                    return true;
                }
            }

            // If the object is in the frustum, we can always see the shadow
            if (mCamera->isVisible(object->getWorldBoundingBox()))
            {
                mCasterList->push_back(object);
                return true;
            }

            // Otherwise, object can only be casting a shadow into our view if
            // the light is outside the frustum (or it's a directional light, 
            // which are always outside), and the object is intersecting
            // on of the volumes formed between the edges of the frustum and the
            // light
            if (!mIsLightInFrustum || mLight->getType() == Light::LT_DIRECTIONAL)
            {
                // Iterate over volumes
                PlaneBoundedVolumeList::const_iterator i, iend;
                iend = mLightClipVolumeList->end();
                for (i = mLightClipVolumeList->begin(); i != iend; ++i)
                {
                    if (i->intersects(object->getWorldBoundingBox()))
                    {
                        mCasterList->push_back(object);
                        return true;
                    }

                }

            }
        }
        return true;
    }
	//---------------------------------------------------------------------
    bool SceneManager::ShadowCasterSceneQueryListener::queryResult(
        SceneQuery::WorldFragment* fragment)
    {
        // don't deal with world geometry
        return true;
    }
	//---------------------------------------------------------------------
    const SceneManager::ShadowCasterList& SceneManager::findShadowCastersForLight(
        const Light* light, const Camera* camera)
    {
        mShadowCasterList.clear();

        if (light->getType() == Light::LT_DIRECTIONAL)
        {
            // Basic AABB query encompassing the frustum and the extrusion of it
            AxisAlignedBox aabb;
            const Vector3* corners = camera->getWorldSpaceCorners();
            Vector3 min, max;
            Vector3 extrude = light->getDirection() * -mShadowDirLightExtrudeDist;
            // do first corner
            min = max = corners[0];
            min.makeFloor(corners[0] + extrude);
            max.makeCeil(corners[0] + extrude);
            for (size_t c = 1; c < 8; ++c)
            {
                min.makeFloor(corners[c]);
                max.makeCeil(corners[c]);
                min.makeFloor(corners[c] + extrude);
                max.makeCeil(corners[c] + extrude);
            }
            aabb.setExtents(min, max);

            if (!mShadowCasterAABBQuery)
                mShadowCasterAABBQuery = createAABBQuery(aabb);
            else
                mShadowCasterAABBQuery->setBox(aabb);
            // Execute, use callback
            mShadowCasterQueryListener.prepare(false, 
                &(light->_getFrustumClipVolumes(camera)), 
                light, camera, &mShadowCasterList, mShadowFarDistSquared);
            mShadowCasterAABBQuery->execute(&mShadowCasterQueryListener);


        }
        else
        {
            Sphere s(light->getPosition(), light->getAttenuationRange());
            // eliminate early if camera cannot see light sphere
            if (camera->isVisible(s))
            {
                if (!mShadowCasterSphereQuery)
                    mShadowCasterSphereQuery = createSphereQuery(s);
                else
                    mShadowCasterSphereQuery->setSphere(s);

                // Determine if light is inside or outside the frustum
                bool lightInFrustum = camera->isVisible(light->getDerivedPosition());
                const PlaneBoundedVolumeList* volList = 0;
                if (!lightInFrustum)
                {
                    // Only worth building an external volume list if
                    // light is outside the frustum
                    volList = &(light->_getFrustumClipVolumes(camera));
                }

                // Execute, use callback
                mShadowCasterQueryListener.prepare(lightInFrustum, 
                    volList, light, camera, &mShadowCasterList, mShadowFarDistSquared);
                mShadowCasterSphereQuery->execute(&mShadowCasterQueryListener);

            }

        }


        return mShadowCasterList;
    }
    //---------------------------------------------------------------------
    void SceneManager::initShadowVolumeMaterials(void)
    {
        Material* matDebug = static_cast<Material*>(
            MaterialManager::getSingleton().getByName("Ogre/Debug/ShadowVolumes"));
        if (!matDebug)
        {
            // Create
            matDebug = static_cast<Material*>(
                MaterialManager::getSingleton().create("Ogre/Debug/ShadowVolumes"));
            mShadowDebugPass = matDebug->getTechnique(0)->getPass(0);
            mShadowDebugPass->setSceneBlending(SBT_ADD); 
            mShadowDebugPass->setLightingEnabled(false);
            mShadowDebugPass->setDepthWriteEnabled(false);
            TextureUnitState* t = mShadowDebugPass->createTextureUnitState();
            t->setColourOperationEx(LBX_MODULATE, LBS_MANUAL, LBS_CURRENT, 
                ColourValue(0.7, 0.0, 0.2));
            mShadowDebugPass->setCullingMode(CULL_NONE);
            if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM))
            {
                // load hardware extrusion programs for point & dir lights
                if (GpuProgramManager::getSingleton().isSyntaxSupported("arbvp1"))
                {
                    // ARBvp1
                    if (!GpuProgramManager::getSingleton().getByName("Ogre/ShadowExtrudePointLightDebug"))
                    {
                        GpuProgram* vp = 
                            GpuProgramManager::getSingleton().createProgramFromString(
                            "Ogre/ShadowExtrudePointLightDebug",
                            ShadowVolumeExtrudeProgram::getPointLightExtruderArbvp1Debug(), 
                            GPT_VERTEX_PROGRAM, "arbvp1");
                        vp->load();
                    }
                    if (!GpuProgramManager::getSingleton().getByName("Ogre/ShadowExtrudeDirLightDebug"))
                    {
                        GpuProgram* vp = 
                            GpuProgramManager::getSingleton().createProgramFromString(
                            "Ogre/ShadowExtrudeDirLightDebug",
                            ShadowVolumeExtrudeProgram::getDirectionalLightExtruderArbvp1Debug(), 
                            GPT_VERTEX_PROGRAM, "arbvp1");
                        vp->load();
                    }
                }
                else if (GpuProgramManager::getSingleton().isSyntaxSupported("vs_1_1"))
                {
                    // vs_1_1
                    if (!GpuProgramManager::getSingleton().getByName("Ogre/ShadowExtrudePointLightDebug"))
                    {
                        GpuProgram* vp = 
                            GpuProgramManager::getSingleton().createProgramFromString(
                            "Ogre/ShadowExtrudePointLightDebug",
                            ShadowVolumeExtrudeProgram::getPointLightExtruderVs_1_1Debug(), 
                            GPT_VERTEX_PROGRAM, "vs_1_1");
                        vp->load();
                    }
                    if (!GpuProgramManager::getSingleton().getByName("Ogre/ShadowExtrudeDirLightDebug"))
                    {
                        GpuProgram* vp = 
                            GpuProgramManager::getSingleton().createProgramFromString(
                            "Ogre/ShadowExtrudeDirLightDebug",
                            ShadowVolumeExtrudeProgram::getDirectionalLightExtruderVs_1_1Debug(), 
                            GPT_VERTEX_PROGRAM, "vs_1_1");
                        vp->load();
                    }

                }
                else
                {
                    Except(Exception::ERR_INTERNAL_ERROR, 
                        "Vertex programs are supposedly supported, but neither "
                        "arbvp1 nor vs_1_1 syntaxes are present.", 
                        "SceneManager::initShadowVolumeMaterials");
                }

                // Enable the point light extruder for now, just to get some params
                mShadowDebugPass->setVertexProgram("Ogre/ShadowExtrudePointLightDebug");
                GpuProgramParametersSharedPtr extrusionParams = 
                    mShadowDebugPass->getVertexProgramParameters();
                extrusionParams->setAutoConstant(0, 
                    GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
                extrusionParams->setAutoConstant(4, 
                    GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE);


            }
            matDebug->compile();
        }

        Material* matStencil = static_cast<Material*>(
            MaterialManager::getSingleton().getByName("Ogre/StencilShadowVolumes"));
        if (!matStencil)
        {
            // Init
            matStencil = static_cast<Material*>(
                MaterialManager::getSingleton().create("Ogre/StencilShadowVolumes"));
            mShadowStencilPass = matStencil->getTechnique(0)->getPass(0);
            if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM))
            {
                // load hardware extrusion programs for point & dir lights
                if (GpuProgramManager::getSingleton().isSyntaxSupported("arbvp1"))
                {
                    // ARBvp1
                    if (!GpuProgramManager::getSingleton().getByName("Ogre/ShadowExtrudePointLight"))
                    {
                        GpuProgram* vp = 
                            GpuProgramManager::getSingleton().createProgramFromString(
                            "Ogre/ShadowExtrudePointLight",
                            ShadowVolumeExtrudeProgram::getPointLightExtruderArbvp1(), 
                            GPT_VERTEX_PROGRAM, "arbvp1");
                        vp->load();
                    }
                    if (!GpuProgramManager::getSingleton().getByName("Ogre/ShadowExtrudeDirLight"))
                    {
                        GpuProgram* vp = 
                            GpuProgramManager::getSingleton().createProgramFromString(
                            "Ogre/ShadowExtrudeDirLight",
                            ShadowVolumeExtrudeProgram::getDirectionalLightExtruderArbvp1(), 
                            GPT_VERTEX_PROGRAM, "arbvp1");
                        vp->load();
                    }
                }
                else if (GpuProgramManager::getSingleton().isSyntaxSupported("vs_1_1"))
                {
                    // vs_1_1
                    if (!GpuProgramManager::getSingleton().getByName("Ogre/ShadowExtrudePointLight"))
                    {
                        GpuProgram* vp = 
                            GpuProgramManager::getSingleton().createProgramFromString(
                            "Ogre/ShadowExtrudePointLight",
                            ShadowVolumeExtrudeProgram::getPointLightExtruderVs_1_1(), 
                            GPT_VERTEX_PROGRAM, "vs_1_1");
                        vp->load();
                    }
                    if (!GpuProgramManager::getSingleton().getByName("Ogre/ShadowExtrudeDirLight"))
                    {
                        GpuProgram* vp = 
                            GpuProgramManager::getSingleton().createProgramFromString(
                            "Ogre/ShadowExtrudeDirLight",
                            ShadowVolumeExtrudeProgram::getDirectionalLightExtruderVs_1_1(), 
                            GPT_VERTEX_PROGRAM, "vs_1_1");
                        vp->load();
                    }

                }
                else
                {
                    Except(Exception::ERR_INTERNAL_ERROR, 
                        "Vertex programs are supposedly supported, but neither "
                        "arbvp1 nor vs_1_1 syntaxes are present.", 
                        "SceneManager::initShadowVolumeMaterials");
                }
                // Enable hardware extrusion
                // Enable the point light extruder for now, just to get some params
                mShadowStencilPass->setVertexProgram("Ogre/ShadowExtrudePointLight");
                GpuProgramParametersSharedPtr extrusionParams = 
                    mShadowStencilPass->getVertexProgramParameters();
                extrusionParams->setAutoConstant(0, 
                    GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
                extrusionParams->setAutoConstant(4, 
                    GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE);


            }
            // Nothing else, we don't use this like a 'real' pass anyway,
            // it's more of a placeholder
        }

        Material* matModStencil = static_cast<Material*>(
            MaterialManager::getSingleton().getByName("Ogre/StencilShadowModulationPass"));
        if (!matModStencil)
        {
            // Init
            matModStencil = static_cast<Material*>(
                MaterialManager::getSingleton().create("Ogre/StencilShadowModulationPass"));
            mShadowModulativePass = matModStencil->getTechnique(0)->getPass(0);
            mShadowModulativePass->setSceneBlending(SBF_DEST_COLOUR, SBF_ZERO); 
            mShadowModulativePass->setLightingEnabled(false);
            mShadowModulativePass->setDepthWriteEnabled(false);
            mShadowModulativePass->setDepthCheckEnabled(false);
            TextureUnitState* t = mShadowModulativePass->createTextureUnitState();
            t->setColourOperationEx(LBX_MODULATE, LBS_MANUAL, LBS_CURRENT, 
                mShadowColour);

        }

        // Also init full screen quad while we're at it
        if (!mFullScreenQuad)
        {
            mFullScreenQuad = new Rectangle2D();
            mFullScreenQuad->setCorners(-1,1,1,-1);
        }

        // Also init shadow caster material for texture shadows
        Material* matPlainBlack = static_cast<Material*>(
            MaterialManager::getSingleton().getByName("Ogre/TextureShadowCaster"));
        if (!matPlainBlack)
        {
            matPlainBlack = static_cast<Material*>(
                MaterialManager::getSingleton().create("Ogre/TextureShadowCaster"));
            mShadowCasterPlainBlackPass = matPlainBlack->getTechnique(0)->getPass(0);
            // Lighting has to be on, because we need shadow coloured objects
            // Note that because we can't predict vertex programs, we'll have to
            // bind light values to those, and so we bind White to ambient
            // reflectance, and we'll set the ambient colour to the shadow colour
            mShadowCasterPlainBlackPass->setAmbient(ColourValue::White);
            mShadowCasterPlainBlackPass->setDiffuse(ColourValue::Black);
            mShadowCasterPlainBlackPass->setSelfIllumination(ColourValue::Black);
            mShadowCasterPlainBlackPass->setSpecular(ColourValue::Black);
            // no textures or anything else, we will bind vertex programs
            // every so often though
        }

        Material* matShadRec = static_cast<Material*>(
            MaterialManager::getSingleton().getByName("Ogre/TextureShadowReceiver"));
        if (!matShadRec)
        {
            matShadRec = static_cast<Material*>(
                MaterialManager::getSingleton().create("Ogre/TextureShadowReceiver"));
            mShadowReceiverPass = matShadRec->getTechnique(0)->getPass(0);
            mShadowReceiverPass->setSceneBlending(SBF_DEST_COLOUR, SBF_ZERO);
            // No lighting, one texture unit 
            // everything else will be bound as needed during the receiver pass
            mShadowReceiverPass->setLightingEnabled(false);
            TextureUnitState* t = mShadowReceiverPass->createTextureUnitState();
            t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
        }

        // Set up spot shadow fade texture (loaded from code data block)
        Texture* spotShadowFadeTex = (Texture*) 
            TextureManager::getSingleton().getByName("spot_shadow_fade.png");
        if (!spotShadowFadeTex)
        {
            // Load the manual buffer into an image
            DataChunk chunk(SPOT_SHADOW_FADE_PNG, SPOT_SHADOW_FADE_PNG_SIZE);
            Image img;
            img.load(chunk, "png");
            spotShadowFadeTex = 
                TextureManager::getSingleton().loadImage("spot_shadow_fade.png", img, TEX_TYPE_2D);
        }




    }
    //---------------------------------------------------------------------
    Pass* SceneManager::deriveShadowCasterPass(Pass* pass)
    {
        switch (mShadowTechnique)
        {
        case SHADOWTYPE_TEXTURE_MODULATIVE:
            if (pass->hasVertexProgram())
            {
                // Have to merge the shadow caster vertex program in
                // This may in fact be blank, in which case it falls back on 
                // fixed function
                mShadowCasterPlainBlackPass->setVertexProgram(
                    pass->getShadowCasterVertexProgramName());
                // Did this result in a new vertex program?
                if (mShadowCasterPlainBlackPass->hasVertexProgram())
                {
                    GpuProgram* prg = mShadowCasterPlainBlackPass->getVertexProgram();
                    // Load this program if not done already
                    if (!prg->isLoaded())
                        prg->load();
                    // Copy params
                    mShadowCasterPlainBlackPass->setVertexProgramParameters(
                        pass->getShadowCasterVertexProgramParameters());
                }
                // Also have to hack the light autoparams, that is done later
            }
            else if (mShadowCasterPlainBlackPass->hasVertexProgram())
            {
                // reset
                mShadowCasterPlainBlackPass->setVertexProgram("");
            }
            return mShadowCasterPlainBlackPass;
        /*
        case SHADOWTYPE_TEXTURE_SHADOWMAP:
            // todo
            return pass;
        */
        default:
            return pass;
        };

    }
    //---------------------------------------------------------------------
    Pass* SceneManager::deriveShadowReceiverPass(Pass* pass)
    {

        switch (mShadowTechnique)
        {
        case SHADOWTYPE_TEXTURE_MODULATIVE:
            if (pass->hasVertexProgram())
            {
                // Have to merge the receiver vertex program in
                // This may return "" which means fixed function will be used
                mShadowReceiverPass->setVertexProgram(
                    pass->getShadowReceiverVertexProgramName());
                // Did this result in a new vertex program?
                if (mShadowReceiverPass->hasVertexProgram())
                {
                    GpuProgram* prg = mShadowReceiverPass->getVertexProgram();
                    // Load this program if required
                    if (!prg->isLoaded())
                        prg->load();
                    // Copy params
                    mShadowReceiverPass->setVertexProgramParameters(
                        pass->getShadowReceiverVertexProgramParameters());
                }
                // Also have to hack the light autoparams, that is done later
            }
            else if (mShadowReceiverPass->hasVertexProgram())
            {
                // reset
                mShadowReceiverPass->setVertexProgram("");

            }

            return mShadowReceiverPass;
        /*
        case SHADOWTYPE_TEXTURE_SHADOWMAP:
            // todo
            return pass;
        */
        default:
            return pass;
        };

    }
    //---------------------------------------------------------------------
    void SceneManager::renderShadowVolumesToStencil(const Light* light, const Camera* camera)
    {

        // Set up scissor test (point & spot lights only)
        bool scissored = false;
        if (light->getType() != Light::LT_DIRECTIONAL && 
            mDestRenderSystem->getCapabilities()->hasCapability(RSC_SCISSOR_TEST))
        {
            // Project the sphere onto the camera
            Real left, right, top, bottom;
            Sphere sphere(light->getDerivedPosition(), light->getAttenuationRange());
            if (camera->projectSphere(sphere, &left, &top, &right, &bottom))
            {
                scissored = true;
                // Turn normalised device coordinates into pixels
                int iLeft, iTop, iWidth, iHeight;
                mCurrentViewport->getActualDimensions(iLeft, iTop, iWidth, iHeight);
                size_t szLeft, szRight, szTop, szBottom;

                szLeft = iLeft + ((left + 1) * 0.5 * iWidth);
                szRight = iLeft + ((right + 1) * 0.5 * iWidth);
                szTop = iTop + ((-top + 1) * 0.5 * iHeight);
                szBottom = iTop + ((-bottom + 1) * 0.5 * iHeight);

                mDestRenderSystem->setScissorTest(true, szLeft, szTop, szRight, szBottom);

            }

        }



        mDestRenderSystem->unbindGpuProgram(GPT_FRAGMENT_PROGRAM);

        // Can we do a 2-sided stencil?
        bool stencil2sided = false;
        if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_TWO_SIDED_STENCIL) && 
            mDestRenderSystem->getCapabilities()->hasCapability(RSC_STENCIL_WRAP))
        {
            // enable
            stencil2sided = true;
        }

        // Do we have access to vertex programs?
        bool extrudeInSoftware = true;
        if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM))
        {
            extrudeInSoftware = false;
            // attach the appropriate extrusion vertex program
            // Note we never unset it because support for vertex programs is constant
            if (light->getType() == Light::LT_DIRECTIONAL)
            {
                mShadowStencilPass->setVertexProgram("Ogre/ShadowExtrudeDirLight", false);
                if (mDebugShadows)
                {
                    mShadowDebugPass->setVertexProgram("Ogre/ShadowExtrudeDirLightDebug", false);
                }
            }
            else
            {
                mShadowStencilPass->setVertexProgram("Ogre/ShadowExtrudePointLight", false);
                if (mDebugShadows)
                {
                    mShadowDebugPass->setVertexProgram("Ogre/ShadowExtrudePointLightDebug", false);
                }
            }

            mDestRenderSystem->bindGpuProgram(mShadowStencilPass->getVertexProgram()->_getBindingDelegate());

        }
        else
        {
            mDestRenderSystem->unbindGpuProgram(GPT_VERTEX_PROGRAM);
        }

        // Add light to internal list for use in render call
        LightList lightList;
        // const_cast is forgiveable here since we pass this const
        lightList.push_back(const_cast<Light*>(light));

        // Turn off colour writing and depth writing
        mDestRenderSystem->_setColourBufferWriteEnabled(false, false, false, false);
        mDestRenderSystem->_setDepthBufferWriteEnabled(false);
        mDestRenderSystem->setStencilCheckEnabled(true);
        mDestRenderSystem->_setDepthBufferFunction(CMPF_LESS);

        // Calculate extrusion distance
        Real extrudeDist;
        if (light->getType() == Light::LT_DIRECTIONAL)
        {
            extrudeDist = mShadowDirLightExtrudeDist;
        }
        else
        {
            extrudeDist = light->getAttenuationRange(); 
        }

        // Figure out the near clip volume
        const PlaneBoundedVolume& nearClipVol = 
            light->_getNearClipVolume(camera);

        // Get the shadow caster list
        const ShadowCasterList& casters = findShadowCastersForLight(light, camera);
        ShadowCasterList::const_iterator si, siend;
        siend = casters.end();

        // Determine whether zfail is required
        // We need to use zfail for ALL objects if we find a single object which
        // requires it
        bool zfailAlgo = false;
        unsigned long flags = 0;

        for (si = casters.begin(); si != siend; ++si)
        {
            ShadowCaster* caster = *si;

            if (nearClipVol.intersects(caster->getWorldBoundingBox()))
            {
                // We have a zfail case, we must use zfail for all objects
                zfailAlgo = true;
                break;
            }
        }

        // Now iterate over the casters and render
        for (si = casters.begin(); si != siend; ++si)
        {
            ShadowCaster* caster = *si;
            flags = 0;

            if (!extrudeInSoftware)
            {
                // hardware extrusion, to infinity (and beyond!)
                flags |= SRF_EXTRUDE_TO_INFINITY;
            }

            if (zfailAlgo)
            {
                // We need to include the light and / or dark cap
                // But only if they will be visible
                if(camera->isVisible(caster->getLightCapBounds()))
                {
                    flags |= SRF_INCLUDE_LIGHT_CAP;
                }
            }
            // Dark cap (no dark cap for directional lights using hardware extrusion)
            if(!(!extrudeInSoftware && light->getType() == Light::LT_DIRECTIONAL) &&
                camera->isVisible(caster->getDarkCapBounds(*light, extrudeDist)))
            {
                flags |= SRF_INCLUDE_DARK_CAP;
            }

            // Get shadow renderables
            ShadowCaster::ShadowRenderableListIterator iShadowRenderables =
                caster->getShadowVolumeRenderableIterator(mShadowTechnique,
                light, &mShadowIndexBuffer, extrudeInSoftware, 
                extrudeDist, flags);

            while (iShadowRenderables.hasMoreElements())
            {
                ShadowRenderable* sr = iShadowRenderables.getNext();
                // omit hidden renderables
                if (sr->isVisible())
                {
                    // render volume, including dark and (maybe) light caps
                    renderSingleShadowVolumeToStencil(sr, zfailAlgo, stencil2sided, &lightList);

                    // optionally render separate light cap
                    if (sr->isLightCapSeparate() &&
                        (flags & SRF_INCLUDE_LIGHT_CAP))
                    {
                        // must always fail depth check
                        mDestRenderSystem->_setDepthBufferFunction(CMPF_ALWAYS_FAIL);
                        assert(sr->getLightCapRenderable() && "Shadow renderable is "
                            "missing a separate light cap renderable!");
                        renderSingleShadowVolumeToStencil(sr->getLightCapRenderable(),
                            zfailAlgo, stencil2sided, &lightList);
                        // reset depth function
                        mDestRenderSystem->_setDepthBufferFunction(CMPF_LESS);
                    }
                }
            }
        }
		// revert colour write state
		mDestRenderSystem->_setColourBufferWriteEnabled(true, true, true, true);
		// revert depth state
		mDestRenderSystem->_setDepthBufferParams();

        mDestRenderSystem->setStencilCheckEnabled(false);

        mDestRenderSystem->unbindGpuProgram(GPT_VERTEX_PROGRAM);

        if (scissored)
        {
            // disable scissor test
            mDestRenderSystem->setScissorTest(false);
        }

    }
    //---------------------------------------------------------------------
    void SceneManager::renderSingleShadowVolumeToStencil(ShadowRenderable* sr,
        bool zfailAlgo, bool stencil2sided, const LightList *manualLightList)
    {
        // Render a shadow volume here
        //  - if we have 2-sided stencil, one render with no culling
        //  - otherwise, 2 renders, one with each culling method and invert the ops
        setShadowVolumeStencilState(false, zfailAlgo, stencil2sided);
        renderSingleObject(sr, mShadowStencilPass, false, manualLightList);

        if (!stencil2sided)
        {
            // Second pass
            setShadowVolumeStencilState(true, zfailAlgo, false);
            renderSingleObject(sr, mShadowStencilPass, false);
        }

        // Do we need to render a debug shadow marker?
        if (mDebugShadows)
        {
            // reset stencil & colour ops
            mDestRenderSystem->setStencilBufferParams();
            setPass(mShadowDebugPass);
            renderSingleObject(sr, mShadowDebugPass, false, manualLightList);
            mDestRenderSystem->_setColourBufferWriteEnabled(false, false, false, false);
        }
    }
    //---------------------------------------------------------------------
    void SceneManager::setShadowVolumeStencilState(bool secondpass, bool zfail, bool twosided)
    {
        // First pass, do front faces if zpass
        // Second pass, do back faces if zpass
        // Invert if zfail
        // this is to ensure we always increment before decrement
        if ( (secondpass || zfail) &&
            !(secondpass && zfail) )
        {
            mDestRenderSystem->_setCullingMode(
                twosided? CULL_NONE : CULL_ANTICLOCKWISE);
            mDestRenderSystem->setStencilBufferParams(
                CMPF_ALWAYS_PASS, // always pass stencil check
                0, // no ref value (no compare)
                0xFFFFFFFF, // no mask
                SOP_KEEP, // stencil test will never fail
                zfail? (twosided? SOP_INCREMENT_WRAP : SOP_INCREMENT) : SOP_KEEP, // back face depth fail
                zfail? SOP_KEEP : (twosided? SOP_DECREMENT_WRAP : SOP_DECREMENT), // back face pass
                twosided
                );
        }
        else
        {
            mDestRenderSystem->_setCullingMode(
                twosided? CULL_NONE : CULL_CLOCKWISE);
            mDestRenderSystem->setStencilBufferParams(
                CMPF_ALWAYS_PASS, // always pass stencil check
                0, // no ref value (no compare)
                0xFFFFFFFF, // no mask
                SOP_KEEP, // stencil test will never fail
                zfail? (twosided? SOP_DECREMENT_WRAP : SOP_DECREMENT) : SOP_KEEP, // front face depth fail
                zfail? SOP_KEEP : (twosided? SOP_INCREMENT_WRAP : SOP_INCREMENT), // front face pass
                twosided
                );
        }
    }
    //---------------------------------------------------------------------
    void SceneManager::setShadowColour(const ColourValue& colour)
    {
        mShadowColour = colour;

        if (!mShadowModulativePass && !mShadowCasterPlainBlackPass)
            initShadowVolumeMaterials();

        mShadowModulativePass->getTextureUnitState(0)->setColourOperationEx(
            LBX_MODULATE, LBS_MANUAL, LBS_CURRENT, colour);
    }
    //---------------------------------------------------------------------
    const ColourValue& SceneManager::getShadowColour(void) const
    {
        return mShadowColour;
    }
    //---------------------------------------------------------------------
    void SceneManager::setShadowFarDistance(Real distance)
    {
        mShadowFarDist = distance;
        mShadowFarDistSquared = distance * distance;
    }
    //---------------------------------------------------------------------
    void SceneManager::setShadowDirectionalLightExtrusionDistance(Real dist)
    {
        mShadowDirLightExtrudeDist = dist;
    }
    //---------------------------------------------------------------------
    Real SceneManager::getShadowDirectionalLightExtrusionDistance(void) const
    {
        return mShadowDirLightExtrudeDist;
    }
    //---------------------------------------------------------------------
 	void SceneManager::setShadowIndexBufferSize(size_t size)
	{
		if (!mShadowIndexBuffer.isNull() && size != mShadowIndexBufferSize)
		{
            // re-create shadow buffer with new size
            mShadowIndexBuffer = HardwareBufferManager::getSingleton().
                createIndexBuffer(HardwareIndexBuffer::IT_16BIT, 
					mShadowIndexBufferSize, 
                	HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
					false);
		}
		mShadowIndexBufferSize = size;
	}
    //---------------------------------------------------------------------
    void SceneManager::setShadowTextureSize(unsigned short size)
    {
        if (!mShadowTextures.empty() && size != mShadowTextureSize)
        {
            // recreate
            createShadowTextures(size, mShadowTextureCount);
        }
        mShadowTextureSize = size;
    }
    //---------------------------------------------------------------------
    void SceneManager::setShadowTextureCount(unsigned short count)
    {
        if (!mShadowTextures.empty() && count != mShadowTextureCount)
        {
            // recreate
            createShadowTextures(mShadowTextureSize, count);
        }
        mShadowTextureCount = count;
    }
    //---------------------------------------------------------------------
    void SceneManager::setShadowTextureSettings(unsigned short size, unsigned short count)
    {
        if (!mShadowTextures.empty() && 
            (count != mShadowTextureCount ||
             size != mShadowTextureSize))
        {
            // recreate
            createShadowTextures(size, count);
        }
        mShadowTextureCount = count;
        mShadowTextureSize = size;
    }
    //---------------------------------------------------------------------
    void SceneManager::createShadowTextures(unsigned short size, unsigned short count)
    {
        static const String baseName = "Ogre/ShadowTexture";

        // destroy existing
        ShadowTextureList::iterator i, iend;
        iend = mShadowTextures.end();
        for (i = mShadowTextures.begin(); i != iend; ++i)
        {
            RenderTexture* r = *i;
            // remove camera and destroy texture
            removeCamera(r->getViewport(0)->getCamera());
            mDestRenderSystem->destroyRenderTexture(r->getName());
        }
        mShadowTextures.clear();

        // Recreate shadow textures
        for (unsigned short t = 0; t < mShadowTextureCount; ++t)
        {
            String targName = baseName + StringConverter::toString(t);
            String matName = baseName + "Mat" + StringConverter::toString(t);
            String camName = baseName + "Cam" + StringConverter::toString(t);

            RenderTexture* shadowTex;
            if (mShadowTechnique == SHADOWTYPE_TEXTURE_MODULATIVE)
            {
                shadowTex = mDestRenderSystem->createRenderTexture( 
                    targName, size, size );
            }
            /*
            else if (mShadowTechnique == SHADOWTYPE_TEXTURE_SHADOWMAP)
            {
                // todo
            }
            */

            // Create a camera to go with this texture
            Camera* cam = createCamera(camName);
            cam->setAspectRatio(1.0f);
            // Create a viewport
            Viewport *v = shadowTex->addViewport(cam);
            v->setClearEveryFrame(true);
            // remove overlays
            v->setOverlaysEnabled(false);
            // Don't update automatically - we'll do it when required
            shadowTex->setAutoUpdated(false);
            mShadowTextures.push_back(shadowTex);

            // Also create corresponding Material used for rendering this shadow
            Material* mat = (Material*)MaterialManager::getSingleton().getByName(matName);
            if (!mat)
            {
                mat = (Material*)MaterialManager::getSingleton().create(matName);
            }
            else
            {
                mat->getTechnique(0)->getPass(0)->removeAllTextureUnitStates();
            }
            // create texture unit referring to render target texture
            TextureUnitState* texUnit = 
                mat->getTechnique(0)->getPass(0)->createTextureUnitState(targName);
            // set projective based on camera
            texUnit->setProjectiveTexturing(true, cam);
            texUnit->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
            mat->touch();

        }
    }
    //---------------------------------------------------------------------
    void SceneManager::prepareShadowTextures(Camera* cam, Viewport* vp)
    {
        // Set the illumination stage, prevents recursive calls
        IlluminationRenderStage savedStage = mIlluminationStage;
        mIlluminationStage = IRS_RENDER_TO_TEXTURE;

        // Determine far shadow distance
        Real shadowDist = mShadowFarDist;
        if (!shadowDist)
        {
            // need a shadow distance, make one up
            shadowDist = cam->getNearClipDistance() * 300;
        }
        // set fogging to hide the shadow edge
        Real shadowOffset = shadowDist * mShadowTextureOffset;
        Real shadowEnd = shadowDist + shadowOffset;
        mShadowReceiverPass->setFog(true, FOG_LINEAR, ColourValue::White, 
            0, shadowEnd * mShadowTextureFadeStart, shadowEnd * mShadowTextureFadeEnd);

        // Iterate over the lights we've found, max out at the limit of light textures

        LightList::iterator i, iend;
        ShadowTextureList::iterator si, siend;
        iend = mLightsAffectingFrustum.end();
        siend = mShadowTextures.end();
        for (i = mLightsAffectingFrustum.begin(), si = mShadowTextures.begin();
             i != iend && si != siend; ++i)
        {
            Light* light = *i;
            RenderTexture* shadowTex = *si;
            // Skip non-shadowing lights
            if (!light->getCastShadows())
                continue;

            // Directional lights 
            if (light->getType() == Light::LT_DIRECTIONAL)
            {

                // set up the shadow texture
                Camera* texCam = shadowTex->getViewport(0)->getCamera();
                // Set ortho projection
                texCam->setProjectionType(PT_ORTHOGRAPHIC);
                // set easy FOV and near dist so that texture covers far dist
                texCam->setFOVy(90);
                texCam->setNearClipDistance(shadowDist);

                // Set size of projection

                // Calculate look at position
                // We want to look at a spot shadowOffset away from near plane
                // 0.5 is a litle too close for angles
                Vector3 target = cam->getDerivedPosition() + 
                    (cam->getDerivedDirection() * shadowOffset);

                // Calculate position
                // We want to be in the -ve direction of the light direction
                // far enough to project for the dir light extrusion distance
                Vector3 pos = target + 
                    (light->getDerivedDirection() * -mShadowDirLightExtrudeDist);

                // Calculate orientation
                Vector3 dir = (pos - target); // backwards since point down -z
                dir.normalise();
                /*
                // Next section (camera oriented shadow map) abandoned
                // Always point in the same direction, if we don't do this then
                // we get 'shadow swimming' as camera rotates
                // As it is, we get swimming on moving but this is less noticeable

                // calculate up vector, we want it aligned with cam direction
                Vector3 up = cam->getDerivedDirection();
                // Check it's not coincident with dir
                if (up.dotProduct(dir) >= 1.0f)
                {
                    // Use camera up
                    up = cam->getUp();
                }
                */
                Vector3 up = Vector3::UNIT_Y;
                // Check it's not coincident with dir
                if (up.dotProduct(dir) >= 1.0f)
                {
                    // Use camera up
                    up = Vector3::UNIT_Z;
                }
                // cross twice to rederive, only direction is unaltered
                Vector3 left = dir.crossProduct(up);
                left.normalise();
                up = dir.crossProduct(left);
                up.normalise();
                // Derive quaternion from axes
                Quaternion q;
                q.FromAxes(left, up, dir);
                texCam->setOrientation(q);

                // Round local x/y position based on a world-space texel; this helps to reduce
                // jittering caused by the projection moving with the camera
                // Viewport is 2 * near clip distance across (90 degree fov)
                Real worldTexelSize = (texCam->getNearClipDistance() * 20) / mShadowTextureSize;
                pos.x -= fmod(pos.x, worldTexelSize);
                pos.y -= fmod(pos.y, worldTexelSize);
                pos.z -= fmod(pos.z, worldTexelSize);
                // Finally set position
                texCam->setPosition(pos);

                if (mShadowTechnique == SHADOWTYPE_TEXTURE_MODULATIVE)
                    shadowTex->getViewport(0)->setBackgroundColour(ColourValue::White);

                // Update target
                shadowTex->update();

                ++si;
            }
            // Spotlight
            else if (light->getType() == Light::LT_SPOTLIGHT)
            {

                // set up the shadow texture
                Camera* texCam = shadowTex->getViewport(0)->getCamera();
                // Set perspective projection
                texCam->setProjectionType(PT_PERSPECTIVE);
                // set FOV slightly larger than the spotlight range to ensure coverage
                texCam->setFOVy(light->getSpotlightOuterAngle()*1.2);
                texCam->setPosition(light->getDerivedPosition());
                texCam->setDirection(light->getDerivedDirection());
                // set near clip the same as main camera, since they are likely
                // to both reflect the nature of the scene
                texCam->setNearClipDistance(cam->getNearClipDistance());

                if (mShadowTechnique == SHADOWTYPE_TEXTURE_MODULATIVE)
                    shadowTex->getViewport(0)->setBackgroundColour(ColourValue::White);

                // Update target
                shadowTex->update();

                ++si;
            }
        }
        // Set the illumination stage, prevents recursive calls
        mIlluminationStage = savedStage;
    }
    //---------------------------------------------------------------------
    AxisAlignedBoxSceneQuery* 
    SceneManager::createAABBQuery(const AxisAlignedBox& box, unsigned long mask)
    {
        DefaultAxisAlignedBoxSceneQuery* q = new DefaultAxisAlignedBoxSceneQuery(this);
        q->setBox(box);
        q->setQueryMask(mask);
        return q;
    }
	//---------------------------------------------------------------------
    SphereSceneQuery* 
    SceneManager::createSphereQuery(const Sphere& sphere, unsigned long mask)
    {
        DefaultSphereSceneQuery* q = new DefaultSphereSceneQuery(this);
        q->setSphere(sphere);
        q->setQueryMask(mask);
        return q;
    }
    //---------------------------------------------------------------------
    PlaneBoundedVolumeListSceneQuery* 
    SceneManager::createPlaneBoundedVolumeQuery(const PlaneBoundedVolumeList& volumes, 
        unsigned long mask)
    {
        DefaultPlaneBoundedVolumeListSceneQuery* q = new DefaultPlaneBoundedVolumeListSceneQuery(this);
        q->setVolumes(volumes);
        q->setQueryMask(mask);
        return q;
    }

	//---------------------------------------------------------------------
    RaySceneQuery* 
    SceneManager::createRayQuery(const Ray& ray, unsigned long mask)
    {
        DefaultRaySceneQuery* q = new DefaultRaySceneQuery(this);
        q->setRay(ray);
        q->setQueryMask(mask);
        return q;
    }
	//---------------------------------------------------------------------
    IntersectionSceneQuery* 
    SceneManager::createIntersectionQuery(unsigned long mask)
    {
        
        DefaultIntersectionSceneQuery* q = new DefaultIntersectionSceneQuery(this);
        q->setQueryMask(mask);
        return q;
    }
	//---------------------------------------------------------------------
    void SceneManager::destroyQuery(SceneQuery* query)
    {
        delete query;
    }
	//---------------------------------------------------------------------
    DefaultIntersectionSceneQuery::DefaultIntersectionSceneQuery(SceneManager* creator)
        : IntersectionSceneQuery(creator)
    {
        // No world geometry results supported
        mSupportedWorldFragments.insert(SceneQuery::WFT_NONE);
    }
	//---------------------------------------------------------------------
    DefaultIntersectionSceneQuery::~DefaultIntersectionSceneQuery()
    {
    }
	//---------------------------------------------------------------------
    void DefaultIntersectionSceneQuery::execute(IntersectionSceneQueryListener* listener)
    {
        // TODO: BillboardSets? Will need per-billboard collision most likely
        // Entities only for now
		SceneManager::EntityList::const_iterator a, b, theEnd;
        theEnd = mParentSceneMgr->mEntities.end();
        int numEntities;
        // Loop a from first to last-1
        a = mParentSceneMgr->mEntities.begin();
        numEntities = (uint)mParentSceneMgr->mEntities.size();
        for (int i = 0; i < (numEntities - 1); ++i, ++a)
        {
            // Skip if a does not pass the mask
            if (! (a->second->getQueryFlags() & mQueryMask))
                continue;

            // Loop b from a+1 to last
            b = a;
            for (++b; b != theEnd; ++b)
            {
                // Apply mask to b (both must pass)
                if (b->second->getQueryFlags() & mQueryMask)
                {
                    const AxisAlignedBox& box1 = a->second->getWorldBoundingBox();
                    const AxisAlignedBox& box2 = b->second->getWorldBoundingBox();

                    if (box1.intersects(box2))
                    {
                        listener->queryResult(a->second, b->second);
                    }
                }

            }
        }
    }
	//---------------------------------------------------------------------
    DefaultAxisAlignedBoxSceneQuery::
    DefaultAxisAlignedBoxSceneQuery(SceneManager* creator)
    : AxisAlignedBoxSceneQuery(creator)
    {
        // No world geometry results supported
        mSupportedWorldFragments.insert(SceneQuery::WFT_NONE);
    }
	//---------------------------------------------------------------------
    DefaultAxisAlignedBoxSceneQuery::~DefaultAxisAlignedBoxSceneQuery()
    {
    }
	//---------------------------------------------------------------------
    void DefaultAxisAlignedBoxSceneQuery::execute(SceneQueryListener* listener)
    {
        // TODO: BillboardSets? Will need per-billboard collision most likely
        // Entities only for now
		SceneManager::EntityList::const_iterator i, iEnd;
        iEnd = mParentSceneMgr->mEntities.end();
        for (i = mParentSceneMgr->mEntities.begin(); i != iEnd; ++i)
        {
            if (mAABB.intersects(i->second->getWorldBoundingBox()))
            {
                listener->queryResult(i->second);
            }
        }
    }
	//---------------------------------------------------------------------
    DefaultRaySceneQuery::
    DefaultRaySceneQuery(SceneManager* creator) : RaySceneQuery(creator)
    {
        // No world geometry results supported
        mSupportedWorldFragments.insert(SceneQuery::WFT_NONE);
    }
	//---------------------------------------------------------------------
    DefaultRaySceneQuery::~DefaultRaySceneQuery()
    {
    }
	//---------------------------------------------------------------------
    void DefaultRaySceneQuery::execute(RaySceneQueryListener* listener)
    {
        // Note that becuase we have no scene partitioning, we actually
        // perform a complete scene search even if restricted results are
        // requested; smarter scene manager queries can utilise the paritioning 
        // of the scene in order to reduce the number of intersection tests 
        // required to fulfil the query

        // TODO: BillboardSets? Will need per-billboard collision most likely
        // Entities only for now
		SceneManager::EntityList::const_iterator i, iEnd;
        iEnd = mParentSceneMgr->mEntities.end();
        for (i = mParentSceneMgr->mEntities.begin(); i != iEnd; ++i)
        {
            // Do ray / box test
            std::pair<bool, Real> result = 
                mRay.intersects(i->second->getWorldBoundingBox());

            if (result.first)
            {
                listener->queryResult(i->second, result.second);
            }
        }

    }
	//---------------------------------------------------------------------
    DefaultSphereSceneQuery::
    DefaultSphereSceneQuery(SceneManager* creator) : SphereSceneQuery(creator)
    {
        // No world geometry results supported
        mSupportedWorldFragments.insert(SceneQuery::WFT_NONE);
    }
	//---------------------------------------------------------------------
    DefaultSphereSceneQuery::~DefaultSphereSceneQuery()
    {
    }
	//---------------------------------------------------------------------
    void DefaultSphereSceneQuery::execute(SceneQueryListener* listener)
    {
        // TODO: BillboardSets? Will need per-billboard collision most likely
        // Entities only for now
		SceneManager::EntityList::const_iterator i, iEnd;
        iEnd = mParentSceneMgr->mEntities.end();
        Sphere testSphere;
        for (i = mParentSceneMgr->mEntities.begin(); i != iEnd; ++i)
        {
            // Skip unattached
            if (!i->second->getParentNode())
                continue;

            // Do sphere / sphere test
            testSphere.setCenter(i->second->getParentNode()->_getDerivedPosition());
            testSphere.setRadius(i->second->getBoundingRadius());
            if (mSphere.intersects(testSphere))
            {
                listener->queryResult(i->second);
            }
        }
    }
    //---------------------------------------------------------------------
    DefaultPlaneBoundedVolumeListSceneQuery::
        DefaultPlaneBoundedVolumeListSceneQuery(SceneManager* creator) 
        : PlaneBoundedVolumeListSceneQuery(creator)
    {
        // No world geometry results supported
        mSupportedWorldFragments.insert(SceneQuery::WFT_NONE);
    }
    //---------------------------------------------------------------------
    DefaultPlaneBoundedVolumeListSceneQuery::~DefaultPlaneBoundedVolumeListSceneQuery()
    {
    }
    //---------------------------------------------------------------------
    void DefaultPlaneBoundedVolumeListSceneQuery::execute(SceneQueryListener* listener)
    {
        // Entities only for now
        SceneManager::EntityList::const_iterator i, iEnd;
        iEnd = mParentSceneMgr->mEntities.end();
        Sphere testSphere;
        for (i = mParentSceneMgr->mEntities.begin(); i != iEnd; ++i)
        {
            PlaneBoundedVolumeList::iterator pi, piend;
            piend = mVolumes.end();
            for (pi = mVolumes.begin(); pi != piend; ++pi)
            {
                PlaneBoundedVolume& vol = *pi;
                // Do AABB / plane volume test
                if (vol.intersects(i->second->getWorldBoundingBox()))
                {
                    listener->queryResult(i->second);
                    break;
                }
            }
        }
    }

}
