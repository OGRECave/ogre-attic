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
    void RenderQueue::addRenderable(Renderable* pRend)
    {
        Material* m = pRend->getMaterial();
        // Find material (if exists in map)
        std::pair<RenderQueueMap::iterator, bool> insRet;
        /*
        RenderQueueMap::iterator i = mQueue.find(m);
        if (i = mQueue.end())
        {
            // Not there yet, insert new material entry
            i = mQueue.insert(RenderQueueMap::value_type(m, newVec));
        }
        */
        std::vector<Renderable*> newVec;
        insRet = mQueue.insert(RenderQueueMap::value_type(m, newVec));
        // Insert new Renderable
        insRet.first->second.push_back(pRend);
    }
    //-----------------------------------------------------------------------
    void RenderQueue::clear(void)
    {
        mQueue.clear();
    }
}

