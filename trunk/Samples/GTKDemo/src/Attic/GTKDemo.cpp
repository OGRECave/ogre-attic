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

#include "ExampleApplication.h"
#include "ExampleFrameListener.h"

#include "GTKWindow.h"

#include <gtkmm/box.h>
#include <gtkmm/scale.h>
#include <gtkmm/window.h>

using namespace Ogre;

class GTKDemoFrameListener : public FrameListener
{
public:
    bool frameStarted(const FrameEvent& evt)
    {
        return true;
    }

    bool frameEnded(const FrameEvent& evt) 
    {
        return true;
    }
};

class GTKDemoApplication : public ExampleApplication, public SigC::Object
{
public:
    void go(void)
    {
        if (!setup())
            return;

        gtk_win->show();
        gtk_win->update();
        Gtk::Main::run();
    }

protected:
    bool on_delete_event(GdkEventAny* event)
    {
        Gtk::Main::quit();
        return false;
    }

    bool on_expose_event(GdkEventExpose* event)
    {
        gtk_win->update();
        return false;
    }

    void on_value_changed(void)
    {
        Real s = hscale->get_value();
        headNode->setScale(s, s, s);
        gtk_win->update();
    }

    void setupGTKWindow(void)
    {
        // Setup our window
        gtk_win = static_cast<GTKWindow*>(mWindow);
        gtk_win->signal_delete_event().connect(SigC::slot(*this,
            &GTKDemoApplication::on_delete_event));
        OGREWidget* owidget = gtk_win->get_ogre_widget();
        owidget->signal_expose_event().connect(SigC::slot(*this,
            &GTKDemoApplication::on_expose_event));
        owidget->reference();
        gtk_win->remove();
        vbox = new Gtk::VBox;
        gtk_win->add(*vbox);
        vbox->show();

        vbox->pack_end(*owidget, true, true);
        owidget->unreference();

        hscale = Gtk::manage(new Gtk::HScale(1.0, 5.0, 0.1));
        hscale->signal_value_changed().connect(SigC::slot(*this,
            &GTKDemoApplication::on_value_changed));
        vbox->pack_end(*hscale, false, true);
        hscale->show();
    }

    void createScene(void)
    {
        setupGTKWindow();

        mRoot->showDebugOverlay(false);

        mSceneMgr->setAmbientLight(ColourValue(0.6, 0.6, 0.6));
        // Setup the actual scene
        Light* l = mSceneMgr->createLight("MainLight");
        l->setPosition(0, 100, 500);

		Entity* head = mSceneMgr->createEntity("head", "ogrehead.mesh");
        headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		headNode->attachObject(head);

        mCamera->setAutoTracking(true, headNode);
    }

    void createFrameListener(void)
    {
		// This is where we instantiate our own frame listener
        mFrameListener= new GTKDemoFrameListener();
        mRoot->addFrameListener(mFrameListener);
    }

private:
    GTKWindow* gtk_win;
    Gtk::VBox* vbox;
    Gtk::HScale* hscale;
    SceneNode* headNode;
};

#if OGRE_PLATFORM == PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    GTKDemoApplication app;
 
    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occured: %s\n",
                e.getFullDescription().c_str());
#endif
    }


    return 0;
}
