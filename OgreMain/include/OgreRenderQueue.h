/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
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
#ifndef __RenderQueue_H__
#define __RenderQueue_H__

#include "OgrePrerequisites.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

    /** Enumeration of queue groups, by which the application may group queued renderables
        so that they are rendered together with events in between
    */
    enum RenderQueueGroupID
    {
        /// Use this queue for objects which must be rendered first e.g. backgrounds
        RENDER_QUEUE_BACKGROUND = 0,
        /// First queue (after backgrounds), used for skyboxes if rendered first
        RENDER_QUEUE_1 = 1,
        RENDER_QUEUE_2 = 2,
        RENDER_QUEUE_3 = 3,
        RENDER_QUEUE_4 = 4,
        /// The default render queue
        RENDER_QUEUE_MAIN = 5,
        RENDER_QUEUE_6 = 6,
        RENDER_QUEUE_7 = 7,
        RENDER_QUEUE_8 = 8,
        /// Penultimate queue(before overlays), used for skyboxes if rendered last
        RENDER_QUEUE_9 = 9,
        /// Use this queue for objects which must be rendered last e.g. overlays
        RENDER_QUEUE_OVERLAY = 10
    };
    /** Class to manage the scene object rendering queue.
        @remarks
            Objects are grouped by material to minimise rendering state changes. The map from
            material to renderable object is wrapped in a class for ease of use.
        @par
            This class now includes the concept of 'queue groups' which allows the application
            adding the renderable to specifically schedule it so that it is included in 
            a discrete group. Good for separating renderables into the main scene,
            backgrounds and overlays, and also could be used in the future for more
            complex multipass routines like stenciling.
    */
    class _OgreExport RenderQueue
    {
        /** Comparator for material map, for sorting materials into render order (e.g. transparent last).
        */
        struct queueItemLess
        {
            _OgreExport bool operator()(const Material* x, const Material* y) const;
        };

        friend class SceneManager;
    public:
        /// Map on material within each queue group, outsort transparent
        typedef std::map<Material*, std::vector<Renderable*>, queueItemLess > RenderQueueMap;
        /// Map on queue group number overall, orders by ascending groupID
        typedef std::map< RenderQueueGroupID, RenderQueueMap* > RenderQueueGroup;
        /// Iterator over queue groups
        typedef MapIterator<RenderQueueGroup> QueueGroupIterator;
    protected:
        RenderQueueGroup mQueues;
    public:
        RenderQueue();
        virtual ~RenderQueue();

        /** Empty the queue - should only be called by SceneManagers.
        */
        void clear(void);

        /** Add a renderable object to the queue.
        @remarks
            This methods adds a Renderable to the queue, which will be rendered later by 
            the SceneManager.
        @note
            Called by implementation of MovableObject::_updateRenderQueue.
        @param
            pRend Pointer to the Renderable to be added to the queue
        @param
            groupID The group the renderable is to be added to. Leave this as default normally. It
            can be used to schedule renderable objects in separate groups such that the SceneManager
            respects the divisions between the groupings and does not reorder them outside these
            boundaries. This can be handy for overlays where no matter what you want the overlay to 
            be rendered last.
        */
        void addRenderable(Renderable* pRend, RenderQueueGroupID groupID = RENDER_QUEUE_MAIN);

        /** Internal method, returns an iterator for the queue groups. */
        QueueGroupIterator _getQueueGroupIterator(void);
    };


}


#endif
