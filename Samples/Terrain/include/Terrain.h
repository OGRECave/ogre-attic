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

RaySceneQuery* raySceneQuery = 0;

// Event handler to add ability to alter curvature
class TerrainFrameListener : public ExampleFrameListener
{
public:
    TerrainFrameListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
        // Reduce move speed
        mMoveSpeed = 50;

    }

    bool frameStarted(const FrameEvent& evt)
    {
        // clamp to terrain
        bool ret = ExampleFrameListener::frameStarted(evt);
        static Ray updateRay;
        updateRay.setOrigin(mCamera->getPosition());
        updateRay.setDirection(Vector3::NEGATIVE_UNIT_Y);
        raySceneQuery->setRay(updateRay);
        RaySceneQueryResult& qryResult = raySceneQuery->execute();
        RaySceneQueryResult::iterator i = qryResult.begin();
        if (i != qryResult.end() && i->worldFragment)
        {
            SceneQuery::WorldFragment* wf = i->worldFragment;
            mCamera->setPosition(mCamera->getPosition().x, 
                i->worldFragment->singleIntersection.y + 10, 
                mCamera->getPosition().z);
        }

        return ret;

    }

};



class TerrainApplication : public ExampleApplication
{
public:
    TerrainApplication() {}

    ~TerrainApplication()
    {
        delete raySceneQuery;
    }

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

        // Fog
        // NB it's VERY important to set this before calling setWorldGeometry 
        // because the vertex program picked will be different
        ColourValue fadeColour(0.93, 0.86, 0.76);
        mSceneMgr->setFog( FOG_EXP, fadeColour, .001);
        mWindow->getViewport(0)->setBackgroundColour(fadeColour);


        mSceneMgr -> setWorldGeometry( "terrain.cfg" );
        // Infinite far plane
        mCamera->setFarClipDistance(0);

        // Define the required skyplane
        Plane plane;
        // 5000 world units from the camera
        plane.d = 5000;
        // Above the camera, facing down
        plane.normal = -Vector3::UNIT_Y;

        // Set a nice viewpoint
        mCamera->setPosition(707,52,528);
        mCamera->setOrientation(Quaternion(-0.3486, 0.0122, 0.9365, 0.0329));
        //mRoot -> showDebugOverlay( true );

        raySceneQuery = mSceneMgr->createRayQuery(
            Ray(mCamera->getPosition(), Vector3::NEGATIVE_UNIT_Y));


    }
    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new TerrainFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }

};
