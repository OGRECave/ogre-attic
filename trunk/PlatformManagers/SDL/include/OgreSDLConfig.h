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

#ifndef __CLICONFIGDIALOG_H__
#define __CLICONFIGDIALOG_H__

#include "OgreConfigDialog.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include <iostream>

namespace Ogre {
    /** Simple CLI config */
    class SDLConfig : public ConfigDialog
    {
    public:
        SDLConfig()
        { }

        /** 
         * Displays a message about reading the config and then attempts to
         * read it from a config file
         */
        bool display(void)
        {
            RenderSystemList* lstRend;
            RenderSystemList::iterator pRend;
            std::cout << "*** Reading config file" << std::endl;
            lstRend = Root::getSingleton().getAvailableRenderers();
            pRend = lstRend->begin();
            std::cout << "Renderer: " << (*pRend)->getName() << std::endl;
            Root::getSingleton().setRenderSystem((*pRend));
            Root::getSingleton().saveConfig();
            
            return true;
        }
    };
}

#endif
