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
        Gui.h
    \brief
        Specialisation of OGRE's framework application to show the
        gui feature
*/

#include "OgreOverlayManager.h"

#include "OgreCursorGuiElement.h"
#include "ExampleApplication.h"
#include "OgreBorderButtonGuiElement.h"
#include "OgreListGuiElement.h"
#include "OgreListChanger.h"
#include "OgreEventProcessor.h"
#include "OgreStringResource.h"

class GuiFrameListener : public ExampleFrameListener
{
protected:
    bool mShutdownRequested;
public:
    // NB using buffered input, this is the only change
    GuiFrameListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam, true, true), mShutdownRequested(false)
    {
    }

    /// Tell the frame listener to exit at the end of the next frame
    void requestShutdown(void)
    {
        mShutdownRequested = true;
    }

    bool frameEnded(const FrameEvent& evt)
    {
        if (mShutdownRequested)
        {
            return false;
        }
        else
        {
            return ExampleFrameListener::frameEnded(evt);
        }
    }

};

class GuiApplication : public ExampleApplication, public ActionListener, public MouseListener
{
public:
    GuiApplication()
    {
    }

protected:
    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a skydome
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);


        // A bit of a hacky test
        Overlay* o = (Overlay*)OverlayManager::getSingleton().getByName("SS/Setup/HostScreen/Overlay");
		ActionTarget* at = static_cast<BorderButtonGuiElement*>(GuiManager::getSingleton().getGuiElement("SS/Setup/HostScreen/Join"));
		at->addActionListener(this);
		at = static_cast<BorderButtonGuiElement*>(GuiManager::getSingleton().getGuiElement("SS/Setup/HostScreen/Exit"));
		at->addActionListener(this);
		o->show();
		//mRoot->showDebugOverlay(false);

		ListChanger* list = static_cast<ListGuiElement*>(GuiManager::getSingleton().getGuiElement("SS/Setup/HostScreen/AvailableGamesList"));

		list->addListItem(new StringResource("test1"));
		list->addListItem(new StringResource("test2"));
		list->addListItem(new StringResource("test3"));
		list->addListItem(new StringResource("test4"));
		list->addListItem(new StringResource("test5"));
		list->addListItem(new StringResource("test6"));
		list->addListItem(new StringResource("test7"));
		list->addListItem(new StringResource("test8"));

		(GuiManager::getSingleton().getGuiElement("Core/CurrFps"))->addMouseListener(this);

		GuiContainer* pCursorGui = OverlayManager::getSingleton().getCursorGui();
		pCursorGui->setMaterialName("Cursor/default");
		pCursorGui->setDimensions(32.0/640.0, 32.0/480.0);
    }

	void actionPerformed(ActionEvent* e) 
	{
        // Think about doing something here
        std::string action = e->getActionCommand();
        LogManager::getSingleton().logMessage("Got event: " + action);

        if (action == "SS/Setup/HostScreen/Exit")
        {
            // Queue a shutdown
            static_cast<GuiFrameListener*>(mFrameListener)->requestShutdown();
        }
	}


	void mouseClicked(MouseEvent* e) {}
	void mouseEntered(MouseEvent* e) 
	{
		int i =5;

	
	}
	void mouseExited(MouseEvent* e) {}
	void mousePressed(MouseEvent* e) {}
	void mouseReleased(MouseEvent* e) {}

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new GuiFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }


    bool setup()
    {
        ExampleApplication::setup();
        LogManager::getSingleton().setLogDetail( LL_BOREME );
        return true;
    }
};
