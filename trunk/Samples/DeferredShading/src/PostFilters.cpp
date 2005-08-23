// The scene has been taken from the "Samples/RenderToTexture"
//
// $HeadURL: svn://localhost/OgreDev/testBed/PostFilters.cpp $
// $Id: PostFilters.cpp,v 1.1 2005-08-23 15:54:12 miathan6 Exp $

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#include "Ogre.h"
#include "ExampleApplication.h"
#include "ExampleFrameListener.h"
#include "OgrePostFilter/OgrePostFilterManager.h"


// if possible, do not "#define" ;D
#define SAFE_DELETE( yourInstance )		 								\
												delete yourInstance;	\
												yourInstance = NULL;
										 


class SharedData : public Ogre::Singleton<SharedData> {

public:

	SharedData()
		: iRoot(0),
		  iCamera(0),
		  iPfManager(0),
		  iWindow(0)
	{
		iActivate = false;
	}

		~SharedData() {}

		// shared data across the application
		Real iLastFrameTime;
		Root *iRoot;
		Camera *iCamera;
		RenderWindow *iWindow;
		std::vector<OgrePostFilter*> iFilters;
		OgrePostFilterManager *iPfManager;
		bool iActivate;

		// overlay stuff.. too lazy to do a good thing for it
		OgrePostFilter* iActiveFilter;

};
template<> SharedData* Singleton<SharedData>::ms_Singleton = 0;


class BloomPf : public OgrePostFilter {

	public:

		BloomPf()
			: OgrePostFilter( "Bloom" )
		{
		}

		virtual ~BloomPf() {

			delete iPass0;
			delete iPass1;
			delete iPass2;

		}


		// from OgrePostFilter
	public:

		virtual RenderTarget* setupSceneRt() {

			unsigned int width = SharedData::getSingleton().iWindow->getWidth();
			unsigned int height = SharedData::getSingleton().iWindow->getHeight();
			iRtScene = createRenderTexture( width, height );
			
			return iRtScene->getBuffer()->getRenderTarget();

		}

		virtual void setupPasses( std::vector<OgrePostFilterPass*>& filterPasses ) {

			TexturePtr rt0, rt1;
			rt0 = createRenderTexture( 128, 128 );
			rt1 = createRenderTexture( 128, 128 );
			
			iPass0 = new OgrePostFilterPass( "PostFilters/Blur0" );
			iPass0->addInputTexture( iRtScene );
			iPass0->setOutputTexture( rt0 );
			filterPasses.push_back( iPass0 );

			iPass1 = new OgrePostFilterPass( "PostFilters/Blur1" );
			iPass1->addInputTexture( rt0 );
			iPass1->setOutputTexture( rt1 );
			filterPasses.push_back( iPass1 );

			iPass2 = new OgrePostFilterPass( "PostFilters/Bloom" );
			iPass2->addInputTexture( iRtScene );
			iPass2->addInputTexture( rt1 );
			iPass2->setOutputTexture( 0 );
			filterPasses.push_back( iPass2 );

		}


	private:

		OgrePostFilterPass *iPass0, *iPass1, *iPass2;
		TexturePtr iRtScene;
		TexturePtr iRtOut;

};


class MotionBlurPf : public OgrePostFilter {

	public:

		MotionBlurPf()
			: OgrePostFilter( "MotionBlur" )
		{
		}

		virtual ~MotionBlurPf() {

			delete iPass0;
			delete iPass1;
			delete iPass2;

		}


		// from OgrePostFilter
	public:

		virtual RenderTarget* setupSceneRt() {

			unsigned int width = SharedData::getSingleton().iWindow->getWidth();
			unsigned int height = SharedData::getSingleton().iWindow->getHeight();
			iRtScene = createRenderTexture( width, height );
			return iRtScene->getBuffer()->getRenderTarget();

		}

		
		virtual void setupPasses( std::vector<OgrePostFilterPass*>& filterPasses ) {

			unsigned int width = SharedData::getSingleton().iWindow->getWidth();
			unsigned int height = SharedData::getSingleton().iWindow->getHeight();
			TexturePtr sum, temp;
			sum = createRenderTexture( width, height );
			temp = createRenderTexture( height, height );

			// "Sum" texture used before writing on it the first time
			// this lead to "scene memory" every time you turn
			// on/off the effect.. try to improve, i only converted
			// from the ATI MotionBlur.rfx example

			iPass0 = new OgrePostFilterPass( "PostFilters/Combine" );
			iPass0->addInputTexture( iRtScene );	// scene
			iPass0->addInputTexture( sum );		// sum
			iPass0->setOutputTexture( temp );	// temp
			filterPasses.push_back( iPass0 );

			iPass1 = new OgrePostFilterPass( "PostFilters/Copyback" );
			iPass1->addInputTexture( temp );	// temp
			iPass1->setOutputTexture( sum );	// sum
			filterPasses.push_back( iPass1 );

			iPass2 = new OgrePostFilterPass( "PostFilters/MotionBlur" );
			iPass2->addInputTexture( sum );		// sum
			iPass2->setOutputTexture( 0 );
			filterPasses.push_back( iPass2 );

		}


	private:

		OgrePostFilterPass *iPass0, *iPass1, *iPass2;
		TexturePtr iRtScene;

};


class HeatVisionPf : public OgrePostFilter,
					 public OgrePostFilterObserver
{

	public:

		HeatVisionPf()
			: OgrePostFilter( "HeatVision" )
		{

			start = end = curr = inc = 0;

		}

		virtual ~HeatVisionPf() {

			delete iPass0;
			delete iPass1;

		}


		// from OgrePostFilter
	public:

		inline virtual String getName() {

			return "HeatVision";

		}

		virtual RenderTarget* setupSceneRt() {

			unsigned int width = SharedData::getSingleton().iWindow->getWidth();
			unsigned int height = SharedData::getSingleton().iWindow->getHeight();
			iRtScene = createRenderTexture( width, height );

			// Return render target to update for scene
			return iRtScene->getBuffer()->getRenderTarget();

		}

		virtual void setupPasses( std::vector<OgrePostFilterPass*>& filterPasses ) {

			TexturePtr rt0 = createRenderTexture( 256, 256 );

			iPass0 = new OgrePostFilterPass( "Fury/HeatVision/LightToHeat" );
			iPass0->addInputTexture( iRtScene );
			iPass0->setOutputTexture( rt0 );

			iPass1 = new OgrePostFilterPass( "Fury/HeatVision/Blur" );
			iPass1->addInputTexture( rt0 );
			iPass1->setOutputTexture( 0 );

			filterPasses.push_back( iPass0 );
			filterPasses.push_back( iPass1 );

		}


		// from OgrePostFilterObserver
	public:

		virtual void preRenderPostFilterPass( MaterialPtr aMaterialPtr ) {

			if ( aMaterialPtr->getName() != "Fury/HeatVision/LightToHeat" ) return;

			// "random_fractions" parameter
			GpuProgramParametersSharedPtr fpParams =
				aMaterialPtr->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
			fpParams->setNamedConstant("random_fractions", Vector4(Math::RangeRandom(0.0, 1.0), Math::RangeRandom(0, 1.0), 0, 0));

			// "depth_modulator" parameter
			inc = SharedData::getSingletonPtr()->iLastFrameTime / 2;

			if ( (abs(curr-end) <= 0.001) ) {
				// take a new value to reach
				end = Math::RangeRandom(0.95, 1.0);
				start = curr;
			} else {
				if (curr > end) curr -= inc;
				else curr += inc;
			}

			fpParams->setNamedConstant("depth_modulator", Vector4(curr, 0, 0, 0));

		}


	private:

		float inc, start, end, curr;
		TexturePtr iRtScene;
		OgrePostFilterPass *iPass0, *iPass1;

};


class RenderToTextureFrameListener : public ExampleFrameListener
{
protected:
	Camera* mReflectCam;
	SceneNode* mPlaneNode;
	Real timeoutDelay ;
	std::vector<OgrePostFilter*>::iterator pfIterator;

public:
	RenderToTextureFrameListener(RenderWindow* window, Camera* maincam, Camera* reflectCam, 
		SceneNode* planeSceneNode)
		:ExampleFrameListener(window, maincam), 
		mReflectCam(reflectCam), mPlaneNode(planeSceneNode)
	{
		pfIterator = SharedData::getSingleton().iFilters.begin();
		timeoutDelay = 0;
	}

	bool frameStarted(const FrameEvent& evt)
	{
		bool result = ExampleFrameListener::frameStarted(evt);
		SharedData::getSingleton().iLastFrameTime = evt.timeSinceLastFrame;

		// Make sure reflection camera is updated too
		mReflectCam->setOrientation(mCamera->getOrientation());
		mReflectCam->setPosition(mCamera->getPosition());

		// Rotate plane
		mPlaneNode->yaw(Degree(30 * evt.timeSinceLastFrame), Node::TS_PARENT);

		return result;        
	}

	virtual bool processUnbufferedKeyInput(const FrameEvent& evt) {

		bool retval = ExampleFrameListener::processUnbufferedKeyInput(evt);

		// "C" switch filters
		if (mInputDevice->isKeyDown(KC_C) && timeoutDelay==0) {

			timeoutDelay = 0.5f;

			SharedData::getSingleton().iPfManager->setPostFilter( *pfIterator );

			// ugly stuff for overlays only ;)
			SharedData::getSingleton().iActiveFilter = *pfIterator;

			pfIterator++;
			if ( pfIterator == SharedData::getSingleton().iFilters.end() ) {

				pfIterator = SharedData::getSingleton().iFilters.begin();

			}

			updateOverlays();

		}

		// "B" activate/deactivate postfilter rendering
		if (mInputDevice->isKeyDown(KC_B) && timeoutDelay==0) {

			timeoutDelay = 0.5f;

			SharedData::getSingleton().iPfManager->getRenderer()
				->setActive( SharedData::getSingleton().iActivate );
			SharedData::getSingleton().iActivate = !SharedData::getSingleton().iActivate;

			updateOverlays();

		}

		timeoutDelay -= evt.timeSinceLastFrame;
		if (timeoutDelay <= 0) timeoutDelay = 0;

		return retval;
	}

	static void updateOverlays() {

		OverlayManager::getSingleton().getOverlayElement( "Example/Shadows/ShadowTechniqueInfo" )
			->setCaption( "" );

		OverlayManager::getSingleton().getOverlayElement( "Example/Shadows/MaterialInfo" )
			->setCaption( "" );

		OverlayManager::getSingleton().getOverlayElement( "Example/Shadows/ShadowTechnique" )
			->setCaption( "[B] Postfilter renderer active: " + StringConverter::toString( !SharedData::getSingleton().iActivate ) );

		OverlayManager::getSingleton().getOverlayElement( "Example/Shadows/Materials" )
			->setCaption( "[C] Change postfilter, current is \"" + SharedData::getSingleton().iActiveFilter->getName() + "\"");

	}

};


class RenderToTextureApplication : public ExampleApplication, public RenderTargetListener
{
public:
    RenderToTextureApplication() : mPlane(0) {
		new SharedData();
	}
    
	~RenderToTextureApplication()
    {
		delete ( SharedData::getSingletonPtr() );

		SAFE_DELETE( iPfManager );
		SAFE_DELETE( iBloomPf );
		SAFE_DELETE( iHeatVisionPf );
        SAFE_DELETE( mPlane );

	}


protected:

	OgrePostFilterManager* iPfManager;
	BloomPf* iBloomPf;
	HeatVisionPf* iHeatVisionPf;
	MotionBlurPf* iMotionBlurPf;

    MovablePlane* mPlane;
    Entity* mPlaneEnt;
    Camera* mReflectCam;
    SceneNode* mPlaneNode;
    // render target events
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Hide plane 
        mPlaneEnt->setVisible(false);

    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        // Show plane 
        mPlaneEnt->setVisible(true);
    }

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
        // Skybox
        mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox");

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        Vector3 dir(0.5, -1, 0);
        dir.normalise();
        l->setDirection(dir);
        l->setDiffuseColour(1.0f, 1.0f, 0.8f);
        l->setSpecularColour(1.0f, 1.0f, 1.0f);


        // Create a prefab plane
        mPlane = new MovablePlane("ReflectPlane");
        mPlane->d = 0;
        mPlane->normal = Vector3::UNIT_Y;
        MeshManager::getSingleton().createPlane("ReflectionPlane", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
            *mPlane, 2000, 2000, 
            1, 1, true, 1, 1, 1, Vector3::UNIT_Z);
        mPlaneEnt = mSceneMgr->createEntity( "Plane", "ReflectionPlane" );

        // Create an entity from a model (will be loaded automatically)
        Entity* knotEnt = mSceneMgr->createEntity("Knot", "knot.mesh");

        // Create an entity from a model (will be loaded automatically)
        Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");

        knotEnt->setMaterialName("Examples/TextureEffect2");

        // Attach the rtt entity to the root of the scene
        SceneNode* rootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mPlaneNode = rootNode->createChildSceneNode();

        // Attach both the plane entity, and the plane definition
        mPlaneNode->attachObject(mPlaneEnt);
        mPlaneNode->attachObject(mPlane);
        mPlaneNode->translate(0, -10, 0);
        // Tilt it a little to make it interesting
        mPlaneNode->roll(Degree(5));

        rootNode->createChildSceneNode( "Head" )->attachObject( ogreHead );

        RenderTexture* rttTex = mRoot->getRenderSystem()->createRenderTexture( "RttTex", 512, 512, TEX_TYPE_2D, PF_R8G8B8 );
        {
            mReflectCam = mSceneMgr->createCamera("ReflectCam");
            mReflectCam->setNearClipDistance(mCamera->getNearClipDistance());
            mReflectCam->setFarClipDistance(mCamera->getFarClipDistance());
            mReflectCam->setAspectRatio(
                (Real)mWindow->getViewport(0)->getActualWidth() / 
                (Real)mWindow->getViewport(0)->getActualHeight());

            Viewport *v = rttTex->addViewport( mReflectCam );
            v->setClearEveryFrame( true );
            v->setBackgroundColour( ColourValue::Black );

            MaterialPtr mat = MaterialManager::getSingleton().create("RttMat",
                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            TextureUnitState* t = mat->getTechnique(0)->getPass(0)->createTextureUnitState("RustedMetal.jpg");
			t = mat->getTechnique(0)->getPass(0)->createTextureUnitState("RttTex");

            // Blend with base texture
            t->setColourOperationEx(LBX_BLEND_MANUAL, LBS_TEXTURE, LBS_CURRENT, ColourValue::White, 
                ColourValue::White, 0.25);


			t->setTextureAddressingMode( Ogre::TextureUnitState::TextureAddressingMode::TAM_CLAMP );
			t->setProjectiveTexturing(true, mReflectCam);
            rttTex->addListener(this);

            // set up linked reflection
            mReflectCam->enableReflection(mPlane);
            // Also clip
            mReflectCam->enableCustomNearClipPlane(mPlane);
        }

        // Give the plane a texture
        mPlaneEnt->setMaterialName("RttMat");


        // Add a whole bunch of extra transparent entities
        Entity *cloneEnt;
        for (int n = 0; n < 10; ++n)
        {
            // Create a new node under the root
            SceneNode* node = mSceneMgr->createSceneNode();
            // Random translate
            Vector3 nodePos;
            nodePos.x = Math::SymmetricRandom() * 750.0;
            nodePos.y = Math::SymmetricRandom() * 100.0 + 25;
            nodePos.z = Math::SymmetricRandom() * 750.0;
            node->setPosition(nodePos);
            rootNode->addChild(node);
            // Clone knot
            char cloneName[12];
            sprintf(cloneName, "Knot%d", n);
            cloneEnt = knotEnt->clone(cloneName);
            // Attach to new node
            node->attachObject(cloneEnt);

        }

        mCamera->setPosition(-50, 100, 500);
        mCamera->lookAt(0,0,0);


		// show overlay
		Overlay* overlay = OverlayManager::getSingleton().getByName("Example/ShadowsOverlay");    
		overlay->show();


		// ENHANCE THE RENDERTOTEXTURE DEMO WITH
		// SOME EXAMPLE POSTPROCESSING SHADERS

		// safely setup application's (not postfilter!) shared data
		SharedData::getSingleton().iCamera = mCamera;
		SharedData::getSingleton().iRoot = mRoot;
		SharedData::getSingleton().iWindow = mWindow;
		SharedData::getSingleton().iActivate = true;

		iPfManager = new OgrePostFilterManager( mRoot, mWindow, mSceneMgr, rootNode, mCamera );
		SharedData::getSingleton().iPfManager = iPfManager;

		iBloomPf = new BloomPf();
		iHeatVisionPf = new HeatVisionPf();
		iMotionBlurPf = new MotionBlurPf();

		// setup an observer for this postfilter
		// because we wanna interact with its fragment params
		iHeatVisionPf->setObserver( iHeatVisionPf );

		iPfManager->setPostFilter( iHeatVisionPf );
		SharedData::getSingleton().iActiveFilter = iHeatVisionPf;

		SharedData::getSingleton().iFilters.push_back( iBloomPf );
		SharedData::getSingleton().iFilters.push_back( iMotionBlurPf );
		SharedData::getSingleton().iFilters.push_back( iHeatVisionPf );


		// initialize overlays
		RenderToTextureFrameListener::updateOverlays();

	}

    void createFrameListener(void)
    {
        mFrameListener= new RenderToTextureFrameListener(mWindow, mCamera, mReflectCam, mPlaneNode);
        mRoot->addFrameListener(mFrameListener);

    }

};





#ifdef __cplusplus
extern "C" {
#endif

//#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
//	INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
//#else
	int main(int argc, char *argv[])
//#endif
	{
		// Create application object
		RenderToTextureApplication app;

		SET_TERM_HANDLER;

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

#ifdef __cplusplus
}
#endif

