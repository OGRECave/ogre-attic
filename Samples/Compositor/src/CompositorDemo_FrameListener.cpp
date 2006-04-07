/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

#include <Ogre.h>

#include "CompositorDemo_FrameListener.h"
#include "Compositor.h"

/*************************************************************************
	                    HeatVisionListener Methods
*************************************************************************/
//---------------------------------------------------------------------------
    HeatVisionListener::HeatVisionListener()
    {
        timer = Ogre::PlatformManager::getSingleton().createTimer();
        start = end = curr = 0.0f;
    }
//---------------------------------------------------------------------------
    HeatVisionListener::~HeatVisionListener()
    {
        Ogre::PlatformManager::getSingleton().destroyTimer(timer);
    }
//---------------------------------------------------------------------------
    void HeatVisionListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        if(pass_id == 0xDEADBABE)
        {
            timer->reset();
            fpParams =
                mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
        }
    }
//---------------------------------------------------------------------------
    void HeatVisionListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        if(pass_id == 0xDEADBABE)
        {
            // "random_fractions" parameter
            fpParams->setNamedConstant("random_fractions", Ogre::Vector4(Ogre::Math::RangeRandom(0.0, 1.0), Ogre::Math::RangeRandom(0, 1.0), 0, 0));

            // "depth_modulator" parameter
            float inc = ((float)timer->getMilliseconds())/1000.0f;
            if ( (abs(curr-end) <= 0.001) ) {
                // take a new value to reach
                end = Ogre::Math::RangeRandom(0.95, 1.0);
                start = curr;
            } else {
                if (curr > end) curr -= inc;
                else curr += inc;
            }
            timer->reset();

            fpParams->setNamedConstant("depth_modulator", Ogre::Vector4(curr, 0, 0, 0));
        }
    }
//---------------------------------------------------------------------------

	/*************************************************************************
	HDRListener Methods
	*************************************************************************/
	//---------------------------------------------------------------------------
	HDRListener::HDRListener()
	{
	}
	//---------------------------------------------------------------------------
	HDRListener::~HDRListener()
	{
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyViewportSize(int width, int height)
	{
		mVpWidth = width;
		mVpHeight = height;
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyCompositor(Ogre::CompositorInstance* instance)
	{
		// Get some RTT dimensions for later calculations
		Ogre::CompositionTechnique::TextureDefinitionIterator defIter = 
			instance->getTechnique()->getTextureDefinitionIterator();
		while (defIter.hasMoreElements())
		{
			Ogre::CompositionTechnique::TextureDefinition* def = 
				defIter.getNext();
			if (def->name == "rt_lum4")
			{
				mLum4Width = def->width;
				mLum4Height = def->height;
				break;
			}
		}
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{
		// Prepare the fragment params offsets
		switch(pass_id)
		{
		case 994: // rt_lum4
			break;
		case 993: // rt_lum3
			break;
		case 992: // rt_lum2
			break;
		case 991: // rt_lum1
			break;
		case 990: // rt_lum0
			break;
		case 800: // rt_brightpass
			break;
		case 701: // rt_bloom1
			break;
		case 700: // rt_bloom0
			break;

		}
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{
	}
	//---------------------------------------------------------------------------

/*************************************************************************
	CompositorDemo_FrameListener methods that handle all input for this Compositor demo.
*************************************************************************/

    CompositorDemo_FrameListener::CompositorDemo_FrameListener(CompositorDemo* main)
        : mMain(main)
        , hvListener(0)
		, hdrListener(0)
        , mTranslateVector(Ogre::Vector3::ZERO)
        , mStatsOn(true)
        , mNumScreenShots(0)
        , mWriteToFile(false)
        , mSkipCount(0)
        , mUpdateFreq(50)
        , mSceneDetailIndex(0)
        , mFiltering(Ogre::TFO_BILINEAR)
        , mAniso(1)
        , mQuit(false)

        , mMoveScale(0.0f)
        , mRotScale(0.0f)
        , mSpeed(MINSPEED)
        , mAvgFrameTime(0.1)
        , mMoveSpeed(100)
        , mRotateSpeed(0)
        , mLastMousePositionSet(false)
        , mTimeUntilNextToggle(0)
        , mRotX(0)
        , mRotY(0)
        , mProcessMovement(false)
        , mUpdateMovement(false)

        , mLMBDown(false)
        , mRMBDown(false)
        , mMoveFwd(false)
        , mMoveBck(false)
        , mMoveLeft(false)
        , mMoveRight(false)
        , mCompositorSelectorViewManager(0)

    {

        Ogre::Root::getSingleton().addFrameListener(this);

        // using buffered input
        mEventProcessor = new Ogre::EventProcessor();
        mEventProcessor->initialise(mMain->getRenderWindow());
        mEventProcessor->startProcessingEvents();
        mEventProcessor->addKeyListener(this);
        mEventProcessor->addMouseMotionListener(this);
        mEventProcessor->addMouseListener(this);
        mInputDevice = mEventProcessor->getInputReader();

        mGuiRenderer = CEGUI::System::getSingleton().getRenderer();

        mGuiAvg   = CEGUI::WindowManager::getSingleton().getWindow("OPAverageFPS");
        mGuiCurr  = CEGUI::WindowManager::getSingleton().getWindow("OPCurrentFPS");
        mGuiBest  = CEGUI::WindowManager::getSingleton().getWindow("OPBestFPS");
        mGuiWorst = CEGUI::WindowManager::getSingleton().getWindow("OPWorstFPS");
        mGuiTris  = CEGUI::WindowManager::getSingleton().getWindow("OPTriCount");
        mGuiDbg   = CEGUI::WindowManager::getSingleton().getWindow("OPDebugMsg");
        mRoot	  = CEGUI::WindowManager::getSingleton().getWindow("root");

        registerCompositors();
		initDebugRTTWindow();
        connectEventHandlers();
    }
//--------------------------------------------------------------------------
    CompositorDemo_FrameListener::~CompositorDemo_FrameListener()
    {
        delete mEventProcessor;
        delete hvListener;
		delete hdrListener;
        delete mCompositorSelectorViewManager;
    }
//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::connectEventHandlers(void)
    {
        CEGUI::Window* wndw = CEGUI::WindowManager::getSingleton().getWindow("root");

        wndw->subscribeEvent(CEGUI::Window::EventMouseMove, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleMouseMove, this));

        wndw->subscribeEvent(CEGUI::Window::EventMouseButtonUp, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleMouseButtonUp, this));

        wndw->subscribeEvent(CEGUI::Window::EventMouseButtonDown, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleMouseButtonDown, this));

        wndw->subscribeEvent(CEGUI::Window::EventMouseWheel, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleMouseWheelEvent, this));
        wndw->subscribeEvent(CEGUI::Window::EventKeyDown, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleKeyDownEvent, this ));
        wndw->subscribeEvent(CEGUI::Window::EventKeyUp, CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleKeyUpEvent, this ));
    }
//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::frameStarted(const Ogre::FrameEvent& evt)
    {
        if (mQuit)
            return false;
        else
        {
            mSkipCount++;
            if (mSkipCount >= mUpdateFreq)
            {
                mSkipCount = 0;
                updateStats();
            }
            // update movement process
            if(mProcessMovement || mUpdateMovement)
            {
                mTranslateVector.x += mMoveLeft ? mAvgFrameTime * -MOVESPEED : 0;
                mTranslateVector.x += mMoveRight ? mAvgFrameTime * MOVESPEED : 0;
                mTranslateVector.z += mMoveFwd ? mAvgFrameTime * -MOVESPEED : 0;
                mTranslateVector.z += mMoveBck ? mAvgFrameTime * MOVESPEED : 0;

                mMain->getCamera()->yaw(Ogre::Angle(mRotX));
                mMain->getCamera()->pitch(Ogre::Angle(mRotY));
                mMain->getCamera()->moveRelative(mTranslateVector);

                mUpdateMovement = false;
                mRotX = 0;
                mRotY = 0;
                mTranslateVector = Ogre::Vector3::ZERO;
            }

            if(mWriteToFile)
            {
                mMain->getRenderWindow()->writeContentsToFile("frame_" +
                    Ogre::StringConverter::toString(++mNumScreenShots) + ".png");
            }
            return true;
        }
    }

//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::mouseMoved (Ogre::MouseEvent *e)
    {
        CEGUI::System::getSingleton().injectMouseMove(e->getRelX() * mGuiRenderer->getWidth(), e->getRelY() * mGuiRenderer->getHeight());
        CEGUI::System::getSingleton().injectMouseWheelChange(e->getRelZ());
        e->consume();
    }

//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::mouseDragged (Ogre::MouseEvent *e)
    {
        mouseMoved(e);
    }

//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::keyPressed (Ogre::KeyEvent *e)
    {
        // give 'quitting' priority
        if (e->getKey() == Ogre::KC_ESCAPE)
        {
            mQuit = true;
            e->consume();
            return;
        }

        if (e->getKey() == Ogre::KC_SYSRQ )
        {
            char tmp[20];
            sprintf(tmp, "screenshot_%d.png", ++mNumScreenShots);
            mMain->getRenderWindow()->writeContentsToFile(tmp);
            //mTimeUntilNextToggle = 0.5;
            mMain->getRenderWindow()->setDebugText(Ogre::String("Wrote ") + tmp);
        }

        // do event injection
        CEGUI::System& cegui = CEGUI::System::getSingleton();

        // key down
        cegui.injectKeyDown(e->getKey());

        // now character
        cegui.injectChar(e->getKeyChar());

        e->consume();
    }

//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::keyReleased (Ogre::KeyEvent *e)
    {
        CEGUI::System::getSingleton().injectKeyUp(e->getKey());
    }

//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::mousePressed (Ogre::MouseEvent *e)
    {
        CEGUI::System::getSingleton().injectMouseButtonDown(convertOgreButtonToCegui(e->getButtonID()));
        e->consume();
    }

//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::mouseReleased (Ogre::MouseEvent *e)
    {
        CEGUI::System::getSingleton().injectMouseButtonUp(convertOgreButtonToCegui(e->getButtonID()));
        e->consume();
    }

//--------------------------------------------------------------------------
    CEGUI::MouseButton CompositorDemo_FrameListener::convertOgreButtonToCegui(int ogre_button_id)
    {
        switch (ogre_button_id)
        {
        case Ogre::MouseEvent::BUTTON0_MASK:
            return CEGUI::LeftButton;
            break;

        case Ogre::MouseEvent::BUTTON1_MASK:
            return CEGUI::RightButton;
            break;

        case Ogre::MouseEvent::BUTTON2_MASK:
            return CEGUI::MiddleButton;
            break;

        case Ogre::MouseEvent::BUTTON3_MASK:
            return CEGUI::X1Button;
            break;

        default:
            return CEGUI::LeftButton;
            break;
        }

    }

//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::updateStats(void)
    {
        static CEGUI::String currFps = "Current FPS: ";
        static CEGUI::String avgFps = "Average FPS: ";
        static CEGUI::String bestFps = "Best FPS: ";
        static CEGUI::String worstFps = "Worst FPS: ";
        static CEGUI::String tris = "Triangle Count: ";


        const Ogre::RenderTarget::FrameStats& stats = mMain->getRenderWindow()->getStatistics();

        mGuiAvg->setText(avgFps + Ogre::StringConverter::toString(stats.avgFPS));
        mGuiCurr->setText(currFps + Ogre::StringConverter::toString(stats.lastFPS));
        mGuiBest->setText(bestFps + Ogre::StringConverter::toString(stats.bestFPS)
            + " " + Ogre::StringConverter::toString(stats.bestFrameTime)+" ms");
        mGuiWorst->setText(worstFps + Ogre::StringConverter::toString(stats.worstFPS)
            + " " + Ogre::StringConverter::toString(stats.worstFrameTime)+" ms");

        mGuiTris->setText(tris + Ogre::StringConverter::toString(stats.triangleCount));
        mGuiDbg->setText(mMain->getRenderWindow()->getDebugText());
        mAvgFrameTime = 1.0f/(stats.avgFPS + 1.0f);
        if (mAvgFrameTime > 0.1f) mAvgFrameTime = 0.1f;

    }


//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleMouseMove(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;

        if( mLMBDown && !mRMBDown)
        {
            // rotate camera
            mRotX += -((const MouseEventArgs&)e).moveDelta.d_x * mAvgFrameTime * 10.0;
            mRotY += -((const MouseEventArgs&)e).moveDelta.d_y * mAvgFrameTime * 10.0;
            MouseCursor::getSingleton().setPosition( mLastMousePosition );
            mUpdateMovement = true;
        }
        else
        {
            if( mRMBDown && !mLMBDown)
            {
                // translate camera
                mTranslateVector.x += ((const MouseEventArgs&)e).moveDelta.d_x * mAvgFrameTime * MOVESPEED;
                mTranslateVector.y += -((const MouseEventArgs&)e).moveDelta.d_y * mAvgFrameTime * MOVESPEED;
                //mTranslateVector.z = 0;
                MouseCursor::getSingleton().setPosition( mLastMousePosition );
                mUpdateMovement = true;
            }
            else
            {
                if( mRMBDown && mLMBDown)
                {
                    mTranslateVector.z += (((const MouseEventArgs&)e).moveDelta.d_x + ((const MouseEventArgs&)e).moveDelta.d_y) * mAvgFrameTime * MOVESPEED;
                    MouseCursor::getSingleton().setPosition( mLastMousePosition );
                    mUpdateMovement = true;
                }

            }
        }

        return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleMouseButtonUp(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;

        //Window* wndw = ((const WindowEventArgs&)e).window;
        if( ((const MouseEventArgs&)e).button == LeftButton )
        {
            mLMBDown = false;
        }

        if( ((const MouseEventArgs&)e).button == RightButton )
        {
            mRMBDown = false;
        }
        if( !mLMBDown && !mRMBDown )
        {
            MouseCursor::getSingleton().show();
            if(mLastMousePositionSet)
            {
                MouseCursor::getSingleton().setPosition( mLastMousePosition );
                mLastMousePositionSet = false;
            }
            mRoot->releaseInput();
        }

        return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleMouseButtonDown(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;

        //Window* wndw = ((const WindowEventArgs&)e).window;
        if( ((const MouseEventArgs&)e).button == LeftButton )
        {
            mLMBDown = true;
        }

        if( ((const MouseEventArgs&)e).button == RightButton )
        {
            mRMBDown = true;
        }

        if( mLMBDown || mRMBDown )
        {
            MouseCursor::getSingleton().hide();
            if (!mLastMousePositionSet)
            {
                mLastMousePosition = MouseCursor::getSingleton().getPosition();
                mLastMousePositionSet = true;
            }
            mRoot->captureInput();
        }

        return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleMouseWheelEvent(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;
        mTranslateVector.z += ((const MouseEventArgs&)e).wheelChange * -5.0;
        mUpdateMovement = true;

        return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleKeyDownEvent(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;

        CheckMovementKeys( ((const KeyEventArgs&)e).scancode , true);

        return true;
    }

//--------------------------------------------------------------------------
    bool CompositorDemo_FrameListener::handleKeyUpEvent(const CEGUI::EventArgs& e)
    {
        using namespace CEGUI;
        CheckMovementKeys( ((const KeyEventArgs&)e).scancode, false );

        return true;
    }

//--------------------------------------------------------------------------
    void CompositorDemo_FrameListener::CheckMovementKeys( CEGUI::Key::Scan scancode, bool state )
    {
        using namespace CEGUI;

        switch ( scancode )
        {
            case Key::A:
                mMoveLeft = state;
                break;

            case Key::D:
                mMoveRight = state;
                break;

            case Key::S:
                mMoveBck = state;
                break;

            case Key::W:
                mMoveFwd = state;
                break;

            default:
                break;

        }

        mProcessMovement = mMoveLeft || mMoveRight || mMoveFwd || mMoveBck;

    }

//-----------------------------------------------------------------------------------
    void CompositorDemo_FrameListener::itemStateChanged(const size_t index, const bool state)
    {
        // get the item text and tell compositor manager to set enable state
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(mMain->getRenderWindow()->getViewport(0),
            mCompositorSelectorViewManager->getItemSelectorText(index), state);
		updateDebugRTTWindow();
    }
//-----------------------------------------------------------------------------------
    void CompositorDemo_FrameListener::registerCompositors(void)
    {
        Ogre::Viewport *vp = mMain->getRenderWindow()->getViewport(0);
        hvListener = new HeatVisionListener();
		hdrListener = new HDRListener();

        mCompositorSelectorViewManager = new ItemSelectorViewManager("CompositorSelectorWin");
        // tell view manager to notify us when an item changes selection state
        mCompositorSelectorViewManager->setItemSelectorController(this);
        //iterate through Compositor Managers resources and add name keys ast Item selectors to Compositor selector view manager
        Ogre::CompositorManager::ResourceMapIterator resourceIterator =
            Ogre::CompositorManager::getSingleton().getResourceIterator();

        // add all compositor resources to the view container
        while (resourceIterator.hasMoreElements())
        {
            Ogre::ResourcePtr resource = resourceIterator.getNext();
            const Ogre::String& compositorName = resource->getName();
            // Don't add base Ogre/Scene compositor to view
            if (compositorName == "Ogre/Scene")
                continue;

            mCompositorSelectorViewManager->addItemSelector(compositorName);
            Ogre::CompositorInstance *instance = Ogre::CompositorManager::getSingleton().addCompositor(vp, compositorName);
            Ogre::CompositorManager::getSingleton().setCompositorEnabled(vp, compositorName, false);
            // special handling for Heat Vision which uses a listener
            if(instance && (compositorName == "Heat Vision"))
                instance->addListener(hvListener);
			else if(instance && (compositorName == "HDR"))
			{
				instance->addListener(hdrListener);
				hdrListener->notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
				hdrListener->notifyCompositor(instance);

			}
        }
    }
	//---------------------------------------------------------------------
	void CompositorDemo_FrameListener::initDebugRTTWindow(void)
	{
		mDebugRTTStaticImage = static_cast<CEGUI::StaticImage*>(
			CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"DebugRTTImage"));
		mDebugRTTListbox = static_cast<CEGUI::Listbox*>(
			CEGUI::WindowManager::getSingleton().getWindow((CEGUI::utf8*)"DebugRTTListbox"));
		mDebugRTTListbox->subscribeEvent(CEGUI::Listbox::EventSelectionChanged, 
			CEGUI::Event::Subscriber(&CompositorDemo_FrameListener::handleRttSelection, this));
	}
	//---------------------------------------------------------------------
	bool CompositorDemo_FrameListener::handleRttSelection(const CEGUI::EventArgs& e)
	{
		if (mDebugRTTListbox->getSelectedCount() > 0)
		{
			// image set is in user data
			CEGUI::Imageset* imgSet = (CEGUI::Imageset*)mDebugRTTListbox->getFirstSelectedItem()->getUserData();
			
			mDebugRTTStaticImage->setImage(&imgSet->getImage("RttImage"));

		}
		else
		{
			mDebugRTTStaticImage->setImage(0);

		}
		return true;
	}
	//---------------------------------------------------------------------
	void CompositorDemo_FrameListener::updateDebugRTTWindow(void)
	{
		// Clear listbox
		mDebugRTTListbox->resetList();
		// Clear imagesets
		mDebugRTTStaticImage->setImage(0);
		for (ImageSetList::iterator isIt = mDebugRTTImageSets.begin(); 
			isIt != mDebugRTTImageSets.end(); ++isIt)
		{
			CEGUI::ImagesetManager::getSingleton().destroyImageset(*isIt);
		}
		mDebugRTTImageSets.clear();
		// Add an entry for each render texture for all active compositors
		Ogre::Viewport* vp = mMain->getRenderWindow()->getViewport(0);
		Ogre::CompositorChain* chain = Ogre::CompositorManager::getSingleton().getCompositorChain(vp);
		Ogre::CompositorChain::InstanceIterator it = chain->getCompositors();
		while (it.hasMoreElements())
		{
			Ogre::CompositorInstance* inst = it.getNext();
			if (inst->getEnabled())
			{
				Ogre::CompositionTechnique::TextureDefinitionIterator texIt = 
					inst->getTechnique()->getTextureDefinitionIterator();
				while (texIt.hasMoreElements())
				{
					Ogre::CompositionTechnique::TextureDefinition* texDef = texIt.getNext();

					// Get instance name of texture
					const Ogre::String& instName = inst->getTextureInstanceName(texDef->name);
					// Create CEGUI texture from name of OGRE texture
					CEGUI::Texture* tex = mMain->getGuiRenderer()->createTexture(instName);
					// Create imageset
					CEGUI::Imageset* imgSet = 
						CEGUI::ImagesetManager::getSingleton().createImageset(
							instName, tex);
					mDebugRTTImageSets.push_back(imgSet);
					imgSet->defineImage((CEGUI::utf8*)"RttImage", 
						CEGUI::Point(0.0f, 0.0f),
						CEGUI::Size(tex->getWidth(), tex->getHeight()),
						CEGUI::Point(0.0f,0.0f));


					CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem(texDef->name, 0, imgSet);
					item->setSelectionBrushImage("TaharezLook", "ListboxSelectionBrush");
					item->setSelectionColours(CEGUI::colour(0,0,1));
					mDebugRTTListbox->addItem(item);

				}

			}

		}

		

	}


