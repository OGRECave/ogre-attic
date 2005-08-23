/******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgrePostFilter/OgrePostFilterManager.h"

#include "DeferredShading.h"

#include "CreateSphere.h"

using namespace Ogre;

/** Postfilter doing full deferred shading with two lights in one pass
*/
class DeferredShadingPf : public OgrePostFilter, public OgrePostFilterPassListener 
{
	public:
		DeferredShadingPf(const TexturePtr &texture0, const TexturePtr &texture1, SceneManager *scenemgr)
			: OgrePostFilter( "DeferredShading" ),
			mTexture0(texture0),
			mTexture1(texture1),
			mSceneMgr(scenemgr)
		{

		}
		// OgrePostFilterPassListener
		virtual void preUpdateNotify()
		{
			// Show light geometry
			mSceneMgr->setVisibilityMask(mSceneMgr->getVisibilityMask() | DeferredShadingSystem::PostVisibilityMask);
		}
		virtual void postUpdateNotify()
		{
			// Hide light geometry
			mSceneMgr->setVisibilityMask(mSceneMgr->getVisibilityMask() & ~DeferredShadingSystem::PostVisibilityMask);
		}
	public:
		virtual RenderTarget* setupSceneRt() { return 0; }
		virtual void setupPasses( std::vector<OgrePostFilterPass*>& filterPasses ) 
		{
			OgrePostFilterPass *iPass2 = new OgrePostFilterPass( "DeferredShading/Post/Single" );
			iPass2->addInputTexture( mTexture0 );
			iPass2->addInputTexture( mTexture1 );
			iPass2->setListener(this);
			iPass2->setOutputTexture( 0 );
			filterPasses.push_back( iPass2 );
		}
	private:
		TexturePtr mTexture0, mTexture1;
		SceneManager *mSceneMgr;
};

/** Postfilter doing full deferred shading with an ambient pass and multiple light passes
*/
class DeferredMultiShadingPf : public OgrePostFilter, public OgrePostFilterPassListener
{
	public:
		DeferredMultiShadingPf(const TexturePtr &texture0, const TexturePtr &texture1, SceneManager *scenemgr)
			: OgrePostFilter( "DeferredMulti" ),
			mTexture0(texture0),
			mTexture1(texture1),
			mSceneMgr(scenemgr)
		{
		}
		~DeferredMultiShadingPf()
		{
		}
		virtual RenderTarget* setupSceneRt() { return 0; }
		virtual void setupPasses( std::vector<OgrePostFilterPass*>& filterPasses ) 
		{
			OgrePostFilterPass *iPass2 = new OgrePostFilterPass( "DeferredShading/Post/Multi" );
			iPass2->addInputTexture( mTexture0 );
			iPass2->addInputTexture( mTexture1 );
			iPass2->setOutputTexture( 0 );
			iPass2->setListener(this);
			filterPasses.push_back( iPass2 );
		}
		virtual void preUpdateNotify()
		{
			// Show light geometry
			mSceneMgr->setVisibilityMask(mSceneMgr->getVisibilityMask() | DeferredShadingSystem::PostVisibilityMask);
		}
		virtual void postUpdateNotify()
		{
			// Hide light geometry
			mSceneMgr->setVisibilityMask(mSceneMgr->getVisibilityMask() & ~DeferredShadingSystem::PostVisibilityMask);
		}
	private:
		TexturePtr mTexture0, mTexture1;
		SceneManager *mSceneMgr;
};


/** Postfilter that shows the normal channel
*/
class ShowNormalPf : public OgrePostFilter 
{
	public:
		ShowNormalPf(const TexturePtr &texture0, const TexturePtr &texture1)
			: OgrePostFilter( "ShowNormal" ),
			mTexture0(texture0), mTexture1(texture1) 
		{ }
	public:
		virtual RenderTarget* setupSceneRt() { return 0; }
		virtual void setupPasses( std::vector<OgrePostFilterPass*>& filterPasses ) 
		{			
			OgrePostFilterPass *iPass2 = new OgrePostFilterPass( "DeferredShading/Post/ShowNormal");
			iPass2->addInputTexture( mTexture0 );
			iPass2->addInputTexture( mTexture1 );
			iPass2->setOutputTexture( 0 );
			filterPasses.push_back( iPass2 );
		}
	private:
		TexturePtr mTexture0, mTexture1;
};

/** Postfilter that shows the depth and specular channel
*/
class ShowDSPf : public OgrePostFilter 
{
	public:
		ShowDSPf(const TexturePtr &texture0, const TexturePtr &texture1)
			: OgrePostFilter( "ShowDepthSpec" ),
			mTexture0(texture0), mTexture1(texture1) 
		{ }
	public:
		virtual RenderTarget* setupSceneRt() { return 0; }
		virtual void setupPasses( std::vector<OgrePostFilterPass*>& filterPasses ) 
		{
			OgrePostFilterPass *iPass2 = new OgrePostFilterPass( "DeferredShading/Post/ShowDS");
			iPass2->addInputTexture( mTexture0 );
			iPass2->addInputTexture( mTexture1 );
			iPass2->setOutputTexture( 0 );
			filterPasses.push_back( iPass2 );
		}
	private:
		TexturePtr mTexture0, mTexture1;
};

/** Postfilter that shows the colour channel
*/
class ShowColourPf : public OgrePostFilter 
{
	public:
		ShowColourPf(const TexturePtr &texture0, const TexturePtr &texture1)
			: OgrePostFilter( "ShowColour" ),
			mTexture0(texture0), mTexture1(texture1) 
		{ }
	public:
		virtual RenderTarget* setupSceneRt() { return 0; }
		virtual void setupPasses( std::vector<OgrePostFilterPass*>& filterPasses ) 
		{
			OgrePostFilterPass *iPass2 = new OgrePostFilterPass( "DeferredShading/Post/ShowColour");
			iPass2->addInputTexture( mTexture0 );
			iPass2->addInputTexture( mTexture1 );
			iPass2->setOutputTexture( 0 );
			filterPasses.push_back( iPass2 );
		}
	private:
		TexturePtr mTexture0, mTexture1;
};

MLight::MLight(SceneManager *mSceneMgr, SceneNode *parent, uint32 visibilityFlags)
{
	nodeM = parent->createChildSceneNode();
	nodeL = nodeM->createChildSceneNode();
	// Set up geometry
	geom = mSceneMgr->createEntity(nodeL->getName(), "PointLightMesh");
	geom->setMaterialName("DeferredShading/Post/LightMaterial");
	// Set render priority to high (just after normal postprocess)
	geom->setRenderQueueGroup(RENDER_QUEUE_2);
	// Hide when created
	//geom->setVisible(false);
	geom->setVisibilityFlags(visibilityFlags);
	// Diffuse and specular colour
	setDiffuseColour(Ogre::ColourValue(1,1,1));
	setSpecularColour(Ogre::ColourValue(0,0,0));

	nodeL->attachObject(geom);
}
void MLight::setFalloff(float c, float b, float a)
{
	// Set falloff parameter to shader
	geom->getSubEntity(0)->setCustomParameter(3, Vector4(c, b, a, 0));

	// Calculate radius from falloff
	int threshold_level = 15;// differece of 10-15 levels deemed unnoticable
	float threshold = 1.0f/((float)threshold_level/256.0f); 

	// Use quadratic formula
	c = c-threshold;
	float d=sqrt(b*b-4*a*c);
	float x=(-2*c)/(b+d);

	// Scale node to radius
	nodeL->setScale(x,x,x);
}
void MLight::setDiffuseColour(const Ogre::ColourValue &col)
{
	geom->getSubEntity(0)->setCustomParameter(1, Vector4(col.r, col.g, col.b, col.a));
}
void MLight::setSpecularColour(const Ogre::ColourValue &col)
{
	geom->getSubEntity(0)->setCustomParameter(2, Vector4(col.r, col.g, col.b, col.a));
}
Ogre::SceneNode *MLight::getNode()
{
	return nodeM;
}

Ogre::ColourValue MLight::getDiffuseColour()
{
	Ogre::Vector4 val = geom->getSubEntity(0)->getCustomParameter(1);
	return Ogre::ColourValue(val[0], val[1], val[2], val[3]);
}

Ogre::ColourValue MLight::getSpecularColour()
{
	Ogre::Vector4 val = geom->getSubEntity(0)->getCustomParameter(2);
	return Ogre::ColourValue(val[0], val[1], val[2], val[3]);
}


DeferredShadingSystem::DeferredShadingSystem(
		RenderTarget *win, SceneManager *sm, SceneNode *rootNode, Camera *cam
	):
	mSceneMgr(sm), mWindow(win), mRootNode(rootNode), mCamera(cam)
{
	iPfManager = 0;
	iDeferredShadingPf = 0;
	iShowNormalPf = 0;
	iShowDSPf = 0;
	iShowColourPf = 0;
	iDeferredMultiShadingPf = 0;
	mLightParentNode = 0;
	rttTex = 0;

	createResources();
	// Hide post geometry
	mSceneMgr->setVisibilityMask(mSceneMgr->getVisibilityMask() & ~PostVisibilityMask);
	// Default to normal deferred shading mode
	setMode(DSM_SINGLEPASS);
	setActive(true);
}

DeferredShadingSystem::~DeferredShadingSystem()
{
	// Delete mini lights
	for(std::set<MLight*>::iterator i=mLights.begin(); i!=mLights.end(); ++i)
	{
		delete (*i);
	}

	delete iDeferredShadingPf;
	delete iShowNormalPf;
	delete iShowDSPf;
	delete iShowColourPf;
	delete iDeferredMultiShadingPf;
	delete iPfManager;
}
void DeferredShadingSystem::setMode(DSMode mode)
{
	switch(mode)
	{
	case DSM_SINGLEPASS:
		iPfManager->setPostFilter(iDeferredShadingPf);
		break;
	case DSM_MULTIPASS:
		iPfManager->setPostFilter(iDeferredMultiShadingPf);
		break;
	case DSM_SHOWCOLOUR:
		iPfManager->setPostFilter(iShowColourPf);
		break;
	case DSM_SHOWNORMALS:
		iPfManager->setPostFilter(iShowNormalPf);
		break;
	case DSM_SHOWDSP:
		iPfManager->setPostFilter(iShowDSPf);
		break;
	}
}
void DeferredShadingSystem::setActive(bool active)
{
	iPfManager->getRenderer()->setActive(active);
}
void DeferredShadingSystem::createResources(void)
{
	// Create 'fat' render target
	unsigned int width = mWindow->getWidth();
	unsigned int height = mWindow->getHeight();
	PixelFormat format = PF_FLOAT16_RGBA;
	//PixelFormat format = PF_BYTE_RGBA;

	mTexture0 = TextureManager::getSingleton().createManual("RttTex0", 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
		width, height, 0, format, TU_RENDERTARGET );
	mTexture1 = TextureManager::getSingleton().createManual("RttTex1", 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, 
		width, height, 0, format, TU_RENDERTARGET );

	rttTex = Ogre::Root::getSingleton().getRenderSystem()->createMultiRenderTarget("MRT");
	rttTex->bindSurface(0, mTexture0->getBuffer()->getRenderTarget());
	rttTex->bindSurface(1, mTexture1->getBuffer()->getRenderTarget());
	rttTex->setAutoUpdated( false );

	// Setup viewport on 'fat' render target
	Viewport* v = rttTex->addViewport( mCamera );
	v->setClearEveryFrame( true );
	v->setOverlaysEnabled( false );
	v->setBackgroundColour( ColourValue( 0, 0, 0, 0) );

	// Create mini lights
	initialiseLightGeometry();

	// Create postfilter manager
	iPfManager = new OgrePostFilterManager( 
		Root::getSingletonPtr(),
		mWindow, mSceneMgr, mRootNode, mCamera 
	);

	// Create filters
	iDeferredShadingPf = new DeferredShadingPf(mTexture0, mTexture1, mSceneMgr);
	iDeferredMultiShadingPf = new DeferredMultiShadingPf(mTexture0, mTexture1, mSceneMgr);
	iShowColourPf = new ShowColourPf(mTexture0, mTexture1);
	iShowNormalPf = new ShowNormalPf(mTexture0, mTexture1);
	iShowDSPf = new ShowDSPf(mTexture0, mTexture1);
}

void DeferredShadingSystem::initialiseLightGeometry()
{
	// Create mesh
	createSphere("PointLightMesh", 1.0f, 6, 6);
	// Create uber node for lights
	mLightParentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("LightSwarm");
	// Hide
	//mLightParentNode->setVisible(false, true);
	// Setup material
	MaterialPtr mat = MaterialManager::getSingleton().getByName("DeferredShading/Post/LightMaterial");
	for(size_t i=0; i<mat->getNumTechniques(); ++i)
	{
		Pass *pass = mat->getTechnique(i)->getPass(0);
		pass->getTextureUnitState(0)->setTextureName(mTexture0->getName());
		pass->getTextureUnitState(1)->setTextureName(mTexture1->getName());
	}

	//GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
	//params->setNamedConstant("texSize", Vector4(mTexture0->getWidth(), mTexture0->getHeight(), 0, 0));
}

void DeferredShadingSystem::setMLightsVisible(bool active)
{
	//static_cast<DeferredShadingPf*>(iDeferredShadingPf)->enableMLights(active);
	//static_cast<DeferredMultiShadingPf*>(iDeferredMultiShadingPf)->enableMLights(active);
}

MLight *DeferredShadingSystem::createMLight()
{
	MLight *rv = new MLight(mSceneMgr, mLightParentNode, PostVisibilityMask);
	mLights.insert(rv);

	return rv;
}
void DeferredShadingSystem::destroyMLight(MLight *m)
{
	mLights.erase(m);
	delete m;
}

void DeferredShadingSystem::update()
{
	rttTex->update();
}
