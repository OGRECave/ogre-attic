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
#include "OgreDynLib.h"

#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    DynLib::DynLib( const String& name )
    {
        OgreGuard("DynLib::DynLib");

        mName = name;
        mIsLoaded = false;
        m_hInst = NULL;

        OgreUnguard();
    }

    //-----------------------------------------------------------------------
    DynLib::~DynLib()
    {
        if( mIsLoaded )
            unload();
    }

    //-----------------------------------------------------------------------
    void DynLib::load()
    {
        OgreGuard("DynLib::load");

        // Log library load
        LogManager::getSingleton().logMessage("Loading library " + mName);

        m_hInst = (DYNLIB_HANDLE)DYNLIB_LOAD( mName.c_str() );

        if( !m_hInst )
            Except(
                Exception::ERR_INTERNAL_ERROR, 
                "Could not load dynamic library " + mName,
                "DynLib::load" );

        mIsLoaded = true;

        OgreUnguard();
    }

    //-----------------------------------------------------------------------
    void DynLib::unload()
    {
        OgreGuard("DynLib::unload");

        // Log library unload
        if (mIsLoaded)
        {
            LogManager::getSingleton().logMessage("Unloading library " + mName);

            if( DYNLIB_UNLOAD( m_hInst ) )
                Except(
                    Exception::ERR_INTERNAL_ERROR, 
                    "Could not unload dynamic library " + mName,
                    "DynLib::unload");
        }
        mIsLoaded = false;

        OgreUnguard();
    }

    //-----------------------------------------------------------------------
    void* DynLib::getSymbol( const String& strName ) const throw()
    {
        return DYNLIB_GETSYM( m_hInst, strName.c_str() );
    }
}
