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
        Gui.cpp
    \brief
        An example of CEGUI's features
*/

#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUILogger.h>
#include <CEGUI/CEGUISchemeManager.h>
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/CEGUIWindow.h>
#include <CEGUI/elements/CEGUIPushButton.h>
#include <CEGUI/elements/CEGUIScrollbar.h>
#include <CEGUI/renderers/OgreGUIRenderer/ogrerenderer.h>
#include <CEGUI/renderers/OgreGUIRenderer/OgreResourceProvider.h>
#include <CEGUI/CEGUIDefaultResourceProvider.h>

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

        e->consume();
    }
};

class GuiApplication : public ExampleApplication
{
private:
    CEGUI::OgreRenderer* mGUIRenderer;
    CEGUI::System* mGUISystem;

public:
    GuiApplication()
      : mGUIRenderer(0),
        mGUISystem(0)
    {
    }

    ~GuiApplication()
    {
        if(mGUISystem)
            delete mGUISystem;
        if(mGUIRenderer)
            delete mGUIRenderer;
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

        // setup GUI system
        mGUIRenderer = new CEGUI::OgreRenderer(mWindow, 
            Ogre::RENDER_QUEUE_OVERLAY, false, 3000);

        mGUISystem = new CEGUI::System(mGUIRenderer);

        CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Informative);

        Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");

        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        headNode->attachObject(ogreHead);


        // Setup Render To Texture for preview window
        RenderTexture* rttTex = mRoot->getRenderSystem()->createRenderTexture( "RttTex", 512, 512, TEX_TYPE_2D, PF_R8G8B8 );
        {
            Camera* rttCam = mSceneMgr->createCamera("RttCam");
            SceneNode* camNode = 
                mSceneMgr->getRootSceneNode()->createChildSceneNode("rttCamNode");
            camNode->attachObject(rttCam);
            rttCam->setPosition(0,0,200);
            //rttCam->setVisible(true);

            Viewport *v = rttTex->addViewport( rttCam );
            v->setOverlaysEnabled(false);
            v->setClearEveryFrame( true );
            v->setBackgroundColour( ColourValue::Black );
        }

        // Retrieve CEGUI texture for the RTT
        CEGUI::Texture* rttTexture = mGUIRenderer->createTexture((CEGUI::utf8*)"RttTex");

        CEGUI::Imageset* rttImageSet = 
            CEGUI::ImagesetManager::getSingleton().createImageset(
                    (CEGUI::utf8*)"RttImageset", rttTexture);

        rttImageSet->defineImage((CEGUI::utf8*)"RttImage", 
                CEGUI::Point(0.0f, 0.0f),
                CEGUI::Size(512,512),
                CEGUI::Point(0.0f,0.0f));

        // load scheme and set up defaults
        CEGUI::SchemeManager::getSingleton().loadScheme(
                (CEGUI::utf8*)"TaharezLook.scheme");
        mGUISystem->setDefaultMouseCursor(
                (CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
        mGUISystem->setDefaultFont((CEGUI::utf8*)"Tahoma-12");

        CEGUI::Window* sheet = 
            CEGUI::WindowManager::getSingleton().loadWindowLayout(
                (CEGUI::utf8*)"ogregui.layout"); 
        mGUISystem->setGUISheet(sheet);

        setupEventHandlers();
    }

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new GuiFrameListener(mWindow, mCamera, mGUIRenderer);
        mRoot->addFrameListener(mFrameListener);
    }

    bool setup()
    {
        ExampleApplication::setup();
        LogManager::getSingleton().setLogDetail( LL_BOREME );

        return true;
    }

    void setupEventHandlers(void)
    {
        CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"OgreGuiDemo/Button")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GuiApplication::handleQuit, this));

        /*
        CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"OgreGuiDemo/Window/Controls/Zoom")->subscribeEvent(CEGUI::Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber(&GuiApplication::handleScrollbar, this));
        */
    }

    bool handleQuit(const CEGUI::EventArgs& e)
    {
        static_cast<GuiFrameListener*>(mFrameListener)->requestShutdown();
        return true;
    }

    bool handleScrollbar(const CEGUI::EventArgs& e)
    {
        Node* rttCamNode =  mSceneMgr->getRootSceneNode()->getChild("rttCamNode");
        const CEGUI::WindowEventArgs& windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>(e);
        float position = static_cast<CEGUI::Scrollbar*>(windowEventArgs.window)->getScrollPosition();

        rttCamNode->translate(0, 0, position);
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

