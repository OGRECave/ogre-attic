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
        TextureFX.h
    \brief
        Shows OGRE's ability to handle different types of texture effects.
*/


#include "ExampleApplication.h"

class TextureEffectsApplication : public ExampleApplication
{
public:
    TextureEffectsApplication() {}

protected:

    void createScalingPlane()
    {
        // Set up a material for the plane

        // Create a prefab plane
        Entity *planeEnt = mSceneMgr->createEntity("Plane", SceneManager::PT_PLANE);
        // Give the plane a texture
        planeEnt->setMaterialName("Examples/TextureEffect1");

        SceneNode* node = static_cast<SceneNode*>(
            mSceneMgr->getRootSceneNode()->createChild(Vector3(-250,-40,-100)));

        node->attachObject(planeEnt);
    }

    void createScrollingKnot()
    {
        Entity *ent = mSceneMgr->createEntity("knot", "knot.mesh");


        ent->setMaterialName("Examples/TextureEffect2");
        // Add entity to the root scene node
        SceneNode* node = static_cast<SceneNode*>(
            mSceneMgr->getRootSceneNode()->createChild(Vector3(200,50,150)));

        node->attachObject(ent);

    }

    void createWateryPlane()
    {
        // Create a prefab plane
        Entity *planeEnt = mSceneMgr->createEntity("WaterPlane", SceneManager::PT_PLANE);
        // Give the plane a texture
        planeEnt->setMaterialName("Examples/TextureEffect3");

        mSceneMgr->getRootSceneNode()->attachObject(planeEnt);
    }

    // Just override the mandatory create scene method
    void createScene(void)
    {

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);


        createScalingPlane();
        createScrollingKnot();
        createWateryPlane();


        // Set up a material for the skydome
        Material *skyMat = mSceneMgr->createMaterial("SkyMat");
        // Perform no dynamic lighting on the sky
        skyMat->setLightingEnabled(false);
        // Use a cloudy sky
        Material::TextureLayer* t = skyMat->addTextureLayer("clouds.jpg");
        // Scroll the clouds
        t->setScrollAnimation(0.15, 0);

        // System will automatically set no depth write

        // Create a skydome
        mSceneMgr->setSkyDome(true, "SkyMat", -5, 2);





    }

};
