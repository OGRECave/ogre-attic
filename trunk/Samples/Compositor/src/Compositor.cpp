/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    \file
        Compositor.cpp
    \brief
        Shows OGRE's Compositor feature
	\author
		W.J. :wumpus: van der Laan
			Ogre composition framework
		Manuel Bua
			Postfilter ideas and original out-of-core implementation
*/

#include "ExampleApplication.h"
#include "ExampleFrameListener.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
#include <OgreCompositor.h>
#include <OgreCompositorManager.h>
#include <OgreCompositorChain.h>
#include <OgreCompositorInstance.h>
#include <OgreCompositionTechnique.h>
#include <OgreCompositionPass.h>
#include <OgreCompositionTargetPass.h>


    /** Listener that keeps and updates private parameters for a HeatVision instance.
    */
    class HeatVisionListener: public CompositorInstance::Listener
    {
    public:
        HeatVisionListener()
        {
            timer = PlatformManager::getSingleton().createTimer();
            start = end = curr = 0.0f;
        }
        virtual ~HeatVisionListener()
        {
            PlatformManager::getSingleton().destroyTimer(timer);
        }
        virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
        {
            if(pass_id == 0xDEADBABE)
            {
                timer->reset();
                fpParams =
                    mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
            }
        }
        virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
        {
            if(pass_id == 0xDEADBABE)
            {
                // "random_fractions" parameter
                fpParams->setNamedConstant("random_fractions", Vector4(Math::RangeRandom(0.0, 1.0), Math::RangeRandom(0, 1.0), 0, 0));

                // "depth_modulator" parameter
                float inc = ((float)timer->getMilliseconds())/1000.0f;
                if ( (abs(curr-end) <= 0.001) ) {
                    // take a new value to reach
                    end = Math::RangeRandom(0.95, 1.0);
                    start = curr;
                } else {
                    if (curr > end) curr -= inc;
                    else curr += inc;
                }
                timer->reset();

                fpParams->setNamedConstant("depth_modulator", Vector4(curr, 0, 0, 0));
            }
        }
    protected:
        GpuProgramParametersSharedPtr fpParams;
        float start, end, curr;
        Timer *timer;
    };

    class CompositorFrameListener : public ExampleFrameListener
    {
    protected:
        float timeoutDelay;
        HeatVisionListener *hvListener;

        struct CompositorEntry
        {
            String CompositorName;
            String DisplayName;
            bool Enabled;
            KeyCode KCode;

            CompositorEntry(void) : Enabled(false), KCode(KC_A) {}
            CompositorEntry(const String& compName, const String& dispName, KeyCode keyCode)
                : CompositorName(compName)
                , DisplayName(dispName)
                , Enabled(false)
                , KCode(keyCode)
                {}
        };

        typedef std::vector<CompositorEntry> CompositorEntryContainer;
        typedef CompositorEntryContainer::iterator CompositorEntryIterator;

        CompositorEntryContainer mRegisteredCompositors;

        OverlayElement* mDescText;

    public:
        CompositorFrameListener(RenderWindow* window, Camera* maincam)
            :ExampleFrameListener(window, maincam)
            , hvListener(0)
        {
            timeoutDelay = 0;
            mDescText = OverlayManager::getSingleton().getOverlayElement("Example/Compositor/ActiveText");
            mDescText->setCaption("None");
            mRegisteredCompositors.reserve(15);

            registerCompositors();
        }
        ~CompositorFrameListener()
        {
            delete hvListener;
        }

        void registerCompositors(void)
        {
            mRegisteredCompositors.push_back(CompositorEntry("Bloom",       "[Bloom] ",         KC_1));
            mRegisteredCompositors.push_back(CompositorEntry("Hurt",        "[Hurt] ",          KC_2));
            mRegisteredCompositors.push_back(CompositorEntry("Glass",       "[Glass] ",         KC_3));
            mRegisteredCompositors.push_back(CompositorEntry("MotionBlur",  "[Motion Blur] ",   KC_4));
            mRegisteredCompositors.push_back(CompositorEntry("Old TV",       "[Old TV] ",        KC_6));
            mRegisteredCompositors.push_back(CompositorEntry("B&W",         "[B&W]",            KC_7));
            mRegisteredCompositors.push_back(CompositorEntry("DOF",         "[DOF] ",           KC_8));
            mRegisteredCompositors.push_back(CompositorEntry("Embossed",    "[Embossed] ",      KC_0));

            Viewport *vp = mWindow->getViewport(0);

            CompositorEntryIterator currentCompEntry = mRegisteredCompositors.begin();
            while (currentCompEntry != mRegisteredCompositors.end())
            {
                /// Add compositors to main viewport
                CompositorManager::getSingleton().addCompositor(vp, currentCompEntry->CompositorName);
                CompositorManager::getSingleton().setCompositorEnabled(vp, currentCompEntry->CompositorName, false);

                ++currentCompEntry;
            }

            hvListener = new HeatVisionListener();
            CompositorInstance *instance = CompositorManager::getSingleton().addCompositor(vp, "HeatVision");
            mRegisteredCompositors.push_back(CompositorEntry("HeatVision",  "[Heat Vision] ",   KC_5));
            CompositorManager::getSingleton().setCompositorEnabled(vp, "HeatVision", false);
            if(instance)
                instance->addListener(hvListener);
        }

        bool frameStarted(const FrameEvent& evt)
        {
            bool result = ExampleFrameListener::frameStarted(evt);

            return result;
        }

        virtual bool processUnbufferedKeyInput(const FrameEvent& evt)
        {
            bool retval = ExampleFrameListener::processUnbufferedKeyInput(evt);
            timeoutDelay -= evt.timeSinceLastFrame;

            if(timeoutDelay < 0.0f)
            {

                timeoutDelay = 0.0f;
                Viewport *vp = mWindow->getViewport(0);

                CompositorEntryIterator currentCompEntry = mRegisteredCompositors.begin();
                while (currentCompEntry != mRegisteredCompositors.end())
                {
                    if(mInputDevice->isKeyDown(currentCompEntry->KCode))
                    {
                        currentCompEntry->Enabled = !currentCompEntry->Enabled;
                        CompositorManager::getSingleton().setCompositorEnabled(vp, currentCompEntry->CompositorName, currentCompEntry->Enabled);
                        timeoutDelay = 0.5f;
                    }

                    ++currentCompEntry;
                }

                if (timeoutDelay >= 0.5f)
                {
                    StringUtil::StrStreamType txt;

                    currentCompEntry = mRegisteredCompositors.begin();
                    while (currentCompEntry != mRegisteredCompositors.end())
                    {
                        if(currentCompEntry->Enabled)
                        {
                            txt << currentCompEntry->DisplayName;
                        }

                        ++currentCompEntry;
                    }

                    if (txt.str().empty())
                        txt << "None ";

                    mDescText->setCaption(txt.str());
                }
            }
            return retval;
        }
    };

// ********** Currently not used
//    class SceneEffectController :  public RenderTargetListener
//    {
//        void preRenderTargetUpdate(const RenderTargetEvent& evt)
//        {
//            // Hide plane
//            mPlaneEnt->setVisible(false);
//
//        }
//        void postRenderTargetUpdate(const RenderTargetEvent& evt)
//        {
//            // Show plane
//            mPlaneEnt->setVisible(true);
//        }
//
//    protected:
//
//        MovablePlane* mPlane;
//        Entity* mPlaneEnt;
//        SceneNode* mPlaneNode;
//
//    public:
//        SceneEffectController() : mPlane(0) {}
//        ~SceneEffectController()
//        {
//            delete mPlane;
//        }
//
//
//    };

class CompositorApplication : public ExampleApplication
{

public:
    CompositorApplication() {}
    ~CompositorApplication()
    {
    }

protected:

    // render target events

	/// Create the postfilter effects
	/// This will be replaced with a .compositor script as soon as the script parser is
	/// finished.
	void createEffects()
	{
	    // Bloom compositor is loaded from script but here is the hard coded equivalent
//		CompositorPtr comp = CompositorManager::getSingleton().create(
//				"Bloom", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
//			);
//		{
//			CompositionTechnique *t = comp->createTechnique();
//			{
//				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt0");
//				def->width = 128;
//				def->height = 128;
//				def->format = PF_A8R8G8B8;
//			}
//			{
//				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt1");
//				def->width = 128;
//				def->height = 128;
//				def->format = PF_A8R8G8B8;
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
//				tp->setOutputName("rt1");
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_NONE);
//				tp->setOutputName("rt0");
//				CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/Blur0");
//				pass->setInput(0, "rt1");
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_NONE);
//				tp->setOutputName("rt1");
//				CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/Blur1");
//				pass->setInput(0, "rt0");
//			}
//			{
//				CompositionTargetPass *tp = t->getOutputTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
//				{ CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/BloomBlend");
//				pass->setInput(0, "rt1");
//				}
//			}
//		}
		/// Just a big red quad
		CompositorPtr redquad = CompositorManager::getSingleton().create(
				"Hurt", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
		{
			CompositionTechnique *t = redquad->createTechnique();
			{
				CompositionTargetPass *tp = t->getOutputTargetPass();
				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
				{ CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("RedQuad");
				}
			}
		}
	    // Glass compositor is loaded from script but here is the hard coded equivalent
		/// Glass effect
//		CompositorPtr comp2 = CompositorManager::getSingleton().create(
//				"Glass", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
//			);
//		{
//			CompositionTechnique *t = comp2->createTechnique();
//			{
//				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt0");
//				def->width = 0;
//				def->height = 0;
//				def->format = PF_R8G8B8;
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
//				tp->setOutputName("rt0");
//			}
//			{
//				CompositionTargetPass *tp = t->getOutputTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_NONE);
//				{ CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/GlassPass");
//				pass->setInput(0, "rt0");
//				}
//			}
//		}
		/// Motion blur effect
		CompositorPtr comp3 = CompositorManager::getSingleton().create(
				"MotionBlur", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
		{
			CompositionTechnique *t = comp3->createTechnique();
			{
				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
				def->width = 0;
				def->height = 0;
				def->format = PF_R8G8B8;
			}
			{
				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("sum");
				def->width = 0;
				def->height = 0;
				def->format = PF_R8G8B8;
			}
			{
				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
				def->width = 0;
				def->height = 0;
				def->format = PF_R8G8B8;
			}
			/// Render scene
			{
				CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
				tp->setOutputName("scene");
			}
			/// Initialisation pass for sum texture
			{
				CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
				tp->setOutputName("sum");
				tp->setOnlyInitial(true);
			}
			/// Do the motion blur
			{
				CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(CompositionTargetPass::IM_NONE);
				tp->setOutputName("temp");
				{ CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/Combine");
				pass->setInput(0, "scene");
				pass->setInput(1, "sum");
				}
			}
			/// Copy back sum texture
			{
				CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(CompositionTargetPass::IM_NONE);
				tp->setOutputName("sum");
				{ CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/Copyback");
				pass->setInput(0, "temp");
				}
			}
			/// Display result
			{
				CompositionTargetPass *tp = t->getOutputTargetPass();
				tp->setInputMode(CompositionTargetPass::IM_NONE);
				{ CompositionPass *pass = tp->createPass();
				pass->setType(CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/MotionBlur");
				pass->setInput(0, "sum");
				}
			}
		}
		/// Heat vision effect
		CompositorPtr comp4 = CompositorManager::getSingleton().create(
				"HeatVision", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
		{
			CompositionTechnique *t = comp4->createTechnique();
			{
				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
				def->width = 256;
				def->height = 256;
				def->format = PF_R8G8B8;
			}
			{
				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
				def->width = 256;
				def->height = 256;
				def->format = PF_R8G8B8;
			}
			/// Render scene
			{
				CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
				tp->setOutputName("scene");
			}
			/// Light to heat pass
			{
				CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(CompositionTargetPass::IM_NONE);
				tp->setOutputName("temp");
				{
					CompositionPass *pass = tp->createPass();
					pass->setType(CompositionPass::PT_RENDERQUAD);
					pass->setIdentifier(0xDEADBABE); /// Identify pass for use in listener
					pass->setMaterialName("Fury/HeatVision/LightToHeat");
					pass->setInput(0, "scene");
				}
			}
			/// Display result
			{
				CompositionTargetPass *tp = t->getOutputTargetPass();
				tp->setInputMode(CompositionTargetPass::IM_NONE);
				{
					CompositionPass *pass = tp->createPass();
					pass->setType(CompositionPass::PT_RENDERQUAD);
					pass->setMaterialName("Fury/HeatVision/Blur");
					pass->setInput(0, "temp");
				}
			}
		}
	}

    // Just override the mandatory create scene method
    void createScene(void)
    {
		MovableObject::setDefaultVisibilityFlags(0x00000001);

		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));

		Light* l = mSceneMgr->createLight("Light2");
		Vector3 dir(-1,-1,0);
		dir.normalise();
		l->setType(Light::LT_DIRECTIONAL);
		l->setDirection(dir);
		l->setDiffuseColour(1, 1, 0.8);
		l->setSpecularColour(1, 1, 1);


		Entity* pEnt;
		pEnt = mSceneMgr->createEntity( "1", "robot.mesh" );
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject( pEnt );

		// pre-load to generate tangents
		MeshPtr msh = MeshManager::getSingleton().load("knot.mesh",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		unsigned short src, dest;
		if (!msh->suggestTangentVectorBuildParams(src, dest))
		{
			msh->buildTangentVectors(src, dest);
		}

		// Does not receive shadows
		pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
		pEnt->setMaterialName("Examples/EnvMappedRustySteel");
		MaterialPtr mat2 = MaterialManager::getSingleton().getByName("Examples/EnvMappedRustySteel");
		mat2->setReceiveShadows(false);
		SceneNode* n2 = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200, 0, -200));
		n2->attachObject( pEnt );

		// Transparent object
		pEnt = mSceneMgr->createEntity( "3.5", "knot.mesh" );
		pEnt->setMaterialName("Examples/TransparentTest");
		MaterialPtr mat3 = MaterialManager::getSingleton().getByName("Examples/TransparentTest");
		pEnt->setCastShadows(false);
		SceneNode* n3 = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(350, 0, -200));
		n3->attachObject( pEnt );

		pEnt = mSceneMgr->createEntity( "4", "knot.mesh" );
		pEnt->setMaterialName("Examples/BumpMapping/MultiLightSpecular");
		SceneNode* n4 = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 200));
		n4->attachObject( pEnt );


		ParticleSystem* pSys2 = mSceneMgr->createParticleSystem("smoke",
			"Examples/Smoke");
		SceneNode* n5 = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-300, -100, 200));
		n5->attachObject(pSys2);


		mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");


		Plane plane;
		plane.normal = Vector3::UNIT_Y;
		plane.d = 100;
		MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
			1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
		Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
		pPlaneEnt->setMaterialName("2 - Default");
		pPlaneEnt->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

		mCamera->setPosition(180, 54, 823);
		mCamera->lookAt(0,20,0);

		/// Create the postfilter effects
		/// This will be replaced with a .compositor script as soon as the script parser is
		/// finished.
		createEffects();


		// show overlay
		Overlay* pOver = OverlayManager::getSingleton().getByName("Example/CompositorOverlay");
		pOver->show();

    }

    void createFrameListener(void)
    {
        mFrameListener= new CompositorFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }

};

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
    CompositorApplication app;

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
#endif
    }


    return 0;
}

#ifdef __cplusplus
}
#endif
