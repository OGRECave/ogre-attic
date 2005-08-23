// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilter.cpp $
// $Id: OgrePostFilter.cpp,v 1.1 2005-08-23 15:54:12 miathan6 Exp $

#include "OgrePostFilter.h"
#include <OgreHardwarePixelBuffer.h>

// initialize statics members here
unsigned int OgrePostFilter::iTextureCount = 0;

OgrePostFilter::OgrePostFilter( const String& aName /* = "" */ )
	: iObserver(0),
	  iShared(0)
{

	iHasBeenSetup = false;
	iPasses.clear();
	iName = aName;

}

OgrePostFilter::~OgrePostFilter() 
{
	/// Delete all passes
	for(std::vector<OgrePostFilterPass*>::iterator i=iPasses.begin(); i!=iPasses.end(); ++i)
	{
		delete (*i);
	}
}

void OgrePostFilter::Initialize( OgrePostFilterShared* aSharedData ) {


	if ( iHasBeenSetup ) return;

	iShared = aSharedData;

	// call the user to create its own stuff
	iRtScene = setupSceneRt();

	// call the user to create its own passes
	setupPasses( iPasses );

	// build a vector of currently used rendertargets
	buildRenderTargetsVector();

	iHasBeenSetup = true;

}

void OgrePostFilter::buildRenderTargetsVector() {

	// setup a vector with postfilter's rendertargets
	iRenderTargets.clear();

	if(iRtScene)
		iRenderTargets.push_back( iRtScene );

	// enumerate all passes and search for rendertargets
	OgrePostFilterPass* pass;
	std::vector<OgrePostFilterPass*>::iterator itPass;
	for ( itPass = iPasses.begin(); itPass != iPasses.end(); itPass++ ) {

		pass = *itPass;
		if(pass->getOutputTexture())
			iRenderTargets.push_back( pass->getOutputTexture() );

		// enumerate all input textures
		std::vector<TexturePtr>::iterator itTexture;
		for ( itTexture = pass->getInputTextures()->begin(); itTexture != pass->getInputTextures()->end(); itTexture++ ) {

			iRenderTargets.push_back( (*itTexture)->getBuffer()->getRenderTarget() );

		}

	}

	// remove duplicates
	std::sort( iRenderTargets.begin(), iRenderTargets.end() );
	iRenderTargets.erase( std::unique( iRenderTargets.begin(), iRenderTargets.end() ), iRenderTargets.end() );

	// change rendertargets priority to be sure our own
	// will be processed firstly
	std::vector<RenderTarget*>::iterator it;
	for ( it = iRenderTargets.begin(); it != iRenderTargets.end(); it++ ) {

		changeRenderTargetPriority( *it, 0 );

	}

}

// "RenderSystem()->createRenderTexture()" don't let's you specify a priority value.
// The "RenderTarget::setPriority()" API reference claim the need to specify the
// priority value *BEFORE* attaching the RenderTarget itself to the RenderSystem, but
// we have no way to change the priority before creating the RenderTexture because
// the "RenderSystem()->createRenderTexture()" attach it to the RenderSystem *BEFORE*
// returning the instance.
// So, detaching the RenderTarget, changing it's priority and then re-attaching it is
// enough.
void OgrePostFilter::changeRenderTargetPriority( RenderTarget* aRenderTarget, uchar aPriority ) {

	iShared->iRoot->getRenderSystem()->detachRenderTarget( aRenderTarget->getName() );
	aRenderTarget->setPriority( aPriority );
	iShared->iRoot->getRenderSystem()->attachRenderTarget( *aRenderTarget );

}

void OgrePostFilter::setupViewPort(RenderTarget *rtt)
{
	rtt->setAutoUpdated( false );

	Viewport* v = rtt->addViewport( iShared->iCamera );
	v->setClearEveryFrame( true );
	v->setOverlaysEnabled( false );
	v->setBackgroundColour( ColourValue( 0, 0, 0 ) );
}

TexturePtr OgrePostFilter::createRenderTexture( const String& aName, unsigned int aWidth, unsigned int anHeight,
												    TextureType aTexType /* = TEX_TYPE_2D */, PixelFormat aPixelFormat /* = PF_X8R8G8B8 */,
													bool doAutoSetupOnViewport /* = true */ )
{
	TexturePtr rt = TextureManager::getSingleton().createManual( aName, 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, aTexType, 
			aWidth, anHeight, 0, aPixelFormat, TU_RENDERTARGET );

	if ( doAutoSetupOnViewport ) {
		setupViewPort(rt->getBuffer()->getRenderTarget());
	}

	iTextureCount++;
	return rt;

}


TexturePtr OgrePostFilter::createRenderTexture( unsigned int aWidth, unsigned int anHeight,
												   TextureType aTexType /* = TEX_TYPE_2D */, PixelFormat aPixelFormat, /* = PF_X8R8G8B8 */
												   bool doAutoSetupOnViewport /* = true */ )
{
	String textureName = "pfRt_auto_" + Ogre::StringConverter::toString( iTextureCount );
	return ( createRenderTexture( textureName, aWidth, anHeight, aTexType, aPixelFormat, doAutoSetupOnViewport ) );

}