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
#ifndef _DynLib_H__
#define _DynLib_H__

#include "OgrePrerequisites.h"

#include "OgreResource.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#    define DYNLIB_HANDLE hInstance
#    define DYNLIB_LOAD( a ) LoadLibrary( a )
#    define DYNLIB_GETSYM( a, b ) GetProcAddress( a, b )
#    define DYNLIB_UNLOAD( a ) !FreeLibrary( a )
#    define DYNLIB_ERROR( )  "Unknown Error"

struct HINSTANCE__;
typedef struct HINSTANCE__* hInstance;

#elif OGRE_PLATFORM == PLATFORM_LINUX || OGRE_PLATFORM == PLATFORM_APPLE
#    define DYNLIB_HANDLE void*
#    define DYNLIB_LOAD( a ) dlopen( a, RTLD_LAZY )
#    define DYNLIB_GETSYM( a, b ) dlsym( a, b )
#    define DYNLIB_UNLOAD( a ) dlclose( a )
#    define DYNLIB_ERROR( ) dlerror( )

#endif

namespace Ogre {

    /** Resource holding data about a dynamic library.
        @remarks
            This class holds the data required to get symbols from
            libraries loaded at run-time (i.e. from DLL's for so's)
        @author
            Adrian Cearnãu (cearny@cearny.ro)
        @since
            27 January 2002
        @see
            Resource
    */
    class _OgreExport DynLib : public Resource
    {
    public:
        /** Default constructor - used by DynLibManager.
            @warning
                Do not call directly
        */
        DynLib( const String& name );

        /** Default destructor.
        */
        ~DynLib();

        /** Generic load - called by DynLibManager.
            @see
                Resource::load
        */
        virtual void load();
        /** Generic unload - called by DynLibManager.
            @see
                Resource::unload
        */
        virtual void unload();

        /**
            Returns the adress of the given symbol from the loaded library.
            @param
                strName The name of the symbol to search for
            @returns
                If the function succeeds, the returned value is a handle to
                the symbol.
            @par
                If the function fails, the returned value is <b>NULL</b>.

        */
        virtual void* getSymbol( const String& strName ) const throw();

    protected:

        /// Handle to the loaded library.
        DYNLIB_HANDLE m_hInst;
    };

}

#endif
