/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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
Filename:    CelShading.cpp
Description: Demo fo cel-shaded graphics using vertex & fragment programs
-----------------------------------------------------------------------------
*/

#include "Ogre.h"

#include "ExampleApplication.h"
#include "OgreStringConverter.h"

SceneNode* rotNode;

// Listener class for frame updates
class CelShadingListener : public ExampleFrameListener
{
protected:
public:
    CelShadingListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }

    bool frameStarted(const FrameEvent& evt)
    {
        rotNode->yaw(evt.timeSinceLastFrame * 30);
        // Call superclass
        return ExampleFrameListener::frameStarted(evt);
    }
};


class CelShadingApplication : public ExampleApplication
{
public:
    CelShadingApplication() { 
    }

    ~CelShadingApplication() {  }

protected:
    
	void createFrameListener(void)
    {
		// This is where we instantiate our own frame listener
        mFrameListener= new CelShadingListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);

    }
    

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // Add light to the scene node
        rotNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        rotNode->createChildSceneNode(Vector3(20,40,50))->attachObject(l);

        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");

        mCamera->setPosition(20, 0, 100);
        mCamera->lookAt(0,0,0);


        // Set materials loaded from Example.material
        ent->getSubEntity(0)->setMaterialName("Examples/OgreCelShading/Eyes");
        ent->getSubEntity(1)->setMaterialName("Examples/OgreCelShading/Skin");
        ent->getSubEntity(2)->setMaterialName("Examples/OgreCelShading/Earring");
        ent->getSubEntity(3)->setMaterialName("Examples/OgreCelShading/Teeth");

        // Add entity to the root scene node
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

        mWindow->getViewport(0)->setBackgroundColour(ColourValue::White);
    }
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
    CelShadingApplication app;

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
