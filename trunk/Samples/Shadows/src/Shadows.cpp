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

/**
    \file 
        Shadows.cpp
    \brief
        Shows a few ways to use Ogre's shadowing techniques
*/

#include "Ogre.h"
#include "ExampleApplication.h"
#include "OgreProgressiveMesh.h"
#include "OgreStringConverter.h"
#include "OgreEdgeListBuilder.h"

/*
#include "OgreNoMemoryMacros.h"
#include <ode/odecpp.h>
#include <ode/odecpp_collision.h>
#include "OgreMemoryMacros.h"
*/

/*
#if OGRE_PLATFORM == PLATFORM_WIN32
#include "OgreNoMemoryMacros.h"
#include <crtdbg.h>
#endi*/

Entity* mEntity;
AnimationState* mAnimState = 0;
Entity* pPlaneEnt;
Light* mLight;

class ShadowsListener : public ExampleFrameListener
{
protected:

public:
    ShadowsListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }


    bool frameEnded(const FrameEvent& evt)
    {
        // local just to stop toggles flipping too fast
        static Real timeUntilNextToggle = 0;

        if (timeUntilNextToggle >= 0) 
            timeUntilNextToggle -= evt.timeSinceLastFrame;

        static bool mWireframe = false;
        if (mInputDevice->isKeyDown(KC_G) && timeUntilNextToggle <= 0)
        {
            mWireframe = !mWireframe;
            if (mWireframe)
            {
                mCamera->setDetailLevel(SDL_WIREFRAME);
            }
            else
            {
                mCamera->setDetailLevel(SDL_SOLID);
            }
            timeUntilNextToggle = 0.5;

        }


        if (mAnimState)
            mAnimState->addTime(evt.timeSinceLastFrame);


        return ExampleFrameListener::frameStarted(evt) && ExampleFrameListener::frameEnded(evt);        

    }


};

class ShadowsApplication : public ExampleApplication
{
public:
    ShadowsApplication() {


    }

    ~ShadowsApplication() 
    {
    }
protected:

    void chooseSceneManager(void)
    {
        mSceneMgr = mRoot->getSceneManager( ST_GENERIC );
    }



    void testStencilShadows()
    {
        mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
        //mSceneMgr->setShowDebugShadows(true);
        //mSceneMgr->setShadowColour(ColourValue(0.4, 0.25, 0.25));
        //mSceneMgr->setShadowFarDistance(800);
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));

        // Point light
        mLight = mSceneMgr->createLight("MainLight");
        mLight->setPosition(-400,400,-300);
        mLight->setDiffuseColour(1.0, 0, 0);
        mLight->setSpecularColour(0.9, 0.9, 1);
        mLight->setAttenuation(1000,1,0.001,0);

        // Directional light
        mLight = mSceneMgr->createLight("Light2");
        Vector3 dir(-1,-1,0);
        dir.normalise();
        mLight->setType(Light::LT_DIRECTIONAL);
        mLight->setDirection(dir);
        mLight->setDiffuseColour(1, 1, 1);
        mLight->setSpecularColour(1, 1, 1);

        SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        Entity* pEnt;
        pEnt = mSceneMgr->createEntity( "1", "ninja.mesh" );
        mAnimState = pEnt->getAnimationState("Walk");
        mAnimState->setEnabled(true);
        //pEnt->setMaterialName("Examples/Rocky");
        node->attachObject( pEnt );

        pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
        pEnt->setMaterialName("Examples/EnvMappedRustySteel");
        //Material* mat2 = (Material*)MaterialManager::getSingleton().getByName("Examples/EnvMappedRustySteel");
        //mat2->setReceiveShadows(false);
        node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200, 0, -200));
        node->attachObject( pEnt );

        //Mesh* msh = MeshManager::getSingleton().load("knot.mesh");
        //msh->buildTangentVectors();
        pEnt = mSceneMgr->createEntity( "4", "knot.mesh" );
        //pEnt->setMaterialName("Examples/BumpMapping/MultiLightSpecular");
        node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 200));
        node->attachObject( pEnt );


        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");


        Plane plane;
        plane.normal = Vector3::UNIT_Y;
        plane.d = 100;
        MeshManager::getSingleton().createPlane("Myplane",plane,
            1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
        Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName("2 - Default");
        pPlaneEnt->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);


    }

    void testTextureShadows()
    {
        mSceneMgr->setShadowTextureSize(512);
        mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
        mSceneMgr->setShadowFarDistance(1500);
        mSceneMgr->setShadowColour(ColourValue(0, 0, 0));
        //mSceneMgr->setShadowFarDistance(800);
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));

        mLight = mSceneMgr->createLight("MainLight");

        /*
        // Directional test
        mLight->setType(Light::LT_DIRECTIONAL);
        Vector3 vec(-1,-1,0);
        vec.normalise();
        mLight->setDirection(vec);
        */

        // Spotlight test
        mLight->setType(Light::LT_SPOTLIGHT);
        mLight->setDiffuseColour(1.0, 1.0, 0.8);
        SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        node->setPosition(800,600,0);
        node->lookAt(Vector3(0,0,0), Node::TS_WORLD, Vector3::UNIT_Z);
        node->attachObject(mLight);

        node = mSceneMgr->getRootSceneNode()->createChildSceneNode();


        Entity* pEnt;
        pEnt = mSceneMgr->createEntity( "1", "ninja.mesh" );
        mAnimState = pEnt->getAnimationState("Walk");
        mAnimState->setEnabled(true);
        node->attachObject( pEnt );
        node->translate(0,-100,0);

        pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
        node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200, 0, -200));
        node->attachObject( pEnt );

        Mesh* msh = MeshManager::getSingleton().load("knot.mesh");
        msh->buildTangentVectors();
        pEnt = mSceneMgr->createEntity( "4", "knot.mesh" );
        //pEnt->setMaterialName("Examples/BumpMapping/MultiLightSpecular");
        node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 200));
        node->attachObject( pEnt );

        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");


        Plane plane;
        plane.normal = Vector3::UNIT_Y;
        plane.d = 100;
        MeshManager::getSingleton().createPlane("Myplane",plane,
            1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
        Entity* pPlaneEnt;
        pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName("2 - Default");
        pPlaneEnt->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

        // Set up a debug panel to display the shadow
        /*
        Material* debugMat = (Material*)MaterialManager::getSingleton().create("Ogre/DebugShadowMap");
        debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
        TextureUnitState *t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState("Ogre/ShadowTexture0");
        t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
        //t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState("spot_shadow_fade.png");
        //t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
        //t->setColourOperation(LBO_ADD);

        GuiContainer* debugPanel = (GuiContainer*)
            (GuiManager::getSingleton().createGuiElement("Panel", "Ogre/DebugShadowPanel"));
        debugPanel->_setPosition(0.8, 0);
        debugPanel->_setDimensions(0.2, 0.3);
        debugPanel->setMaterialName("Ogre/DebugShadowMap");
        Overlay* debugOverlay = (Overlay*)OverlayManager::getSingleton().getByName("Core/DebugOverlay");
        debugOverlay->add2D(debugPanel);
        */

    }

    // Just override the mandatory create scene method
    void createScene(void)
    {
        testStencilShadows();
        //testTextureShadows();
    }
    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new ShadowsListener(mWindow, mCamera);
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);

    }


public:
    void go(void)
    {
        if (!setup())
            return;

        mRoot->startRendering();
    }


};



#if OGRE_PLATFORM == PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{
    // Create application object
    ShadowsApplication app;

    SET_TERM_HANDLER;
    
    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
}
