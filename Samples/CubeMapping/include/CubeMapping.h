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
        CubeMapping.h
    \brief
        Specialisation of OGRE's framework application to show the
        cube mapping feature where a wrap-around environment is reflected
        off of an object.
*/

#include "ExampleApplication.h"

class CubeMapApplication : public ExampleApplication
{
public:
    CubeMapApplication() {}

protected:

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a skybox
        mSceneMgr->setSkyBox(true, "Examples/SceneSkyBox");

        // Set direction of camera
        mCamera->setDirection(Vector3::UNIT_Z);
        mCamera->setPosition(0,0,0);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        SceneNode* s = static_cast<SceneNode*>(mSceneMgr->getRootSceneNode()->createChild());
        s->setPosition(20,20,600);

        Entity* ent = mSceneMgr->createEntity( "sphere", "sphere.mesh" );
        ent->setMaterialName( "Examples/SceneCubeMap" );

        s->attachObject( ent );
    }

};
