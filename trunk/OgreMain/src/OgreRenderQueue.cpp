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

#include "OgreRenderQueue.h"

#include "OgreRenderable.h"
#include "OgreMaterial.h"

namespace Ogre {

    //---------------------------------------------------------------------
    RenderQueue::RenderQueue()
    {
        // Create the 'main' queue up-front since we'll always need that
        mQueues.insert(RenderQueueGroup::value_type(RENDER_QUEUE_MAIN, new RenderQueueMap()));

    }
    //---------------------------------------------------------------------
    RenderQueue::~RenderQueue()
    {
        // Destroy the queues for good
        RenderQueueGroup::iterator i, iend;
        i = mQueues.begin();
        iend = mQueues.end();
        for (; i != iend; ++i)
        {
            delete i->second;
        }
        mQueues.clear();


    }
    //---------------------------------------------------------------------
    bool RenderQueue::queueItemLess::operator() (const Material* x, const Material* y) const
    {
        // If x transparent and y not, x > y (since x has to overlap y)
        if (x->isTransparent() && !y->isTransparent())
        {
            return false;
        }
        // If y is transparent and x not, x < y
        else if (!x->isTransparent() && y->isTransparent())
        {
            return true;
        }
        else
        {
            // Otherwise don't care (both transparent or both solid)
            // Just arbitrarily use pointer
            return x < y;
        }

    }
    //-----------------------------------------------------------------------
    void RenderQueue::addRenderable(Renderable* pRend, RenderQueueGroupID groupID)
    {
        // Find group first 
        RenderQueueGroup::iterator groupIt;
        RenderQueueMap* pQueue;

        groupIt = mQueues.find(groupID);
        if (groupIt == mQueues.end())
        {
            // Insert new
            pQueue = new RenderQueueMap();
            mQueues.insert(RenderQueueGroup::value_type(groupID, pQueue));
        }
        else
        {
            pQueue = groupIt->second;
        }

        // Try to insert vector of renderables by map
        // This will just return an iterator to the existing vector
        // if already there which is OK
        Material* m = pRend->getMaterial();
        // Find material (if exists in map)
        std::pair<RenderQueueMap::iterator, bool> queuePair;
        std::vector<Renderable*> newVec;

        queuePair = pQueue->insert(RenderQueueMap::value_type(m, newVec));
        // Insert new Renderable
        // queuePair.first is iterator on map (Material*, std::vector)
        // queuePair.first->second is the vector of renderables
        queuePair.first->second.push_back(pRend);
    }
    //-----------------------------------------------------------------------
    void RenderQueue::clear(void)
    {
        // Clear the queues
        RenderQueueGroup::iterator i, iend;
        i = mQueues.begin();
        iend = mQueues.end();
        for (; i != iend; ++i)
        {
            i->second->clear();
        }

        // NB this leaves the RenderQueueMap items present (but empty)
        //  and also leaves the entries in the groupID
        // We're assuming that frame-by-frame, the same groups are likely to 
        //  be used, so no point destroying the vectors and incurring the overhead
        //  that would cause, let them be destroyed in the destructor.
    }
    //-----------------------------------------------------------------------
    RenderQueue::QueueGroupIterator RenderQueue::_getQueueGroupIterator(void)
    {
        return QueueGroupIterator(mQueues.begin(), mQueues.end());
    }
}

