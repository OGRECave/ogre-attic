/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreRenderTarget.h"
#include "OgreGuiElement.h"
#include "OgreGuiManager.h"
#include "OgreStringConverter.h"

#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRenderTargetListener.h"
#include "OgrePlatformManager.h"
#include "OgreRoot.h"

namespace Ogre {

    RenderTarget::RenderTarget()
    {
        // Default to no stats display
        mStatFlags = SF_NONE;
        mActive = true;
        mPriority = OGRE_DEFAULT_RT_GROUP;
        mTimer = Root::getSingleton().getTimer();
        resetStatistics();
    }

    RenderTarget::~RenderTarget()
    {
        // Delete viewports
        for (ViewportList::iterator i = mViewportList.begin();
            i != mViewportList.end(); ++i)
        {
            delete (*i).second;
        }


        // Write closing message
        LogManager::getSingleton().logMessage(
            LML_NORMAL,
            "Render Target '%s' Average FPS: %f Best FPS: %f Worst FPS: %f", 
            mName.c_str(), mAvgFPS, mBestFPS, mWorstFPS );

    }

    String RenderTarget::getName(void)
    {
        return mName;
    }


    void RenderTarget::getMetrics(int &width, int &height, int &colourDepth)
    {
        width = mWidth;
        height = mHeight;
        colourDepth = mColourDepth;
    }

    int RenderTarget::getWidth(void)
    {
        return mWidth;
    }
    int RenderTarget::getHeight(void)
    {
        return mHeight;
    }
    int RenderTarget::getColourDepth(void)
    {
        return mColourDepth;
    }

    void RenderTarget::update(void)
    {

        // notify listeners (pre)
        firePreUpdate();

        mTris = 0;
        // Go through viewports in Z-order
        // Tell each to refresh
        ViewportList::iterator it = mViewportList.begin();
        while (it != mViewportList.end())
        {
            fireViewportPreUpdate((*it).second);
            (*it).second->update();
            mTris += (*it).second->_getNumRenderedFaces();
            fireViewportPostUpdate((*it).second);
			++it;
        }

        // notify listeners (post)
        firePostUpdate();

        // Update statistics (always on top)
        updateStats();


    }

    Viewport* RenderTarget::addViewport(Camera* cam, int ZOrder, float left, float top ,
        float width , float height)
    {
        // Check no existing viewport with this Z-order
        ViewportList::iterator it = mViewportList.find(ZOrder);

        if (it != mViewportList.end())
        {
            char msg[256];
            sprintf(msg, "Can't create another viewport for %s with Z-Order %i "
                " because a viewport exists with this Z-Order already.",
                this->getName().c_str(), ZOrder);
            Except(9999, msg, "RenderTarget::addViewport");
        }
        // Add viewport to list
        // Order based on Z-Order
        Viewport* vp = new Viewport(cam, this, left, top, width, height, ZOrder);

        mViewportList.insert(ViewportList::value_type(ZOrder, vp));

        return vp;
    }

    void RenderTarget::removeViewport(int ZOrder)
    {
        ViewportList::iterator it = mViewportList.find(ZOrder);

        if (it != mViewportList.end())
        {
            delete (*it).second;
            mViewportList.erase(ZOrder);
        }
    }

    void RenderTarget::removeAllViewports(void)
    {


        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            delete (*it).second;
        }

        mViewportList.clear();

    }
    void RenderTarget::setStatsDisplay(StatFlags sf)
    {
        mStatFlags = sf;
    }

    void RenderTarget::getStatistics(float& lastFPS, float& avgFPS,
            float& bestFPS, float& worstFPS)
    {

        // Note - the will have been updated by the last render
        lastFPS = mLastFPS;
        avgFPS = mAvgFPS;
        bestFPS = mBestFPS;
        worstFPS = mWorstFPS;


    }

    float RenderTarget::getLastFPS()
    {
        return mLastFPS;
    }
    float RenderTarget::getAverageFPS()
    {
        return mAvgFPS;
    }
    float RenderTarget::getBestFPS()
    {
        return mBestFPS;
    }
    float RenderTarget::getWorstFPS()
    {
        return mWorstFPS;
    }


    void RenderTarget::resetStatistics(void)
    {
        mAvgFPS = 0.0;
        mBestFPS = 0.0;
        mLastFPS = 0.0;
        mLastFPS = 0.0;
        mWorstFPS = 999.0;
		mBestFrameTime = 999.0;
		mWorstFrameTime = 0.0;
    }

    void RenderTarget::updateStats(void)
    {
        static unsigned long lastSecond = 0; // in ms
		static unsigned long lastTime = 0; 
        static long numFrames  = 0;
		static bool firstRun = true ;
		static unsigned long bestFrameTime = 999999;
		static unsigned long worstFrameTime = 0;
		bool needUpdate ;

		if (firstRun) { 
			firstRun = false ;
			needUpdate = true ;
		} else { 
			// measure statistics
			needUpdate = false ;
        ++numFrames;
			unsigned long thisTime = mTimer->getMilliseconds();

			// check frame time
			unsigned long frameTime = thisTime - lastTime ;
			if (frameTime > worstFrameTime)
				worstFrameTime = frameTime ;
			if (frameTime < bestFrameTime)
				bestFrameTime = frameTime ;
			lastTime = thisTime ;
			
			// check if new second
			if (thisTime - lastSecond > 1000) { 
				// new second - not 100% precise
				needUpdate = true ;
				mLastFPS = (float)numFrames / (float)(thisTime - lastSecond) * 1000.0;

                if (mAvgFPS == 0)
                    mAvgFPS = mLastFPS;
                else
                    mAvgFPS = (mAvgFPS + mLastFPS) / 2;

                if (mBestFPS < mLastFPS)
                    mBestFPS = mLastFPS;

                if (mWorstFPS > mLastFPS)
                    mWorstFPS = mLastFPS;

				lastSecond = thisTime ;
            numFrames  = 0;

				mBestFrameTime = bestFrameTime ;
				mWorstFrameTime = worstFrameTime ;
				
				bestFrameTime = 999999;
				worstFrameTime = 0 ;
			}
		}

        static String currFps = "Current FPS: ";
        static String avgFps = "Average FPS: ";
        static String bestFps = "Best FPS: ";
        static String worstFps = "Worst FPS: ";
        static String tris = "Triangle Count: ";

		if (needUpdate) {
			// update stats when necessary
        GuiElement* guiAvg = GuiManager::getSingleton().getGuiElement("Core/AverageFps");
        GuiElement* guiCurr = GuiManager::getSingleton().getGuiElement("Core/CurrFps");
        GuiElement* guiBest = GuiManager::getSingleton().getGuiElement("Core/BestFps");
        GuiElement* guiWorst = GuiManager::getSingleton().getGuiElement("Core/WorstFps");

        guiAvg->setCaption(avgFps + StringConverter::toString(mAvgFPS));
        guiCurr->setCaption(currFps + StringConverter::toString(mLastFPS));
	        guiBest->setCaption(bestFps + StringConverter::toString(mBestFPS)
				+" "+StringConverter::toString(mBestFrameTime)+" ms");
	        guiWorst->setCaption(worstFps + StringConverter::toString(mWorstFPS)
				+" "+StringConverter::toString(mWorstFrameTime)+" ms");
		}

        GuiElement* guiTris = GuiManager::getSingleton().getGuiElement("Core/NumTris");

        guiTris->setCaption(tris + StringConverter::toString(mTris));

        GuiElement* guiDbg = GuiManager::getSingleton().getGuiElement("Core/DebugText");
        guiDbg->setCaption(mDebugText);

    }

    void RenderTarget::getCustomAttribute(String name, void* pData)
    {
        Except(Exception::ERR_INVALIDPARAMS, "Attribute not found.", "RenderTarget::getCustomAttribute");
    }
    //-----------------------------------------------------------------------
    void RenderTarget::setDebugText(const String& text)
    {
        mDebugText = text;
    }
	//-----------------------------------------------------------------------
	const String & RenderTarget::getDebugText() const
	{ 
		return mDebugText; 
	}
    //-----------------------------------------------------------------------
    void RenderTarget::addListener(RenderTargetListener* listener)
    {
        mListeners.push_back(listener);
    }
    //-----------------------------------------------------------------------
    void RenderTarget::removeListener(RenderTargetListener* listener)
    {
        RenderTargetListenerList::iterator i;
        for (i = mListeners.begin(); i != mListeners.end(); ++i)
        {
            if (*i == listener)
            {
                mListeners.erase(i);
                break;
            }
        }

    }
    //-----------------------------------------------------------------------
    void RenderTarget::removeAllListeners(void)
    {
        mListeners.clear();
    }
    //-----------------------------------------------------------------------
    void RenderTarget::firePreUpdate(void)
    {
        RenderTargetEvent evt;
        evt.source = this;

        RenderTargetListenerList::iterator i, iend;
        i = mListeners.begin();
        iend = mListeners.end();
        for(; i != iend; ++i)
        {
            (*i)->preRenderTargetUpdate(evt);
        }


    }
    //-----------------------------------------------------------------------
    void RenderTarget::firePostUpdate(void)
    {
        RenderTargetEvent evt;
        evt.source = this;

        RenderTargetListenerList::iterator i, iend;
        i = mListeners.begin();
        iend = mListeners.end();
        for(; i != iend; ++i)
        {
            (*i)->postRenderTargetUpdate(evt);
        }
    }
    //-----------------------------------------------------------------------
    unsigned short RenderTarget::getNumViewports(void)
    {
        return (unsigned short)mViewportList.size();

    }
    //-----------------------------------------------------------------------
    Viewport* RenderTarget::getViewport(unsigned short index)
    {
        assert (index < mViewportList.size() && "Index out of bounds");

        ViewportList::iterator i = mViewportList.begin();
        while (index--)
            ++i;
        return i->second;
    }
    //-----------------------------------------------------------------------
    bool RenderTarget::isActive() const
    {
        return mActive;
    }
    //-----------------------------------------------------------------------
    void RenderTarget::setActive( bool state )
    {
        mActive = state;
    }
    //-----------------------------------------------------------------------
    void RenderTarget::fireViewportPreUpdate(Viewport* vp)
    {
        RenderTargetViewportEvent evt;
        evt.source = vp;

        RenderTargetListenerList::iterator i, iend;
        i = mListeners.begin();
        iend = mListeners.end();
        for(; i != iend; ++i)
        {
            (*i)->preViewportUpdate(evt);
        }
    }
    //-----------------------------------------------------------------------
    void RenderTarget::fireViewportPostUpdate(Viewport* vp)
    {
        RenderTargetViewportEvent evt;
        evt.source = vp;

        RenderTargetListenerList::iterator i, iend;
        i = mListeners.begin();
        iend = mListeners.end();
        for(; i != iend; ++i)
        {
            (*i)->postViewportUpdate(evt);
        }
    }

    void RenderTarget::writeContentsToTimestampedFile(const String& filenamePrefix, const String& filenameSuffix)
    {
	struct tm *pTime;
        time_t ctTime; time(&ctTime);
        pTime = localtime( &ctTime );
        std::ostringstream oss;
	oss	<< std::setw(2) << std::setfill('0') << pTime->tm_mon
		<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_mday
		<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_year
		<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_hour
        	<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_min
        	<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_sec
		<< ":" << std::setw(3) << std::setfill('0') << (mTimer->getMilliseconds() % 1000);
	    writeContentsToFile(filenamePrefix + String(oss.str()) + filenameSuffix);
	    
    }
}        
