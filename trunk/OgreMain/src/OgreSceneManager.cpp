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

// This class implements the most basic scene manager

#include <cstdio>

namespace Ogre {


    SceneManager::SceneManager()
    {
        // Root scene node
        mSceneRoot = new SceneNode(this, "root node");

        // No sky by default
        mSkyPlaneEnabled = false;
        mSkyBoxEnabled = false;
        mSkyDomeEnabled = false;

        // init sky
        mSkyPlaneEntity = 0;
        uint i;
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
        mFullScreenQuad = 0;


		// init render queues that do not need shadows
		mRenderQueue.getQueueGroup(RENDER_QUEUE_BACKGROUND)->setShadowsEnabled(false);
		mRenderQueue.getQueueGroup(RENDER_QUEUE_OVERLAY)->setShadowsEnabled(false);
        mRenderQueue.getQueueGroup(RENDER_QUEUE_SKIES_EARLY)->setShadowsEnabled(false);
        mRenderQueue.getQueueGroup(RENDER_QUEUE_SKIES_LATE)->setShadowsEnabled(false);
    }

    SceneManager::~SceneManager()
    {
        clearScene();
        delete mSceneRoot;
        delete mFullScreenQuad;
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
    void SceneManager::clearScene(void)
    {
        // Delete all SceneNodes, except root that is
        for (SceneNodeList::iterator i = mSceneNodes.begin();
            i != mSceneNodes.end(); ++i)
        {
            delete i->second;
        }
        mSceneNodes.clear();

        // Clear root node of all children
        mSceneRoot->removeAllChildren();
        mSceneRoot->detachAllObjects();

        // Delete all entities
        for (EntityList::iterator ei = mEntities.begin();
            ei != mEntities.end(); ++ei)
        {
            delete ei->second;
        }
        mEntities.clear();

        // Delete all Cameras
        for (CameraList::iterator ci = mCameras.begin();
            ci != mCameras.end(); ++ci)
        {
            delete ci->second;
        }
        mCameras.clear();

        // Delete all BillboardSets
        for (BillboardSetList::iterator bi = mBillboardSets.begin();
            bi != mBillboardSets.end(); ++bi)
        {
            delete bi->second;
        }
        mBillboardSets.clear();
        // Clear lights
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
    void SceneManager::setPass(Pass* pass)
    {
		static bool lastUsedVertexProgram = false;
		static bool lastUsedFragmentProgram = false;

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

    }
    //-----------------------------------------------------------------------
    void SceneManager::_renderScene(Camera* camera, Viewport* vp, bool includeOverlays)
    {
        Root::getSingleton()._setCurrentSceneManager(this);
		// Prep Pass for use in debug shadows
		initShadowVolumeMaterials();

        mCameraInProgress = camera;
        mCamChanged = true;


        // Set the viewport
        setViewport(vp);

        // Tell params about camera
        mAutoParamDataSource.setCurrentCamera(camera);

		// Tell params about current ambient light
		mAutoParamDataSource.setAmbientLightColour(mAmbientLight);


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

        if (camera->isReflected())
        {
            mDestRenderSystem->setInvertVertexWinding(true);
        }
        else
        {
            mDestRenderSystem->setInvertVertexWinding(false);
        }

        // Set camera window clipping planes (if any)
		if (mDestRenderSystem->getCapabilities()->hasCapability(RSC_USER_CLIP_PLANES))
		{
			if (camera->isWindowSet())  
			{
				const std::vector<Plane>& planeList = 
					camera->getWindowPlanes();
				for (int i = 0; i < 4; ++i)
				{
					mDestRenderSystem->enableClipPlane(i, true);
					mDestRenderSystem->setClipPlane(i, planeList[i]);
				}
			}
			else
			{
				for (int i = 0; i < 4; ++i)
				{
					mDestRenderSystem->enableClipPlane(i, false);
				}
			}
		}
        // Clear the render queue
        mRenderQueue.clear();

        // Are we using any shadows at all?
        if (mShadowTechnique != SHADOWTYPE_NONE)
        {
            // Locate any lights which could be affecting the frustum
            findLightsAffectingFrustum(camera);
        }
        // Deal with shadow setup
        if (mShadowTechnique == SHADOWTYPE_STENCIL_ADDITIVE)
        {
            // Additive stencil, we need to split everything by light
            // TODO - add a different queue handler to do this
        }

        // Parse the scene and tag visibles
        _findVisibleObjects(camera);
        // Add overlays, if viewport deems it
        if (vp->getOverlaysEnabled())
        {
            OverlayManager::getSingleton()._queueOverlaysForRendering(camera, &mRenderQueue, vp);
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
    void SceneManager::_findVisibleObjects(Camera* cam)
    {
        // Tell nodes to find, cascade down all nodes
        mSceneRoot->_findVisibleObjects(cam, &mRenderQueue, true, mDisplayNodes);

    }
    //-----------------------------------------------------------------------
    void SceneManager::_renderVisibleObjects(void)
    {
        // Render each separate queue
        RenderQueue::QueueGroupIterator queueIt = mRenderQueue._getQueueGroupIterator();

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
	void SceneManager::renderAdditiveStencilShadowedQueueGroupObjects(RenderQueueGroup* group)
	{
		/* We need to do the entire process once per light, and we need to 
		   separate the process of rendering certain types of objects.
	   */

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

			// Do solids
			renderObjects(pPriorityGrp->mSolidPasses, true);
		}

        // Clear stencil
        mDestRenderSystem->clearFrameBuffer(FBT_STENCIL);

		// Iterate over lights, render all volumes to stencil
		LightList::const_iterator li, liend;
		liend = mLightsAffectingFrustum.end();

		for (li = mLightsAffectingFrustum.begin(); li != liend; ++li)
		{
			Light* l = *li;
			renderShadowVolumesToStencil(l, mCameraInProgress);

		}// for each priority

		// render full-screen shadow modulator for all lights
		setPass(mShadowModulativePass);
        // turn stencil check on
        // NB we render where the stencil is not equal to zero to render shadows, not lit areas
        mDestRenderSystem->setStencilBufferParams(CMPF_NOT_EQUAL, 0);
        mDestRenderSystem->setStencilCheckEnabled(true);
        renderSingleObject(mFullScreenQuad, mShadowModulativePass, false);
        // Reset stencil params
        mDestRenderSystem->setStencilBufferParams();
        mDestRenderSystem->setStencilCheckEnabled(false);

		// Iterate again - variable name changed to appease gcc.
        RenderQueueGroup::PriorityMapIterator groupIt2 = pGroup->getIterator();
		while (groupIt2.hasMoreElements())
		{
			RenderPriorityGroup* pPriorityGrp = groupIt2.getNext();

			// Do transparents
			renderObjects(pPriorityGrp->mTransparentPasses, true);

		}// for each priority

	}
	//-----------------------------------------------------------------------
	void SceneManager::renderObjects(
        const RenderPriorityGroup::SolidRenderablePassMap& objs, bool doLightIteration)
	{
		// ----- SOLIDS LOOP -----
		RenderPriorityGroup::SolidRenderablePassMap::const_iterator ipass, ipassend;
		ipassend = objs.end();
		for (ipass = objs.begin(); ipass != ipassend; ++ipass)
		{
			// Fast bypass if this group is now empty
			if (ipass->second->empty()) continue;
			// For solids, we try to do each pass in turn
			setPass(ipass->first);
			RenderPriorityGroup::RenderableList* rendList = ipass->second;
			RenderPriorityGroup::RenderableList::const_iterator irend, irendend;
			irendend = rendList->end();
			for (irend = rendList->begin(); irend != irendend; ++irend)
			{
				// Render a single object, this will set up auto params if required
				renderSingleObject(*irend, ipass->first, doLightIteration);
			}
		} 
	}
	//-----------------------------------------------------------------------
	void SceneManager::renderObjects(
        const RenderPriorityGroup::TransparentRenderablePassList& objs, bool doLightIteration)
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
			renderSingleObject(itrans->renderable, itrans->pass, doLightIteration);
		}

	}
	//-----------------------------------------------------------------------
	void SceneManager::renderQueueGroupObjects(RenderQueueGroup* pGroup)
	{
		// Redirect to alternate versions if stencil shadows in use
		if (pGroup->getShadowsEnabled() && 
			mShadowTechnique == SHADOWTYPE_STENCIL_ADDITIVE)
		{
			renderAdditiveStencilShadowedQueueGroupObjects(pGroup);
		}
		else if (pGroup->getShadowsEnabled() && 
			mShadowTechnique == SHADOWTYPE_STENCIL_MODULATIVE)
		{
			renderModulativeStencilShadowedQueueGroupObjects(pGroup);
		}
		else
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
				renderObjects(pPriorityGrp->mSolidPasses, true);
				// Do transparents
				renderObjects(pPriorityGrp->mTransparentPasses, true);


			}// for each priority
		}


	}
    //-----------------------------------------------------------------------
    void SceneManager::renderSingleObject(Renderable* rend, Pass* pass, bool doLightIteration)
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
        else // no light processing
        {
            // Do we need to update GPU program parameters?
            if (pass->isProgrammable())
            {
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
        const Matrix4& viewMatrix, const Matrix4& projMatrix) 
    {
        mDestRenderSystem->_setViewport(vp);
        mDestRenderSystem->_setWorldMatrix(worldMatrix);
        mDestRenderSystem->_setViewMatrix(viewMatrix);
        mDestRenderSystem->_setProjectionMatrix(projMatrix);

        mDestRenderSystem->_beginFrame();

        setPass(pass);
        mDestRenderSystem->_render(*rend);

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
            mRenderQueue.addRenderable(mSkyPlaneEntity->getSubEntity(0), qid, RENDERABLE_DEFAULT_PRIORITY);
        }

        uint plane;
        if (mSkyBoxEnabled)
        {
            qid = mSkyBoxDrawFirst? 
                        RENDER_QUEUE_SKIES_EARLY : RENDER_QUEUE_SKIES_LATE;

            for (plane = 0; plane < 6; ++plane)
            {
                mRenderQueue.addRenderable(
                    mSkyBoxEntity[plane]->getSubEntity(0), qid, RENDERABLE_DEFAULT_PRIORITY);
            }
        }

        if (mSkyDomeEnabled)
        {
            qid = mSkyDomeDrawFirst? 
                        RENDER_QUEUE_SKIES_EARLY : RENDER_QUEUE_SKIES_LATE;

            for (plane = 0; plane < 5; ++plane)
            {
                mRenderQueue.addRenderable(
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
    void SceneManager::setShadowTechnique(ShadowTechnique technique,
        bool debug)
    {
        mShadowTechnique = technique;
        mDebugShadows = debug;
        if (technique == SHADOWTYPE_STENCIL_ADDITIVE || 
            technique == SHADOWTYPE_STENCIL_MODULATIVE)
        {
            // Create an estimated sized shadow index buffer
            mShadowIndexBuffer = HardwareBufferManager::getSingleton().
                createIndexBuffer(HardwareIndexBuffer::IT_16BIT, 50000, 
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false);
            // tell all meshes to prepare shadow volumes
            MeshManager::getSingleton().setPrepareAllMeshesForShadowVolumes(true);
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
        mCasterList->push_back(object);
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
            // Hmm, how to efficiently locate shadow casters for an infinite light?
            // TODO
        }
        else
        {
            SphereSceneQuery* ssc = createSphereQuery(
                Sphere(light->getPosition(), light->getAttenuationRange()) );

            // Execute, use callback
            ShadowCasterSceneQueryListener listener(&mShadowCasterList);
            ssc->execute(&listener);

            delete ssc;

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
                // TODO, add hardware extrusion program
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
                // TODO, add hardware extrusion program
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
                ColourValue(0.25, 0.25, 0.25));

        }

        // Also init full screen quad while we're at it
        if (!mFullScreenQuad)
        {
            mFullScreenQuad = new Rectangle2D();
            mFullScreenQuad->setCorners(-1,1,1,-1);
        }



    }
    //---------------------------------------------------------------------
    void SceneManager::renderShadowVolumesToStencil(const Light* light, const Camera* camera)
    {

        mDestRenderSystem->unbindGpuProgram(GPT_VERTEX_PROGRAM);
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
            // TODO
            //extrudeInSoftware = false;
        }

        // Turn off colour writing and depth writing
        mDestRenderSystem->_setColourBufferWriteEnabled(false, false, false, false);
        mDestRenderSystem->_setDepthBufferWriteEnabled(false);
        mDestRenderSystem->setStencilCheckEnabled(true);
        mDestRenderSystem->_setDepthBufferFunction(CMPF_LESS);


        // Figure out the near clip volume
        const PlaneBoundedVolume& nearClipVol = 
            light->_getNearClipVolume(camera);

        const ShadowCasterList& casters = findShadowCastersForLight(light, camera);
        ShadowCasterList::const_iterator si, siend;
        siend = casters.end();
        for (si = casters.begin(); si != siend; ++si)
        {
            ShadowCaster* caster = *si;

            if (caster->getCastShadows())
            {
                bool zfailAlgo = false;
                unsigned long flags = 0;

                // Determine if zfail is required
                zfailAlgo = nearClipVol.intersects(caster->getWorldBoundingBox());

                if (zfailAlgo)
                {
                    // We need to include the light and / or dark cap
                    // But only if they will be visible
                    if(camera->isVisible(caster->getLightCapBounds()))
                    {
                        flags |= SRF_INCLUDE_LIGHT_CAP;
                    }
                    // Dark caps are not needed for directional lights if
                    // extrusion is done in hardware (since extruded to infinity)
                    if((light->getType() != Light::LT_DIRECTIONAL || extrudeInSoftware)
                        && camera->isVisible(caster->getDarkCapBounds(*light)))
                    {
                        flags |= SRF_INCLUDE_DARK_CAP;
                    }
                }

                // Get shadow renderables
                ShadowCaster::ShadowRenderableListIterator iShadowRenderables =
                    caster->getShadowVolumeRenderableIterator(mShadowTechnique,
                    light, &mShadowIndexBuffer, extrudeInSoftware, flags);

                while (iShadowRenderables.hasMoreElements())
                {
                    ShadowRenderable* sr = iShadowRenderables.getNext();
                    // render volume, including light and dark caps
                    renderSingleShadowVolumeToStencil(sr, zfailAlgo, stencil2sided);

                }

            }
        }
		// revert colour write state
		mDestRenderSystem->_setColourBufferWriteEnabled(true, true, true, true);
		// revert depth write state
		mDestRenderSystem->_setDepthBufferWriteEnabled(true);
        mDestRenderSystem->_setDepthBufferFunction(CMPF_LESS_EQUAL);

        mDestRenderSystem->setStencilCheckEnabled(false);

    }
    //---------------------------------------------------------------------
    void SceneManager::renderSingleShadowVolumeToStencil(ShadowRenderable* sr,
        bool zfailAlgo, bool stencil2sided)
    {
        // Render a shadow volume here
        //  - if we have 2-sided stencil, one render with no culling
        //  - otherwise, 2 renders, one with each culling method and invert the ops
        setShadowVolumeStencilState(false, zfailAlgo, stencil2sided);
        renderSingleObject(sr, mShadowStencilPass, false);

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
            renderSingleObject(sr, mShadowDebugPass, false);
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
            // Do sphere / sphere test
            testSphere.setCenter(i->second->getParentNode()->_getDerivedPosition());
            testSphere.setRadius(i->second->getBoundingRadius());
            if (mSphere.intersects(testSphere))
            {
                listener->queryResult(i->second);
            }
        }
    }


}
