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
        SkyBox.h
    \brief
        Specialisation of OGRE's framework application to show the
        skybox feature where a wrap-around environment is projected
        onto a cube around the camera.
*/

#include "ExampleApplication.h"

class SkyBoxFrameListener : public ExampleFrameListener
{
private:
    static float fDefDim;
    static float fDefVel;

public:
    SkyBoxFrameListener(RenderWindow* win, Camera* cam) : ExampleFrameListener( win, cam )
    {
    }

    bool frameStarted( const FrameEvent& evt )
    {
        bool bOK = ExampleFrameListener::frameStarted( evt );

        if( mInputDevice->isKeyDown( KC_N ) )
        {
            ParticleSystem* pSys = reinterpret_cast<ParticleSystem*>( mCamera->getSceneManager()->getRootSceneNode()->getChild( 0 )->getAttachedObject( 0 ) );
            pSys->setDefaultDimensions( fDefDim + 0.25, fDefDim + 0.25 );
            fDefDim += 0.25;
        }

        if( mInputDevice->isKeyDown( KC_M ) )
        {
            ParticleSystem* pSys = reinterpret_cast<ParticleSystem*>( mCamera->getSceneManager()->getRootSceneNode()->getChild( 0 )->getAttachedObject( 0 ) );
            pSys->setDefaultDimensions( fDefDim - 0.25, fDefDim - 0.25 );
            fDefDim -= 0.25;
        }

        if( mInputDevice->isKeyDown( KC_H ) )
        {
            ParticleSystem* pSys = reinterpret_cast<ParticleSystem*>( mCamera->getSceneManager()->getRootSceneNode()->getChild( 0 )->getAttachedObject( 0 ) );
            pSys->getEmitter( 0 )->setParticleVelocity( fDefVel + 1 );
            pSys->getEmitter( 1 )->setParticleVelocity( fDefVel + 1 );
            fDefVel += 1;            
        }

        if( mInputDevice->isKeyDown( KC_J ) && !( fDefVel < 0.0f ) )
        {
            ParticleSystem* pSys = reinterpret_cast<ParticleSystem*>( mCamera->getSceneManager()->getRootSceneNode()->getChild( 0 )->getAttachedObject( 0 ) );
            pSys->getEmitter( 0 )->setParticleVelocity( fDefVel - 1 );
            pSys->getEmitter( 1 )->setParticleVelocity( fDefVel - 1 );
            fDefVel -= 1;            
        }

        return bOK;
    }
};

float SkyBoxFrameListener::fDefDim = 25.0f;
float SkyBoxFrameListener::fDefVel = 50.0f;

class SkyBoxApplication : public ExampleApplication
{
public:
    SkyBoxApplication() {}

protected:
    virtual void createFrameListener(void)
    {
        mFrameListener= new SkyBoxFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a skybox
        mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 50 );

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        // Also add a nice starship in
        Entity *ent = mSceneMgr->createEntity( "razor", "razor.mesh" );

        mSceneMgr->getRootSceneNode()->attachObject( ent );

        ParticleSystem *pThrusters = ParticleSystemManager::getSingleton().createSystem( "ParticleSys1", 200 );

        pThrusters ->setMaterialName( "Examples/Flare" );
        pThrusters ->setDefaultDimensions( 25, 25 );

        ParticleEmitter *pEmit1 = pThrusters ->addEmitter( "Point" );
        ParticleEmitter *pEmit2 = pThrusters ->addEmitter( "Point" );

        // Thruster 1
        pEmit1->setAngle( 3 );
        pEmit1->setTimeToLive( 0.2 );
        pEmit1->setEmissionRate( 70 );

        pEmit1->setParticleVelocity( 50 );

        pEmit1->setDirection(- Vector3::UNIT_Z);
        pEmit1->setColour( ColourValue::White, ColourValue::Red);        

        // Thruster 2
        pEmit2->setAngle( 3 );
        pEmit2->setTimeToLive( 0.2 );
        pEmit2->setEmissionRate( 70 );

        pEmit2->setParticleVelocity( 50 );

        pEmit2->setDirection( -Vector3::UNIT_Z );
        pEmit2->setColour( ColourValue::White, ColourValue::Red );

        // Set the position of the thrusters
        pEmit1->setPosition( Vector3( 5.7f, 0.0f, 0.0f ) );
        pEmit2->setPosition( Vector3( -18.0f, 0.0f, 0.0f ) );

        mSceneMgr->getRootSceneNode()->createChild( Vector3( 0.0f, 6.5f, -67.0f ) )->attachObject(pThrusters);
    }

};
