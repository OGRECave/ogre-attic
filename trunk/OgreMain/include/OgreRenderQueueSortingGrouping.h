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
#ifndef __RenderQueueSortingGrouping_H__
#define __RenderQueueSortingGrouping_H__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreIteratorWrappers.h"
#include "OgreMaterial.h"

namespace Ogre {

    /** Renderables in the queue grouped by priority.
    @remarks
        This class simply groups renderables for rendering. All the 
        renderables contained in this class are destined for the same
        RenderQueueGroup (coarse groupings like those between the main
        scene and overlays) and have the same priority (fine groupings
        for detailed overlap control).
    @par
        This class optimises the grouped renderables by sorting them by
        material to reduce render state changes, and outsorts transparent
        objects.
    */
    class RenderPriorityGroup
    {
        friend class Ogre::SceneManager;
        /** Comparator for material map, for sorting materials into render order (e.g. transparent last).
        */
        struct queueItemLess
        {
            _OgreExport bool operator()(const Material* x, const Material* y) const
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
        };
    public:
        /// Map on material within each queue group, outsort transparent
        typedef std::vector<Renderable*> RenderableList;
        typedef std::map<Material*, RenderableList, queueItemLess > MaterialGroupMap;
    protected:
        MaterialGroupMap mMaterialGroups;

    public:
        RenderPriorityGroup() {}

        ~RenderPriorityGroup() {}

        /** Add a renderable to this group. */
        void addRenderable(Renderable* pRend)
        {
            std::pair<MaterialGroupMap::iterator, bool> retPair;
            RenderableList newList;

            Material* pMat = pRend->getMaterial();
            // Try to insert, if already there existing will be returned
            retPair = mMaterialGroups.insert(MaterialGroupMap::value_type(pMat, newList));

            // Insert new Renderable
            // retPair.first is iterator on map (Material*, std::vector)
            // retPair.first->second is the vector of renderables
            retPair.first->second.push_back(pRend);

        }

        /** Clears this group of renderables. 
        */
        void clear(void)
        {
            mMaterialGroups.clear();

        }



    };


    /** A grouping level underneath RenderQueue which groups renderables
    to be issued at coarsely the same time to the renderer.
    @remarks
        Each instance of this class itself hold RenderPriorityGroup instances, 
        which are the groupings of renderables by priority for fine control
        of ordering (not required for most instances).
    @par
        This is an internal OGRE class, not intended for apps to use.
    */
    class RenderQueueGroup
    {
    public:
        typedef std::map<ushort, RenderPriorityGroup*, std::less<ushort> > PriorityMap;
        typedef MapIterator<PriorityMap> PriorityMapIterator;
    protected:
        // Map of RenderPriorityGroup objects
        PriorityMap mPriorityGroups;


    public:
        RenderQueueGroup() {}

        ~RenderQueueGroup() {
            // destroy contents now
            PriorityMap::iterator i;
            for (i = mPriorityGroups.begin(); i != mPriorityGroups.end(); ++i)
            {
                delete i->second;
            }
        }

        /** Get an iterator for browsing through child contents. */
        PriorityMapIterator getIterator(void)
        {
            return PriorityMapIterator(mPriorityGroups.begin(), mPriorityGroups.end());
        }

        /** Add a renderable to this group, with the given priority. */
        void addRenderable(Renderable* pRend, ushort priority)
        {
            // Check if priority group is there
            PriorityMap::iterator i = mPriorityGroups.find(priority);
            RenderPriorityGroup* pPriorityGrp;
            if (i == mPriorityGroups.end())
            {
                // Missing, create
                pPriorityGrp = new RenderPriorityGroup();
                mPriorityGroups.insert(PriorityMap::value_type(priority, pPriorityGrp));
            }
            else
            {
                pPriorityGrp = i->second;
            }

            // Add
            pPriorityGrp->addRenderable(pRend);

        }

        /** Clears this group of renderables. 
        @remarks
            Doesn't delete any priority groups, just empties them. Saves on 
            memory deallocations since the chances are rougly the same kinds of 
            renderables are going to be sent to the queue again next time.
        */
        void clear(void)
        {
            PriorityMap::iterator i, iend;
            iend = mPriorityGroups.end();
            for (i = mPriorityGroups.begin(); i != iend; ++i)
            {
                i->second->clear();
            }

        }


    };



}

#endif


