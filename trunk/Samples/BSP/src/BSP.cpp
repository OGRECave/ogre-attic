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

/**
    \file 
        BSP.cpp
    \brief
        Shows the indoor level rendering (Binary Space Partition or BSP based).
    \par
        Also demonstrates loading levels from Quake3Arena and using
        curved bezier surfaces (as demonstrated in the Bezier example)
        in a large level.
*/

#include "Ogre.h"
#include "BSP.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char**argv)
#endif
{
    // Create application object
    BspApplication app;

    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << 
            e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
}
