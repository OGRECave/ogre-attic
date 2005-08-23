// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilter.h $
// $Id: OgrePostFilter.h,v 1.1 2005-08-23 15:54:12 miathan6 Exp $

#pragma once

#include "Ogre.h"
#include "OgrePostFilterShared.h"
#include "OgrePostFilterPass.h"
#include "OgrePostFilterObserver.h"

using namespace Ogre;


// forward declaration
class OgrePostFilterRenderer;
class RenderTargetStateMap;


class OgrePostFilter {

	friend class OgrePostFilterRenderer;

	public:

		OgrePostFilter( const String& aName = "" );
		virtual ~OgrePostFilter();


	public:

		inline String& getName() {

			return iName;

		}


		// dbg, i'll replace pointers with iterators

		inline std::vector<OgrePostFilterPass*>* getPasses() {

			return &iPasses;

		}

		inline std::vector<RenderTarget*>* getRenderTargets() {

			return &iRenderTargets;

		}

		inline RenderTarget *getSceneRt() {

			return iRtScene;

		}

		inline std::vector<RenderTargetStateMap*>* getSceneRenderTargetsStateMap() {

			return &iRtStates;

		}

		inline void setObserver( OgrePostFilterObserver* anObserver ) {

			iObserver = anObserver;

		}

		inline OgrePostFilterObserver* getObserver() {

			return iObserver;

		}

	public:

		// user callback
		virtual RenderTarget *setupSceneRt() = 0;
		virtual void setupPasses( std::vector<OgrePostFilterPass*>& filterPasses ) = 0;

		// Be careful with this one!
		// It's called when you call "OgrePostFilterManager::setPostFilter"
		virtual void customizePostFilterMaterial( MaterialPtr aPostFilterMaterial ) {};


	protected:

		TexturePtr createRenderTexture( const String& aName, unsigned int aWidth, unsigned int anHeight,
											TextureType aTexType = TEX_TYPE_2D, PixelFormat aPixelFormat = PF_X8R8G8B8,
											bool doAutoSetupOnViewport = true );

		TexturePtr createRenderTexture( unsigned int aWidth, unsigned int anHeight,
											TextureType aTexType = TEX_TYPE_2D, PixelFormat aPixelFormat = PF_X8R8G8B8,
											bool doAutoSetupOnViewport = true );
		void setupViewPort(RenderTarget *rtt);
		static unsigned int iTextureCount;


	private:

		// call back the user and lets
		// him setup textures and passes
		void Initialize( OgrePostFilterShared* aSharedData );
		void buildRenderTargetsVector();
		void changeRenderTargetPriority( RenderTarget* aRenderTarget, uchar aPriority );


		bool iHasBeenSetup;
		RenderTarget *iRtScene;
		OgrePostFilterObserver* iObserver;
		std::vector<RenderTarget*> iRenderTargets;
		std::vector<OgrePostFilterPass*> iPasses;
		OgrePostFilterShared* iShared;
		std::vector<RenderTargetStateMap*> iRtStates;
		String iName;

};

