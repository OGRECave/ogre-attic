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
#ifndef __OverlayManager_H__
#define __OverlayManager_H__

#include "OgrePrerequisites.h"
#include "OgreResourceManager.h"
#include "OgreSingleton.h"

namespace Ogre {

    /** Manages Overlay objects, parsing them from .overlay files and
        storing a lookup library of them.
    */
    class _OgreExport OverlayManager : public ResourceManager, public Singleton<OverlayManager>
    {
    public:
        OverlayManager();
        virtual ~OverlayManager();

        /** Parses an overlay file passed as a chunk. */
        void parseOverlayFile(DataChunk& chunk);
        /** Parses all overlay files in resource folders & archives. */
        void parseAllSources(const String& extension = ".overlay");


        /** Create implementation required by ResourceManager. */
        virtual Resource* create( const String& name);

        /** Internal method for queueing the visible overlays for rendering. */
        void _queueOverlaysForRendering(Camera* cam, RenderQueue* pQueue);


        

        /** Override standard Singleton retrieval.
            Why do we do this? Well, it's because the Singleton implementation is in a .h file,
            which means it gets compiled into anybody who includes it. This is needed for the Singleton
            template to work, but we actually only want it compiled into the implementation of the
            class based on the Singleton, not all of them. If we don't change this, we get link errors
            when trying to use the Singleton-based class from an outside dll.</p>
            This method just delegates to the template version anyway, but the implementation stays in this
            single compilation unit, preventing link errors.
        */
        static OverlayManager& getSingleton(void);




    };



}


#endif 
