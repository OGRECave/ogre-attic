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
#include "OgreConfigFile.h"

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
        mCamera->pitch(90); // Quake uses X/Y horizon, Z up
        mCamera->rotate(vp.orientation);
        // Don't yaw along variable axis, causes leaning
        mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);


    }

};
