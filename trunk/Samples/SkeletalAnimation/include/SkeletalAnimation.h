/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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

#define NUM_ROBOTS 10
#define ROW_COUNT 10
AnimationState* mAnimState[NUM_ROBOTS];
Real mAnimationSpeed[NUM_ROBOTS];

// Event handler to animate
class SkeletalAnimationFrameListener : public ExampleFrameListener
{
protected:
public:
    SkeletalAnimationFrameListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }

    bool frameStarted(const FrameEvent& evt)
    {
        for (int i = 0; i < NUM_ROBOTS; ++i)
        {
            mAnimState[i]->addTime(evt.timeSinceLastFrame * mAnimationSpeed[i]);
        }

        // Call default
        return ExampleFrameListener::frameStarted(evt);

    }
};



class SkeletalApplication : public ExampleApplication
{
public:
    SkeletalApplication() {}

protected:

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Setup animation default
        Animation::setDefaultInterpolationMode(Animation::IM_LINEAR);
        Animation::setDefaultRotationInterpolationMode(Animation::RIM_LINEAR);

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));


        
        Entity *ent;
        int row = 0;
        int column = 0;
        for (int i = 0; i < NUM_ROBOTS; ++i, ++column)
        {
            if (column > ROW_COUNT)
            {
                ++row;
                column = 0;
            }

            ent = mSceneMgr->createEntity("robot" + StringConverter::toString(i), "robot.mesh");
            // Add entity to the scene node
            mSceneMgr->getRootSceneNode()->createChildSceneNode(
                Vector3(-(row*100), 0,(column*50)))->attachObject(ent);
            mAnimState[i] = ent->getAnimationState("Walk");
            mAnimState[i]->setEnabled(true);
            mAnimationSpeed[i] = Math::RangeRandom(0.5, 1.5);
        }



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

        // Report whether hardware skinning is enabled or not
        Technique* t = ent->getSubEntity(0)->getMaterial()->getBestTechnique();
        Pass* p = t->getPass(0);
        if (p->hasVertexProgram() && 
            p->getVertexProgram()->isSkeletalAnimationIncluded())
        {
            mWindow->setDebugText("Hardware skinning is enabled");
        }
        else
        {
            mWindow->setDebugText("Software skinning is enabled");
        }




    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new SkeletalAnimationFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }


};

