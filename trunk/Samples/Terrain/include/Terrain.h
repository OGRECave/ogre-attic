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

/**
    \file 
        Terrain.h
    \brief
        Specialisation of OGRE's framework application to show the
        terrain rendering plugin 
*/

#include "ExampleApplication.h"
#include "OgreStringConverter.h"

class TerrainListener : public ExampleFrameListener
{
  public:
    TerrainListener(RenderWindow* win, Camera* cam) :ExampleFrameListener(win, cam) { };

 // Override frameStarted event to process that (don't care about frameEnded)
    bool frameStarted(const FrameEvent& evt)
    {
        float moveScale;
        float rotScale;
        // local just to stop toggles flipping too fast
        static Real timeUntilNextToggle = 0;

        String text = "Time: ";
        text += StringConverter::toString(evt.timeSinceLastFrame);
        mWindow->setDebugText(text);

        if (timeUntilNextToggle >= 0)
            timeUntilNextToggle -= evt.timeSinceLastFrame;

        // If this is the first frame, pick a speed
        if (evt.timeSinceLastFrame == 0)
        {
            moveScale = 1;
            rotScale = 0.1;
        }
        // Otherwise scale movement units by time passed since last frame
        else
        {
            // Move about 100 units per second,
            moveScale = 10.0 * evt.timeSinceLastFrame;
            // Take about 10 seconds for full rotation
            rotScale = 36 * evt.timeSinceLastFrame;
        }

        // Grab input device state
        mInputDevice->capture();

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
            Root::getSingleton().showDebugOverlay(mStatsOn);

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
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));


        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        mSceneMgr -> setWorldGeometry( "terrain.cfg" );

        mSceneMgr->setFog( FOG_EXP2, ColourValue::White, .008, 0,  250 );
        mRoot -> showDebugOverlay( true );

    }

};
