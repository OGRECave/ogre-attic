/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    \file 
        Shadows.cpp
    \brief
        Shows a few ways to use Ogre's shadowing techniques
*/

#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUILogger.h>
#include <CEGUI/CEGUISchemeManager.h>
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/CEGUIWindow.h>
#include <CEGUI/elements/CEGUICombobox.h>
#include <CEGUI/elements/CEGUIComboDropList.h>
#include <CEGUI/elements/CEGUIEditbox.h>
#include <CEGUI/elements/CEGUIListbox.h>
#include <CEGUI/elements/CEGUIListboxTextItem.h>
#include <CEGUI/elements/CEGUIPushButton.h>
#include <CEGUI/elements/CEGUIScrollbar.h>
#include <CEGUI/elements/CEGUIRadioButton.h>
#include "OgreCEGUIRenderer.h"
#include "OgreCEGUIResourceProvider.h"
#include "ExampleApplication.h"

/*
#include "OgreNoMemoryMacros.h"
#include <ode/odecpp.h>
#include <ode/odecpp_collision.h>
#include "OgreMemoryMacros.h"
*/

/*
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include "OgreNoMemoryMacros.h"
#include <crtdbg.h>
#endi*/

Entity* mAthene;
AnimationState* mAnimState = 0;
Entity* pPlaneEnt;
std::vector<Entity*> pColumns;
Light* mLight;
Light* mSunLight;
SceneNode* mLightNode = 0;
AnimationState* mLightAnimationState = 0;
ColourValue mMinLightColour(0.2, 0.1, 0.0);
ColourValue mMaxLightColour(0.5, 0.3, 0.1);
Real mMinFlareSize = 40;
Real mMaxFlareSize = 80;

#define NUM_SHADOW_TECH 8
String mShadowTechDescriptions[NUM_SHADOW_TECH] = 
{
    "Stencil Shadows (Additive)",
    "Stencil Shadows (Modulative)",
	"Texture Shadows (Additive)",
    "Texture Shadows (Modulative)",
	"Texture Shadows (Soft Modulative)",
	"Texture Shadows (Additive + Plane Optimal)",
	"Custom Shadowmapping (Additive + Plane Optimal)",
    "None"
};
enum DemoShadowTech
{
	STENCIL_ADDITIVE = 0,
	STENCIL_MODULATIVE = 1,
	TEXTURE_ADDITIVE = 2,
	TEXTURE_MODULATIVE = 3,
	TEXTURE_SOFT_MODULATIVE = 4,
	TEXTURE_ADDITIVE_PLANE_OPTIMAL = 5,
	CUSTOM_DEPTH_SHADOWMAPPING = 6,
	NONE = 7
};
bool mShadowTechSupported[NUM_SHADOW_TECH];

String SHADOW_COMPOSITOR_NAME("Gaussian Blur");
//String CUSTOM_ROCKWALL_MATERIAL("Ogre/CustomShadows/SimpleRock");	
//String CUSTOM_CASTER_MATERIAL("CustomShadows/ShadowCaster");
//String CUSTOM_RECEIVER_MATERIAL("CustomShadows/ShadowReceiver");

// New depth shadowmapping
String CUSTOM_ROCKWALL_MATERIAL("Ogre/DepthShadowmap/Receiver/RockWall");
String CUSTOM_CASTER_MATERIAL("Ogre/DepthShadowmap/Caster/Float");
String CUSTOM_RECEIVER_MATERIAL("Ogre/DepthShadowmap/Receiver/Float");

String BASIC_ROCKWALL_MATERIAL("Examples/Rockwall");



OverlayElement* mShadowTechniqueInfo;
OverlayElement* mMaterialInfo;
OverlayElement* mInfo;


/** This class 'wibbles' the light and billboard */
class LightWibbler : public ControllerValue<Real>
{
protected:
    Light* mLight;
    Billboard* mBillboard;
    ColourValue mColourRange;
    ColourValue mMinColour;
    Real mMinSize;
    Real mSizeRange;
    Real intensity;
public:
    LightWibbler(Light* light, Billboard* billboard, const ColourValue& minColour, 
        const ColourValue& maxColour, Real minSize, Real maxSize)
    {
        mLight = light;
        mBillboard = billboard;
        mMinColour = minColour;
        mColourRange.r = maxColour.r - minColour.r;
        mColourRange.g = maxColour.g - minColour.g;
        mColourRange.b = maxColour.b - minColour.b;
        mMinSize = minSize;
        mSizeRange = maxSize - minSize;
    }

    virtual Real  getValue (void) const
    {
        return intensity;
    }

    virtual void  setValue (Real value)
    {
        intensity = value;

        ColourValue newColour;

        // Attenuate the brightness of the light
        newColour.r = mMinColour.r + (mColourRange.r * intensity);
        newColour.g = mMinColour.g + (mColourRange.g * intensity);
        newColour.b = mMinColour.b + (mColourRange.b * intensity);

        mLight->setDiffuseColour(newColour);
        mBillboard->setColour(newColour);
        // set billboard size
        Real newSize = mMinSize + (intensity * mSizeRange);
        mBillboard->setDimensions(newSize, newSize);

    }
};

Real timeDelay = 0;
#define KEY_PRESSED(_key,_timeDelay, _macro) \
{ \
    if (mKeyboard->isKeyDown(_key) && timeDelay <= 0) \
{ \
    timeDelay = _timeDelay; \
    _macro ; \
} \
}


//---------------------------------------------------------------------------
class GaussianListener: public Ogre::CompositorInstance::Listener
{
protected:
	int mVpWidth, mVpHeight;
	// Array params - have to pack in groups of 4 since this is how Cg generates them
	// also prevents dependent texture read problems if ops don't require swizzle
	float mBloomTexWeights[15][4];
	float mBloomTexOffsetsHorz[15][4];
	float mBloomTexOffsetsVert[15][4];
public:
	GaussianListener() {}
	virtual ~GaussianListener() {}
	void notifyViewportSize(int width, int height)
	{
		mVpWidth = width;
		mVpHeight = height;
		// Calculate gaussian texture offsets & weights
		float deviation = 3.0f;
		float texelSize = 1.0f / (float)std::min(mVpWidth, mVpHeight);

		// central sample, no offset
		mBloomTexOffsetsHorz[0][0] = 0.0f;
		mBloomTexOffsetsHorz[0][1] = 0.0f;
		mBloomTexOffsetsVert[0][0] = 0.0f;
		mBloomTexOffsetsVert[0][1] = 0.0f;
		mBloomTexWeights[0][0] = mBloomTexWeights[0][1] = 
			mBloomTexWeights[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
		mBloomTexWeights[0][3] = 1.0f;

		// 'pre' samples
		for(int i = 1; i < 8; ++i)
		{
			mBloomTexWeights[i][0] = mBloomTexWeights[i][1] = 
				mBloomTexWeights[i][2] = Ogre::Math::gaussianDistribution(i, 0, deviation);
			mBloomTexWeights[i][3] = 1.0f;
			mBloomTexOffsetsHorz[i][0] = i * texelSize;
			mBloomTexOffsetsHorz[i][1] = 0.0f;
			mBloomTexOffsetsVert[i][0] = 0.0f;
			mBloomTexOffsetsVert[i][1] = i * texelSize;
		}
		// 'post' samples
		for(int i = 8; i < 15; ++i)
		{
			mBloomTexWeights[i][0] = mBloomTexWeights[i][1] = 
				mBloomTexWeights[i][2] = mBloomTexWeights[i - 7][0];
			mBloomTexWeights[i][3] = 1.0f;

			mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 7][0];
			mBloomTexOffsetsHorz[i][1] = 0.0f;
			mBloomTexOffsetsVert[i][0] = 0.0f;
			mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 7][1];
		}

	}
	virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{
		// Prepare the fragment params offsets
		switch(pass_id)
		{
		case 701: // blur horz
			{
				// horizontal bloom
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams = 
					mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				// A bit hacky - Cg & HLSL index arrays via [0], GLSL does not
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

				break;
			}
		case 700: // blur vert
			{
				// vertical bloom 
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams = 
					mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

				break;
			}
		}

	}
	virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{

	}
};
GaussianListener gaussianListener;


class ShadowsListener : public ExampleFrameListener, public OIS::MouseListener, 
	public OIS::KeyListener
{
protected:
    SceneManager* mSceneMgr;
	Viewport *mShadowVp;
	CompositorInstance* mShadowCompositor;
	bool mShutdownRequested;
	CEGUI::Window* mRootGuiPanel;
	bool mLMBDown;
	bool mRMBDown;
	bool mProcessMovement;
	bool mUpdateMovement;
	bool mMoveFwd;
	bool mMoveBck;
	bool mMoveLeft;
	bool mMoveRight;
	CEGUI::Point mLastMousePosition;
	bool mLastMousePositionSet;
	float mAvgFrameTime;
	Camera* mTexCam;


	//----------------------------------------------------------------//
	CEGUI::MouseButton convertOISMouseButtonToCegui(int buttonID)
	{
		switch (buttonID)
		{
		case 0: return CEGUI::LeftButton;
		case 1: return CEGUI::RightButton;
		case 2:	return CEGUI::MiddleButton;
		case 3: return CEGUI::X1Button;
		default: return CEGUI::LeftButton;
		}
	}
public:
	ShadowsListener(RenderWindow* win, Camera* cam, SceneManager* sm)
        : ExampleFrameListener(win, cam, true, true)
		, mSceneMgr(sm)
		, mShadowVp(0)
		, mShadowCompositor(0)
		, mShutdownRequested(false)
		, mLMBDown(false)
		, mRMBDown(false)
		, mProcessMovement(false)
		, mUpdateMovement(false)
		, mMoveFwd(false)
		, mMoveBck(false)
		, mMoveLeft(false)
		, mMoveRight(false)
		, mLastMousePositionSet(false)
		, mAvgFrameTime(0.1f)

    {
		mMouse->setEventCallback(this);
		mKeyboard->setEventCallback(this);

		mRootGuiPanel = CEGUI::WindowManager::getSingleton().getWindow("Shadows");

		mMoveSpeed = 10.0f;

		// Set up a debug panel to display the shadow
		MaterialPtr debugMat = MaterialManager::getSingleton().create(
			"Ogre/DebugShadowMap0", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		TexturePtr shadowTex = mSceneMgr->getShadowTexture(0);
		TextureUnitState *t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState(shadowTex->getName());
		t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);

		debugMat = MaterialManager::getSingleton().create(
			"Ogre/DebugShadowMap1", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		shadowTex = mSceneMgr->getShadowTexture(1);
		t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState(shadowTex->getName());
		t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);

		OverlayContainer* debugPanel = (OverlayContainer*)
			(OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugShadowPanel0"));
		debugPanel->_setPosition(0.8, 0);
		debugPanel->_setDimensions(0.2, 0.2);
		debugPanel->setMaterialName("Ogre/DebugShadowMap0");
		Overlay* debugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
		debugOverlay->add2D(debugPanel);
		
		debugPanel = (OverlayContainer*)
			(OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugShadowPanel1"));
		debugPanel->_setPosition(0.8, 0.2);
		debugPanel->_setDimensions(0.2, 0.2);
		debugPanel->setMaterialName("Ogre/DebugShadowMap1");
		debugOverlay->add2D(debugPanel);
		

    }

	void configureShadowTechnique(int preTechnique, int currentTechnique)
	{
		switch(currentTechnique)
		{
		case STENCIL_ADDITIVE:
			mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
			break;
		case STENCIL_MODULATIVE:
			mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_MODULATIVE);
			break;
		case TEXTURE_ADDITIVE:
		case TEXTURE_ADDITIVE_PLANE_OPTIMAL:
		case CUSTOM_DEPTH_SHADOWMAPPING:
			mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
			break;
		case TEXTURE_MODULATIVE:
		case TEXTURE_SOFT_MODULATIVE:
			mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
			break;
		case NONE:
			mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);
			break;

		}

	}

	void configureLights(int preTechnique, int currentTechnique)
	{
		Vector3 dir;
		switch (currentTechnique)
		{
		case STENCIL_ADDITIVE:
			// Fixed light, dim
			mSunLight->setCastShadows(true);

			// Point light, movable, reddish
			mLight->setType(Light::LT_POINT);
			mLight->setCastShadows(true);
			mLight->setDiffuseColour(mMinLightColour);
			mLight->setSpecularColour(1, 1, 1);
			mLight->setAttenuation(8000,1,0.0005,0);

			break;
		case STENCIL_MODULATIVE:
			// Multiple lights cause obvious silhouette edges in modulative mode
			// So turn off shadows on the direct light
			// Fixed light, dim
			mSunLight->setCastShadows(false);

			// Point light, movable, reddish
			mLight->setType(Light::LT_POINT);
			mLight->setCastShadows(true);
			mLight->setDiffuseColour(mMinLightColour);
			mLight->setSpecularColour(1, 1, 1);
			mLight->setAttenuation(8000,1,0.0005,0);
			break;
		case TEXTURE_SOFT_MODULATIVE:
		case TEXTURE_MODULATIVE:
		case TEXTURE_ADDITIVE:
			// Fixed light, dim
			mSunLight->setCastShadows(currentTechnique != TEXTURE_SOFT_MODULATIVE);

			// Change moving light to spotlight
			// Point light, movable, reddish
			mLight->setType(Light::LT_SPOTLIGHT);
			mLight->setDirection(Vector3::NEGATIVE_UNIT_Z);
			mLight->setCastShadows(true);
			mLight->setDiffuseColour(mMinLightColour);
			mLight->setSpecularColour(1, 1, 1);
			mLight->setAttenuation(8000,1,0.0005,0);
			mLight->setSpotlightRange(Degree(80),Degree(90));


			break;
		default:
			break;
		};

	}
	void cleanupCompositors(int preTechnique, int currentTechnique)
	{
		if (preTechnique == TEXTURE_SOFT_MODULATIVE && 
			preTechnique != currentTechnique)
		{
			// Clean up compositors
			mShadowCompositor->removeListener(&gaussianListener);
			CompositorManager::getSingleton().setCompositorEnabled(mShadowVp, 
				SHADOW_COMPOSITOR_NAME, false);
			// Remove entire compositor chain
			CompositorManager::getSingleton().removeCompositorChain(mShadowVp);
			mShadowVp = 0;
			mShadowCompositor = 0;
		}


	}
	void configureCompositors(int preTechnique, int currentTechnique)
	{
		RenderTarget* shadowRtt;
		TexturePtr shadowTex;
		switch(currentTechnique)
		{
		case TEXTURE_SOFT_MODULATIVE:
			// set up compositors
			shadowTex = mSceneMgr->getShadowTexture(0);
			shadowRtt = shadowTex->getBuffer()->getRenderTarget();
			mShadowVp = shadowRtt->getViewport(0);
			mShadowCompositor = 
				CompositorManager::getSingleton().addCompositor(mShadowVp, SHADOW_COMPOSITOR_NAME);
			CompositorManager::getSingleton().setCompositorEnabled(
				mShadowVp, SHADOW_COMPOSITOR_NAME, true);
			mShadowCompositor->addListener(&gaussianListener);
			gaussianListener.notifyViewportSize(mShadowVp->getActualWidth(), mShadowVp->getActualHeight());

			break;
		default:
			break;

		};

	}
	void configureTextures(int preTechnique, int currentTechnique)
	{
		switch(currentTechnique)
		{
		case CUSTOM_DEPTH_SHADOWMAPPING:
			mSceneMgr->setShadowTexturePixelFormat(PF_FLOAT32_R);
			break;
		default:
			mSceneMgr->setShadowTexturePixelFormat(PF_X8R8G8B8);
		}

	}
	void configureShadowCasterReceiverMaterials(int preTechnique, int currentTechnique)
	{
		switch(currentTechnique)
		{
		case CUSTOM_DEPTH_SHADOWMAPPING:
			mSceneMgr->setShadowTextureCasterMaterial(CUSTOM_CASTER_MATERIAL);
			mSceneMgr->setShadowTextureReceiverMaterial(CUSTOM_RECEIVER_MATERIAL);
			mSceneMgr->setShadowTextureSelfShadow(true);	// NOTE: need to perform depth biasing for this to work
			// Sort out base materials
			pPlaneEnt->setMaterialName(CUSTOM_ROCKWALL_MATERIAL);
			for (std::vector<Entity*>::iterator i = pColumns.begin();
				i != pColumns.end(); ++i)
			{
				(*i)->setMaterialName(CUSTOM_ROCKWALL_MATERIAL);
			}
			break;

		default:
			mSceneMgr->setShadowTextureCasterMaterial(StringUtil::BLANK);
			mSceneMgr->setShadowTextureReceiverMaterial(StringUtil::BLANK);
			mSceneMgr->setShadowTextureSelfShadow(false);	
			// Sort out base materials for additive modes
			pPlaneEnt->setMaterialName(BASIC_ROCKWALL_MATERIAL);
			for (std::vector<Entity*>::iterator i = pColumns.begin();
				i != pColumns.end(); ++i)
			{
				(*i)->setMaterialName(BASIC_ROCKWALL_MATERIAL);
			}

		}

	}

	/*
	void configureShadowCameras(int preTechnique, int currentTechnique)
	{
		switch(currentTechnique)
		{
		case TEXTURE_ADDITIVE_PLANE_OPTIMAL:
			// Create custom camera setup class
			{
				ShadowCameraSetupPtr planeOptPtr1(new PlaneOptimalShadowCameraSetup(mPlane));
				mSunLight->setCustomShadowCameraSetup(planeOptPtr1);
				ShadowCameraSetupPtr planeOptPtr2(new PlaneOptimalShadowCameraSetup(mPlane));
				mLight->setCustomShadowCameraSetup(planeOptPtr2);
			}
			break;
		default:
			// Default shadow camera setup
			mSunLight->resetCustomShadowCameraSetup();
			mLight->resetCustomShadowCameraSetup();
			break;

		}

	}
	*/

	/*
    void changeShadowTechnique()
    {
		int prevTech = mCurrentShadowTechnique;
        mCurrentShadowTechnique = ++mCurrentShadowTechnique % NUM_SHADOW_TECH;
		if (!mShadowTechSupported[mCurrentShadowTechnique])
		{
			// Skip unsupported
			mCurrentShadowTechnique = ++mCurrentShadowTechnique % NUM_SHADOW_TECH;
		}
        mShadowTechniqueInfo->setCaption("Current: " + mShadowTechDescriptions[mCurrentShadowTechnique]);

		cleanupCompositors(prevTech, mCurrentShadowTechnique);

		configureShadowTechnique(prevTech, mCurrentShadowTechnique);

		if (mCurrentShadowTechnique != NONE)
		{
			configureShadowCameras(prevTech, mCurrentShadowTechnique);
			configureLights(prevTech, mCurrentShadowTechnique);
			configureCompositors(prevTech, mCurrentShadowTechnique);
			configureTextures(prevTech, mCurrentShadowTechnique);
			configureShadowCasterReceiverMaterials(prevTech, mCurrentShadowTechnique);
		}

    }
	*/

	bool frameStarted(const FrameEvent& evt)
	{
		CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);

		mMouse->capture();
		mKeyboard->capture();

		if (mShutdownRequested)
			return false;
		else
		{
			// update movement process
			if(mProcessMovement || mUpdateMovement)
			{
				mTranslateVector.x += mMoveLeft ? mAvgFrameTime * -mMoveSpeed : 0;
				mTranslateVector.x += mMoveRight ? mAvgFrameTime * mMoveSpeed : 0;
				mTranslateVector.z += mMoveFwd ? mAvgFrameTime * -mMoveSpeed : 0;
				mTranslateVector.z += mMoveBck ? mAvgFrameTime * mMoveSpeed : 0;

				mCamera->yaw(mRotX);
				mCamera->pitch(mRotY);
				mCamera->moveRelative(mTranslateVector);

				mUpdateMovement = false;
				mRotX = 0;
				mRotY = 0;
				mTranslateVector = Ogre::Vector3::ZERO;
			}

		}

		return true;
	}

    bool frameEnded(const FrameEvent& evt)
    {
		if (mAnimState)
			mAnimState->addTime(evt.timeSinceLastFrame);

		if (mShutdownRequested)
			return false;
		else
			return ExampleFrameListener::frameEnded(evt);

    }
	//----------------------------------------------------------------//
	bool mouseMoved( const OIS::MouseEvent &arg )
	{
		CEGUI::System::getSingleton().injectMouseMove( arg.state.X.rel, arg.state.Y.rel );
		return true;
	}

	//----------------------------------------------------------------//
	bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
	{
		CEGUI::System::getSingleton().injectMouseButtonDown(convertOISMouseButtonToCegui(id));
		return true;
	}

	//----------------------------------------------------------------//
	bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
	{
		CEGUI::System::getSingleton().injectMouseButtonUp(convertOISMouseButtonToCegui(id));
		return true;
	}

	//----------------------------------------------------------------//
	bool keyPressed( const OIS::KeyEvent &arg )
	{
		if( arg.key == OIS::KC_ESCAPE )
			mShutdownRequested = true;

		if (arg.key == OIS::KC_SYSRQ)
			mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");

		CEGUI::System::getSingleton().injectKeyDown( arg.key );
		CEGUI::System::getSingleton().injectChar( arg.text );
		return true;
	}

	//----------------------------------------------------------------//
	bool keyReleased( const OIS::KeyEvent &arg )
	{
		CEGUI::System::getSingleton().injectKeyUp( arg.key );
		return true;
	}
	//--------------------------------------------------------------------------
	bool handleMouseMove(const CEGUI::EventArgs& e)
	{
		const CEGUI::MouseEventArgs& me = static_cast<const CEGUI::MouseEventArgs&>(e);

		if( mLMBDown && !mRMBDown)
		{
			// rotate camera
			mRotX += Degree(-me.moveDelta.d_x * mAvgFrameTime * 3.0);
			mRotY += Degree(-me.moveDelta.d_y * mAvgFrameTime * 3.0);
			CEGUI::MouseCursor::getSingleton().setPosition( mLastMousePosition );
			mUpdateMovement = true;
		}
		else
		{
			if( mRMBDown && !mLMBDown)
			{
				// translate camera
				mTranslateVector.x += me.moveDelta.d_x * mAvgFrameTime * mMoveSpeed;
				mTranslateVector.y += -me.moveDelta.d_y * mAvgFrameTime * mMoveSpeed;
				//mTranslateVector.z = 0;
				CEGUI::MouseCursor::getSingleton().setPosition( mLastMousePosition );
				mUpdateMovement = true;
			}
			else
			{
				if( mRMBDown && mLMBDown)
				{
					mTranslateVector.z += (me.moveDelta.d_x + me.moveDelta.d_y) * mAvgFrameTime * mMoveSpeed;
					CEGUI::MouseCursor::getSingleton().setPosition( mLastMousePosition );
					mUpdateMovement = true;
				}

			}
		}

		return true;
	}

	//--------------------------------------------------------------------------
	bool handleMouseButtonUp(const CEGUI::EventArgs& e)
	{
		const CEGUI::MouseEventArgs& me = static_cast<const CEGUI::MouseEventArgs&>(e);

		//Window* wndw = ((const WindowEventArgs&)e).window;
		if( me.button == CEGUI::LeftButton )
		{
			mLMBDown = false;
		}

		if( me.button == CEGUI::RightButton )
		{
			mRMBDown = false;
		}
		if( !mLMBDown && !mRMBDown )
		{
			CEGUI::MouseCursor::getSingleton().show();
			if(mLastMousePositionSet)
			{
				CEGUI::MouseCursor::getSingleton().setPosition( mLastMousePosition );
				mLastMousePositionSet = false;
			}
			mRootGuiPanel->releaseInput();
		}

		return true;
	}

	//--------------------------------------------------------------------------
	bool handleMouseButtonDown(const CEGUI::EventArgs& e)
	{
		const CEGUI::MouseEventArgs& me = static_cast<const CEGUI::MouseEventArgs&>(e);

		//Window* wndw = ((const WindowEventArgs&)e).window;
		if( me.button == CEGUI::LeftButton )
		{
			mLMBDown = true;
		}

		if( me.button == CEGUI::RightButton )
		{
			mRMBDown = true;
		}

		if( mLMBDown || mRMBDown )
		{
			CEGUI::MouseCursor::getSingleton().hide();
			if (!mLastMousePositionSet)
			{
				mLastMousePosition = CEGUI::MouseCursor::getSingleton().getPosition();
				mLastMousePositionSet = true;
			}
			mRootGuiPanel->captureInput();
		}

		return true;
	}

	//--------------------------------------------------------------------------
	bool handleMouseWheelEvent(const CEGUI::EventArgs& e)
	{
		const CEGUI::MouseEventArgs& me = static_cast<const CEGUI::MouseEventArgs&>(e);
		mTranslateVector.z += me.wheelChange * -5.0;
		mUpdateMovement = true;

		return true;
	}

	//--------------------------------------------------------------------------
	void checkMovementKeys(CEGUI::Key::Scan scancode, bool state )
	{

		switch ( scancode )
		{
		case CEGUI::Key::A:
			mMoveLeft = state;
			break;

		case CEGUI::Key::D:
			mMoveRight = state;
			break;

		case CEGUI::Key::S:
			mMoveBck = state;
			break;

		case CEGUI::Key::W:
			mMoveFwd = state;
			break;

		default:
			break;

		}

		mProcessMovement = mMoveLeft || mMoveRight || mMoveFwd || mMoveBck;

	}
	//--------------------------------------------------------------------------
	bool handleKeyDownEvent(const CEGUI::EventArgs& e)
	{
		const CEGUI::KeyEventArgs& ke = static_cast<const CEGUI::KeyEventArgs&>(e);

		checkMovementKeys(ke.scancode , true);

		return true;
	}

	//--------------------------------------------------------------------------
	bool handleKeyUpEvent(const CEGUI::EventArgs& e)
	{
		const CEGUI::KeyEventArgs& ke = static_cast<const CEGUI::KeyEventArgs&>(e);
		checkMovementKeys(ke.scancode, false );

		return true;
	}

    


};

class ShadowsApplication : public ExampleApplication
{
protected:
	enum ShadowProjection
	{
		UNIFORM,
		UNIFORM_FOCUSED,
		LISPSM,
		PLANE_OPTIMAL
	};

	enum ShadowMaterial
	{
		MAT_STANDARD,
		MAT_DEPTH_FLOAT,
		MAT_DEPTH_FLOAT_PCF
	};
	CEGUI::OgreCEGUIRenderer* mGUIRenderer;
	CEGUI::System* mGUISystem;
	CEGUI::Window* mDescWindow;
	CEGUI::Editbox* mFixedBias;
	CEGUI::Editbox* mGradientBias;
	CEGUI::Editbox* mGradientClamp;

	ShadowTechnique mCurrentShadowTechnique;
	ShadowProjection mCurrentProjection;
	ShadowMaterial mCurrentMaterial;

	GpuProgramParametersSharedPtr mCustomRockwallVparams;
	GpuProgramParametersSharedPtr mCustomRockwallFparams;

	ShadowCameraSetupPtr mCurrentShadowCameraSetup;
	/// Plane that defines plane-optimal shadow mapping basis
	MovablePlane* mPlane;
	// transient pointer to LiSPSM setup if present
	LiSPSMShadowCameraSetup* mLiSPSMSetup;

public:
	ShadowsApplication() : 
	  mGUIRenderer(0),
	  mGUISystem(0),
	  mPlane(0)
	{


    }

    ~ShadowsApplication() 
    {
		mDescWindow = 0;

		if(mGUISystem)
		{
			delete mGUISystem;
			mGUISystem = 0;
		}
		if(mGUIRenderer)
		{
			delete mGUIRenderer;
			mGUIRenderer = 0;
		}

		delete mPlane;
    }
protected:

	// Override this to ensure FPU mode
	bool configure(void)
	{
		// Show the configuration dialog and initialise the system
		// You can skip this and use root.restoreConfig() to load configuration
		// settings if you were sure there are valid ones saved in ogre.cfg
		if(mRoot->showConfigDialog())
		{
			// Custom option - to use PlaneOptimalShadowCameraSetup we must have
			// double-precision. Thus, set the D3D floating point mode if present, 
			// no matter what was chosen
			ConfigOptionMap& optMap = mRoot->getRenderSystem()->getConfigOptions();
			ConfigOptionMap::iterator i = optMap.find("Floating-point mode");
			if (i != optMap.end())
			{
				if (i->second.currentValue != "Consistent")
				{
					i->second.currentValue = "Consistent";
					LogManager::getSingleton().logMessage("Demo_Shadows: overriding "
						"D3D floating point mode to 'Consistent' to ensure precision "
						"for plane-optimal camera setup option");

				}
			}

			// If returned true, user clicked OK so initialise
			// Here we choose to let the system create a default rendering window by passing 'true'
			mWindow = mRoot->initialise(true);
			return true;
		}
		else
		{
			return false;
		}
	}

	void generalSceneSetup()
    {
        // do this first so we generate edge lists
        mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
		mCurrentShadowTechnique = SHADOWTYPE_STENCIL_ADDITIVE;

		// default to all techs supported
		for (int i = 0; i < NUM_SHADOW_TECH; ++i)
			mShadowTechSupported[i] = true;

        // Set the default Athene material
        // We'll default it to the normal map for ps_2_0 capable hardware
        // everyone else will default to the basic
        if (!GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0") &&
            !GpuProgramManager::getSingleton().isSyntaxSupported("arbfp1"))
        {
			// no SM2
			mShadowTechSupported[CUSTOM_DEPTH_SHADOWMAPPING] = false;
			mShadowTechSupported[TEXTURE_SOFT_MODULATIVE] = false;
        }

		if (!mRoot->getRenderSystem()->getCapabilities()->hasCapability(RSC_TEXTURE_FLOAT))
		{
			mShadowTechSupported[CUSTOM_DEPTH_SHADOWMAPPING] = false;
		}

        // Set ambient light off
        mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));

        // Fixed light, dim
        mSunLight = mSceneMgr->createLight("SunLight");
        mSunLight->setType(Light::LT_SPOTLIGHT);
        mSunLight->setPosition(1500,1750,1300);
        mSunLight->setSpotlightRange(Degree(30), Degree(50));
        Vector3 dir;
        dir = -mSunLight->getPosition();
        dir.normalise();
        mSunLight->setDirection(dir);
        mSunLight->setDiffuseColour(0.35, 0.35, 0.38);
        mSunLight->setSpecularColour(0.9, 0.9, 1);

        // Point light, movable, reddish
        mLight = mSceneMgr->createLight("Light2");
        mLight->setDiffuseColour(mMinLightColour);
        mLight->setSpecularColour(1, 1, 1);
        mLight->setAttenuation(8000,1,0.0005,0);

        // Create light node
        mLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(
            "MovingLightNode");
        mLightNode->attachObject(mLight);
        // create billboard set
        BillboardSet* bbs = mSceneMgr->createBillboardSet("lightbbs", 1);
        bbs->setMaterialName("Examples/Flare");
        Billboard* bb = bbs->createBillboard(0,0,0,mMinLightColour);
        // attach
        mLightNode->attachObject(bbs);

        // create controller, after this is will get updated on its own
        ControllerFunctionRealPtr func = ControllerFunctionRealPtr(
            new WaveformControllerFunction(Ogre::WFT_SINE, 0.75, 0.5));
        ControllerManager& contMgr = ControllerManager::getSingleton();
        ControllerValueRealPtr val = ControllerValueRealPtr(
            new LightWibbler(mLight, bb, mMinLightColour, mMaxLightColour, 
            mMinFlareSize, mMaxFlareSize));
        Controller<Real>* controller = contMgr.createController(
            contMgr.getFrameTimeSource(), val, func);

        //mLight->setPosition(Vector3(300,250,-300));
        mLightNode->setPosition(Vector3(300,1750,-700));


        // Create a track for the light
        Animation* anim = mSceneMgr->createAnimation("LightTrack", 20);
        // Spline it for nice curves
        anim->setInterpolationMode(Animation::IM_SPLINE);
        // Create a track to animate the camera's node
        NodeAnimationTrack* track = anim->createNodeTrack(0, mLightNode);
        // Setup keyframes
        TransformKeyFrame* key = track->createNodeKeyFrame(0); // A startposition
        key->setTranslate(Vector3(300,750,-700));
        key = track->createNodeKeyFrame(2);//B
        key->setTranslate(Vector3(150,800,-250));
        key = track->createNodeKeyFrame(4);//C
        key->setTranslate(Vector3(-150,850,-100));
        key = track->createNodeKeyFrame(6);//D
        key->setTranslate(Vector3(-400,700,-200));
        key = track->createNodeKeyFrame(8);//E
        key->setTranslate(Vector3(-200,700,-400));
        key = track->createNodeKeyFrame(10);//F
        key->setTranslate(Vector3(-100,850,-200));
        key = track->createNodeKeyFrame(12);//G
        key->setTranslate(Vector3(-100,575,180));
        key = track->createNodeKeyFrame(14);//H
        key->setTranslate(Vector3(0,750,300));
        key = track->createNodeKeyFrame(16);//I
        key->setTranslate(Vector3(100,850,100));
        key = track->createNodeKeyFrame(18);//J
        key->setTranslate(Vector3(250,800,0));
        key = track->createNodeKeyFrame(20);//K == A
        key->setTranslate(Vector3(300,750,-700));
        // Create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("LightTrack");
        mAnimState->setEnabled(true);
        // Make light node look at origin, this is for when we
        // change the moving light to a spotlight
        mLightNode->setAutoTracking(true, mSceneMgr->getRootSceneNode());

        // Prepare athene mesh for normalmapping
        MeshPtr pAthene = MeshManager::getSingleton().load("athene.mesh", 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        unsigned short src, dest;
        if (!pAthene->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
        {
            pAthene->buildTangentVectors(VES_TANGENT, src, dest);
        }

        SceneNode* node;
        node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mAthene = mSceneMgr->createEntity( "athene", "athene.mesh" );
        mAthene->setMaterialName("Examples/Athene/NormalMapped");
        node->attachObject( mAthene );
        node->translate(0,-27, 0);
        node->yaw(Degree(90));

        Entity* pEnt;
		// Columns
		for (int x = -2; x <= 2; ++x)
		{
			for (int z = -2; z <= 2; ++z)
			{
				if (x != 0 || z != 0)
				{
					StringUtil::StrStreamType str;
					str << "col" << x << "_" << z;
					node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					pEnt = mSceneMgr->createEntity( str.str(), "column.mesh" );
					pEnt->setMaterialName(BASIC_ROCKWALL_MATERIAL);
					pColumns.push_back(pEnt);
					node->attachObject( pEnt );
					node->translate(x*300,0, z*300);

				}
			}

		}


        // Skybox
        mSceneMgr->setSkyBox(true, "Examples/StormySkyBox");

        // Floor plane (use POSM plane def)
		mPlane = new MovablePlane("*mPlane");
        mPlane->normal = Vector3::UNIT_Y;
        mPlane->d = 107;
        MeshManager::getSingleton().createPlane("Myplane",
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, *mPlane,
            1500,1500,50,50,true,1,5,5,Vector3::UNIT_Z);
        pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName(BASIC_ROCKWALL_MATERIAL);
        pPlaneEnt->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

		if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(RSC_HWRENDER_TO_TEXTURE))
        {
            // In D3D, use a 1024x1024 shadow texture
            mSceneMgr->setShadowTextureSettings(1024, 2);
        }
        else
        {
            // Use 512x512 texture in GL since we can't go higher than the window res
            mSceneMgr->setShadowTextureSettings(512, 2);
        }
        mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));

        // incase infinite far distance is not supported
        mCamera->setFarClipDistance(100000);

        //mSceneMgr->setShowDebugShadows(true);

		const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
		if (!caps->hasCapability(RSC_VERTEX_PROGRAM) || !(caps->hasCapability(RSC_FRAGMENT_PROGRAM)))
		{
			mShadowTechSupported[TEXTURE_SOFT_MODULATIVE] = false;
			mShadowTechSupported[CUSTOM_DEPTH_SHADOWMAPPING] = false;
		}
		else
		{
			if (!GpuProgramManager::getSingleton().isSyntaxSupported("glsl") &&
				!GpuProgramManager::getSingleton().isSyntaxSupported("ps_2_0") 
				)
			{
				mShadowTechSupported[TEXTURE_SOFT_MODULATIVE] = false;
				mShadowTechSupported[CUSTOM_DEPTH_SHADOWMAPPING] = false;
			}
		}

		mCamera->setPosition(250, 20, 400);
		mCamera->lookAt(0, 10, 0);

    }
	
    // Just override the mandatory create scene method
    void createScene(void)
    {
        // set up general scene (this defaults to additive stencils)
        generalSceneSetup();

		setupGUI();

		// Uncomment the below and LiSPSM loses warping factor????!?!?

		//changeShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
		//mSceneMgr->setShadowCameraSetup(ShadowCameraSetupPtr(new LiSPSMShadowCameraSetup()));

    }

	/// Change basic shadow technique 
	void changeShadowTechnique(ShadowTechnique newTech)
	{
		mSceneMgr->setShadowTechnique(newTech);

		// Below is for projection
		//configureShadowCameras(mCurrentShadowTechnique, newTech);

		configureLights(newTech);

		// Advanced modes - materials / compositors
		//configureCompositors(mCurrentShadowTechnique, newTech);
		//configureTextures(mCurrentShadowTechnique, newTech);
		//configureShadowCasterReceiverMaterials(mCurrentShadowTechnique, newTech);

		updateGUI(newTech);

		mCurrentShadowTechnique = newTech;


	}



	void configureLights(ShadowTechnique newTech)
	{
		Vector3 dir;
		switch (newTech)
		{
		case SHADOWTYPE_STENCIL_ADDITIVE:
			// Fixed light, dim
			mSunLight->setCastShadows(true);

			// Point light, movable, reddish
			mLight->setType(Light::LT_POINT);
			mLight->setCastShadows(true);
			mLight->setDiffuseColour(mMinLightColour);
			mLight->setSpecularColour(1, 1, 1);
			mLight->setAttenuation(8000,1,0.0005,0);

			break;
		case SHADOWTYPE_STENCIL_MODULATIVE:
			// Multiple lights cause obvious silhouette edges in modulative mode
			// So turn off shadows on the direct light
			// Fixed light, dim
			mSunLight->setCastShadows(false);

			// Point light, movable, reddish
			mLight->setType(Light::LT_POINT);
			mLight->setCastShadows(true);
			mLight->setDiffuseColour(mMinLightColour);
			mLight->setSpecularColour(1, 1, 1);
			mLight->setAttenuation(8000,1,0.0005,0);
			break;
		case SHADOWTYPE_TEXTURE_MODULATIVE:
		case SHADOWTYPE_TEXTURE_ADDITIVE:
			// Fixed light, dim
			mSunLight->setCastShadows(true);

			// Change moving light to spotlight
			// Point light, movable, reddish
			mLight->setType(Light::LT_SPOTLIGHT);
			mLight->setDirection(Vector3::NEGATIVE_UNIT_Z);
			mLight->setCastShadows(true);
			mLight->setDiffuseColour(mMinLightColour);
			mLight->setSpecularColour(1, 1, 1);
			mLight->setAttenuation(8000,1,0.0005,0);
			mLight->setSpotlightRange(Degree(80),Degree(90));


			break;
		default:
			break;
		};

	}

	void setupGUI()
	{
		// setup GUI system
		mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, 
			Ogre::RENDER_QUEUE_OVERLAY, false, 3000, mSceneMgr);

		mGUISystem = new CEGUI::System(mGUIRenderer);

		CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Informative);

		// load scheme and set up defaults
		CEGUI::SchemeManager::getSingleton().loadScheme(
			(CEGUI::utf8*)"TaharezLookSkin.scheme");
		mGUISystem->setDefaultMouseCursor(
			(CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
		mGUISystem->setDefaultFont((CEGUI::utf8*)"BlueHighway-12");

		CEGUI::Window* sheet = 
			CEGUI::WindowManager::getSingleton().loadWindowLayout(
			(CEGUI::utf8*)"shadows.layout"); 
		mGUISystem->setGUISheet(sheet);

		// Tooltips aren't big enough, do our own
		//mGUISystem->setDefaultTooltip("TaharezLook/Tooltip");

		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		// Get description window
		mDescWindow = wmgr.getWindow("Shadows/Desc");

		CEGUI::Window *wnd = wmgr.getWindow("Shadows/Stencil");
		wnd->subscribeEvent(CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseEnter, this));
		wnd->subscribeEvent(CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseLeave, this));
		wnd = wmgr.getWindow("Shadows/Texture");
		wnd->subscribeEvent(CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseEnter, this));
		wnd->subscribeEvent(CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseLeave, this));
		wnd = wmgr.getWindow("Shadows/Additive");
		wnd->subscribeEvent(CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseEnter, this));
		wnd->subscribeEvent(CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseLeave, this));
		wnd = wmgr.getWindow("Shadows/Modulative");
		wnd->subscribeEvent(CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseEnter, this));
		wnd->subscribeEvent(CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseLeave, this));
		
		// Combo doesn't raise enter / exit itself, have to grab subcomponents?
		CEGUI::Combobox* cbo = static_cast<CEGUI::Combobox*>(wmgr.getWindow("Shadows/Projection"));
		cbo->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
			CEGUI::Event::Subscriber(&ShadowsApplication::handleProjectionChanged, this));
		cbo->getEditbox()->subscribeEvent(CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseEnterCombo, this));
		cbo->getEditbox()->subscribeEvent(CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseLeave, this));
		cbo->getDropList()->subscribeEvent(CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseEnterCombo, this));
		cbo->getDropList()->subscribeEvent(CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseLeave, this));
		cbo->getPushButton()->subscribeEvent(CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseEnterCombo, this));
		cbo->getPushButton()->subscribeEvent(CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseLeave, this));
		// Populate projection
		// Get a select image to be used for highlighting items in listbox when mouse moves over them
		const CEGUI::Image* selectImage = &CEGUI::ImagesetManager::getSingleton().getImageset("TaharezLook")->getImage("MultiListSelectionBrush");
		CEGUI::ListboxTextItem* li = new CEGUI::ListboxTextItem("Uniform", UNIFORM);
        li->setSelectionBrushImage(selectImage);
		li->setTooltipText("Uniform: Shadows are rendered and projected using a uniform "
			"frustum for the whole light coverage. Simple and lowest quality.");
		cbo->addItem(li);
		cbo->setItemSelectState(li, true);
		cbo->setText("Uniform");
		mCurrentProjection = UNIFORM;
		li = new CEGUI::ListboxTextItem("Uniform Focused", UNIFORM_FOCUSED);
        li->setSelectionBrushImage(selectImage);
		li->setTooltipText("Uniform Focused: As Uniform except that the frustum is "
			"focused on the visible area of the camera. Better quality than Uniform "
			"at the expense of some 'swimming'.");
		cbo->addItem(li);
		li = new CEGUI::ListboxTextItem("LiSPSM", LISPSM);
        li->setSelectionBrushImage(selectImage);
		li->setTooltipText("LiSPSM: The frustum is distorted to take into account "
			"the perspective of the camera, and focused on the visible area. "
			"Good quality & flexibility.");
		cbo->addItem(li);
		li = new CEGUI::ListboxTextItem("Plane Optimal", PLANE_OPTIMAL);
        li->setSelectionBrushImage(selectImage);
		li->setTooltipText("Plane Optimal: The frustum is optimised to project "
			"shadows onto a plane of interest. Best possible quality for the "
			"plane, less good for other receiver angles.");
		cbo->addItem(li);

		
		cbo = static_cast<CEGUI::Combobox*>(wmgr.getWindow("Shadows/Material"));
		cbo->getEditbox()->subscribeEvent(CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseEnterCombo, this));
		cbo->getEditbox()->subscribeEvent(CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseLeave, this));
		cbo->getDropList()->subscribeEvent(CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseEnterCombo, this));
		cbo->getDropList()->subscribeEvent(CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseLeave, this));
		cbo->getPushButton()->subscribeEvent(CEGUI::Window::EventMouseEnters, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseEnterCombo, this));
		cbo->getPushButton()->subscribeEvent(CEGUI::Window::EventMouseLeaves, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMouseLeave, this));
		cbo->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
			CEGUI::Event::Subscriber(&ShadowsApplication::handleMaterialChanged, this));

		li = new CEGUI::ListboxTextItem("Standard", MAT_STANDARD);
		li->setSelectionBrushImage(selectImage);
		li->setTooltipText("Standard Material: Shadows are rendered into a simple RGB texture "
			" and are received only by objects that are not themselves shadow casters "
			" (no self-shadowing)");
		cbo->addItem(li);
		cbo->setItemSelectState(li, true);
		cbo->setText("Standard");
		mCurrentMaterial = MAT_STANDARD;

		li = new CEGUI::ListboxTextItem("Depth Shadowmap", MAT_DEPTH_FLOAT);
		li->setSelectionBrushImage(selectImage);
		li->setTooltipText("Depth Shadowmap: Shadow caster depth is rendered into a "
			" floating point texture and a depth comparison is performed on receivers "
			" (self-shadowing allowed). Requires floating point textures and shader support.");
		cbo->addItem(li);

		li = new CEGUI::ListboxTextItem("Depth Shadowmap (PCF)", MAT_DEPTH_FLOAT_PCF);
		li->setSelectionBrushImage(selectImage);
		li->setTooltipText("Depth Shadowmap (PCF): Shadow caster depth is rendered into a "
			" floating point texture and a depth comparison is performed on receivers "
			" (self-shadowing allowed), with a percentage closest filter. Requires "
			"floating point textures and shader support.");
		cbo->addItem(li);

		CEGUI::RadioButton* radio = static_cast<CEGUI::RadioButton*>(
			wmgr.getWindow("Shadows/Stencil"));
		radio->setSelected(true);
		radio->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleShadowTypeChanged, this));
		radio = static_cast<CEGUI::RadioButton*>(wmgr.getWindow("Shadows/Texture"));
		radio->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleShadowTypeChanged, this));
		radio = static_cast<CEGUI::RadioButton*>(wmgr.getWindow("Shadows/Modulative"));
		radio->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleShadowTypeChanged, this));
		radio = static_cast<CEGUI::RadioButton*>(wmgr.getWindow("Shadows/Additive"));
		radio->setSelected(true);
		radio->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleShadowTypeChanged, this));



		mFixedBias = static_cast<CEGUI::Editbox*>(wmgr.getWindow("Shadows/Main/FixedBias"));
		mFixedBias->setText("0.002");
		mFixedBias->subscribeEvent(CEGUI::Editbox::EventTextAccepted, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleParamsChanged, this));
		mGradientBias = static_cast<CEGUI::Editbox*>(wmgr.getWindow("Shadows/Main/GradientBias"));
		mGradientBias->setText("0.0008");
		mGradientBias->subscribeEvent(CEGUI::Editbox::EventTextAccepted, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleParamsChanged, this));
		mGradientClamp = static_cast<CEGUI::Editbox*>(wmgr.getWindow("Shadows/Main/GradientClamp"));
		mGradientClamp->setText("0.02");
		mGradientClamp->subscribeEvent(CEGUI::Editbox::EventTextAccepted, 
			CEGUI::Event::Subscriber(&ShadowsApplication::handleParamsChanged, this));

		

		updateGUI(mCurrentShadowTechnique);



	}

	void updateGUI(ShadowTechnique newTech)
	{
		bool isTextureBased = (newTech & SHADOWDETAILTYPE_TEXTURE) != 0;

		// Stencil based technique, turn off the texture-specific options
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		CEGUI::Window* win = wmgr.getWindow("Shadows/Projection");
		win->setEnabled(isTextureBased);
		win = wmgr.getWindow("Shadows/Material");
		win->setEnabled(isTextureBased);

	}

	/// callback when mouse enters a described field (non-combo)
	bool handleMouseEnter(const CEGUI::EventArgs& e)
	{
		const CEGUI::WindowEventArgs& winargs = 
			static_cast<const CEGUI::WindowEventArgs&>(e);
		mDescWindow->setText(winargs.window->getTooltipText());

		return true;
	}
	/// callback when mouse leaves a described field
	bool handleMouseLeave(const CEGUI::EventArgs& e)
	{
		//if (mDescWindow)
		//	mDescWindow->setText("");

		return true;
	}

	void updateTipForCombo(CEGUI::Combobox* cbo)
	{

		CEGUI::String text = cbo->getTooltipText();

		text.append(" ");
		if (cbo->getSelectedItem())
			text.append(cbo->getSelectedItem()->getTooltipText());
		mDescWindow->setText(text);

	}
	/// callback when mouse enters a described field (combo)
	bool handleMouseEnterCombo(const CEGUI::EventArgs& e)
	{
		const CEGUI::WindowEventArgs& winargs = 
			static_cast<const CEGUI::WindowEventArgs&>(e);
		// get tooltip from parent combo (events raised on contained components)
		CEGUI::Combobox* cbo = static_cast<CEGUI::Combobox*>(winargs.window->getParent());
		updateTipForCombo(cbo);

		return true;
	}
	// Callback when a shadow type combo changed
	bool handleShadowTypeChanged(const CEGUI::EventArgs& e)
	{
		// Only trigger change on selected
		const CEGUI::WindowEventArgs& we = static_cast<const CEGUI::WindowEventArgs&>(e);
		CEGUI::RadioButton* radio = static_cast<CEGUI::RadioButton*>(we.window);
		if (radio->isSelected())
		{
			ShadowTechnique newTech = mCurrentShadowTechnique;
			switch (radio->getID())
			{
			case 1:
				// stencil 
				newTech = static_cast<ShadowTechnique>(
					(newTech & ~SHADOWDETAILTYPE_TEXTURE) | SHADOWDETAILTYPE_STENCIL);
				resetMaterials();
				break;
			case 2:
				// texture
				newTech = static_cast<ShadowTechnique>(
					(newTech & ~SHADOWDETAILTYPE_STENCIL) | SHADOWDETAILTYPE_TEXTURE);
				break;
			case 3:
				// additive
				newTech = static_cast<ShadowTechnique>(
					(newTech & ~SHADOWDETAILTYPE_MODULATIVE) | SHADOWDETAILTYPE_ADDITIVE);
				break;
			case 4:
				// modulative
				newTech = static_cast<ShadowTechnique>(
					(newTech & ~SHADOWDETAILTYPE_ADDITIVE) | SHADOWDETAILTYPE_MODULATIVE);
				break;


			}

			changeShadowTechnique(newTech);

		}

		return true;
	}

	bool handleProjectionChanged(const CEGUI::EventArgs& e)
	{
		const CEGUI::WindowEventArgs& winargs = 
			static_cast<const CEGUI::WindowEventArgs&>(e);
		CEGUI::Combobox* cbo = static_cast<CEGUI::Combobox*>(winargs.window);

		if (cbo->getSelectedItem())
		{
			ShadowProjection proj = (ShadowProjection)cbo->getSelectedItem()->getID();
			if (proj != mCurrentProjection)
			{
				switch(proj)
				{
				case UNIFORM:
					mCurrentShadowCameraSetup = 
						ShadowCameraSetupPtr(new DefaultShadowCameraSetup());
					break;
				case UNIFORM_FOCUSED:
					mCurrentShadowCameraSetup = 
						ShadowCameraSetupPtr(new FocusedShadowCameraSetup());
					break;
				case LISPSM:
					{
						mLiSPSMSetup = new LiSPSMShadowCameraSetup();
						//mLiSPSMSetup->setUseAggressiveFocusRegion(false);
						mCurrentShadowCameraSetup = ShadowCameraSetupPtr(mLiSPSMSetup);
					}
					break;
				case PLANE_OPTIMAL:
					mCurrentShadowCameraSetup = 
						ShadowCameraSetupPtr(new PlaneOptimalShadowCameraSetup(mPlane));
					break;

				};
				mCurrentProjection = proj;

				mSceneMgr->setShadowCameraSetup(mCurrentShadowCameraSetup);

				updateTipForCombo(cbo);
				if (!mCustomRockwallVparams.isNull() && !mCustomRockwallFparams.isNull())
				{
					// set
					setDefaultDepthShadowParams();
				}

			}
		}

		return true;
	}

	void updateDepthShadowParams()
	{
		mCustomRockwallFparams->setNamedConstant("fixedDepthBias", 
			StringConverter::parseReal(mFixedBias->getText().c_str()));
		mCustomRockwallFparams->setNamedConstant("gradientScaleBias",
			StringConverter::parseReal(mGradientBias->getText().c_str()));
		mCustomRockwallFparams->setNamedConstant("gradientClamp",
			StringConverter::parseReal(mGradientClamp->getText().c_str()));
	}

	void setDefaultDepthShadowParams()
	{
		switch(mCurrentProjection)
		{
		case UNIFORM:
		case UNIFORM_FOCUSED:
		case PLANE_OPTIMAL:
			mFixedBias->setText("0");
			mGradientBias->setText("0");
			break;
		case LISPSM:
			mFixedBias->setText("0.009");
			mGradientBias->setText("0.04");
			break;
		};

		updateDepthShadowParams();

	}

	bool handleParamsChanged(const CEGUI::EventArgs& e)
	{
		if (!mCustomRockwallVparams.isNull() && !mCustomRockwallFparams.isNull())
		{
			updateDepthShadowParams();
		}

		return true;
	}

	void rebindDebugShadowOverlays()
	{
		MaterialPtr debugMat = 
			MaterialManager::getSingleton().getByName("Ogre/DebugShadowMap0");
		TexturePtr shadowTex = mSceneMgr->getShadowTexture(0);
		debugMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(shadowTex->getName());

		debugMat = 
			MaterialManager::getSingleton().getByName("Ogre/DebugShadowMap1");
		shadowTex = mSceneMgr->getShadowTexture(1);
		debugMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(shadowTex->getName());
	}

	void resetMaterials()
	{
		// Sort out base materials
		pPlaneEnt->setMaterialName(BASIC_ROCKWALL_MATERIAL);
		for (std::vector<Entity*>::iterator i = pColumns.begin();
			i != pColumns.end(); ++i)
		{
			(*i)->setMaterialName(BASIC_ROCKWALL_MATERIAL);
		}

		mCustomRockwallVparams.setNull();
		mCustomRockwallFparams.setNull();

	}
	bool handleMaterialChanged(const CEGUI::EventArgs& e)
	{
		const CEGUI::WindowEventArgs& winargs = 
			static_cast<const CEGUI::WindowEventArgs&>(e);
		CEGUI::Combobox* cbo = static_cast<CEGUI::Combobox*>(winargs.window);

		if (cbo->getSelectedItem())
		{
			ShadowMaterial mat = (ShadowMaterial)cbo->getSelectedItem()->getID();
			MaterialPtr themat;
			if (mat != mCurrentMaterial)
			{
				switch(mat)
				{
				case MAT_STANDARD:
					mSceneMgr->setShadowTexturePixelFormat(PF_L8);
					mSceneMgr->setShadowTextureCasterMaterial(StringUtil::BLANK);
					mSceneMgr->setShadowTextureReceiverMaterial(StringUtil::BLANK);
					mSceneMgr->setShadowTextureSelfShadow(false);	

					resetMaterials();

					break;
				case MAT_DEPTH_FLOAT:
					mSceneMgr->setShadowTexturePixelFormat(PF_FLOAT32_R);
					mSceneMgr->setShadowTextureCasterMaterial(CUSTOM_CASTER_MATERIAL);
					mSceneMgr->setShadowTextureReceiverMaterial(CUSTOM_RECEIVER_MATERIAL);
					mSceneMgr->setShadowTextureSelfShadow(true);	
					// Sort out base materials
					pPlaneEnt->setMaterialName(CUSTOM_ROCKWALL_MATERIAL);
					for (std::vector<Entity*>::iterator i = pColumns.begin();
						i != pColumns.end(); ++i)
					{
						(*i)->setMaterialName(CUSTOM_ROCKWALL_MATERIAL);
					}

					themat = MaterialManager::getSingleton().getByName(CUSTOM_ROCKWALL_MATERIAL);
					mCustomRockwallVparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverVertexProgramParameters();
					mCustomRockwallFparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverFragmentProgramParameters();

					// set the current params
					setDefaultDepthShadowParams();
					break;
				case MAT_DEPTH_FLOAT_PCF:
					mSceneMgr->setShadowTexturePixelFormat(PF_FLOAT32_R);
					mSceneMgr->setShadowTextureCasterMaterial(CUSTOM_CASTER_MATERIAL);
					mSceneMgr->setShadowTextureReceiverMaterial(CUSTOM_RECEIVER_MATERIAL + "/PCF");
					mSceneMgr->setShadowTextureSelfShadow(true);	
					// Sort out base materials
					pPlaneEnt->setMaterialName(CUSTOM_ROCKWALL_MATERIAL + "/PCF");
					for (std::vector<Entity*>::iterator i = pColumns.begin();
						i != pColumns.end(); ++i)
					{
						(*i)->setMaterialName(CUSTOM_ROCKWALL_MATERIAL + "/PCF");
					}

					themat = MaterialManager::getSingleton().getByName(CUSTOM_ROCKWALL_MATERIAL + "/PCF");
					mCustomRockwallVparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverVertexProgramParameters();
					mCustomRockwallFparams = themat->getTechnique(0)->getPass(1)->getShadowReceiverFragmentProgramParameters();

					// set the current params
					setDefaultDepthShadowParams();
					break;
				};
				mCurrentMaterial = mat;

				updateTipForCombo(cbo);
				rebindDebugShadowOverlays();

			}
		}

		return true;
	}

    // Create new frame listener
    void createFrameListener(void)
    {
        ShadowsListener* shadowListener = new ShadowsListener(mWindow, mCamera, mSceneMgr);
		mFrameListener = shadowListener;
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);

		// Hook up Root sheet (handles main input)
		CEGUI::Window* wnd = CEGUI::WindowManager::getSingleton().getWindow("Shadows");
		wnd->subscribeEvent(CEGUI::Window::EventMouseMove, 
			CEGUI::Event::Subscriber(&ShadowsListener::handleMouseMove, shadowListener));
		wnd->subscribeEvent(CEGUI::Window::EventMouseButtonUp, 
			CEGUI::Event::Subscriber(&ShadowsListener::handleMouseButtonUp, shadowListener));
		wnd->subscribeEvent(CEGUI::Window::EventMouseButtonDown, 
			CEGUI::Event::Subscriber(&ShadowsListener::handleMouseButtonDown, shadowListener));
		wnd->subscribeEvent(CEGUI::Window::EventMouseWheel, 
			CEGUI::Event::Subscriber(&ShadowsListener::handleMouseWheelEvent, shadowListener));
		wnd->subscribeEvent(CEGUI::Window::EventKeyDown, 
			CEGUI::Event::Subscriber(&ShadowsListener::handleKeyDownEvent, shadowListener ));
		wnd->subscribeEvent(CEGUI::Window::EventKeyUp, 
			CEGUI::Event::Subscriber(&ShadowsListener::handleKeyUpEvent, shadowListener ));


    }


public:
    void go(void)
    {
        if (!setup())
            return;

        mRoot->startRendering();
    }


};



#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{
    // Create application object
    ShadowsApplication app;

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
