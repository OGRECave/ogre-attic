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
http://www.gnu.org/copyleft/lgpl.html.
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

namespace Ogre {

    RenderTarget::RenderTarget()
    {
        // Default to no stats display
        mStatFlags = SF_NONE;
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
            (*it).second->update();
            mTris += (*it++).second->_getNumRenderedFaces();

        }

        // notify listeners (post)
        firePostUpdate();

        // Update statistics (always on top)
        updateStats();


    }

    Viewport* RenderTarget::addViewport(Camera* cam, int ZOrder, float left, float top ,
        float width , float height)
    {
        char msg[100];
        // Check no existing viewport with this Z-order
        ViewportList::iterator it = mViewportList.find(ZOrder);

        if (it != mViewportList.end())
        {
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
    }

    void RenderTarget::updateStats(void)
    {
        static float lastTime = 0.0f;
        static long numFrames  = 0;

        // Keep track of the time lapse and frame count
        float fTime = ((float)clock())/CLOCKS_PER_SEC; // Get current time in seconds
        ++numFrames;

        // Update the frame rate once per second
        if( fTime - lastTime > 1.0f )
        {
            // Don't update stats first time (first time includes init times)
            if (lastTime)
            {
                mLastFPS = numFrames / (fTime - lastTime);
                if (mAvgFPS == 0)
                    mAvgFPS = mLastFPS;
                else
                    mAvgFPS = (mAvgFPS + mLastFPS) / 2;

                if (mBestFPS < mLastFPS)
                    mBestFPS = mLastFPS;

                if (mWorstFPS > mLastFPS)
                    mWorstFPS = mLastFPS;
            }


            lastTime = fTime;
            numFrames  = 0;
        }


        static String currFps = "CURRENT FPS: ";
        static String avgFps = "AVERAGE FPS: ";
        static String bestFps = "BEST FPS: ";
        static String worstFps = "WORST FPS: ";
        static String tris = "#TRIS: ";

        GuiElement* guiAvg = GuiManager::getSingleton().getGuiElement("Core/AverageFps");
        GuiElement* guiCurr = GuiManager::getSingleton().getGuiElement("Core/CurrFps");
        GuiElement* guiBest = GuiManager::getSingleton().getGuiElement("Core/BestFps");
        GuiElement* guiWorst = GuiManager::getSingleton().getGuiElement("Core/WorstFps");
        GuiElement* guiTris = GuiManager::getSingleton().getGuiElement("Core/NumTris");

        guiAvg->setCaption(avgFps + StringConverter::toString(mAvgFPS));
        guiCurr->setCaption(currFps + StringConverter::toString(mLastFPS));
        guiBest->setCaption(bestFps + StringConverter::toString(mBestFPS));
        guiWorst->setCaption(worstFps + StringConverter::toString(mWorstFPS));
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

}        
