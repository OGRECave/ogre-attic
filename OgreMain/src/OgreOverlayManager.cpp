/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#include "OgreOverlayManager.h"
#include "OgreStringVector.h"
#include "OgreOverlay.h"


namespace Ogre {

    //---------------------------------------------------------------------
    template<> OverlayManager *Singleton<OverlayManager>::ms_Singleton = 0;
    //---------------------------------------------------------------------
    OverlayManager::OverlayManager()
    {
    }
    //---------------------------------------------------------------------
    OverlayManager::~OverlayManager()
    {
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseOverlayFile(DataChunk& chunk)
    {
        // TODO
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseAllSources(const String& extension)
    {
        StringVector overlayFiles;
        DataChunk* pChunk;

        std::vector<ArchiveEx*>::iterator i = mVFS.begin();

        // Specific archives
        for (; i != mVFS.end(); ++i)
        {
            overlayFiles = (*i)->getAllNamesLike( "./", extension);
            for (StringVector::iterator si = overlayFiles.begin(); si!=overlayFiles.end(); ++si)
            {
                DataChunk dat; pChunk = &dat;
                (*i)->fileRead(si[0], &pChunk );
                parseOverlayFile(dat);
            }

        }
        // search common archives
        for (i = mCommonVFS.begin(); i != mCommonVFS.end(); ++i)
        {
            overlayFiles = (*i)->getAllNamesLike( "./", extension);
            for (StringVector::iterator si = overlayFiles.begin(); si!=overlayFiles.end(); ++si)
            {
                DataChunk dat; pChunk = &dat;
                (*i)->fileRead(si[0], &pChunk );
                parseOverlayFile(dat);
            }
        }
    }
    //---------------------------------------------------------------------
    Resource* OverlayManager::create( const String& name)
    {
        Overlay* s = new Overlay(name);
        load(s,1);
        return s;
    }
    //---------------------------------------------------------------------
    void OverlayManager::_queueOverlaysForRendering(Camera* cam, RenderQueue* pQueue)
    {
        ResourceMap::iterator i, iend;
        iend = mResources.end();
        for (i = mResources.begin(); i != iend; ++i)
        {
            Overlay* o = (Overlay*)i->second;
            o->_findVisibleObjects(cam, pQueue);
        }
    }
    //---------------------------------------------------------------------
    OverlayManager& OverlayManager::getSingleton(void)
    {
        return Singleton<OverlayManager>::getSingleton();
    }




}

