// TutorialApplication.cpp: implementation of the TutorialApplication class.
//
//////////////////////////////////////////////////////////////////////

#include "TutorialApplication.h"

class TutorialFrameListener : public ExampleFrameListener
{
protected:
     SceneNode* mShipNode;

public:
    TutorialFrameListener(RenderWindow* win, Camera* cam, SceneNode* shipNode) : 
        ExampleFrameListener(win, cam)
    { 
        mShipNode = shipNode;
    };
    bool frameStarted(const FrameEvent& evt);
};

bool TutorialFrameListener::frameStarted(const FrameEvent& evt)
{
	Real MoveFactor = 80.0 * evt.timeSinceLastFrame;

	mInputDevice->capture();

	if(mInputDevice->isKeyDown(Ogre::KC_UP))
		mShipNode->translate(0.0, MoveFactor, 0.0);

	if(mInputDevice->isKeyDown(Ogre::KC_DOWN))
		mShipNode->translate(0.0, -MoveFactor, 0.0);

	if(mInputDevice->isKeyDown(Ogre::KC_LEFT))
		mShipNode->translate(-MoveFactor, 0.0, 0.0);

	if(mInputDevice->isKeyDown(Ogre::KC_RIGHT))
		mShipNode->translate(MoveFactor, 0.0, 0.0);

	return true;
}

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
	// Create the SkyBox
	mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");

	// Create our ship entity
	mShip = mSceneMgr->createEntity("razor", "razor.mesh");

	// Create our SceneNode
	mShipNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

	// Attach the ship entity to the sceneNode
	mShipNode->attachObject(mShip);

	// lets make it brighter
	mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
}

void TutorialApplication::createFrameListener(void)
{
	mFrameListener = new TutorialFrameListener(mWindow, mCamera, mShipNode);
	mRoot->addFrameListener(mFrameListener);
}