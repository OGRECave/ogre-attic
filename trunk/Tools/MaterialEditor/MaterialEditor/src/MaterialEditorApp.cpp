#include "MaterialEditorApp.h"

#include <wx/splash.h>

#include "OgreConfigFile.h"
#include "OgreLog.h"
#include "OgreLogManager.h"
#include "OgreResourceManager.h"

#include "MaterialEditorFrame.h"

using Ogre::ConfigFile;
using Ogre::LogManager;
using Ogre::ResourceGroupManager;

MaterialEditorApp::~MaterialEditorApp()
{
	if (mRoot) delete mRoot;
}

bool MaterialEditorApp::OnInit()
{
	wxInitAllImageHandlers();

	wxBitmap bitmap;
	if(bitmap.LoadFile("resources/images/splash.png", wxBITMAP_TYPE_PNG))
	{
		wxSplashScreen* splash = new wxSplashScreen(bitmap, wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT,
			2000, NULL, -1, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER | wxSTAY_ON_TOP);
	}

	wxYield();

	// Create Log Manager and default log
	new LogManager();
	LogManager::getSingletonPtr()->createLog("OGRE", true, true, false);

	// Create Ogre Root
	mRoot = new Ogre::Root("plugins.cfg", "ogre.cfg", "OGRE");

	ConfigFile cf;
	cf.load("resources.cfg");

	ConfigFile::SectionIterator seci = cf.getSectionIterator();

	Ogre::String secName, typeName, archName;
	while(seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for(i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
		}
	}

	// cant init here, no rendersystem!
	//mRoot->initialise(false);

	MaterialEditorFrame* frame = new MaterialEditorFrame(NULL);
	frame->Show(TRUE);

	SetTopWindow(frame);

	return TRUE;
}