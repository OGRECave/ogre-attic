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
        BSP.h
    \brief
        Specialisation of OGRE's framework application to show the
        indoor level rendering (Binary Space Partition or BSP based).
    \par
        Also demonstrates loading levels from Quake3Arena and using
        curved bezier surfaces (as demonstrated in the Bezier example)
        in a large level.
*/

#include "ExampleApplication.h"

class BspApplication : public ExampleApplication
{
public:
    BspApplication()
    {


    }

protected:

    String mQuakePk3;
    String mQuakeLevel;

    // Override resource sources (include Quake3 archives)
    void setupResources(void)
    {

        // Load Quake3 locations from a file
        ConfigFile cf;

        cf.load("quake3settings.cfg");

        mQuakePk3 = cf.getSetting("Pak0Location");
        mQuakeLevel = cf.getSetting("Map");

		ExampleApplication::setupResources();
        ResourceManager::addCommonArchiveEx(mQuakePk3, "Zip");

    }
    // Override scene manager (use indoor instead of generic)
    void chooseSceneManager(void)
    {
        mSceneMgr = mRoot->getSceneManager(ST_INTERIOR);
    }
    // Scene creation
    void createScene(void)
    {

        // Load world geometry
        mSceneMgr->setWorldGeometry(mQuakeLevel);

        // modify camera for close work
        mCamera->setNearClipDistance(4);
        mCamera->setFarClipDistance(4000);

        // Also change position, and set Quake-type orientation
        // Get random player start point
        ViewPoint vp = mSceneMgr->getSuggestedViewpoint(true);
        mCamera->setPosition(vp.position);
        mCamera->pitch(Degree(90)); // Quake uses X/Y horizon, Z up
        mCamera->rotate(vp.orientation);
        // Don't yaw along variable axis, causes leaning
        mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);


    }

};
