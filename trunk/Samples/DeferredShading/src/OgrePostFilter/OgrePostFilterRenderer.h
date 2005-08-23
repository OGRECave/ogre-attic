// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilterRenderer.h $
// $Id: OgrePostFilterRenderer.h,v 1.1 2005-08-23 15:54:12 miathan6 Exp $

#pragma once

#include "Ogre.h"
#include "OgrePostFilterShared.h"
#include "OgrePostFilter.h"

using namespace Ogre;


class OgrePostFilterRenderer : public RenderTargetListener,
							   public OgrePostFilterObserver
{

	public:

		OgrePostFilterRenderer( OgrePostFilterShared* aSharedData );
		virtual ~OgrePostFilterRenderer();


	public:

		// specifing a NULL postfilter will deactivate the renderer
		void setPostFilter( OgrePostFilter* aPostFilter );
		void setActive( bool aBoolValue );

		inline bool isActive() {

			return iIsActive;

		}


	// from RenderTargetListener
	public:

		virtual void preRenderTargetUpdate(const RenderTargetEvent& evt);
		virtual void postRenderTargetUpdate(const RenderTargetEvent& evt);


	private:

		// given resources
		OgrePostFilterShared* iShared;

		// own-allocated resources
		SceneNode* iPostFilterNode;
		Rectangle2D* iFsQuad;
		OgrePostFilter* iPostFilter;
		bool iIsActive;
		// Current pass
		OgrePostFilterPass* pass;
	

};
