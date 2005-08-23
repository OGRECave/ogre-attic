// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilterManager.h $
// $Id: OgrePostFilterManager.h,v 1.1 2005-08-23 15:54:12 miathan6 Exp $

#pragma once

#include "OgrePostFilter.h"
#include "OgrePostFilterRenderer.h"

// This should be an Ogre::Singleton, but you
// need to take into account its destruction, whereas
// the singleton pattern can let's you forget about this
// So, consider singleton-ize it in your framework
class OgrePostFilterManager {

	public:

		OgrePostFilterManager( Root* anOgreRoot,
							   RenderTarget* aRenderWindow,
							   SceneManager* aSceneManager,
							   SceneNode* aMainSceneNode,
							   Camera* aCamera );

		~OgrePostFilterManager();

		inline OgrePostFilterRenderer* getRenderer() {

			return iRenderer;

		}

	public:

		void setPostFilter( OgrePostFilter* aPostFilter );


	private:

		OgrePostFilterShared* iShared;
		OgrePostFilterRenderer* iRenderer;

};
