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
        Nature.h
    \brief
        Test for the nature plugin 
*/

#include "ExampleApplication.h"

#define FLOW_SPEED 0.2
#define FLOW_HEIGHT 0.8

class NatureListener : public ExampleFrameListener
{
  public:
    NatureListener(SceneManager *sceneMgr, RenderWindow* win, Camera* cam) :ExampleFrameListener(win, cam) 
    { mSceneMgr = sceneMgr; };

 // Override frameStarted event to process that (don't care about frameEnded)
    bool frameStarted(const FrameEvent& evt)
    {
        float moveScale;
        Radian rotScale;
        float waterFlow;
        static float flowAmount = 0.0f;
        static bool flowUp = true;
	static bool wireframe = false;
	static Real targetQuality = 3.0, minimumQuality = 10;

        // local just to stop toggles flipping too fast
        static Real timeUntilNextToggle = 0;

        if (timeUntilNextToggle >= 0)
            timeUntilNextToggle -= evt.timeSinceLastFrame;

        // If this is the first frame, pick a speed
        if (evt.timeSinceLastFrame == 0)
        {
            moveScale = 1;
            rotScale = Degree(0.1);
            waterFlow = 0.0f;
        }
        // Otherwise scale movement units by time passed since last frame
        else
        {
            // Move about 2 units per second,
            moveScale = 50 * evt.timeSinceLastFrame;
            // Take about 10 seconds for full rotation
            rotScale = Degree(36 * evt.timeSinceLastFrame);
            
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
        Radian rotX = Degree(-mInputDevice->getMouseRelativeX() * 0.13);
        Radian rotY = Degree(-mInputDevice->getMouseRelativeY() * 0.13);


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
            showDebugOverlay(mStatsOn);

            timeUntilNextToggle = 1;
        }

        if (mInputDevice->isKeyDown(KC_T) && timeUntilNextToggle <= 0)
        {
	    wireframe = !wireframe;
	    if (wireframe)
		mCamera->setDetailLevel(SDL_WIREFRAME);
	    else
		mCamera->setDetailLevel(SDL_SOLID);

            timeUntilNextToggle = 0.5;
        }

	if (mInputDevice->isKeyDown(KC_9) && timeUntilNextToggle <= 0)
	{
	    targetQuality += 0.25;
	    mSceneMgr->setOption("TARGET_QUALITY", &targetQuality);
	    timeUntilNextToggle = 0.20;
	}

	if (mInputDevice->isKeyDown(KC_8) && timeUntilNextToggle <= 0)
	{
	    targetQuality -= 0.25;
	    mSceneMgr->setOption("TARGET_QUALITY", &targetQuality);
	    timeUntilNextToggle = 0.20;
	}


	if (mInputDevice->isKeyDown(KC_6) && timeUntilNextToggle <= 0)
	{
	    minimumQuality += 1;
	    mSceneMgr->setOption("MINIMUM_QUALITY", &minimumQuality);
	    timeUntilNextToggle = 0.20;
	}

	if (mInputDevice->isKeyDown(KC_5) && timeUntilNextToggle <= 0)
	{
	    minimumQuality -= 1;
	    mSceneMgr->setOption("MINIMUM_QUALITY", &minimumQuality);
	    timeUntilNextToggle = 0.20;
	}

        // Return true to continue rendering
        return true;
    }

private:
    SceneManager *mSceneMgr;

};


class NatureApplication : public ExampleApplication
{
public:
    NatureApplication() {}

protected:
    virtual void createFrameListener(void)
    {
        mFrameListener= new NatureListener(mSceneMgr, mWindow, mCamera);
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
    }


    virtual void chooseSceneManager(void)
    {
        // Get the SceneManager, in this case a generic one
        mSceneMgr = mRoot->getSceneManager( ST_EXTERIOR_FAR );
    }

    virtual void createCamera(void)
    {
        // Create the camera
        mCamera = mSceneMgr->createCamera("PlayerCam");

        // Position it at 500 in Z direction
        mCamera->setPosition(Vector3(250, 50, 250));
        // Look back along -Z
        mCamera->lookAt(Vector3(0,0,0));
        mCamera->setNearClipDistance( 5 );

        mCamera->setFarClipDistance( 5000 );
    }
   
    virtual void createViewports(void)
    {
        // Create one viewport, entire window
        Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(ColourValue(0.77, 0.86, 1.0));
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
        waterPlane.d = -30; 
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

        SceneNode *waterNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("WaterNode"); 
        waterNode->attachObject(waterEntity); 
        waterNode->translate(1000, 0, 1000);



        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        mSceneMgr->setWorldGeometry( "world.cfg" );

        mSceneMgr->setFog(FOG_EXP2, ColourValue(0.77, 0.86, 1.0), 0.0015, 0,0 );
        //mSceneMgr->setFog(FOG_LINEAR, ColourValue(0.77, 0.86, 1.0), 0, 150, 500);
       //mRoot -> showDebugOverlay( true );

       mCamera->setPosition(2000,100,2000);
    }

};
