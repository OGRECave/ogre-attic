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

namespace Ogre {

    /** Class to manage the scene object rendering queue.
        @remarks
            Objects are grouped by material to minimise rendering state changes. The map from
            material to renderable object is wrapped in a class for ease of use.
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
    protected:
        typedef std::map<Material*, std::vector<Renderable*>, queueItemLess > RenderQueueMap;
        RenderQueueMap mQueue;
    public:
        /** Empty the queue - should only be called by SceneManagers.
        */
        void clear(void);

        /** Add a renderable object to the queue.
            @note
                Called by implementation of MovableObject::_updateRenderQueue.
        */
        void addRenderable(Renderable* pRend);
    };


}


#endif
