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
#include "OgreStringConverter.h"

#define FLOW_SPEED 0.2
#define FLOW_HEIGHT 0.8

class TerrainListener : public ExampleFrameListener
{
  public:
    TerrainListener(RenderWindow* win, Camera* cam) :ExampleFrameListener(win, cam) { };

 // Override frameStarted event to process that (don't care about frameEnded)
    bool frameStarted(const FrameEvent& evt)
    {
        float moveScale;
        float rotScale;
        float waterFlow;
        static float flowAmount = 0.0f;
        static bool flowUp = true;

        // local just to stop toggles flipping too fast
        static Real timeUntilNextToggle = 0;

        if (timeUntilNextToggle >= 0)
            timeUntilNextToggle -= evt.timeSinceLastFrame;

        // If this is the first frame, pick a speed
        if (evt.timeSinceLastFrame == 0)
        {
            moveScale = 1;
            rotScale = 0.1;
            waterFlow = 0.0f;
        }
        // Otherwise scale movement units by time passed since last frame
        else
        {
            // Move about 100 units per second,
            moveScale = 10.0 * evt.timeSinceLastFrame;
            // Take about 10 seconds for full rotation
            rotScale = 36 * evt.timeSinceLastFrame;
            
            // set a nice waterflow rate
            waterFlow = FLOW_SPEED * evt.timeSinceLastFrame;            
        }

        // Grab input device state
        mInputDevice->capture();

        SceneNode *waterNode = static_cast<SceneNode*>(
            mCamera->getSceneManager()->getRootSceneNode()->getChild("WaterNode"));
        if(waterNode)
        {
            if(flowUp)
                flowAmount += waterFlow;
            else
                flowAmount -= waterFlow;

            if(flowAmount >= FLOW_HEIGHT)
                flowUp = false;
            else if(flowAmount <= 0.0f)
                flowUp = true;

            waterNode->translate(0, (flowUp ? waterFlow : -waterFlow), 0);
        }

        static Vector3 vec;

        vec = Vector3::ZERO;

        if (mInputDevice->isKeyDown(KC_A))
        {
            // Move camera left
            vec.x = -moveScale;
        }

        if (mInputDevice->isKeyDown(KC_D))
        {
            // Move camera RIGHT
            vec.x = moveScale;
        }

        if (mInputDevice->isKeyDown(KC_UP) || mInputDevice->isKeyDown(KC_W))
        {
            // Move camera forward
            vec.z = -moveScale;
        }

        if (mInputDevice->isKeyDown(KC_DOWN) || mInputDevice->isKeyDown(KC_S))
        {
            // Move camera backward
            vec.z = moveScale;
        }

        if (mInputDevice->isKeyDown(KC_PGUP))
        {
            // Move camera up
            vec.y = moveScale;
        }

        if (mInputDevice->isKeyDown(KC_PGDOWN))
        {
            // Move camera down
            vec.y = -moveScale;
        }

        if (mInputDevice->isKeyDown(KC_RIGHT))
        {
            mCamera->yaw(-rotScale);
        }
        if (mInputDevice->isKeyDown(KC_LEFT))
        {
            mCamera->yaw(rotScale);
        }

        if( mInputDevice->isKeyDown( KC_ESCAPE) )
        {            
            return false;
        }

        // Rotate view by mouse relative position
        float rotX, rotY;
        rotX = -mInputDevice->getMouseRelativeX() * 0.13;
        rotY = -mInputDevice->getMouseRelativeY() * 0.13;


        // Make all the changes to the camera
        // Note that YAW direction is around a fixed axis (freelook stylee) rather than a natural YAW (e.g. airplane)
        mCamera->yaw(rotX);
        mCamera->pitch(rotY);
        mCamera->moveRelative(vec);

        // Rotate scene node if required
        SceneNode* node = mCamera->getSceneManager()->getRootSceneNode();
        if (mInputDevice->isKeyDown(KC_O))
        {
            node->yaw(rotScale);
        }
        if (mInputDevice->isKeyDown(KC_P))
        {
            node->yaw(-rotScale);
        }
        if (mInputDevice->isKeyDown(KC_I))
        {
            node->pitch(rotScale);
        }
        if (mInputDevice->isKeyDown(KC_K))
        {
            node->pitch(-rotScale);
        }

        if (mInputDevice->isKeyDown(KC_F) && timeUntilNextToggle <= 0)
        {
            mStatsOn = !mStatsOn;
            //Root::getSingleton().showDebugOverlay(mStatsOn);
            showDebugOverlay(mStatsOn);

            timeUntilNextToggle = 1;
        }


        // Return true to continue rendering
        return true;
    }

};


class TerrainApplication : public ExampleApplication
{
public:
    TerrainApplication() {}

protected:
    virtual void createFrameListener(void)
    {
        mFrameListener= new TerrainListener(mWindow, mCamera);
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
    }


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
   
   virtual void createViewports(void)
    {
        // Create one viewport, entire window
        Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(ColourValue::White);
    }

    // Just override the mandatory create scene method
    void createScene(void)
    {
        Entity *waterEntity;
        Plane waterPlane;

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
      
        // create a water plane/scene node
        waterPlane.normal = Vector3::UNIT_Y; 
        waterPlane.d = -1.5; 
        MeshManager::getSingleton().createPlane(
            "WaterPlane",
            waterPlane,
            2800, 2800,
            20, 20,
            true, 1, 
            10, 10,
            Vector3::UNIT_Z
        );

        waterEntity = mSceneMgr->createEntity("water", "WaterPlane"); 
        waterEntity->setMaterialName("Examples/TextureEffect4"); 

        SceneNode *waterNode = 
            mSceneMgr->getRootSceneNode()->createChildSceneNode("WaterNode"); 
        waterNode->attachObject(waterEntity); 
        waterNode->translate(1000, 0, 1000);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        mSceneMgr -> setWorldGeometry( "terrain.cfg" );

        mSceneMgr->setFog( FOG_EXP2, ColourValue::White, .008, 0,  250 );
        //mRoot -> showDebugOverlay( true );

    }

};
