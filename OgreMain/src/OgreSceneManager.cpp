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

        // No fog
        mFogMode = FOG_NONE;
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
        mDestRenderSystem->_removeAllLights();
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
            delete *i;
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
        //removeAllLights();



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
        mSceneNodes.push_back(sn);
        return sn;
    }

    //-----------------------------------------------------------------------
    SceneNode* SceneManager::getRootSceneNode(void)
    {
        return mSceneRoot;
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
    void SceneManager::_renderScene(Camera* camera, Viewport* vp)
    {
        mCameraInProgress = camera;

        _updateSceneGraph(camera);
        _updateDynamicLights();
        _findVisibleObjects(camera);


        // Set viewport, view and projection matrices
        mDestRenderSystem->_setViewport(vp);
        mDestRenderSystem->_setViewMatrix(camera->getViewMatrix());
        mDestRenderSystem->_setProjectionMatrix(camera->getProjectionMatrix());

        mDestRenderSystem->_beginGeometryCount();
        // Begin the frame
        mDestRenderSystem->_beginFrame();

        // Update controllers (after begineFrame since some are frameTime dependent)
        ControllerManager::getSingleton().updateAllControllers();

        // Render the sky (first)
        if (mSkyPlaneEnabled && mSkyPlaneDrawFirst)
            _renderSkyPlane(camera);
        if (mSkyBoxEnabled && mSkyBoxDrawFirst)
            _renderSkyBox(camera);
        if (mSkyDomeEnabled && mSkyDomeDrawFirst)
            _renderSkyDome(camera);

        // Render scene content (only entities in this SceneManager, no world geometry)
        //_renderEntities();
        _renderVisibleObjects();

        // Render the sky (last)
        if (mSkyPlaneEnabled && !mSkyPlaneDrawFirst)
            _renderSkyPlane(camera);
        if (mSkyBoxEnabled && !mSkyBoxDrawFirst)
            _renderSkyBox(camera);
        if (mSkyDomeEnabled && !mSkyDomeDrawFirst)
            _renderSkyDome(camera);

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
        bool drawFirst )
    {
        mSkyPlaneEnabled = enable;
        if (enable)
        {
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

            mSkyPlaneMatHdl = m->getHandle();
            mSkyPlaneDrawFirst = drawFirst;

            // Set up the plane
            Mesh* planeMesh = (Mesh*)MeshManager::getSingleton().getByName("SkyPlane");
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

            // Create skybox based on new parameters
            planeMesh = MeshManager::getSingleton().createPlane("SkyPlane", plane, gscale * 100, gscale * 100,
                1,1, false, 1, tiling, tiling, up);

            //planeMesh->_dumpContents("skyplane.txt");


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
            mSkyBoxDist = distance;
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

            mSkyBoxMatHdl = m->getHandle();
            mSkyBoxDrawFirst = drawFirst;

            // Set up the box (6 planes)
            for (int i = 0; i < 6; ++i)
            {
                createSkyboxPlane((BoxPlane)i, distance, orientation);
            }

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
            mSkyDomeDist = distance;
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

            mSkyDomeMatHdl = m->getHandle();
            mSkyDomeDrawFirst = drawFirst;

            // Set up the dome (5 planes)
            for (int i = 0; i < 5; ++i)
            {
                createSkydomePlane((BoxPlane)i, curvature, tiling, distance, orientation);
            }

        }
    }
    //-----------------------------------------------------------------------
    void SceneManager::createSkyboxPlane(
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

    }
    //-----------------------------------------------------------------------
    void SceneManager::createSkydomePlane(
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
            return;
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

    }
    //-----------------------------------------------------------------------
    void SceneManager::_renderSkyPlane(Camera* cam)
    {
        int matLayers;
        Material *m;
        Mesh* planeMesh;
        SubMesh* sm;

        planeMesh = (Mesh*)MeshManager::getSingleton().getByName("SkyPlane");
        sm = planeMesh->getSubMesh(0);

        // Translate the plane by the camera position (constant distance)
        // The plane position relative to the camera has already been set up
        Matrix4 mat = Matrix4::IDENTITY;
        mat.setTrans(cam->getPosition());
        mDestRenderSystem->_setWorldMatrix(mat);

        m = getMaterial(mSkyPlaneMatHdl);
        // Set the material for the camera
        matLayers = m->getNumTextureLayers();
        do
        {
            matLayers = setMaterial(m, matLayers);
            _renderSubMesh(sm);

        } while (matLayers > 0);

    }

    //-----------------------------------------------------------------------
    void SceneManager::_renderSkyBox(Camera* cam)
    {
        int matLayers;
        Material *m;
        Mesh* planeMesh;
        SubMesh* sm;
        String baseName, meshName;

        baseName = "SkyBoxPlane_";

        // Translate the box by the camera position (constant distance)
        // The plane position relative to the camera has already been set up
        Matrix4 mat = Matrix4::IDENTITY;
        mat.setTrans(cam->getPosition());
        mDestRenderSystem->_setWorldMatrix(mat);

        m = getMaterial(mSkyBoxMatHdl);
        for (int plane = 0; plane < 6; ++plane)
        {
            switch(plane)
            {
            case BP_FRONT:
                //??? Why is back/front BACKWARDS from 3D Studio???
                // I thought LEFT was relative to looking FRONT, but appears not when you look at the output
                // Quake3 images also have this
                m->getTextureLayer(0)->setCurrentFrame(Material::TextureLayer::CUBE_BACK);
                meshName = baseName + "Front";
                break;
            case BP_BACK:
                //??? Why is back/front BACKWARDS from 3D Studio???
                m->getTextureLayer(0)->setCurrentFrame(Material::TextureLayer::CUBE_FRONT);
                meshName = baseName + "Back";
                break;
            case BP_LEFT:
                m->getTextureLayer(0)->setCurrentFrame(Material::TextureLayer::CUBE_LEFT);
                meshName = baseName + "Left";
                break;
            case BP_RIGHT:
                m->getTextureLayer(0)->setCurrentFrame(Material::TextureLayer::CUBE_RIGHT);
                meshName = baseName + "Right";
                break;
            case BP_UP:
                m->getTextureLayer(0)->setCurrentFrame(Material::TextureLayer::CUBE_UP);
                meshName = baseName + "Up";
                break;
            case BP_DOWN:
                m->getTextureLayer(0)->setCurrentFrame(Material::TextureLayer::CUBE_DOWN);
                meshName = baseName + "Down";
                break;
            }

            planeMesh = (Mesh*)MeshManager::getSingleton().getByName(meshName);
            sm = planeMesh->getSubMesh(0);


            // Set the material for the camera
            matLayers = m->getNumTextureLayers();
            do
            {
                matLayers = setMaterial(m, matLayers);
                _renderSubMesh(sm);

            } while (matLayers > 0);
        }


    }
    //-----------------------------------------------------------------------
    void SceneManager::_renderSkyDome(Camera* cam)
    {
        int matLayers;
        Material *m;
        Mesh* planeMesh;
        SubMesh* sm;
        String baseName, meshName;

        // Translate the box by the camera position (constant distance)
        // The plane position relative to the camera has already been set up
        Matrix4 mat = Matrix4::IDENTITY;
        mat.setTrans(cam->getPosition());
        mDestRenderSystem->_setWorldMatrix(mat);

        baseName = "SkyDomePlane_";

        m = getMaterial(mSkyDomeMatHdl);
        // Set the material
        matLayers = m->getNumTextureLayers();
        do
        {
            matLayers = setMaterial(m, matLayers);

            for (int plane = 0; plane < 5; ++plane)
            {
                switch(plane)
                {
                case BP_FRONT:
                    meshName = baseName + "Front";
                    break;
                case BP_BACK:
                    meshName = baseName + "Back";
                    break;
                case BP_LEFT:
                    meshName = baseName + "Left";
                    break;
                case BP_RIGHT:
                    meshName = baseName + "Right";
                    break;
                case BP_UP:
                    meshName = baseName + "Up";
                    break;
                }

                planeMesh = (Mesh*)MeshManager::getSingleton().getByName(meshName);
                sm = planeMesh->getSubMesh(0);

                _renderSubMesh(sm);
            }

        } while (matLayers > 0);
    }
    //-----------------------------------------------------------------------
    void SceneManager::_renderSubMesh(SubMesh* sm)
    {

        static RenderOperation ro; // to avoid creating / destroying every time but must be careful to set all fields
        /*

        // SubMeshes always use indexes
        ro.useIndexes = true;
        GeometryData* geom;

        if (sm->useTriStrips)
            ro.operationType = RenderOperation::OT_TRIANGLE_STRIP;
        else
            ro.operationType = RenderOperation::OT_TRIANGLE_LIST;

        if (sm->useSharedVertices)
        {
            geom = &(sm->parent->sharedGeometry);
        }
        else
        {
            geom = &(sm->geometry);
        }

        if (geom->numTexCoords > 0)
        {
            ro.vertexOptions = RenderOperation::VO_TEXTURE_COORDS;
            ro.numTextureCoordSets = geom->numTexCoords;
            for (int tex = 0; tex < ro.numTextureCoordSets; ++tex)
            {
                ro.numTextureDimensions[tex] = geom->numTexCoordDimensions[tex];
                ro.pTexCoords[tex] = geom->pTexCoords[tex];
                ro.texCoordStride[tex] = geom->texCoordStride[tex];
            }

        }

        if (geom->hasNormals)
        {
            ro.vertexOptions |= RenderOperation::VO_NORMALS;
            ro.pNormals = geom->pNormals;
        }

        if (geom->hasColours)
        {
            ro.vertexOptions = RenderOperation::VO_DIFFUSE_COLOURS;
            ro.pDiffuseColour = geom->pColours;
        }

        ro.numVertices = geom->numVertices;
        ro.pVertices = geom->pVertices;
        ro.diffuseStride = geom->colourStride;
        ro.normalStride= geom->normalStride;
        ro.vertexStride = geom->vertexStride;

        if (sm->useTriStrips)
            ro.numIndexes = sm->numFaces + 2;
        else
            ro.numIndexes = sm->numFaces * 3;

        ro.pIndexes = sm->faceVertexIndices;

        */

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
        mRenderQueue.clear();
        mSceneRoot->_findVisibleObjects(cam, &mRenderQueue, true);

    }
    //-----------------------------------------------------------------------
    void SceneManager::_renderVisibleObjects(void)
    {
        // Just render each entity in turn, grouped by material
        RenderQueue::RenderQueueMap::iterator imat, imatend;
        imatend = mRenderQueue.mQueue.end();
        Matrix4 xform;
        RenderOperation ro;

        for (imat = mRenderQueue.mQueue.begin(); imat != imatend; ++imat)
        {
            // Set Material
            Material* thisMaterial = imat->first;
            int matLayersLeft = thisMaterial->getNumTextureLayers();

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
                    (*irend)->getWorldTransforms(&xform);
                    mDestRenderSystem->_setWorldMatrix(xform);

                    // Set up rendering operation
                    (*irend)->getRenderOperation(ro);

                    if( ro.numVertices )
                        mDestRenderSystem->_render(ro);

                }
            } while (matLayersLeft > 0);


        } // for each material

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


}
