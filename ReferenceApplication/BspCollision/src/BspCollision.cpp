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
/*
-----------------------------------------------------------------------------
Filename:    BspCollision.cpp
Description: Somewhere to play in the sand...
-----------------------------------------------------------------------------
*/

#include "OgreReferenceAppLayer.h"

#include "ExampleApplication.h"
#include <ode/collision.h>
#include "OgreStringConverter.h"

using namespace OgreRefApp;

// Hacky globals
World* pWorld;
ApplicationObject *head, *plane, *box, *ball;


// Event handler to add ability to alter curvature
class BspCollisionListener : public ExampleFrameListener
{
protected:
public:
    BspCollisionListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }


    bool frameEnded(const FrameEvent& evt)
    {
        // local just to stop toggles flipping too fast
        static Real timeUntilNextToggle = 0;

        // Deal with time delays that are too large
        // If we exceed this limit, we ignore
        static const Real MAX_TIME_INCREMENT = 0.5f;
        if (evt.timeSinceLastEvent > MAX_TIME_INCREMENT)
        {
            return true;
        }
        
        if (timeUntilNextToggle >= 0) 
            timeUntilNextToggle -= evt.timeSinceLastFrame;

        // Call superclass
        bool ret = ExampleFrameListener::frameStarted(evt);        

        if (mInputDevice->isKeyDown(KC_SPACE) && timeUntilNextToggle <= 0)
        {
            timeUntilNextToggle = 2;
            ball->setPosition(mCamera->getPosition());
            ball->setLinearVelocity(mCamera->getDirection() * 200);
            ball->setAngularVelocity(Vector3::ZERO);
        }


        // Perform simulation step
        pWorld->simulationStep(evt.timeSinceLastFrame);


        return ret;

    }
};

class BspCollisionApplication : public ExampleApplication
{
public:
    BspCollisionApplication() {
    

    
    }

    ~BspCollisionApplication() 
    {  
        delete pWorld;
    }

protected:
    
    void chooseSceneManager(void)
    {
        mSceneMgr = mRoot->getSceneManager( ST_INTERIOR );
    }
    // Just override the mandatory create scene method
    void createScene(void)
    {
       // Load Quake3 locations from a file
        ConfigFile cf;

        cf.load("quake3settings.cfg");

		ExampleApplication::setupResources();
        ResourceManager::addCommonArchiveEx(cf.getSetting("Pak0Location"), "Zip");

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.6, 0.6, 0.6));
        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setPosition(20,80,50);

        // Cam setup
        mCamera->setPosition(0,0,300);
		mCamera->lookAt(0,0,0);

        // Create World
        pWorld = new World(mSceneMgr, World::WT_REFAPP_BSP);
        pWorld->setGravity(Vector3(0, 0, -60));
        pWorld->getSceneManager()->setWorldGeometry(cf.getSetting("Map"));

        // modify camera for close work
        mCamera->setNearClipDistance(4);
        mCamera->setFarClipDistance(4000);

        // Also change position, and set Quake-type orientation
        // Get random player start point
        ViewPoint vp = mSceneMgr->getSuggestedViewpoint(true);
        mCamera->setPosition(vp.position);
        mCamera->pitch(90); // Quake uses X/Y horizon, Z up
        mCamera->rotate(vp.orientation);
        // Don't yaw along variable axis, causes leaning
        mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);

        ball = pWorld->createBall("ball", 15, vp.position + Vector3(0,0,80));
        ball->setDynamicsEnabled(true);
        ball->getEntity()->setMaterialName("Ogre/Eyes");

        ball = pWorld->createBall("ball2", 10, vp.position + Vector3(50,0,80));
        ball->setDynamicsEnabled(true);
        ball->setLinearVelocity(-10, 0, 0);
        ball->getEntity()->setMaterialName("Ogre/Eyes");

    }
    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new BspCollisionListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }

public:

};



#if OGRE_PLATFORM == PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    BspCollisionApplication app;

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occured: %s\n",
                e.getFullDescription().c_str());
#endif
    }


    return 0;
}







