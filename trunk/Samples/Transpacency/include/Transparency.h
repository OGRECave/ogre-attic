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
        Transparency.h
    \brief
        Specialisation of OGRE's framework application to show the transparency,
        or scene blending features.
    \par
        Note that this is a little rudimentary - it's because whilst
        OGRE supports lots of blending options, the SceneManager has
        to ensure the rendering order is correct when object transparency
        is enabled. Right now this is not quite right in the default
        manager so this scene is kept deliberately simple.
*/

#include "ExampleApplication.h"

class TransApplication : public ExampleApplication
{
public:
    TransApplication() {}

protected:

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


        // Create a prefab plane
        Entity *planeEnt = mSceneMgr->createEntity("Plane", SceneManager::PT_PLANE);
        // Give the plane a texture
        planeEnt->setMaterialName("Examples/BumpyMetal");

        // Create an entity from a model (will be loaded automatically)
        Entity* knotEnt = mSceneMgr->createEntity("Knot", "knot.oof");

        knotEnt->setMaterialName("Examples/TransparentTest");

        // Attach the 2 new entities to the root of the scene
        SceneNode* rootNode = mSceneMgr->getRootSceneNode();
        rootNode->attachObject(planeEnt);
        rootNode->attachObject(knotEnt);

        // Add a whole bunch of extra transparent entities
        Entity *cloneEnt;
        for (int n = 0; n < 10; ++n)
        {
            // Create a new node under the root
            SceneNode* node = mSceneMgr->createSceneNode();
            // Random translate
            Vector3 nodePos;
            nodePos.x = Math::getSingleton().SymmetricRandom() * 500;
            nodePos.y = Math::getSingleton().SymmetricRandom() * 500;
            nodePos.z = Math::getSingleton().SymmetricRandom() * 500;
            node->setPosition(nodePos);
            rootNode->addChild(node);
            // Clone knot
            char cloneName[12];
            sprintf(cloneName, "Knot%d", n);
            cloneEnt = knotEnt->clone(cloneName);
            // Attach to new node
            node->attachObject(cloneEnt);

        }



    }

};
