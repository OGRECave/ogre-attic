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

/**
    \file 
        Lighting.h
    \brief
        Shows lighting support in OGRE.
*/

#include "ExampleApplication.h"

class LightsApplication : public ExampleApplication
{
public:
    LightsApplication() {}

protected:

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create an entity from a model (will be loaded automatically)
        Entity* ent = mSceneMgr->createEntity("Ogre", "ogrehead.mesh");

        // Create 3 lights each of a different colour
        Light* l = mSceneMgr->createLight("RedLight");
        l->setPosition(200,80,100);
        l->setDiffuseColour(ColourValue::Red);

        l = mSceneMgr->createLight("BlueLight");
        l->setPosition(-200,-80,-100);
        l->setDiffuseColour(ColourValue::Blue);

        l = mSceneMgr->createLight("GreenLight");
        l->setPosition(0,0,-100);
        l->setDiffuseColour(ColourValue::Green);


        // Set material
        //ent->setMaterialName("Examples/DarkMaterial");
        // Attach the entity to the root of the scene
        mSceneMgr->getRootSceneNode()->attachObject(ent);



    }

};
