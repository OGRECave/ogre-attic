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
#ifndef __RenderQueueSortingGrouping_H__
#define __RenderQueueSortingGrouping_H__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreIteratorWrappers.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreMaterialManager.h"

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
        /** Internal struct reflecting a single Pass for a Renderable. 
        This is used to sort transparent objects.
        */
        struct RenderablePass
        {
            /// Pointer to the Renderable details
            Renderable* renderable;
            /// Pointer to the Pass
            Pass* pass;

            RenderablePass(Renderable* rend, Pass* p) :renderable(rend), pass(p) {}
        };

        friend class Ogre::SceneManager;
        /// Comparator to order non-transparent object passes
        struct SolidQueueItemLess
        {
            _OgreExport bool operator()(const Pass* a, const Pass* b) const
            {
                // Sort by passHash, which is pass, then texture unit changes
                unsigned long hasha = a->getHash();
                unsigned long hashb = a->getHash();
                if (hasha == hashb)
                {
                    // Must differentiate by pointer incase 2 passes end up with the same hash
                    return a < b;
                }
                else
                {
                    return hasha < hashb;
                }
            }
        };
        /// Comparator to order transparent object passes
        struct TransparentQueueItemLess
        {
            const Camera* camera;

            _OgreExport bool operator()(const RenderablePass& a, const RenderablePass& b) const
            {
                if (a.renderable == b.renderable)
                {
                    // Same renderable, sort by pass
                    return a.pass < b.pass;
                }
                else
                {
                    // Different renderables, sort by depth
                    Real adepth = a.renderable->getSquaredViewDepth(camera);
                    Real bdepth = b.renderable->getSquaredViewDepth(camera);
				    if (adepth == bdepth)
				    {
                        // Must return deterministic result, doesn't matter what
                        return a.pass < b.pass;
				    }
				    else
				    {
				        // Sort DESCENDING by depth (ie far objects first)
					    return (adepth > bdepth);
				    }
                }

            }
        };
    public:
        /** Vector of RenderablePass objects, this is built on the assumption that
         vectors only ever increase in size, so even if we do clear() the memory stays
         allocated, ie fast */
        typedef std::vector<RenderablePass> TransparentRenderablePassList;
        typedef std::vector<Renderable*> RenderableList;
        /** Map of pass to renderable lists, used for solid objects since ordering within objects not
        important. */
        typedef std::map<Pass*, RenderableList*, SolidQueueItemLess> SolidRenderablePassMap;
    protected:
        /// Solid pass list
        SolidRenderablePassMap mSolidPasses;
		/// Transparent list
		TransparentRenderablePassList mTransparentPasses;

        /** Storage of transparent RenderablePass structs, this is separate from list because
            it makes sorting faster */
        typedef std::vector<RenderablePass> TransparentRenderablePasses;
        TransparentRenderablePasses mRenderablePasses;

    public:
        RenderPriorityGroup() {}

        ~RenderPriorityGroup() {
            // destroy all the pass map entries
            SolidRenderablePassMap::iterator i, iend;
            iend = mSolidPasses.end();
            for (i = mSolidPasses.begin(); i != iend; ++i)
            {
                // Free the list associated with this pass
                delete i->second;
            }
            mSolidPasses.clear();
            mTransparentPasses.clear();

        }

        /** Add a renderable to this group. */
        void addRenderable(Renderable* pRend)
        {
            // Check material & technique supplied (the former since the default implementation
            // of getTechnique is based on it for backwards compatibility
            Technique* pTech;
            if(!pRend->getMaterial() || !pRend->getTechnique())
            {
                // Use default base white
                pTech = static_cast<Material*>(
                    MaterialManager::getSingleton().getByName("BaseWhite"))->getTechnique(0);
            }
            else
            {
                // Get technique
                pTech = pRend->getTechnique();
            }
            Technique::PassIterator pi = pTech->getPassIterator();

            if (pTech->isTransparent())
			{
                while (pi.hasMoreElements())
                {
				    // Insert into transparent list
				    mTransparentPasses.push_back(RenderablePass(pRend, pi.getNext()));
			    }
            }
			else
			{
                while (pi.hasMoreElements())
                {
                    // Insert into solid list
                    Pass* p = pi.getNext();
                    SolidRenderablePassMap::iterator i = mSolidPasses.find(p);
                    if (i == mSolidPasses.end())
                    {
                        std::pair<SolidRenderablePassMap::iterator, bool> retPair;
                        // Create new pass entry, build a new list
                        // Note that this pass and list are never destroyed until the engine
                        // shuts down, although the lists will be cleared
                        retPair = mSolidPasses.insert(
                            SolidRenderablePassMap::value_type(p, new RenderableList() ) );
                        assert(retPair.second && "Error inserting new pass entry into SolidRenderablePassMap");
                        i = retPair.first;
                    }
                    // Insert renderable
                    i->second->push_back(pRend);
			    }
            }

        }

		/** Sorts the objects which have been added to the queue; transparent objects by their 
            depth in relation to the passed in Camera. */
		void sort(const Camera* cam)
		{
            TransparentQueueItemLess transFunctor;
            transFunctor.camera = cam;

			std::stable_sort(mTransparentPasses.begin(), mTransparentPasses.end(), 
                transFunctor);
		}

		 

        /** Clears this group of renderables. 
        */
        void clear(void)
        {
            // NB we do not clear the solid pass map, only the contents of each list
            // This is because we assume passes are reused a lot and it saves resorting
            SolidRenderablePassMap::iterator i, iend;
            iend = mSolidPasses.end();
            for (i = mSolidPasses.begin(); i != iend; ++i)
            {
                // Clear the list associated with this pass, but leave the pass entry
                i->second->clear();
            }
            mTransparentPasses.clear();

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


