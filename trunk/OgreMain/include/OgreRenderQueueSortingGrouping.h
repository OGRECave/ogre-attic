/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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

        /// Comparator to order non-transparent object passes
        struct SolidQueueItemLess
        {
            _OgreExport bool operator()(const Pass* a, const Pass* b) const
            {
                // Sort by passHash, which is pass, then texture unit changes
                unsigned long hasha = a->getHash();
                unsigned long hashb = b->getHash();
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
                    // Same renderable, sort by pass hash
                    return a.pass->getHash() < b.pass->getHash();
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
        /// Parent queue group
        RenderQueueGroup* mParent;
        bool mSplitPassesByLightingType;
        /// Solid pass list, used when no shadows, modulative shadows, or ambient passes for additive
        SolidRenderablePassMap mSolidPasses;
        /// Solid per-light pass list, used with additive shadows
        SolidRenderablePassMap mSolidPassesDiffuseSpecular;
        /// Solid decal (texture) pass list, used with additive shadows
        SolidRenderablePassMap mSolidPassesDecal;

		/// Transparent list
		TransparentRenderablePassList mTransparentPasses;

        /// Totally empties and destroys a solid pass map
        void destroySolidPassMap(SolidRenderablePassMap& passmap);

        /// remove a pass entry from all solid pass maps
        void removeSolidPassEntry(Pass* p);

        /// Clear a solid pass map at the end of a frame
        void clearSolidPassMap(SolidRenderablePassMap& passmap);
        /// Internal method for adding a solid renderable
        void addSolidRenderable(Technique* pTech, Renderable* rend);
        /// Internal method for adding a solid renderable
        void addSolidRenderableSplitByLightType(Technique* pTech, Renderable* rend);
        /// Internal method for adding a transparent renderable
        void addTransparentRenderable(Technique* pTech, Renderable* rend);

    public:
        RenderPriorityGroup(RenderQueueGroup* parent, bool splitPassesByLightingType) 
            :mParent(parent), mSplitPassesByLightingType(splitPassesByLightingType) { }

        ~RenderPriorityGroup() {
            // destroy all the pass map entries
            destroySolidPassMap(mSolidPasses);
            destroySolidPassMap(mSolidPassesDecal);
            destroySolidPassMap(mSolidPassesDiffuseSpecular);
            mTransparentPasses.clear();

        }

        /** Get the collection of solid passes currently queued */
        const SolidRenderablePassMap& _getSolidPasses(void) 
        { return mSolidPasses; }
        /** Get the collection of solid passes currently queued (per-light) */
        const SolidRenderablePassMap& _getSolidPassesDiffuseSpecular(void) 
        { return mSolidPassesDiffuseSpecular; }
        /** Get the collection of solid passes currently queued (decal textures)*/
        const SolidRenderablePassMap& _getSolidPassesDecal(void) 
        { return mSolidPassesDecal; }
        /** Get the collection of transparent passes currently queued */
        const TransparentRenderablePassList& _getTransparentPasses(void)
        { return mTransparentPasses; }



        /** Add a renderable to this group. */
        void addRenderable(Renderable* pRend);

		/** Sorts the objects which have been added to the queue; transparent objects by their 
            depth in relation to the passed in Camera. */
		void sort(const Camera* cam);

        /** Clears this group of renderables. 
        */
        void clear(void);

        /** Sets whether or not the queue will split passes by their lighting type,
        ie ambient, per-light and decal. 
        */
        void setSplitPassesByLightingType(bool split)
        {
            mSplitPassesByLightingType = split;
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
        RenderQueue* mParent;
        bool mSplitPassesByLightingType;
        /// Map of RenderPriorityGroup objects
        PriorityMap mPriorityGroups;
		/// Whether shadows are enabled for this queue
		bool mShadowsEnabled;


    public:
		RenderQueueGroup(RenderQueue* parent, bool splitPassesByLightingType) 
            :mParent(parent), mSplitPassesByLightingType(splitPassesByLightingType),
            mShadowsEnabled(true) {}

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
                pPriorityGrp = new RenderPriorityGroup(this, mSplitPassesByLightingType);
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

		/** Indicate whether a given queue group will be doing any
		shadow setup.
		@remarks
		This method allows you to inform the queue about a queue group, and to 
		indicate whether this group will require shadow processing of any sort.
		In order to preserve rendering order, OGRE has to treat queue groups
		as very separate elements of the scene, and this can result in it
		having to duplicate shadow setup for each group. Therefore, if you
		know that a group which you are using will never need shadows, you
		should preregister the group using this method in order to improve
		the performance.
		*/
		void setShadowsEnabled(bool enabled) { mShadowsEnabled = enabled; }

		/** Are shadows enabled for this queue? */
		bool getShadowsEnabled(void) { return mShadowsEnabled; }

        /** Sets whether or not the queue will split passes by their lighting type,
        ie ambient, per-light and decal. 
        */
        void setSplitPassesByLightingType(bool split)
        {
            mSplitPassesByLightingType = split;
            PriorityMap::iterator i, iend;
            iend = mPriorityGroups.end();
            for (i = mPriorityGroups.begin(); i != iend; ++i)
            {
                i->second->setSplitPassesByLightingType(split);
            }
        }

    };



}

#endif


