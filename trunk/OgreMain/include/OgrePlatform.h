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
#ifndef __Platform_H_
#define __Platform_H_

#include "OgreConfig.h"

/* Initial platform/compiler-related stuff to set.
*/
#define PLATFORM_WIN32 1
#define PLATFORM_LINUX 2
#define PLATFORM_APPLE 3

#define COMPILER_MSVC 1
#define COMPILER_GNUC 2
#define COMPILER_BORL 3

#define ENDIAN_LITTLE 1
#define ENDIAN_BIG 2

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
   the release build in VC7.   
*/
#if OGRE_COMPILER == COMPILER_MSVC
    // Has to be nested because some compilers return strings for their version
#   if OGRE_COMP_VER == 1300
#       define _STLP_LIB_BASENAME "stlport_vc7"
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

/* Finds the current platform */

#if defined( __WIN32__ ) || defined( _WIN32 )
#   define OGRE_PLATFORM PLATFORM_WIN32
#elif defined( __APPLE_CC__)
#   define OGRE_PLATFORM PLATFORM_APPLE
#else
#   define OGRE_PLATFORM PLATFORM_LINUX
#endif

// For generating compiler warnings - should work on any compiler
// As a side note, if you start your message with 'Warning: ', the MSVC
// IDE actually does catch a warning :)
#define _QUOTE_INPLACE_(x) # x
#define QUOTE(x) _QUOTE_INPLACE_(x)
#define warn( x )  message( __FILE__ "(" QUOTE( __LINE__ ) ") : " x "\n" )

//----------------------------------------------------------------------------
// Windows Settings
#if OGRE_PLATFORM == PLATFORM_WIN32

// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#   if defined( OGRE_NONCLIENT_BUILD )
#       define _OgreExport __declspec( dllexport )
#   else
#       define _OgreExport __declspec( dllimport )
#   endif

// Win32 compilers use _DEBUG for specifying debug builds.
#   ifdef _DEBUG
#       define OGRE_DEBUG_MODE 1
#   else
#       define OGRE_DEBUG_MODE 0
#   endif

// A quick define to overcome different names for the same function
#	define snprintf _snprintf

#endif
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Linux/Apple Settings
#if OGRE_PLATFORM == PLATFORM_LINUX || OGRE_PLATFORM == PLATFORM_APPLE

// Linux compilers don't have symbol import/export directives.
#   define _OgreExport

// A quick define to overcome different names for the same function
#   define stricmp strcasecmp

// Unlike the Win32 compilers, Linux compilers seem to use DEBUG for when
// specifying a debug build.
#   ifdef DEBUG
#       define OGRE_DEBUG_MODE 1
#   else
#       define OGRE_DEBUG_MODE 0
#   endif

#endif
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Endian Settings
// check for BIG_ENDIAN config flag, set OGRE_ENDIAN correctly
#ifdef CONFIG_BIG_ENDIAN
#    define OGRE_ENDIAN ENDIAN_BIG
#else
#    define OGRE_ENDIAN ENDIAN_LITTLE
#endif


#endif
