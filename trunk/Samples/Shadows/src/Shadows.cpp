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
SceneNode* mLightNode = 0;
AnimationState* mLightAnimationState = 0;
ColourValue mMinLightColour(0.3, 0.0, 0);
ColourValue mMaxLightColour(0.4, 0.2, 0.0);
Real mMinFlareSize = 20;
Real mMaxFlareSize = 40;




/** This class 'wibbles' the light and billboard */
class LightWibbler : public ControllerValue<Real>
{
protected:
    Light* mLight;
    Billboard* mBillboard;
    ColourValue mColourRange;
    ColourValue mMinColour;
    Real mMinSize;
    Real mSizeRange;
    Real intensity;
public:
    LightWibbler(Light* light, Billboard* billboard, const ColourValue& minColour, 
        const ColourValue& maxColour, Real minSize, Real maxSize)
    {
        mLight = light;
        mBillboard = billboard;
        mMinColour = minColour;
        mColourRange.r = maxColour.r - minColour.r;
        mColourRange.g = maxColour.g - minColour.g;
        mColourRange.b = maxColour.b - minColour.b;
        mMinSize = minSize;
        mSizeRange = maxSize - minSize;
    }

    virtual Real  getValue (void) const
    {
        return intensity;
    }

    virtual void  setValue (Real value)
    {
        intensity = value;

        ColourValue newColour;

        // Attenuate the brightness of the light
        newColour.r = mMinColour.r + (mColourRange.r * intensity);
        newColour.g = mMinColour.g + (mColourRange.g * intensity);
        newColour.b = mMinColour.b + (mColourRange.b * intensity);

        mLight->setDiffuseColour(newColour);
        mBillboard->setColour(newColour);
        // set billboard size
        Real newSize = mMinSize + (intensity * mSizeRange);
        mBillboard->setDimensions(newSize, newSize);

    }
};

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

        // Fixed light, dim
        mLight = mSceneMgr->createLight("SunLight");
        mLight->setPosition(1000,1450,500);
        mLight->setDiffuseColour(0.45, 0.45, 0.48);
        mLight->setSpecularColour(0.9, 0.9, 1);
        mLight->setAttenuation(10000,1,0.001,0);

        // Point light, movable, reddish
        mLight = mSceneMgr->createLight("Light2");
        mLight->setDiffuseColour(mMinLightColour);
        mLight->setSpecularColour(1, 1, 1);
        mLight->setAttenuation(10000,1,0.001,0);

        // Create light node
        mLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(
            "MovingLightNode");
        mLightNode->attachLight(mLight);
        // create billboard set
        BillboardSet* bbs = mSceneMgr->createBillboardSet("lightbbs", 1);
        bbs->setMaterialName("Examples/Flare");
        Billboard* bb = bbs->createBillboard(0,0,0,mMinLightColour);
        // attach
        mLightNode->attachObject(bbs);
        
        // create controller, after this is will get updated on its own
        WaveformControllerFunction* func = new WaveformControllerFunction(Ogre::WFT_SINE, 0.75, 0.5);
        ControllerManager& contMgr = ControllerManager::getSingleton();
        LightWibbler* val = new LightWibbler(mLight, bb, mMinLightColour, mMaxLightColour, 
            mMinFlareSize, mMaxFlareSize);
        Controller<Real>* controller = contMgr.createController(
            contMgr.getFrameTimeSource(), val, func);

        //mLight->setPosition(Vector3(300,250,-300));
        mLightNode->setPosition(Vector3(300,250,-300));


        // Create a track for the light
        Animation* anim = mSceneMgr->createAnimation("LightTrack", 42);
        // Spline it for nice curves
        anim->setInterpolationMode(Animation::IM_SPLINE);
        // Create a track to animate the camera's node
        AnimationTrack* track = anim->createTrack(0, mLightNode);
        // Setup keyframes
        KeyFrame* key = track->createKeyFrame(0); // startposition
        key = track->createKeyFrame(2);//A
        key->setTranslate(Vector3(300,250,-300));
        key = track->createKeyFrame(4);//B
        key->setTranslate(Vector3(150,300,-250));
        key = track->createKeyFrame(6);//C
        key->setTranslate(Vector3(-150,350,-100));
        key = track->createKeyFrame(8);//D
        key->setTranslate(Vector3(-400,200,-200));
        key = track->createKeyFrame(10);//E
        key->setTranslate(Vector3(-200,200,-400));
        key = track->createKeyFrame(12);//F
        key->setTranslate(Vector3(-100,150,-200));
        key = track->createKeyFrame(14);//G
        key->setTranslate(Vector3(-100,75,180));
        key = track->createKeyFrame(16);//H
        key->setTranslate(Vector3(0,250,300));
        key = track->createKeyFrame(18);//I
        key->setTranslate(Vector3(100,350,100));
        key = track->createKeyFrame(20);//J
        key->setTranslate(Vector3(250,300,0));
        key = track->createKeyFrame(22);//K == A
        key->setTranslate(Vector3(300,250,-300));
        // Create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("LightTrack");
        mAnimState->setEnabled(true);




        // Prepare athene mesh
        Mesh* pAthene = MeshManager::getSingleton().load("athene.mesh");
        pAthene->buildTangentVectors(0,1);

        Entity* pEnt;
        SceneNode* node;
        node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        pEnt = mSceneMgr->createEntity( "1", "athene.mesh" );
        //mAnimState = pEnt->getAnimationState("Walk");
        //mAnimState->setEnabled(true);
        pEnt->setMaterialName("Examples/Athene/NormalMapped");
        node->attachObject( pEnt );
        node->translate(0,-20, 0);
        node->yaw(90);

        node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        pEnt = mSceneMgr->createEntity( "col1", "column.mesh" );
        //mAnimState = pEnt->getAnimationState("Walk");
        //mAnimState->setEnabled(true);
        pEnt->setMaterialName("Examples/Rockwall");
        node->attachObject( pEnt );
        node->translate(200,0, -200);

        node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        pEnt = mSceneMgr->createEntity( "col2", "column.mesh" );
        //mAnimState = pEnt->getAnimationState("Walk");
        //mAnimState->setEnabled(true);
        pEnt->setMaterialName("Examples/Rockwall");
        node->attachObject( pEnt );
        node->translate(200,0, 200);

        node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        pEnt = mSceneMgr->createEntity( "col3", "column.mesh" );
        //mAnimState = pEnt->getAnimationState("Walk");
        //mAnimState->setEnabled(true);
        pEnt->setMaterialName("Examples/Rockwall");
        node->attachObject( pEnt );
        node->translate(-200,0, -200);

        node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        pEnt = mSceneMgr->createEntity( "col4", "column.mesh" );
        //mAnimState = pEnt->getAnimationState("Walk");
        //mAnimState->setEnabled(true);
        pEnt->setMaterialName("Examples/Rockwall");
        node->attachObject( pEnt );
        node->translate(-200,0, 200);

        /*
        pEnt = mSceneMgr->createEntity( "3", "column.mesh" );
        pEnt->setMaterialName("Examples/Rockwall");
        //Material* mat2 = (Material*)MaterialManager::getSingleton().getByName("Examples/EnvMappedRustySteel");
        //mat2->setReceiveShadows(false);
        node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200, 0, -200));
        node->attachObject( pEnt );
        */

        mSceneMgr->setSkyBox(true, "Examples/StormySkyBox");


        Plane plane;
        plane.normal = Vector3::UNIT_Y;
        plane.d = 100;
        MeshManager::getSingleton().createPlane("Myplane",plane,
            1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
        Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName("Examples/Rockwall");
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
