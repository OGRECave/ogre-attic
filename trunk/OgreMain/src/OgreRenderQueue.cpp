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

#include "OgreRenderQueue.h"

#include "OgreRenderable.h"
#include "OgreMaterial.h"
#include "OgreRenderQueueSortingGrouping.h"

namespace Ogre {

    //---------------------------------------------------------------------
    RenderQueue::RenderQueue()
    {
        // Create the 'main' queue up-front since we'll always need that
        mGroups.insert(RenderQueueGroupMap::value_type(RENDER_QUEUE_MAIN, new RenderQueueGroup()));

        // set default queue
        mDefaultQueueGroup = RENDER_QUEUE_MAIN;

    }
    //---------------------------------------------------------------------
    RenderQueue::~RenderQueue()
    {
        // Destroy the queues for good
        RenderQueueGroupMap::iterator i, iend;
        i = mGroups.begin();
        iend = mGroups.end();
        for (; i != iend; ++i)
        {
            delete i->second;
        }
        mGroups.clear();


    }
    //-----------------------------------------------------------------------
    void RenderQueue::addRenderable(Renderable* pRend, RenderQueueGroupID groupID, ushort priority)
    {
        // Find group
        RenderQueueGroupMap::iterator groupIt;
        RenderQueueGroup* pGroup;

        groupIt = mGroups.find(groupID);
        if (groupIt == mGroups.end())
        {
            // Insert new
            pGroup = new RenderQueueGroup();
            mGroups.insert(RenderQueueGroupMap::value_type(groupID, pGroup));
        }
        else
        {
            pGroup = groupIt->second;
        }

        pGroup->addRenderable(pRend, priority);

    }
    //-----------------------------------------------------------------------
    void RenderQueue::clear(void)
    {
        // Clear the queues
        RenderQueueGroupMap::iterator i, iend;
        i = mGroups.begin();
        iend = mGroups.end();
        for (; i != iend; ++i)
        {
            i->second->clear();
        }

        // NB this leaves the items present (but empty)
        // We're assuming that frame-by-frame, the same groups are likely to 
        //  be used, so no point destroying the vectors and incurring the overhead
        //  that would cause, let them be destroyed in the destructor.
    }
    //-----------------------------------------------------------------------
    RenderQueue::QueueGroupIterator RenderQueue::_getQueueGroupIterator(void)
    {
        return QueueGroupIterator(mGroups.begin(), mGroups.end());
    }
    //-----------------------------------------------------------------------
    void RenderQueue::addRenderable(Renderable* pRend, ushort priority)
    {
        addRenderable(pRend, mDefaultQueueGroup, priority);
    }
    //-----------------------------------------------------------------------
    RenderQueueGroupID RenderQueue::getDefaultQueueGroup(void)
    {
        return mDefaultQueueGroup;
    }
    //-----------------------------------------------------------------------
    void RenderQueue::setDefaultQueueGroup(RenderQueueGroupID grp)
    {
        mDefaultQueueGroup = grp;
    }
}

