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

#include "Ogre.h"
#include "ExampleApplication.h"

/**
    \file 
        BSP.cpp
    \brief
        Shows the indoor level rendering (Binary Space Partition or BSP based).
    \par
        Also demonstrates loading levels from Quake3Arena and using
        curved bezier surfaces (as demonstrated in the Bezier example)
        in a large level.
*/

class BspApplication : public ExampleApplication, public ResourceGroupListener
{
public:
	BspApplication()
		:mLoading(false)
	{


	}

	// ResourceGroupListener callbacks
	void resourceGroupScriptingStarted(const String& groupName, size_t scriptCount)
	{
		if (mLoading)
		{
			mLoadingDescriptionElement->setCaption("Parsing scripts...");
			mWindow->update();
		}
	}
	void scriptParseStarted(const String& scriptName)
	{
	}
	void scriptParseEnded(void)
	{
	}
	void resourceGroupScriptingEnded(const String& groupName)
	{
	}
	void resourceGroupLoadStarted(const String& groupName, size_t resourceCount)
	{
	}
	void resourceLoadStarted(const ResourcePtr& resource)
	{
	}
	void resourceLoadEnded(void)
	{
	}
	void worldGeometryStageStarted(const String& description)
	{
	}
	void worldGeometryStageEnded(void)
	{
	}
	void resourceGroupLoadEnded(const String& groupName)
	{
	}

protected:

	String mQuakePk3;
	String mQuakeLevel;
	bool mLoading;
	OverlayElement* mLoadingBarElement;
	OverlayElement* mLoadingDescriptionElement;
	OverlayElement* mLoadingCommentElement;

	// Create our resource listener here
	void createResourceListener(void)
	{
		// self is listener
		ResourceGroupManager::getSingleton().addResourceGroupListener(this);
	}
	/*
	void loadResources(void)
	{
		// We need to pre-initialise the 'Bootstrap' group so we can use
		// the basic contents in the loading screen
		ResourceGroupManager::getSingleton().initialiseResourceGroup("Bootstrap");

		mLoading = true;
		// Set up a really basic loading screen
		// No scripts have been parsed yet, so everything here has to be 
		// made in code
		// Note, you could use some scripts by defining a small 'bootstrap'
		// resource group including the .fontdef, .material scripts you needed
		// and load that earlier, I'm not doing
		OverlayManager& omgr = OverlayManager::getSingleton();
		Overlay* loadOverlay = (Overlay*)omgr.getByName("Core/LoadOverlay");
		//loadOverlay->show();

		// Save links to the bar and to the loading text, for updates as we go
		mLoadingBarElement = omgr.getOverlayElement("Core/LoadPanel/Bar/Progress");
		mLoadingCommentElement = omgr.getOverlayElement("Core/LoadPanel/Comment");
		mLoadingDescriptionElement = omgr.getOverlayElement("Core/LoadPanel/Description");

		// Turn off rendering of everything except overlays
		mSceneMgr->clearSpecialCaseRenderQueues();
		mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
		mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);

		// Initialise the rest of the resource groups, parse scripts etc
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		// hide loading screen
		loadOverlay->hide();

		// Back to full rendering
		mSceneMgr->clearSpecialCaseRenderQueues();
		mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);
		mLoading = false;


	}
	*/

	// Override resource sources (include Quake3 archives)
	void setupResources(void)
	{

		// Load Quake3 locations from a file
		ConfigFile cf;

		cf.load("quake3settings.cfg");

		mQuakePk3 = cf.getSetting("Pak0Location");
		mQuakeLevel = cf.getSetting("Map");

		ExampleApplication::setupResources();
		ResourceGroupManager::getSingleton().addResourceLocation(
			mQuakePk3, "Zip", ResourceGroupManager::getSingleton().getWorldResourceGroupName());

	}
	// Override scene manager (use indoor instead of generic)
	void chooseSceneManager(void)
	{
		mSceneMgr = mRoot->getSceneManager(ST_INTERIOR);
	}
	// Scene creation
	void createScene(void)
	{

		// Load world geometry
		mSceneMgr->setWorldGeometry(mQuakeLevel);

		// modify camera for close work
		mCamera->setNearClipDistance(4);
		mCamera->setFarClipDistance(4000);

		// Also change position, and set Quake-type orientation
		// Get random player start point
		ViewPoint vp = mSceneMgr->getSuggestedViewpoint(true);
		mCamera->setPosition(vp.position);
		mCamera->pitch(Degree(90)); // Quake uses X/Y horizon, Z up
		mCamera->rotate(vp.orientation);
		// Don't yaw along variable axis, causes leaning
		mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);


	}

};


#if OGRE_PLATFORM == PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char**argv)
#endif
{
    // Create application object
    BspApplication app;

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
