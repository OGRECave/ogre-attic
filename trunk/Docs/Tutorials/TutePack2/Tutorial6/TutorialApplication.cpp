// TutorialApplication.cpp: implementation of the TutorialApplication class.
//
//////////////////////////////////////////////////////////////////////

#include "TutorialApplication.h"
#include "TutorialFrameListener.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TutorialApplication::TutorialApplication()
{

}


TutorialApplication::~TutorialApplication()
{

}


void TutorialApplication::createScene()
{


	// lets make it brighter
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

	// Create the SkyDome
	mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

	// Create some terrain
	mSceneMgr->setWorldGeometry( "terrain.cfg" );

	// Create our ship entity
	mShip = mSceneMgr->createEntity("razor", "razor.mesh");

	// Create our control SceneNode
	mControlNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	
	// Create a SceneNode for the ship
	mShipNode = mControlNode->createChildSceneNode();

	// Attach the ship entity to the sceneNode
	mShipNode->attachObject(mShip);
	// Center the ship
	mShipNode->setPosition( Vector3( 6, -10, 0) );


	// Create a SceneNode for the Camera
	mCameraNode = mControlNode->createChildSceneNode();

	// Attached the camera to the SceneNode
	mCameraNode->attachCamera( mCamera );
	// above and behind
	mCameraNode->setPosition( Vector3( 0, 50, -200) );

	// Position it at 0
	mCamera->setPosition(Vector3(0,0,0));
	// Look back along Z
	mCamera->lookAt(Vector3(0,0,100));

	// kind of centered
	mControlNode->setPosition(Vector3(4000,500,4000));


	// Water
	Entity *pWaterEntity;
	Plane nWaterPlane;


	// create a water plane/scene node
	nWaterPlane.normal = Vector3::UNIT_Y; 
	nWaterPlane.d = -1.5; 
	MeshManager::getSingleton().createPlane(
		"WaterPlane",
		nWaterPlane,
		8000, 8000,
		20, 20,
		true, 1, 
		10, 10,
		Vector3::UNIT_Z
	);

	pWaterEntity = mSceneMgr->createEntity("water", "WaterPlane"); 
	pWaterEntity->setMaterialName("Examples/TextureEffect4"); 

	SceneNode *waterNode = 
		mSceneMgr->getRootSceneNode()->createChildSceneNode("WaterNode"); 
	waterNode->attachObject(pWaterEntity); 
	waterNode->translate(4000, 50, 4000);

	// Create a light
	Light* pLight = mSceneMgr->createLight("MainLight");
	pLight->setType( Light::LT_DIRECTIONAL );
	pLight->setDirection( 0, -100, 0 );
}




void TutorialApplication::createFrameListener(void)
{
	mFrameListener = new TutorialFrameListener(mWindow, mCamera, mControlNode, mShipNode, mCameraNode );
	mRoot->addFrameListener(mFrameListener);
}

void TutorialApplication::chooseSceneManager(void)
{
	// Get the SceneManager, in this case the terrain one
    mSceneMgr = mRoot->getSceneManager( ST_EXTERIOR_CLOSE );
}