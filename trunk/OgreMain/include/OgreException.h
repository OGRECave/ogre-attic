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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
#ifndef __Exception_H_
#define __Exception_H_

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreString.h"

#define Except( a, b, c ) throw( Exception( a, b, c, __FILE__, __LINE__ ) )

// Stack unwinding options
#if OGRE_STACK_UNWINDING == 1
#   if OGRE_COMPILER != COMPILER_BORL
#       define OgreGuard( a ) Exception::_pushFunction( (a) )
#   else
#       define OgreGuard( a ) Exception::_pushFunction( __FUNC__ )
#   endif

#   define OgreUnguard() Exception::_popFunction()
#   define OgreUnguardRet( a ) { Exception::_popFunction(); return a; }

#else
#   define OgreGuard( a )
#   define OgreUnguard()
#   define OgreUnguardRet( a ) return a;

#endif

// Release asserts - throw exceptions
#if OGRE_RELEASE_ASSERT == 1
#   ifdef _DEBUG
#       define OgreAssert( a, b ) assert( (a) && (b) )

#   else
#       if OGRE_COMP != COMP_BORL
#           define OgreAssert( a, b ) if( !(a) ) Except( Exception::ERR_RT_ASSERTION_FAILED, (b), "no function info" )
#       else
#           define OgreAssert( a, b ) if( !(a) ) Except( Exception::ERR_RT_ASSERTION_FAILED, (b), __FUNC__ )
#       endif

#   endif

#else
#   define OgreAssert( a, b ) assert( (a) && (b) )

#endif

namespace Ogre {
    /** When thrown, provides information about an error that has occurred inside the engine.
        @remarks
            OGRE never uses return values to indicate errors. Instead, if an
            error occurs, an exception is thrown, and this is the object that
            encapsulates the detail of the problem. The application using
            OGRE should always ensure that the exceptions are caught, so all
            OGRE engine functions should occur within a
            try{} catch(Ogre::Exception& e) {} block.
        @par
            The user application should never create any instances of this
            object unless it wishes to unify its error handling using the
            same object.
    */
    class _OgreExport Exception
    {
    protected:
        long line;
        int number;
        String description;
        String source;
        String file;
        static Exception* last;
        static std::list<String> msFunctionStack;
    public:
        /** Static definitions of error codes.
            @todo
                Add many more exception codes, since we want the user to be able
                to catch most of them.
        */
        enum ExceptionCodes {
            UNIMPLEMENTED_FEATURE,
            ERR_CANNOT_WRITE_TO_FILE,
            ERR_NO_RENDERSYSTEM_SELECTED,
            ERR_DIALOG_OPEN_ERROR,
            ERR_INVALIDPARAMS,
            ERR_RENDERINGAPI_ERROR,
            ERR_DUPLICATE_ITEM,
            ERR_ITEM_NOT_FOUND,
            ERR_FILE_NOT_FOUND,
            ERR_INTERNAL_ERROR,
            ERR_RT_ASSERTION_FAILED
        };

        /** Default constructor.
        */
        Exception( int number, const String& description, const String& source );

        /** Advanced constructor.
        */
        Exception( int number, const String& description, const String& source, char* file, long line );

        /** Copy constructor.
        */
        Exception(const Exception& rhs);

        /** Assignment operator.
        */
        void operator = (const Exception& rhs);

        /** Returns a string with the full description of this error.
            @remarks
                The description contains the error number, the description
                supplied by the thrower, what routine threw the exception,
                and will also supply extra platform-specific information
                where applicable. For example - in the case of a rendering
                library error, the description of the error will include both
                the place in which OGRE found the problem, and a text
                description from the 3D rendering library, if available.
        */
        String getFullDescription(void) const;

        /** Gets the error code.
        */
        int getNumber(void) throw();

        /** Retrieves a pointer to the last exception created.
        */
        static Exception* getLastException(void) throw();

        /** Pushes a function on the stack.
        */
        static void _pushFunction( const String& strFuncName ) throw();
        /** Pops a function from the stack.
        */
        static void _popFunction() throw();

    };
} // Namespace Ogre
#endif
