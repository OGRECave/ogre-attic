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
#ifndef _PLATFORM_H__
#define _PLATFORM_H__

#include "OgreConfig.h"

/* Initial platform/compiler-related stuff to set.
*/
#define PLATFORM_WIN32 1
#define PLATFORM_LINUX 2
#define PLATFORM_APPLE 3

#define COMPILER_MSVC 1
#define COMPILER_GNUC 2
#define COMPILER_BORL 3

/* Finds the compiler type and version.
*/
#if defined( _MSC_VER )
#   define OGRE_COMPILER COMPILER_MSVC
#   define OGRE_COMP_VER _MSC_VER

#elif defined( __GNUC__ )
#   define OGRE_COMPILER COMPILER_GNUC
#   define OGRE_COMP_VER __VERSION__

#elif defined( __BORLANDC__ )
#   define OGRE_COMPILER COMPILER_BORL
#   define OGRE_COMP_VER __BCPLUSPLUS__

#else
#   pragma error "No known compiler. Abort! Abort!"

#endif

/* Hack to get the release version of STLport 4.5.3 to compile when using
   the release build

   Has to be nested because some compilers return strings for their version
*/
#if OGRE_COMPILER == COMPILER_MSVC
#if OGRE_COMP_VER == 1300
#   define _STLP_LIB_BASENAME "stlport_vc7"
#   endif
#endif

/* See if we can use __forceinline or if we need to use __inline instead */
#if OGRE_COMPILER == COMPILER_MSVC
#   if OGRE_COMP_VER >= 1200
#       define FORCEINLINE __forceinline
#   endif
#else
#   define FORCEINLINE __inline
#endif

/* Finds the current platform - note Apple is not yet
   searched for.
*/
#if defined( __WIN32__ ) || defined( _WIN32 )
#   define OGRE_PLATFORM PLATFORM_WIN32
#else
#   define OGRE_PLATFORM PLATFORM_LINUX
#endif

/* Sets the standard character type for OGRE strings
*/
#if OGRE_WCHAR_T_STRINGS == 1
#   define OGRE_CHAR_TYPE wchar_t
#else
#   define OGRE_CHAR_TYPE char
#endif

// For generating compiler warnings - should work on any compiler
#define _QUOTE_INPLACE_(x) # x
#define QUOTE(x) _QUOTE_INPLACE_(x)
#define warn( x )  message( __FILE__ "(" QUOTE( __LINE__ ) ") : " x "\n" )

// ----------------------------------------------------------------------------
// Tell STLport that we want to link to the DLL version under Win32
// must define the _STLP_USE_DYNAMIC_LIB macro at the compiler input.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Windows Settings
// ----------------------------------------------------------------------------
#if OGRE_PLATFORM == PLATFORM_WIN32
#   if OGRE_DYNAMIC_LINKAGE == 0
#       define _OgreExport
#   elif defined( OGRE_LIBRARY_IMPORTS )
#       define _OgreExport __declspec(dllimport)
#   else
#       define _OgreExport __declspec(dllexport)
#   endif
#endif // OGRE_WIN32

//-----------------------------------------------------------------------
// Linux Settings
//-----------------------------------------------------------------------
#if OGRE_PLATFORM == PLATFORM_LINUX
#    define _OgreExport
#    define stricmp strcasecmp
#endif // OGRE_LINUX


#endif // _PLATFORM_H__
