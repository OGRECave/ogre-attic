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
Filename:    PlayPen.cpp
Description: Somewhere to play in the sand...
-----------------------------------------------------------------------------
*/

#include "Ogre.h"

#include "ExampleApplication.h"

AnimationState* mAnimState;

// Event handler 
class CameraTrackListener: public ExampleFrameListener
{
protected:
public:
    CameraTrackListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }

    bool frameStarted(const FrameEvent& evt)
    {

        mAnimState->addTime(evt.timeSinceLastFrame);

        // Call default
        return ExampleFrameListener::frameStarted(evt);
        

    }
};

class CameraTrackApplication : public ExampleApplication
{
public:
    CameraTrackApplication() {
    

    
    }

    ~CameraTrackApplication() {  }

protected:
    SceneNode* mFountainNode;

    // Just override the mandatory create scene method
    void createScene(void)
    {

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

        // Create a skydome
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        Entity *ent;

        // Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane("FloorPlane",p,200000,200000,20,20,true,1,50,50,Vector3::UNIT_Z);

        // Create an entity (the floor)
        ent = mSceneMgr->createEntity("floor", "FloorPlane");
        ent->setMaterialName("Examples/RustySteel");
        // Attach to child of root node, better for culling (otherwise bounds are the combination of the 2)
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

        // Add a head, give it it's own node
        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
        headNode->attachObject(ent);

        // Make sure the camera track this node
        mCamera->setAutoTracking(true, headNode);

        // Create the camera node & attach camera
        SceneNode* camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        camNode->attachObject(mCamera);

        // set up spline animation of node
        Animation* anim = mSceneMgr->createAnimation("CameraTrack", 10);
        // Spline it for nice curves
        anim->setInterpolationMode(Animation::IM_SPLINE);
        // Create a track to animate the camera's node
        AnimationTrack* track = anim->createTrack(0, camNode);
        // Setup keyframes
        KeyFrame* key = track->createKeyFrame(0); // startposition
        key = track->createKeyFrame(2.5);
        key->setTranslate(Vector3(500,500,-1000));
        key = track->createKeyFrame(5);
        key->setTranslate(Vector3(-1500,1000,-600));
        key = track->createKeyFrame(7.5);
        key->setTranslate(Vector3(0,-100,0));
        key = track->createKeyFrame(10);
        key->setTranslate(Vector3(0,0,0));
        // Create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("CameraTrack");
        mAnimState->setEnabled(true);

        // Put in a bit of fog for the hell of it
        mSceneMgr->setFog(FOG_EXP, ColourValue::White, 0.0002);

    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new CameraTrackListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);

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
    CameraTrackApplication app;

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







