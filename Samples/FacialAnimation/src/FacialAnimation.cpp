/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    \file 
        FacialAnimation.cpp
    \brief
        Demonstration of facial animation features, using Pose animation
*/

#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUILogger.h>
#include <CEGUI/CEGUISchemeManager.h>
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/CEGUIWindow.h>
#include <CEGUI/elements/CEGUICombobox.h>
#include <CEGUI/elements/CEGUIListbox.h>
#include <CEGUI/elements/CEGUIListboxTextItem.h>
#include <CEGUI/elements/CEGUIPushButton.h>
#include <CEGUI/elements/CEGUIScrollbar.h>
#include <CEGUI/elements/CEGUIStaticImage.h>
#include "OgreCEGUIRenderer.h"
#include "OgreCEGUIResourceProvider.h"

#include "ExampleApplication.h"

CEGUI::MouseButton convertOgreButtonToCegui(int buttonID)
{
    switch (buttonID)
    {
    case MouseEvent::BUTTON0_MASK:
        return CEGUI::LeftButton;
    case MouseEvent::BUTTON1_MASK:
        return CEGUI::RightButton;
    case MouseEvent::BUTTON2_MASK:
        return CEGUI::MiddleButton;
    case MouseEvent::BUTTON3_MASK:
        return CEGUI::X1Button;
    default:
        return CEGUI::LeftButton;
    }
}

AnimationState* speakAnimState;

class GuiFrameListener : public ExampleFrameListener, public MouseMotionListener, public MouseListener
{
private:
    CEGUI::Renderer* mGUIRenderer;
    bool mShutdownRequested;

public:
    // NB using buffered input, this is the only change
    GuiFrameListener(RenderWindow* win, Camera* cam, CEGUI::Renderer* renderer)
        : ExampleFrameListener(win, cam, true, true), 
          mGUIRenderer(renderer),
          mShutdownRequested(false)
    {
        mEventProcessor->addMouseMotionListener(this);
        mEventProcessor->addMouseListener(this);
		mEventProcessor->addKeyListener(this);
    }

    /// Tell the frame listener to exit at the end of the next frame
    void requestShutdown(void)
    {
        mShutdownRequested = true;
    }

    bool frameEnded(const FrameEvent& evt)
    {
        if (mShutdownRequested)
            return false;
        else
            return ExampleFrameListener::frameEnded(evt);
    }

    void mouseMoved (MouseEvent *e)
    {
        CEGUI::System::getSingleton().injectMouseMove(
                e->getRelX() * mGUIRenderer->getWidth(), 
                e->getRelY() * mGUIRenderer->getHeight());
        e->consume();
    }

    void mouseDragged (MouseEvent *e) 
    { 
        mouseMoved(e);
    }

    void mousePressed (MouseEvent *e)
    {
        CEGUI::System::getSingleton().injectMouseButtonDown(
          convertOgreButtonToCegui(e->getButtonID()));
        e->consume();
    }

    void mouseReleased (MouseEvent *e)
    {
        CEGUI::System::getSingleton().injectMouseButtonUp(
          convertOgreButtonToCegui(e->getButtonID()));
        e->consume();
    }

	void mouseClicked(MouseEvent* e) {}
	void mouseEntered(MouseEvent* e) {}
	void mouseExited(MouseEvent* e) {}

    void keyPressed(KeyEvent* e)
    {
        if(e->getKey() == KC_ESCAPE)
        {
            mShutdownRequested = true;
            e->consume();
            return;
        }

        CEGUI::System::getSingleton().injectKeyDown(e->getKey());
		CEGUI::System::getSingleton().injectChar(e->getKeyChar());
        e->consume();
    }

	void keyReleased(KeyEvent* e)
	{
		CEGUI::System::getSingleton().injectKeyUp(e->getKey());
		e->consume();
	}
	void keyClicked(KeyEvent* e) 
	{
		// Do nothing
		e->consume();
	}

	bool frameStarted(const FrameEvent& evt)
	{
		speakAnimState->addTime(evt.timeSinceLastFrame);
		return ExampleFrameListener::frameStarted(evt);

	}
};

class GuiApplication : public ExampleApplication
{
private:
    CEGUI::OgreCEGUIRenderer* mGUIRenderer;
    CEGUI::System* mGUISystem;
    CEGUI::Window* mEditorGuiSheet;
	CEGUI::Scrollbar* mRed;
	CEGUI::Scrollbar* mGreen;
	CEGUI::Scrollbar* mBlue;
	CEGUI::StaticImage* mPreview;
	CEGUI::Window* mTip;
	CEGUI::Listbox* mList;
	CEGUI::Window* mEditBox;

public:
    GuiApplication()
      : mGUIRenderer(0),
        mGUISystem(0),
        mEditorGuiSheet(0)
    {

	}

    ~GuiApplication()
    {
        if(mEditorGuiSheet)
        {
            CEGUI::WindowManager::getSingleton().destroyWindow(mEditorGuiSheet);
        }
        if(mGUISystem)
        {
            delete mGUISystem;
            mGUISystem = 0;
        }
        if(mGUIRenderer)
        {
            delete mGUIRenderer;
            mGUIRenderer = 0;
        }
    }

protected:
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
		l->setDiffuseColour(1.0, 1.0, 1.0);

		// Create a light
		l = mSceneMgr->createLight("MainLight2");
		// Accept default settings: point light, white diffuse, just set position
		// NB I could attach the light to a SceneNode if I wanted it to move automatically with
		//  other objects, but I don't
		l->setPosition(-120,-80,-50);
		l->setDiffuseColour(0.7, 0.7, 0.6);

		// setup GUI system
        mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, 
            Ogre::RENDER_QUEUE_OVERLAY, false, 3000);

        mGUISystem = new CEGUI::System(mGUIRenderer);

        CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Informative);

        Entity* head = mSceneMgr->createEntity("Head", "facial.mesh");
		speakAnimState = head->getAnimationState("Speak");
		speakAnimState->setEnabled(true);

        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        headNode->attachObject(head);

		mCamera->setPosition(-20, 50, 150);
		mCamera->lookAt(0,35,0);

        // load scheme and set up defaults
        CEGUI::SchemeManager::getSingleton().loadScheme(
                (CEGUI::utf8*)"TaharezLookSkin.scheme");
        mGUISystem->setDefaultMouseCursor(
                (CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
        mGUISystem->setDefaultFont((CEGUI::utf8*)"Tahoma-12");

        CEGUI::Window* sheet = 
            CEGUI::WindowManager::getSingleton().loadWindowLayout(
                (CEGUI::utf8*)"facial.layout"); 
        mGUISystem->setGUISheet(sheet);

		/*
        CEGUI::Combobox* objectComboBox = (CEGUI::Combobox*)CEGUI::WindowManager::getSingleton().getWindow("OgreGuiDemo/TabCtrl/Page2/ObjectTypeList");

        CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"FrameWindow", 0);
        objectComboBox->addItem(item);
        item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"Horizontal Scrollbar", 1);
        objectComboBox->addItem(item);
        item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"Vertical Scrollbar", 2);
        objectComboBox->addItem(item);
        item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"StaticText", 3);
        objectComboBox->addItem(item);
        item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"StaticImage", 4);
        objectComboBox->addItem(item);
        item = new CEGUI::ListboxTextItem((CEGUI::utf8*)"Render to Texture", 5);
        objectComboBox->addItem(item);
		*/

        setupEventHandlers();
    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new GuiFrameListener(mWindow, mCamera, mGUIRenderer);
        mRoot->addFrameListener(mFrameListener);
    }

    void setupEventHandlers(void)
    {
		/*
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page1/QuitButton")
			->subscribeEvent(
				CEGUI::PushButton::EventClicked, 
				CEGUI::Event::Subscriber(&GuiApplication::handleQuit, this));
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page1/NewButton")
			->subscribeEvent(
				CEGUI::PushButton::EventClicked, 
				CEGUI::Event::Subscriber(&GuiApplication::handleNew, this));
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page1/LoadButton")
			->subscribeEvent(
				CEGUI::PushButton::EventClicked, 
				CEGUI::Event::Subscriber(&GuiApplication::handleLoad, this));
        wmgr.getWindow((CEGUI::utf8*)"OgreGuiDemo/TabCtrl/Page2/ObjectTypeList")
			->subscribeEvent(
				CEGUI::Combobox::EventListSelectionAccepted, 
				CEGUI::Event::Subscriber(&GuiApplication::handleObjectSelection, this));
		*/

    }


	void setupLoadedLayoutHandlers(void)
	{
		/*
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		mRed = static_cast<CEGUI::Scrollbar*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Red"));
		mGreen = static_cast<CEGUI::Scrollbar*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Green"));
		mBlue = static_cast<CEGUI::Scrollbar*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Blue"));
		mPreview = static_cast<CEGUI::StaticImage*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/ColourSample"));
		mList = static_cast<CEGUI::Listbox*>(
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Listbox"));
		mEditBox = 
			wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Editbox");
	
		mRed->subscribeEvent(
				CEGUI::Scrollbar::EventScrollPositionChanged, 
				CEGUI::Event::Subscriber(&GuiApplication::handleColourChanged, this));
		mGreen->subscribeEvent(
			CEGUI::Scrollbar::EventScrollPositionChanged, 
			CEGUI::Event::Subscriber(&GuiApplication::handleColourChanged, this));
		mBlue->subscribeEvent(
			CEGUI::Scrollbar::EventScrollPositionChanged, 
			CEGUI::Event::Subscriber(&GuiApplication::handleColourChanged, this));

		wmgr.getWindow((CEGUI::utf8*)"Demo8/Window1/Controls/Add")
			->subscribeEvent(
			CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&GuiApplication::handleAdd, this));

		CEGUI::Window* root = wmgr.getWindow("Demo8");
		setupEnterExitEvents(root);
		*/


	}

    bool handleQuit(const CEGUI::EventArgs& e)
    {
        static_cast<GuiFrameListener*>(mFrameListener)->requestShutdown();
        return true;
    }


};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{

    // Create application object
    GuiApplication app;

    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
#endif
    }


    return 0;
}


