/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    \file 
        Terrain.h
    \brief
        Specialisation of OGRE's framework application to show the
        terrain rendering plugin 
*/

#include "ExampleApplication.h"

class TerrainApplication : public ExampleApplication
{
public:
    TerrainApplication() {}

protected:


    virtual void chooseSceneManager(void)
    {
        // Get the SceneManager, in this case a generic one
        mSceneMgr = mRoot->getSceneManager( ST_EXTERIOR_CLOSE );
    }

    virtual void createCamera(void)
    {
        // Create the camera
        mCamera = mSceneMgr->createCamera("PlayerCam");

        // Position it at 500 in Z direction
        mCamera->setPosition(Vector3(128,25,128));
        // Look back along -Z
        mCamera->lookAt(Vector3(0,0,-300));
        mCamera->setNearClipDistance( 1 );
        mCamera->setFarClipDistance( 384 );

    }
   
    // Just override the mandatory create scene method
    void createScene(void)
    {
        Entity *waterEntity;
        Plane waterPlane;

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        mSceneMgr -> setWorldGeometry( "terrain.cfg" );
        // Infinite far plane
        mCamera->setFarClipDistance(0);

        // Define the required skyplane
        Plane plane;
        // 5000 world units from the camera
        plane.d = 5000;
        // Above the camera, facing down
        plane.normal = -Vector3::UNIT_Y;

        ColourValue fadeColour(0.93, 0.86, 0.76);

        mSceneMgr->setFog( FOG_EXP2, fadeColour, .001);
        mWindow->getViewport(0)->setBackgroundColour(fadeColour);
        mCamera->setPosition(100,100,100);
        mCamera->lookAt(512,100,512);
        //mRoot -> showDebugOverlay( true );

    }

};
