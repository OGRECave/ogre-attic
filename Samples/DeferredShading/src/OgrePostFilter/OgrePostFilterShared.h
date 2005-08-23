// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilterShared.h $
// $Id$

#pragma once

#include "Ogre.h"

using namespace Ogre;


class OgrePostFilterShared {

	public:

		OgrePostFilterShared()
			: iSceneManager(0),
			  iMainSceneNode(0),
			  iRoot(0),
			  iWindow(0),
			  iCamera(0),
			  iRtScene(0) {}

		~OgrePostFilterShared() {}


		// SHARED DATA
		// we don't need setters/getters here

		SceneManager* iSceneManager;
		SceneNode *iMainSceneNode;
		Root* iRoot;
		RenderTarget* iWindow;
		Camera* iCamera;
		RenderTarget* iRtScene;

};
