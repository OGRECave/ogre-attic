// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilterRenderer.cpp $
// $Id: OgrePostFilterRenderer.cpp,v 1.2 2005-08-27 15:49:51 miathan6 Exp $

#include "OgrePostFilterRenderer.h"
#include "OgreHardwarePixelBuffer.h"
using namespace Ogre;

OgrePostFilterRenderer::OgrePostFilterRenderer( OgrePostFilterShared* aSharedData ) {

	iIsActive = false;
	iShared = aSharedData;

	// create the postprocess node
	iPostFilterNode = iShared->iSceneManager->createSceneNode( "postFilterNode" );

	// setup the fullscreen quad
	iFsQuad = new Rectangle2D( true );
	iFsQuad->setVisible( true );
	iFsQuad->setCorners( -1, 1, 1, -1 );
	//iFsQuad->setRenderQueueGroup( RENDER_QUEUE_OVERLAY );
	iFsQuad->setRenderQueueGroup( RENDER_QUEUE_1 ); // first render queue, above skies but below entities
	iPostFilterNode->attachObject( iFsQuad );
	iFsQuad->setBoundingBox( AxisAlignedBox( -1000 * Ogre::Vector3::UNIT_SCALE, 1000 * Ogre::Vector3::UNIT_SCALE ) );

}

OgrePostFilterRenderer::~OgrePostFilterRenderer() 
{
	setPostFilter( NULL );

	iPostFilterNode->detachObject( iFsQuad );
	delete iFsQuad;

}

void OgrePostFilterRenderer::setActive( bool aBoolValue ) {

	if ( !iPostFilter ) return;

	iIsActive = aBoolValue;

	if ( iIsActive ) {

		// We need to disable the autoupdated feature
		// on scene specific rendertargets in order to
		// render a postfilter correctly.
		//disableSceneRenderTargets();

		// Add "this" as a listener for the render window.
		//
		// Not an elegant choice, but there is no "findListener"
		// or something like. However, the computational complexity of
		// "removeListener + addListener" combo is near to the one
		// of a "if ( findListener() ) addListener()"
		iShared->iWindow->removeListener( this );
		iShared->iWindow->addListener( this );

		// add the postfilter node
		iShared->iSceneManager->getRootSceneNode()->addChild( iPostFilterNode );

	} else {

		// remove the postfilter node
		iShared->iSceneManager->getRootSceneNode()->removeChild( iPostFilterNode );

		// remove the framelistener
		iShared->iWindow->removeListener( this );

		// show the scene
		iShared->iMainSceneNode->setVisible( true, true );

	}

}

void OgrePostFilterRenderer::setPostFilter( OgrePostFilter* aPostFilter ) {

	if ( aPostFilter ) {

		// If a postfilter has been previously set and
		// it's still active, then is better to restore
		// the scene-only rendertargets when it was active
		if ( iPostFilter && iIsActive ) {

//			restoreSceneRenderTargets();

		}

		iPostFilter = aPostFilter;

		iPostFilter->Initialize( iShared );

		// setup additionally shared data coming from this pass
		iShared->iRtScene = iPostFilter->getSceneRt();

		// let's the user customize the final material
		// ie. scene blending, depth write, ...
		//iPostFilter->customizePostFilterMaterial( iPostFilterMat );

	} else {

		setActive( false );

	}


}

void OgrePostFilterRenderer::preRenderTargetUpdate(const RenderTargetEvent& evt) {

	OgrePostFilterObserver* observer = ( iPostFilter->getObserver() ) ? iPostFilter->getObserver() : this;

	// take a snapshot of the real scene
	if(iShared->iRtScene)
		iShared->iRtScene->update();

	// hide the scene
	iShared->iMainSceneNode->setVisible( false, true );

	// only the quad will be visible
	iFsQuad->setVisible( true );

	// iterate through passes and render
	MaterialPtr mat;
	TexturePtr inputTexture;
	unsigned int tus;
	std::vector<TexturePtr>::iterator itTexture;
	std::vector<OgrePostFilterPass*>::iterator itPass;
	for ( itPass = iPostFilter->getPasses()->begin(); itPass != iPostFilter->getPasses()->end(); itPass++ ) {

		pass = *itPass;

		// set the material
		iFsQuad->setMaterial( pass->getMaterialName() );
		mat = iFsQuad->getMaterial();

		// perform material adjustments on input
		// textures as requested by the user
		tus = 0;
		for ( itTexture = pass->getInputTextures()->begin(); itTexture != pass->getInputTextures()->end(); itTexture++, tus++ ) {
			inputTexture = *itTexture;
			Technique *tech = mat->getBestTechnique(0);
			if(!tech)
				// No technique?
				continue;
			for(unsigned short pass=0; pass<tech->getNumPasses(); ++pass)
			{
				tech->getPass(pass)->getTextureUnitState(tus)->setTextureName( 
					inputTexture->getName() 
				);
			}
		}

		// call back the user in order to let's him
		// setup custom parameters on his material
		observer->preRenderPostFilterPass( mat );

		// update the output texture for this pass
		if(pass->getOutputTexture())
		{
			pass->preUpdateNotify();
			pass->getOutputTexture()->update();
			pass->postUpdateNotify();
		}

		// Show alternative renderables for node/pass
	}

	// let's the postFilterMat render the result
	pass->preUpdateNotify();
}

void OgrePostFilterRenderer::postRenderTargetUpdate(const RenderTargetEvent& evt)
{
	pass->postUpdateNotify();
	// show the original scene
	iShared->iMainSceneNode->setVisible( true, true );

	// make sure to hide the quad because it's occluding
	// the whole scene
	iFsQuad->setVisible( false );
}
