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

