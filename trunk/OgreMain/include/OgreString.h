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
#ifndef _String_H__
#define _String_H__

#include "OgrePrerequisites.h"

namespace Ogre {

#if OGRE_WCHAR_T_STRINGS
    typedef std::wstring _StringBase;
#else
    typedef std::string _StringBase;
#endif
}

// If we're using the GCC 3.1 C++ Std lib
#if defined( GCC_3_1 )

#include <ext/hash_map>
namespace __gnu_cxx
{
    template <> struct hash< Ogre::_StringBase >
    {
        size_t operator()( const Ogre::_StringBase _stringBase ) const 
        { 
            register size_t ret = 0;
            for( Ogre::_StringBase::const_iterator it = _stringBase.begin(); it != _stringBase.end(); ++it )
                ret = 5 * ret + *it;

            return ret;
        }
    };
}

// If we're using plain vanilla VC7 Std lib
#elif !defined( _STLP_HASH_FUN_H )

namespace std 
{
    template<> size_t hash_compare< Ogre::_StringBase, std::less< Ogre::_StringBase > >::operator ()( const Ogre::_StringBase& _stringBase ) const
    {
        register size_t ret = 0;
        for( Ogre::_StringBase::const_iterator it = _stringBase.begin(); it != _stringBase.end(); ++it )
            ret = 5 * ret + *it;

        return ret;
    }
}

#endif

namespace Ogre {

    /** Wrapper object for String to allow DLL export.
        @note
            Derived from std::string.
    */
    class _OgreExport String : public _StringBase
    {
    public:
        typedef std::stringstream StrStreamType;
    public:
        /** Default constructor.
        */
        String() : _StringBase() {}

        String(const String& rhs) : _StringBase( static_cast< const _StringBase& >( rhs ) ) {}

        /** Copy constructor for std::string's.
        */
        String( const _StringBase& rhs ) : _StringBase( rhs ) {}

        /** Copy-constructor for C-style strings.
        */
        String( const char* rhs ) : _StringBase( static_cast< const _StringBase& >( rhs ) ) {}

        /** Used for interaction with functions that require the old C-style
            strings.
        */
        operator const char* () const { return c_str(); }

        /** Removes any whitespace characters, be it standard space or
            TABs and so on.
            @remarks
                The user may specify wether they want to trim only the
                beginning or the end of the String ( the default action is
                to trim both).
        */
        void trim( bool left = true, bool right = true );

        /** Returns a StringVector that contains all the substrings delimited
            by the characters in the passed <code>delims</code> argument.
            @param 
                delims A list of delimiter characters to split by
            @param 
                maxSplits The maximum number of splits to perform (0 for unlimited splits). If this
                parameters is > 0, the splitting process will stop after this many splits, left to right.
        */
        std::vector< String > split( const String& delims = "\t\n ", unsigned int maxSplits = 0) const;

        /** Upper-cases all the characters in the string.
        */
        String toLowerCase( void );

        /** Lower-cases all the characters in the string.
        */
        String toUpperCase( void );

        /** Converts the contents of the string to a Real.
        @remarks
            Assumes the only contents of the string are a valid parsable Real. Defaults to  a
            value of 0.0 if conversion is not possible.
        */
        Real toReal(void) const;

	/*        
	operator _StringBase()
        {
            return *this;
        }
	*/

        /** Template operator for appending another type into the string. 
            @remarks
                Because this operator is templated, you can append any value into a string as
                long as there is an operator<<(std::basic_iostream, type) or similar method defined somewhere.
                All the primitive types have this already, and many of the Ogre types do too (see Vector3
                for an example).
        */
        template< typename T > String& operator << (T value)
        {
            // Create stringstream based on *this contents
            StrStreamType sstr;
            sstr.str(*this);
            // Seek to end
            sstr.seekp(0, std::ios_base::end);
            // Write
            sstr << value;
            // Assign back
            *this = _StringBase( sstr.str() );

            return *this;
        }
    };

#ifdef GCC_3_1
    typedef ::__gnu_cxx::hash< _StringBase > _StringHash;    
#elif !defined( _STLP_HASH_FUN_H )
    typedef std::hash_compare< _StringBase, std::less< _StringBase > > _StringHash;
#else
    typedef std::hash< _StringBase > _StringHash;
#endif

} // namespace Ogre

#endif // _String_H__
