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

// This class implements the most basic scene manager

#include <cstdio>

namespace Ogre {

    SceneManager::SceneManager()
    {
        // Root scene node
        mSceneRoot = new SceneNode(this);


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
    }

    SceneManager::~SceneManager()
    {
        clearScene();
        delete mSceneRoot;
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
        mLights.insert(LightList::value_type(name, l));
        // Add light to render system
        mDestRenderSystem->_addLight(l);
        return l;
    }

    //-----------------------------------------------------------------------
    Light* SceneManager::getLight(const String& name)
    {
        LightList::iterator i = mLights.find(name);
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
        LightList::iterator i = mLights.begin();
        for (; i != mLights.end(); ++i)
        {
            if (i->second == l)
            {
                mLights.erase(i);
                mDestRenderSystem->_removeLight(l);
                delete l;
                break;
            }
        }

    }

    //-----------------------------------------------------------------------
    void SceneManager::removeLight(const String& name)
    {
        // Find in list
        LightList::iterator i = mLights.find(name);
        if (i != mLights.end())
        {
            delete i->second;
            mDestRenderSystem->_removeLight(i->second);
            mLights.erase(i);
        }

    }

    //-----------------------------------------------------------------------
    void SceneManager::removeAllLights(void)
    {

        LightList::iterator i = mLights.begin();
        for (; i != mLights.end(); ++i)
        {
            delete i->second;
        }
        // BUG
        // When shutting down plugins, this gets called by destructor
        // But if this is a plugin, then sometimes rendersystem has already been destroyed
        // TODO: plugins should all shutdown before destroying?
        //mDestRenderSystem->_removeAllLights();
        mLights.clear();
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
                "An entity with the name " + entityName + "already exists",
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
    void SceneManager::addMaterial(const Material& mat)
    {
        // Add using MaterialManager
        /* Material* newMat = */ MaterialManager::getSingleton().add(mat);
    }

    //-----------------------------------------------------------------------
    Material* SceneManager::getMaterial(const String& name)
    {
        return (Material*)MaterialManager::getSingleton().getByName(name);
    }

    //-----------------------------------------------------------------------
    Material* SceneManager::getMaterial(int handle)
    {
        return MaterialManager::getSingleton().getByHandle(handle);
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
    int SceneManager::setMaterial(Material* mat, int numLayersLeft)
    {
        static bool firstTime = true;
        static bool lastUsedFallback = false;
        static int lastNumTexUnitsUsed = 0;
        static Material lastMat; // Last material settings, to minimise render state changes

        // Recent changes: eliminated the need to copy all material settings to set with RenderSystem
        // Now only issues required render state changes to the render system for maximum performance

        // Set surface properties
        if (firstTime || mat->_compareSurfaceParams(lastMat) == false)
        {
           ColourValue a, b, c, d;
           a = mat->getAmbient();
           b = mat->getDiffuse();
           c = mat->getSpecular();
           d = mat->getSelfIllumination();
            mDestRenderSystem->_setSurfaceParams( a, b, c, d, mat->getShininess() );
        }

        // Set global blending, play it safe if last one was fallback
        if (firstTime || lastUsedFallback ||
        (lastMat.getSourceBlendFactor() != mat->getSourceBlendFactor() ||
         lastMat.getDestBlendFactor() != mat->getDestBlendFactor()))
        {
            mDestRenderSystem->_setSceneBlending(mat->getSourceBlendFactor(), mat->getDestBlendFactor());
        }

        // Fog
        // New fog params can either be from scene or from material
        FogMode newFogMode;
        ColourValue newFogColour;
        Real newFogStart, newFogEnd, newFogDensity;
        static FogMode oldFogMode;
        static ColourValue oldFogColour;
        static Real oldFogStart, oldFogEnd, oldFogDensity;
        if (mat->getFogOverride())
        {
            // New fog params from material
            newFogMode = mat->getFogMode();
            newFogColour = mat->getFogColour();
            newFogStart = mat->getFogStart();
            newFogEnd = mat->getFogEnd();
            newFogDensity = mat->getFogDensity();
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
        if (firstTime || newFogMode != oldFogMode || newFogColour != oldFogColour ||
            newFogStart != oldFogStart || newFogEnd != oldFogEnd ||
            newFogDensity != oldFogDensity)
        {
            mDestRenderSystem->_setFog(newFogMode, newFogColour, newFogDensity, newFogStart, newFogEnd);
            oldFogMode = newFogMode;
            oldFogColour = newFogColour;
            oldFogStart = newFogStart;
            oldFogEnd = newFogEnd;
            oldFogDensity = newFogDensity;
        }



        // Texture layers
        int texLayer = mat->getNumTextureLayers() - numLayersLeft;
        int thisUnitsRequested = numLayersLeft;
        int texUnits = mDestRenderSystem->_getNumTextureUnits();

#if OGRE_TEST_MULTIPASS == 1
        texUnits = 1;
#endif

        // Iterate over texture units, set them up to the higher of the last texturing units used and the current to be used
        //   but no higher than the number of units
        lastUsedFallback = false;
       int unit;
        for (unit = 0;
            (unit < lastNumTexUnitsUsed || unit < thisUnitsRequested) && unit < texUnits;
            ++unit, ++texLayer)
        {
            if (unit >= thisUnitsRequested)
            {
                // We've run out of texture layers before we ran out of units to set
                // Turn off the texturing for this unit
                mDestRenderSystem->_disableTextureUnit(unit);
            }
            else
            {
                Material::TextureLayer* pTex = mat->getTextureLayer(texLayer);
                // We still have texture layers to put in this unit
                if (unit == 0 && thisUnitsRequested > 0 && thisUnitsRequested < mat->getNumTextureLayers())
                {
                    // We're on the second (or more) leg of a multipass render
                    //  because remaining layers is not the total number

                    // So we need to use the multipass fallback and override first texture layer blend
                    lastUsedFallback = true;

                    // Copy texture layer info and set custom blending
                    Material::TextureLayer newTex = *pTex;

                    // Set multitexture blend to NO blend (avoids any modulation with material colour)
                    newTex.setColourOperation(LBO_REPLACE);
                    // Set scene blending to colour blending equivalent (fallback)
                    mDestRenderSystem->_setSceneBlending(pTex->getColourBlendFallbackSrc(), pTex->getColourBlendFallbackDest());

                    // Set texture unit settings
                    // NB rendersystem will know to only change relevant settings
                    mDestRenderSystem->_setTextureUnitSettings(unit, newTex);
                }
                else
                {
                    if (lastUsedFallback)
                    {
                        // Ensure that fallback alpha on bottom layer does not mask out alpha on this layer
                        pTex->setAlphaOperation(LBX_ADD);
                    }

                    // Standard issue
                    mDestRenderSystem->_setTextureUnitSettings(unit, *pTex);
                }

                numLayersLeft--;
            }
        }

        // Set up non-texture related material settings
        // Depth buffer settings
        if (firstTime || lastMat.getDepthFunction() != mat->getDepthFunction())
        {
            mDestRenderSystem->_setDepthBufferFunction(mat->getDepthFunction());
        }
        if (firstTime || lastMat.getDepthCheckEnabled() != mat->getDepthCheckEnabled())
        {
            mDestRenderSystem->_setDepthBufferCheckEnabled(mat->getDepthCheckEnabled());
        }
        if (firstTime || lastMat.getDepthWriteEnabled() != mat->getDepthWriteEnabled())
        {
            mDestRenderSystem->_setDepthBufferWriteEnabled(mat->getDepthWriteEnabled());
        }
        if (firstTime || lastMat.getDepthBias() != mat->getDepthBias())
        {
            mDestRenderSystem->_setDepthBias(mat->getDepthBias());
        }


        // Culling mode
        if (firstTime || lastMat.getCullingMode() != mat->getCullingMode())
        {
            mDestRenderSystem->_setCullingMode(mat->getCullingMode());

        }
        // Dynamic lighting enabled
        if (firstTime || lastMat.getLightingEnabled() != mat->getLightingEnabled())
        {
            mDestRenderSystem->setLightingEnabled(mat->getLightingEnabled());
        }
        // Shading
        if (firstTime || lastMat.getShadingMode() != mat->getShadingMode())
        {
            mDestRenderSystem->setShadingType(mat->getShadingMode());
        }
        // Texture filtering
        if (firstTime || lastMat.getTextureFiltering() != mat->getTextureFiltering())
        {
            mDestRenderSystem->setTextureFiltering(mat->getTextureFiltering());
        }


        firstTime = false;
        // Copy material settings from last render
        lastMat = *mat;
        lastNumTexUnitsUsed = unit;
        return numLayersLeft;

    }
    //-----------------------------------------------------------------------
    void SceneManager::_renderScene(Camera* camera, Viewport* vp, bool includeOverlays)
    {
        mCameraInProgress = camera;
        mCamChanged = true;


        // Set the viewport
        setViewport(vp);


        // Update the scene
        _applySceneAnimations();
        _updateSceneGraph(camera);
        _updateDynamicLights();

        // Auto-track camera if required
        camera->_autoTrack();

        // Clear the render queue
        mRenderQueue.clear();

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

        // Render scene content (only entities in this SceneManager, no world geometry)
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
                char errMsg[128];
                sprintf(errMsg, "Sky plane material '%s' not found.", materialName.c_str());
                Except(Exception::ERR_INVALIDPARAMS, errMsg, "SceneManager::setSkyPlane");
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
                char errMsg[128];
                sprintf(errMsg, "Sky box material '%s' not found.", materialName.c_str());
                Except(Exception::ERR_INVALIDPARAMS, errMsg, "SceneManager::setSkyBox");
            }
            // Make sure the material doesn't update the depth buffer
            m->setDepthWriteEnabled(false);
            // Also clamp texture, don't wrap (otherwise edges can get filtered)
            m->getTextureLayer(0)->setTextureAddressingMode(Material::TextureLayer::TAM_CLAMP);

            // Ensure loaded
            m->load();

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
                // Have to create 6 materials, one for each frame
                // Used to use combined material but now we're using queue we can't split to change frame
                // This doesn't use much memory because textures aren't duplicated
                Material* boxMat = (Material*)matMgr.getByName(entName);
                if (!boxMat)
                {
                    // Create new by clone
                    boxMat = m->clone(entName);
                }
                else
                {
                    // Copy over existing
                    m->copyDetailsTo(boxMat);
                }
                // Set active frame
                //??? Why is back/front BACKWARDS from 3D Studio???
                // I thought LEFT was relative to looking FRONT, but appears not when you look at the output
                // Quake3 images also have this
                if ((Material::TextureLayer::TextureCubeFace)i == Material::TextureLayer::CUBE_BACK)
                {
                    boxMat->getTextureLayer(0)->setCurrentFrame(Material::TextureLayer::CUBE_FRONT);
                }
                else if ((Material::TextureLayer::TextureCubeFace)i == Material::TextureLayer::CUBE_FRONT)
                {
                    boxMat->getTextureLayer(0)->setCurrentFrame(Material::TextureLayer::CUBE_BACK);
                }
                else
                {
                    boxMat->getTextureLayer(0)->setCurrentFrame(i);
                }
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
                char errMsg[128];
                sprintf(errMsg, "Sky box material '%s' not found.", materialName.c_str());
                Except(Exception::ERR_INVALIDPARAMS, errMsg, "SceneManager::setSkyDome");
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
        planeMesh = mm.createPlane(meshName, plane, planeSize, planeSize, BOX_SEGMENTS, BOX_SEGMENTS, false, 1, 1, 1, up);

        //planeMesh->_dumpContents(meshName);

        // Now we've created a basic plane, modify the texture coordinates to appear curved
        // Imagine a large sphere with the camera located near the top
        // The lower the curvature, the larger the sphere
        // Use the angle from viewer to the points on the plane
        // Credit to Aftershock for the general approach
        Real* pTex;
        Vector3 vertPos;  // position relative to camera
        Real sphDist;      // Distance from camera to sphere along box vertex vector
        // Vector3 camToSph; // camera position to sphere
        Real sphereRadius;// Sphere radius
        Real camPos;      // Camera position relative to sphere center

        // Derive sphere radius
        // Actual values irrelevant, it's the relation between sphere radius and camera position that's important
        const Real SPHERE_RAD = 100.0;
        const Real CAM_DIST = 5.0;

        sphereRadius = SPHERE_RAD - curvature;
        camPos = sphereRadius - CAM_DIST;

        for (int y = 0; y < BOX_SEGMENTS + 1; ++y)
        {
            for (int x = 0; x < BOX_SEGMENTS + 1; ++x)
            {
                pTex = planeMesh->sharedGeometry.pTexCoords[0] + (((y * (BOX_SEGMENTS+1)) + x) * 2);

                // Get position of box vertex in view space
                vertPos = Vector3(planeMesh->sharedGeometry.pVertices + (((y * (BOX_SEGMENTS+1)) + x) * 3));
                // Adjust by -orientation to return to +y up
                vertPos = orientation.Inverse() * vertPos;
                // Normalise
                vertPos.normalise();
                // Find distance to sphere
                sphDist = Math::getSingleton().Sqrt(camPos*camPos * (vertPos.y*vertPos.y-1.0) + sphereRadius*sphereRadius) - camPos*vertPos.y;

                vertPos.x *= sphDist;
                vertPos.z *= sphDist;

                // Use x and y on sphere as texture coordinates, tiled
                pTex[0] = vertPos.x * (0.01 * tiling);
                pTex[1] = vertPos.z * (0.01 * tiling);

            }
        }

        return planeMesh;

    }
    //-----------------------------------------------------------------------
    void SceneManager::_renderSubMesh(SubMesh* sm)
    {

        static RenderOperation ro; // to avoid creating / destroying every time but must be careful to set all fields

        sm->_getRenderOperation(ro);
        mDestRenderSystem->_render(ro);


    }


    //-----------------------------------------------------------------------
    void SceneManager::_updateSceneGraph(Camera* cam)
    {
        // Cascade down the graph updating transforms & world bounds
        // In this implementation, just update from the root
        // Smarter SceneManager subclasses may choose to update only
        //   certain scene graph branches
        mSceneRoot->_update(cam, true);


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

                // Iterate through priorities
                RenderQueueGroup::PriorityMapIterator groupIt = pGroup->getIterator();

                while (groupIt.hasMoreElements())
                {
                    RenderPriorityGroup* pPriorityGrp = groupIt.getNext();

                    // Render each non-transparent entity in turn, grouped by material
                    RenderPriorityGroup::MaterialGroupMap::iterator imat, imatend;
                    imatend = pPriorityGrp->mMaterialGroups.end();
                    static Matrix4 xform[256];
                    RenderOperation ro;
                    int matLayersLeft;
                    Material* thisMaterial;
                    unsigned short numMatrices;

                    // ----- NON-TRANSPARENT ENTITY LOOP -----
                    for (imat = pPriorityGrp->mMaterialGroups.begin(); imat != imatend; ++imat)
                    {
                        // Set Material
                        thisMaterial = imat->first;
                        matLayersLeft = thisMaterial->getNumTextureLayers();

                        // NB do at least one rendering pass even if no layers! (Untextured materials)
                        do
                        {
                            // Set material - will return non-zero if multipass required so loop will continue, 0 otherwise
                            matLayersLeft = setMaterial(thisMaterial, matLayersLeft);


                            // Iterate through renderables and render
                            // Note this may happen multiple times for multipass render
                            std::vector<Renderable*>::iterator irend, irendend;
                            irendend = imat->second.end();

                            for (irend = imat->second.begin(); irend != irendend; ++irend)
                            {
                                // Set world transformation
                                (*irend)->getWorldTransforms(xform);
                                numMatrices = (*irend)->getNumWorldTransforms();
                                if (numMatrices > 1)
                                {
                                    mDestRenderSystem->_setWorldMatrices(xform, numMatrices);
                                }
                                else
                                {
                                    mDestRenderSystem->_setWorldMatrix(*xform);
                                }

                                // Issue view / projection changes if any
                                useRenderableViewProjMode(*irend);

                                // Set up rendering operation
                                (*irend)->getRenderOperation(ro);

                                if( ro.numVertices )
                                    mDestRenderSystem->_render(ro);

                            }
                        } while (matLayersLeft > 0);


                    } // for each material

                    // ----- TRANSPARENT ENTITY LOOP -----
                    // This time we render by Z, not by material
                    // The mTransparentObjects set needs to be ordered first
                    pPriorityGrp->sortTransparentObjects(mCameraInProgress);

                    std::vector<Renderable*>::iterator iTrans, iTransEnd;
                    iTransEnd = pPriorityGrp->mTransparentObjects.end();
                    for (iTrans = pPriorityGrp->mTransparentObjects.begin(); 
                    iTrans != iTransEnd; ++iTrans)
                    {
                        thisMaterial = (*iTrans)->getMaterial();
                        matLayersLeft = thisMaterial->getNumTextureLayers();
                        // NB do at least one rendering pass even if no layers! (Untextured materials)
                        do
                        {
                            // Set material - will return non-zero if multipass required so loop will continue, 0 otherwise
                            matLayersLeft = setMaterial(thisMaterial, matLayersLeft);

                            // Set world transformation
                            (*iTrans)->getWorldTransforms(xform);
                            numMatrices = (*iTrans)->getNumWorldTransforms();
                            if (numMatrices > 1)
                            {
                                mDestRenderSystem->_setWorldMatrices(xform, numMatrices);
                            }
                            else
                            {
                                mDestRenderSystem->_setWorldMatrix(*xform);
                            }

                            // Issue view / projection changes if any
                            useRenderableViewProjMode(*iTrans);

                            // Set up rendering operation
                            (*iTrans)->getRenderOperation(ro);

                            if( ro.numVertices )
                                mDestRenderSystem->_render(ro);

                        } while (matLayersLeft > 0);


                    }




                }// for each priority
            
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
    void SceneManager::_updateDynamicLights(void)
    {
        // Update all lights
        LightList::iterator i;
        Light* lt;
        for (i = mLights.begin(); i != mLights.end(); ++i)
        {
            lt = i->second;
            if (lt->isModified())
                mDestRenderSystem->_modifyLight(lt);

        }
    }
    //-----------------------------------------------------------------------
    void SceneManager::setAmbientLight(ColourValue colour)
    {
        mAmbientLight = colour;
        mDestRenderSystem->setAmbientLight(colour.r, colour.g, colour.b);
    }
    //-----------------------------------------------------------------------
    ColourValue SceneManager::getAmbientLight(void)
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
    void SceneManager::displaySplashScreen( Viewport* vp, const String& name )
    {
        mDestRenderSystem->_setViewport(vp);
        // Reset all matrices
        mDestRenderSystem->_setWorldMatrix(Matrix4::IDENTITY);
        mDestRenderSystem->_setViewMatrix(Matrix4::IDENTITY);
        mDestRenderSystem->_setProjectionMatrix(Matrix4::IDENTITY);

        // Make sure texture is loaded
        Material mat;
        mat.addTextureLayer(name);
        mat.setLightingEnabled(false);
        mat.setCullingMode(CULL_NONE);

        Mesh* msh = (Mesh*)MeshManager::getSingleton().getByName("Prefab_Splash_Screen");


        // Begin the frame
        mDestRenderSystem->_beginFrame();

        setMaterial(&mat,1);
        _renderSubMesh(msh->getSubMesh(0));


        mDestRenderSystem->_endFrame();
    }
    //-----------------------------------------------------------------------
    void SceneManager::setFog(FogMode mode, ColourValue colour, Real density, Real start, Real end)
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
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot find animation with name " + name, 
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
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot find animation with name " + name, 
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
                "Cannot create, AnimationState already exists.", 
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
    //---------------------------------------------------------------------
    void SceneManager::manualRender(RenderOperation* rend, 
        Material* mat, Viewport* vp, const Matrix4& worldMatrix, 
        const Matrix4& viewMatrix, const Matrix4& projMatrix) 
    {
        mDestRenderSystem->_setViewport(vp);
        mDestRenderSystem->_setWorldMatrix(worldMatrix);
        mDestRenderSystem->_setViewMatrix(viewMatrix);
        mDestRenderSystem->_setProjectionMatrix(projMatrix);

        mDestRenderSystem->_beginFrame();

        // NB do at least one rendering pass even if no layers! (Untextured materials)
        unsigned short layersLeft = mat->getNumTextureLayers(); 
        do
        {
            layersLeft = setMaterial(mat, layersLeft);
            mDestRenderSystem->_render(*rend);
        } while (layersLeft > 0);

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
            mSkyBoxNode->setPosition(cam->getDerivedPosition());

        if (mSkyDomeNode)
            mSkyDomeNode->setPosition(cam->getDerivedPosition());

        RenderQueueGroupID qid;
        if (mSkyPlaneEnabled)
        {
            qid = mSkyPlaneDrawFirst? 
                        RENDER_QUEUE_1 : RENDER_QUEUE_9;
            mRenderQueue.addRenderable(mSkyPlaneEntity->getSubEntity(0), qid);
        }

        uint plane;
        if (mSkyBoxEnabled)
        {
            qid = mSkyBoxDrawFirst? 
                        RENDER_QUEUE_1 : RENDER_QUEUE_9;

            for (plane = 0; plane < 6; ++plane)
            {
                mRenderQueue.addRenderable(
                    mSkyBoxEntity[plane]->getSubEntity(0), qid);
            }
        }

        if (mSkyDomeEnabled)
        {
            qid = mSkyDomeDrawFirst? 
                        RENDER_QUEUE_1 : RENDER_QUEUE_9;

            for (plane = 0; plane < 5; ++plane)
            {
                mRenderQueue.addRenderable(
                    mSkyDomeEntity[plane]->getSubEntity(0), qid);
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
	bool SceneManager::getShowBoundingBoxes() 
	{
		return mShowBoundingBoxes;
	}


}
