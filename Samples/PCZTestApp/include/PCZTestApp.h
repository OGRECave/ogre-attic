/*
-----------------------------------------------------------------------------
This file based on the Example App framework from Ogre3d (www.ogre3d.org)


-----------------------------------------------------------------------------
*/

/**
    \file 
        PCZTestApp.h
    \brief
        Specialisation of OGRE's framework application to test/demo the
		Portal Connected Zone (PCZ) Scene Manager Plugin.
*/

#include "ExampleApplication.h" 
#include "RoomObject.h"
#include "OgreTerrainZone.h"

ParticleSystem *pThrusters;
PCZSceneNode * shipNode;
PCZSceneNode * buildingNode;
Vector3 buildingTranslate;
RaySceneQuery* raySceneQuery = 0;
MovableObject* targetMO = 0;


class PCZTestFrameListener : public ExampleFrameListener
{
private:
    static float fDefDim;
    static float fDefVel;

public:
    PCZTestFrameListener(RenderWindow* win, Camera* cam) : ExampleFrameListener( win, cam )
    {
		mMoveSpeed = 15.0;
    }

	void moveCamera()
	{
		// Make all the spatial changes to the camera's scene node
		// Note that YAW direction is around a fixed axis (freelook style) rather than a natural YAW
		//(e.g. airplane)
        mCamera->getParentSceneNode()->translate(mTranslateVector, Node::TS_LOCAL);
		mCamera->getParentSceneNode()->pitch(mRotY);
		mCamera->getParentSceneNode()->yaw(mRotX, Node::TS_WORLD);
        buildingNode->translate(buildingTranslate, Node::TS_LOCAL);
	}

    bool frameStarted( const FrameEvent& evt )
    {
        if( ExampleFrameListener::frameStarted( evt ) == false )
		return false;

        buildingTranslate = Vector3(0,0,0);
		if( mKeyboard->isKeyDown( OIS::KC_LBRACKET ) )
        {
            buildingTranslate = Vector3(0,-10,0);
        }
		if( mKeyboard->isKeyDown( OIS::KC_RBRACKET ) )
        {
            buildingTranslate = Vector3(0,10,0);
        }

        if( mKeyboard->isKeyDown( OIS::KC_N ) )
        {
            pThrusters->setDefaultDimensions( fDefDim + 0.25, fDefDim + 0.25 );
            fDefDim += 0.25;
        }

        if( mKeyboard->isKeyDown( OIS::KC_M ) )
        {
            pThrusters->setDefaultDimensions( fDefDim - 0.25, fDefDim - 0.25 );
            fDefDim -= 0.25;
        }

        if( mKeyboard->isKeyDown( OIS::KC_H ) )
        {
            pThrusters->getEmitter( 0 )->setParticleVelocity( fDefVel + 1 );
            pThrusters->getEmitter( 1 )->setParticleVelocity( fDefVel + 1 );
            fDefVel += 1;            
        }

        if( mKeyboard->isKeyDown( OIS::KC_J ) && !( fDefVel < 0.0f ) )
        {
            pThrusters->getEmitter( 0 )->setParticleVelocity( fDefVel - 1 );
            pThrusters->getEmitter( 1 )->setParticleVelocity( fDefVel - 1 );
            fDefVel -= 1;            
        }

        // test the ray scene query by showing bounding box of whatever the camera is pointing directly at 
        // (takes furthest hit)
        static Ray updateRay;
        updateRay.setOrigin(mCamera->getParentSceneNode()->getPosition());
        updateRay.setDirection(mCamera->getParentSceneNode()->getOrientation()*Vector3::NEGATIVE_UNIT_Z);
        raySceneQuery->setRay(updateRay);
        PCZone * zone = ((PCZSceneNode*)(mCamera->getParentSceneNode()))->getHomeZone();
        ((PCZRaySceneQuery*)raySceneQuery)->setStartZone(zone);
        ((PCZRaySceneQuery*)raySceneQuery)->setExcludeNode(mCamera->getParentSceneNode());
        RaySceneQueryResult& qryResult = raySceneQuery->execute();
        RaySceneQueryResult::iterator i = qryResult.begin();
        if (i != qryResult.end())
        {
			RaySceneQueryResult::reverse_iterator ri = qryResult.rbegin();
            MovableObject * mo = ri->movable;
            if (targetMO != mo)
            {
                if (targetMO != 0)
                {
                    targetMO->getParentSceneNode()->showBoundingBox(false);
                }
                targetMO = mo;
                targetMO->getParentSceneNode()->showBoundingBox(true);
            }
        }

        return true;
    }
};

float PCZTestFrameListener::fDefDim = 25.0f;
float PCZTestFrameListener::fDefVel = 50.0f;

class PCZTestApplication : public ExampleApplication
{
public:
    PCZTestApplication() {}
    ~PCZTestApplication() 
    {
        delete raySceneQuery;
    }

protected:
	SceneNode * mCameraNode;

	virtual void chooseSceneManager(void)
    {
        // Create the SceneManager, in this case a generic one
        mSceneMgr = mRoot->createSceneManager("PCZSceneManager", "PCZSceneManager");
		// initialize the scene manager using terrain as default zone
		String zoneTypeName = "ZoneType_Terrain";
		String zoneFilename = "none";
		((PCZSceneManager*)mSceneMgr)->init(zoneTypeName);
		//mSceneMgr->showBoundingBoxes(true);
    }
    virtual void createFrameListener(void)
    {
        mFrameListener= new PCZTestFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }
    virtual void createCamera(void)
    {
        // Create the camera
        mCamera = mSceneMgr->createCamera("PlayerCam");

		// NEW: create a node for the camera and control that instead of camera directly.
		// We do this because PCZSceneManager requires camera to have a node 
		mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("PlayerCamNode");
		// attach the camera to the node
		mCameraNode->attachObject(mCamera);
		// fix the yaw axis of the camera
		mCameraNode->setFixedYawAxis(true);

        mCamera->setNearClipDistance(2);
        mCamera->setFarClipDistance( 1000 );
		// set camera zone
//		((PCZSceneNode*)(mCameraNode))->setHomeZone(((PCZSceneManager*)(mSceneMgr))->getDefaultZone());

    }
 
    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.25, 0.25, 0.25));

        // Create a skybox
        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox", 500 );
		// put the skybox node in the default zone
		((PCZSceneManager*)mSceneMgr)->setSkyZone(0);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setPosition(0,0,0);
        l->setAttenuation(500, 0.5, 1.0, 0.0);
        // Accept default settings: point light, white diffuse, just set position
        // attach light to a scene node so the PCZSM can handle it properly (zone-wise)
        // IMPORTANT: Lights (just like cameras) MUST be connected to a scene node!
	    SceneNode * lightNode = mCameraNode->createChildSceneNode("lightNode");
		lightNode->attachObject(l);      

        // Fog
        // NB it's VERY important to set this before calling setWorldGeometry 
        // because the vertex program picked will be different
        ColourValue fadeColour(0.101, 0.125, 0.1836);
        mSceneMgr->setFog( FOG_LINEAR, fadeColour, .001, 500, 1000);
        mWindow->getViewport(0)->setBackgroundColour(fadeColour);

		// load terrain into the terrain zone
        std::string terrain_cfg("terrain.cfg");
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        terrain_cfg = mResourcePath + terrain_cfg;
#endif
		((PCZSceneManager*)mSceneMgr)->setZoneGeometry( "DefaultZone", (PCZSceneNode*)mSceneMgr->getRootSceneNode(), terrain_cfg );

        // Infinite far plane?
        if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(RSC_INFINITE_FAR_PLANE))
        {
            mCamera->setFarClipDistance(0);
        }

		// create test buildinig
		RoomObject roomObj;
		buildingNode = roomObj.createTestBuilding(mSceneMgr);
		buildingNode->setPosition(500, 165, 570);
		Ogre::Radian r = Radian(3.1416/7.0);
		buildingNode->rotate(Vector3::UNIT_Y, r);

        // Position camera in the center of the building
        mCameraNode->setPosition(buildingNode->getPosition());
        // Look back along -Z
		mCamera->lookAt(mCameraNode->_getDerivedPosition() + Vector3(0,0,-300));
		// Update bounds for camera
		mCameraNode->_updateBounds();

        // create the ray scene query
        raySceneQuery = mSceneMgr->createRayQuery(
            Ray(mCamera->getParentNode()->getPosition(), Vector3::NEGATIVE_UNIT_Z));
        raySceneQuery->setSortByDistance(true, 5);

    }

};
