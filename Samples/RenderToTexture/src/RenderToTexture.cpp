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

/**
    \file 
        RenderToTexture.cpp
    \brief
        Shows OGRE's RenderToTexture, or scene blending features.
    \par
        Note that this is a little rudimentary - it's because whilst
        OGRE supports lots of blending options, the SceneManager has
        to ensure the rendering order is correct when object RenderToTexture
        is enabled. Right now this is not quite right in the default
        manager so this scene is kept deliberately simple.
*/

#include "Ogre.h"
#include "RenderToTexture.h"

int main(int argc, char ** argv)
{

    // Create application object
    RenderToTextureApplication app;

    app.go();

    return 0;
}
