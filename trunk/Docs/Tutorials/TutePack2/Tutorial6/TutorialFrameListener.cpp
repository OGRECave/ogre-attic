// TutorialFrameListener.cpp: implementation of the TutorialFrameListener class.
//
//////////////////////////////////////////////////////////////////////

#include "TutorialFrameListener.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define FLOW_SPEED 0.4
#define FLOW_HEIGHT 10

TutorialFrameListener::TutorialFrameListener(RenderWindow* win, Camera* cam, 
	SceneNode* controlNode, SceneNode* shipNode, SceneNode* cameraNode) : 
	ExampleFrameListener(win, cam)
{ 
	mControlNode = controlNode;
	mShipNode = shipNode;
	mCameraNode = cameraNode;

	mbFirstPerson = false;

	mfAfterburner = 0;
	mfSpeed = 0;
	mfPitch = 0;
	mfYaw = 0;
	mfRoll = 0;
	mfShipRoll = 0;
	mfShipPitch = 0;

}


bool TutorialFrameListener::frameStarted(const FrameEvent& evt)
{
	Real MoveFactor = 80.0 * evt.timeSinceLastFrame;

	mInputDevice->capture();

	if(mInputDevice->isKeyDown(Ogre::KC_ESCAPE))
		return false;

	// Increase speed
	if (mInputDevice->isKeyDown(KC_EQUALS) || mInputDevice->isKeyDown(KC_ADD))
	{
		mfSpeed += 10;
		if (mfSpeed > 200)
			mfSpeed = 200;
	}

	// Decrease speed
	if (mInputDevice->isKeyDown(KC_MINUS) || mInputDevice->isKeyDown(KC_SUBTRACT))
	{
		mfSpeed -= 5;
		if (mfSpeed < 0)
			mfSpeed = 0;
	}

	// hit our afterburners
	mfAfterburner = 0;
	if (mInputDevice->isKeyDown(KC_TAB) )
	{
		mfAfterburner = 500;
	}


	/* Rotation factors */
	float rotX = 0, rotY = 0;


	rotX = -mInputDevice->getMouseRelativeX() * 0.5;
	rotY = mInputDevice->getMouseRelativeY() * 0.5;

	// process mfYaw (rudder function):
	mfYaw += rotX;
	mfShipRoll -= rotX / 5;

	// process mfPitch (elevator function):
	mfPitch += rotY;
	mfShipPitch += rotY / 5;


	if (mfPitch > 45) // (higher value, turn sharper)
		mfPitch = 45;
	if (mfPitch < -45)
		mfPitch = -45;
	if (mfYaw > 45)
		mfYaw = 45;
	if (mfYaw < -45)
		mfYaw = -45;
	if (mfRoll > 45)
		mfRoll = 45;
	if (mfRoll < -45)
		mfRoll = -45;

	if (mfShipPitch > 35)
		mfShipPitch = 35;
	if (mfShipPitch < -35)
		mfShipPitch = -35;
	if (mfShipRoll > 90)
		mfShipRoll = 90;
	if (mfShipRoll < -90)
		mfShipRoll = -90;


	// dampen the changes
	mfPitch *= 0.9;
	mfYaw *= 0.9;
	mfRoll *= 0.9;

	mfShipPitch *= 0.95;
	mfShipRoll *= 0.95;


	// set angles for controlNode
	if ((Math::Abs( mfPitch ) > 0.01) ||
		(Math::Abs( mfYaw ) > 0.01) ||
		(Math::Abs( mfRoll ) > 0.01))
	{
		mControlNode->roll( mfRoll * evt.timeSinceLastFrame );
		mControlNode->pitch( mfPitch * evt.timeSinceLastFrame );
		mControlNode->yaw( mfYaw * evt.timeSinceLastFrame );
	}


	// set angles for ship node to make it looked banked in third person
	if ((Math::Abs( mfShipRoll ) > 0.01) ||
		(Math::Abs( mfShipPitch ) > 0.01))
	{
		Quaternion qRoll, qPitch;
		qRoll.FromAngleAxis(Math::AngleUnitsToRadians(mfShipRoll),Vector3::UNIT_Z);
		qPitch.FromAngleAxis(Math::AngleUnitsToRadians(mfShipPitch),Vector3::UNIT_X);
		mShipNode->setOrientation( qRoll * qPitch );

	};

	static Vector3 vec;

	vec = Vector3::ZERO;
	vec.z = (mfSpeed + mfAfterburner) * evt.timeSinceLastFrame;

	// Translate the controlNode by the speed
	Vector3 trans = mControlNode->getOrientation() * vec;
	mControlNode->translate(trans);


	if(mInputDevice->isKeyDown(Ogre::KC_F))
	{
		while (mInputDevice->isKeyDown(Ogre::KC_F))
			mInputDevice->capture();

		if (mbFirstPerson)
		{
			// third person
			mCameraNode->setPosition( Vector3( 0, 50, -200) );
			mbFirstPerson = false;
		}
		else
		{
			// first person
			mCameraNode->setPosition( Vector3( 0, 5, 40) );
			mbFirstPerson = true;
		};
	}


	float fWaterFlow = FLOW_SPEED * evt.timeSinceLastFrame;
	static float fFlowAmount = 0.0f;
	static bool fFlowUp = true;

	SceneNode *pWaterNode = static_cast<SceneNode*>(
	mCamera->getSceneManager()->getRootSceneNode()->getChild("WaterNode"));
	if(pWaterNode)
	{
		if(fFlowUp)
			fFlowAmount += fWaterFlow;
		else
			fFlowAmount -= fWaterFlow;

		if(fFlowAmount >= FLOW_HEIGHT)
			fFlowUp = false;
		else if(fFlowAmount <= 0.0f)
			fFlowUp = true;

		pWaterNode->translate(0, (fFlowUp ? fWaterFlow : -fWaterFlow), 0);
	}

	return true;
}