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

/**
    \file 
        Lighting.h
    \brief
        Shows lighting support in OGRE. Also demonstrates the use of billboards
        and controllers for automatic time-relative behaviour.
*/


#include "ExampleApplication.h"


/** This class turns lights & billboards linked to them on & off */
class LightFlasher : public ControllerValue
{
protected:
	Light* mLight;
	Billboard* mBillboard;
	ColourValue mMaxColour;
	Real intensity;
public:
	LightFlasher(Light* light, Billboard* billboard, ColourValue maxColour)
	{
		mLight = light;
		mBillboard = billboard;
		mMaxColour = maxColour;
	}

	virtual Real  getValue (void)
	{
		return intensity;
	}

	virtual void  setValue (Real value)
	{
		intensity = value;

		ColourValue newColour;

		// Attenuate the brightness of the light
		newColour.r = mMaxColour.r * intensity;
		newColour.g = mMaxColour.g * intensity;
		newColour.b = mMaxColour.b * intensity;

		mLight->setDiffuseColour(newColour);
		mBillboard->setColour(newColour);
	}
};


/** Controller function mapping waveform to light intensity */
class LightFlasherControllerFunction : public WaveformControllerFunction
{
public:
	LightFlasherControllerFunction(WaveformType wavetype, Real frequency, Real phase) : WaveformControllerFunction(wavetype, 0, frequency, phase, 1, true)
	{

	}
};


// Some global node data
SceneNode* mRedYellowLightsNode;
SceneNode* mGreenBlueLightsNode;

// Listener class for frame updates
class LightingListener : public ExampleFrameListener
{
protected:
public:
    LightingListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }

    bool frameStarted(const FrameEvent& evt)
    {
        mRedYellowLightsNode->yaw(evt.timeSinceLastFrame * 10);
        mGreenBlueLightsNode->pitch(evt.timeSinceLastFrame * 20);
        // Call superclass
        return ExampleFrameListener::frameStarted(evt);
    }
};

/** Application class */
class LightingApplication : public ExampleApplication
{
protected:

	// The set of all the billboards used for the lights
    // 2 sets because we'll rotate them differently
	BillboardSet* mRedYellowLights;
	BillboardSet* mGreenBlueLights;

	// Billboards
	Billboard* mRedLightBoard;
	Billboard* mBlueLightBoard;
	Billboard* mYellowLightBoard;
	Billboard* mGreenLightBoard;

	// Lights
	Light* mRedLight;
	Light* mBlueLight;
	Light* mYellowLight;
	Light* mGreenLight;


	// Light flashers
	LightFlasher* mRedLightFlasher;
	LightFlasher* mBlueLightFlasher;
	LightFlasher* mYellowLightFlasher;
	LightFlasher* mGreenLightFlasher;

	// Light controller functions
	LightFlasherControllerFunction* mRedLightControllerFunc;
	LightFlasherControllerFunction* mBlueLightControllerFunc;
	LightFlasherControllerFunction* mYellowLightControllerFunc;
	LightFlasherControllerFunction* mGreenLightControllerFunc;

	// Light controllers
	Controller* mRedLightController;
	Controller* mBlueLightController;
	Controller* mYellowLightController;
	Controller* mGreenLightController;

	void createScene(void)
    {
		// Set a very low level of ambient lighting
		mSceneMgr->setAmbientLight(ColourValue(0.1, 0.1, 0.1));

        // Use the "Space" skybox
        mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");

		// Load ogre head
		Entity* head = mSceneMgr->createEntity("head", "ogrehead.mesh");

		// Attach the head to the scene
		mSceneMgr->getRootSceneNode()->attachObject(head);

		// Create nodes for the lights to be rotated with
        mRedYellowLightsNode = mSceneMgr->getRootSceneNode()->createChild();
        mGreenBlueLightsNode = mSceneMgr->getRootSceneNode()->createChild();


        // First create the BillboardSets. This will define the materials for the billboards
		// in its set to use
		mRedYellowLights = mSceneMgr->createBillboardSet("RedYellowLights");
		mRedYellowLights->setMaterialName("Examples/Flare");
		mRedYellowLightsNode->attachObject(mRedYellowLights);

		mGreenBlueLights = mSceneMgr->createBillboardSet("GreenBlueLights");
		mGreenBlueLights->setMaterialName("Examples/Flare");
		mGreenBlueLightsNode->attachObject(mGreenBlueLights);

		// Red light billboard, in "off" state
		Vector3 redLightPosition(78, -8, -70);
		mRedLightBoard = mRedYellowLights->createBillboard(redLightPosition);
		mRedLightBoard->setColour(ColourValue::Black);


		// Blue light billboard, in "off" state
		Vector3 blueLightPosition(-90, -8, -70);
		mBlueLightBoard = mGreenBlueLights->createBillboard(blueLightPosition);
		mBlueLightBoard->setColour(ColourValue::Black);


		// Yellow light billboard, in "off" state
		Vector3 yellowLightPosition(-4.5, 30, -80);
		mYellowLightBoard = mRedYellowLights->createBillboard(yellowLightPosition);
		mYellowLightBoard->setColour(ColourValue::Black);

		// Green light billboard, in "off" state
		Vector3 greenLightPosition(50, 70, 80);
		mGreenLightBoard = mGreenBlueLights->createBillboard(greenLightPosition);
		mGreenLightBoard->setColour(ColourValue::Black);

		// Red light, in "off" state
		mRedLight = mSceneMgr->createLight("RedFlyingLight");
		mRedLight->setType(Light::LT_POINT);
		mRedLight->setPosition(redLightPosition);
		mRedLight->setDiffuseColour(ColourValue::Black);
		mRedYellowLightsNode->attachLight(mRedLight);

		// Blue light, in "off" state
		mBlueLight = mSceneMgr->createLight("BlueFlyingLight");
		mBlueLight->setType(Light::LT_POINT);
		mBlueLight->setPosition(blueLightPosition);
		mBlueLight->setDiffuseColour(ColourValue::Black);
		mGreenBlueLightsNode->attachLight(mBlueLight);

		// Yellow light in "off" state
		mYellowLight = mSceneMgr->createLight("YellowFlyingLight");
		mYellowLight->setType(Light::LT_POINT);
		mYellowLight->setPosition(yellowLightPosition);
		mYellowLight->setDiffuseColour(ColourValue::Black);
		mRedYellowLightsNode->attachLight(mYellowLight);

		// Yellow light in "off" state
		mGreenLight = mSceneMgr->createLight("GreenFlyingLight");
		mGreenLight->setType(Light::LT_POINT);
		mGreenLight->setPosition(greenLightPosition);
		mGreenLight->setDiffuseColour(ColourValue::Black);
		mGreenBlueLightsNode->attachLight(mGreenLight);

		// Light flashers
		mRedLightFlasher = new LightFlasher(mRedLight, mRedLightBoard, ColourValue::Red);
		mBlueLightFlasher = new LightFlasher(mBlueLight, mBlueLightBoard, ColourValue::Blue);
		mYellowLightFlasher = new LightFlasher(mYellowLight, mYellowLightBoard, ColourValue(1.0, 1.0, 0.0));
		mGreenLightFlasher = new LightFlasher(mGreenLight, mGreenLightBoard, ColourValue::Green);

		// Light controller functions
		mRedLightControllerFunc = new LightFlasherControllerFunction(Ogre::WFT_SINE, 0.5, 0.0);
		mBlueLightControllerFunc = new LightFlasherControllerFunction(Ogre::WFT_SINE, 0.75, 0.5);
		mYellowLightControllerFunc = new LightFlasherControllerFunction(Ogre::WFT_TRIANGLE, 0.25, 0.0);
		mGreenLightControllerFunc = new LightFlasherControllerFunction(Ogre::WFT_SINE, 0.25, 0.5);

		// Light controllers
		ControllerManager* mControllerManager = &ControllerManager::getSingleton();
		mRedLightController = mControllerManager->createController(mControllerManager->getFrameTimeSource(), mRedLightFlasher, mRedLightControllerFunc);
		mBlueLightController = mControllerManager->createController(mControllerManager->getFrameTimeSource(), mBlueLightFlasher, mBlueLightControllerFunc);
		mYellowLightController = mControllerManager->createController(mControllerManager->getFrameTimeSource(), mYellowLightFlasher, mYellowLightControllerFunc);
		mGreenLightController = mControllerManager->createController(mControllerManager->getFrameTimeSource(), mGreenLightFlasher, mGreenLightControllerFunc);

    }

	void createFrameListener(void)
    {
		// This is where we instantiate our own frame listener
        mFrameListener= new LightingListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);

    }

};
