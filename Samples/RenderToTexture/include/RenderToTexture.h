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
        RenderToTexture.h
    \brief
        Specialisation of OGRE's framework application to show the RenderToTexture,
        or scene blending features.
    \par
        Note that this is a little rudimentary - it's because whilst
        OGRE supports lots of blending options, the SceneManager has
        to ensure the rendering order is correct when object RenderToTexture
        is enabled. Right now this is not quite right in the default
        manager so this scene is kept deliberately simple.
*/

#include "ExampleApplication.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"

class RttFrameListener : public ExampleFrameListener
{
public:
    RttFrameListener(RenderWindow* win, Camera* cam, bool useBufferedInput = false)
        : ExampleFrameListener( win, cam, useBufferedInput )
    {
    }
    bool frameStarted(const FrameEvent& evt)
    {
        Camera *rttCam = mCamera->getSceneManager()->getCamera( "rttCam" );
        rttCam->setPosition( mCamera->getDerivedPosition() );
        rttCam->setDirection( mCamera->getDerivedDirection() );

        Quaternion q = rttCam->getOrientation();
        Vector3 vPos = rttCam->getDerivedPosition();
        Vector3 vDir = rttCam->getDerivedDirection();

        SceneNode *head = static_cast<SceneNode*>(
            mCamera->getSceneManager()->getRootSceneNode()->getChild( "Head" ));
        head->setOrientation( q );
        vDir.normalise();
        head->setPosition( vPos + vDir * -250.0 );

        return ExampleFrameListener::frameStarted( evt );
    }
};

class RenderToTextureApplication : public ExampleApplication
{
public:
    RenderToTextureApplication() {}

protected:
    virtual void createFrameListener(void)
    {
        mFrameListener= new RttFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);


        // Create a prefab plane
        Entity *planeEnt = mSceneMgr->createEntity( "Plane", SceneManager::PT_PLANE );
        // Give the plane a texture
        //planeEnt->setMaterialName("Examples/BumpyMetal");

        // Create an entity from a model (will be loaded automatically)
        Entity* knotEnt = mSceneMgr->createEntity("Knot", "knot.mesh");

        // Create an entity from a model (will be loaded automatically)
        Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");

        knotEnt->setMaterialName("Examples/TextureEffect2");

        // Attach the rtt entity to the root of the scene
        SceneNode* rootNode = mSceneMgr->getRootSceneNode();
        rootNode->attachObject(planeEnt);

        rootNode->createChildSceneNode( "Head" )->attachObject( ogreHead );

        Camera *rttCam = mSceneMgr->createCamera( "rttCam" );
        rootNode->attachCamera( rttCam ); rttCam->setPosition( 0.0, 0.0, -0.1 ); rttCam->setDirection( 0.0, 0.0, -100.0 );

        RenderTexture* rttTex = mRoot->getRenderSystem()->createRenderTexture( "RttTex", 512, 512 );
        {
            Viewport *v = rttTex->addViewport( rttCam );
            v->setClearEveryFrame( true );
            v->setBackgroundColour( ColourValue::Black );

            Material* mat = mSceneMgr->createMaterial("RttMat");
            mat->getTechnique(0)->getPass(0)->createTextureUnitState("RttTex");
        }

        // Give the plane a texture
        planeEnt->setMaterialName("RttMat");

        //static_cast< Material * >( MaterialManager::getSingleton().getByName( "Examples/BumpyMetal" ) )->getTextureLayer( 0 )->setTextureName( "RttTex" );

        // Add a whole bunch of extra transparent entities
        Entity *cloneEnt;
        for (int n = 0; n < 10; ++n)
        {
            // Create a new node under the root
            SceneNode* node = mSceneMgr->createSceneNode();
            // Random translate
            Vector3 nodePos;
            nodePos.x = Math::SymmetricRandom() * 500.0;
            nodePos.y = Math::SymmetricRandom() * 500.0;
            nodePos.z = Math::SymmetricRandom() * 500.0;
            node->setPosition(nodePos);
            rootNode->addChild(node);
            // Clone knot
            char cloneName[12];
            sprintf(cloneName, "Knot%d", n);
            cloneEnt = knotEnt->clone(cloneName);
            // Attach to new node
            node->attachObject(cloneEnt);

        }
    }

};
