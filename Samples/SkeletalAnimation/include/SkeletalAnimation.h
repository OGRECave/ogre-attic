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
/*
-----------------------------------------------------------------------------
Filename:    SkeletalAnimation.h
Description: Specialisation of OGRE's framework application to show the
             skeletal animation feature, including spline animation.
-----------------------------------------------------------------------------
*/


#include "ExampleApplication.h"


// Event handler to animate
class SkeletalAnimationFrameListener : public ExampleFrameListener
{
protected:
    AnimationState* mAnimState;
public:
    SkeletalAnimationFrameListener(RenderWindow* win, Camera* cam, AnimationState* anim)
        : ExampleFrameListener(win, cam)
    {
        mAnimState = anim;
    }

    bool frameStarted(const FrameEvent& evt)
    {
        mAnimState->addTime(evt.timeSinceLastFrame);

        // Call default
        return ExampleFrameListener::frameStarted(evt);

    }
};



class SkeletalApplication : public ExampleApplication
{
public:
    SkeletalApplication() {}

protected:
    AnimationState* mAnimState;

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Setup animation default
        Animation::setDefaultInterpolationMode(Animation::IM_SPLINE);

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));


       
        Entity *ent = mSceneMgr->createEntity("robot", "robot.mesh");

        // Add entity to the root scene node
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

        mAnimState = ent->getAnimationState("Walk");
        mAnimState->setEnabled(true);

        // Give it a little ambience with lights
        Light* l;
        l = mSceneMgr->createLight("BlueLight");
        l->setPosition(-200,-80,-100);
        l->setDiffuseColour(0.5, 0.5, 1.0);

        l = mSceneMgr->createLight("GreenLight");
        l->setPosition(0,0,-100);
        l->setDiffuseColour(0.5, 1.0, 0.5);

        // Position the camera
        mCamera->setPosition(100,50,100);
        mCamera->lookAt(-50,50,0);


    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new SkeletalAnimationFrameListener(mWindow, mCamera, mAnimState);
        mRoot->addFrameListener(mFrameListener);
    }


};

