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
/*
-----------------------------------------------------------------------------
Filename:    ParticleApplication.cpp
Description: Specialisation of OGRE's framework application to show the
             environment mapping feature.
-----------------------------------------------------------------------------
*/


#include "ExampleApplication.h"


// Event handler to add ability to alter curvature
class ParticleFrameListener : public ExampleFrameListener
{
protected:
    SceneNode* mFountainNode;
public:
    ParticleFrameListener(RenderWindow* win, Camera* cam, SceneNode* fountainNode)
        : ExampleFrameListener(win, cam)
    {
        mFountainNode = fountainNode;
    }

    bool frameStarted(const FrameEvent& evt)
    {

        // Rotate fountains
        mFountainNode->yaw(evt.timeSinceLastFrame * 30);

        // Call default
        return ExampleFrameListener::frameStarted(evt);

    }
};



class ParticleApplication : public ExampleApplication
{
public:
    ParticleApplication() {}

protected:
    SceneNode* mFountainNode;

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));


       
        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");

        // Add entity to the root scene node
        mSceneMgr->getRootSceneNode()->createChild()->attachObject(ent);


        // Green nimbus around Ogre
        ParticleSystem* pSys1 = ParticleSystemManager::getSingleton().createSystem("Nimbus", 
            "Examples/GreenyNimbus");
        mSceneMgr->getRootSceneNode()->createChild()->attachObject(pSys1);


        // Create shared node for 2 fountains
        mFountainNode = mSceneMgr->getRootSceneNode()->createChild();

        // fountain 1
        ParticleSystem* pSys2 = ParticleSystemManager::getSingleton().createSystem("fountain1", 
            "Examples/PurpleFountain");
        // Point the fountain at an angle
        SceneNode* fNode = mFountainNode->createChild();
        fNode->translate(200,-100,0);
        fNode->rotate(Vector3::UNIT_Z, 20);
        fNode->attachObject(pSys2);

        // fountain 2
        ParticleSystem* pSys3 = ParticleSystemManager::getSingleton().createSystem("fountain2", 
            "Examples/PurpleFountain");
        // Point the fountain at an angle
        fNode = mFountainNode->createChild();
        fNode->translate(-200,-100,0);
        fNode->rotate(Vector3::UNIT_Z, -20);
        fNode->attachObject(pSys3);




        // Create a rainstorm 
        ParticleSystem* pSys4 = ParticleSystemManager::getSingleton().createSystem("rain", 
            "Examples/Rain");
        SceneNode* rNode = mSceneMgr->getRootSceneNode()->createChild();
        rNode->translate(0,1000,0);
        rNode->attachObject(pSys4);
        // Fast-forward the rain so it looks more natural
        pSys4->fastForward(5);


    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new ParticleFrameListener(mWindow, mCamera, mFountainNode);
        mRoot->addFrameListener(mFrameListener);
    }


};

