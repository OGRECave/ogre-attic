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
#ifndef __Config_H_
#define __Config_H_

/** Because of transferring STL containers (which allocate memory) from memory
    space to memory space (ie allocate in a DLL and clean or use in the main 
    app), we have to kink to the dynamic version of the STLport libaray. This 
    way, all the (de)allocations are done in STLport's memory space and everyone
    is happy.
*/
// Probably obsolete now
#ifndef _STLP_USE_DYNAMIC_LIB
#define _STLP_USE_DYNAMIC_LIB
#endif

/** If set to 1, stack unwinding code is compiled into the library and called
    in case an exception is thrown in order to show the call stack.
*/
#define OGRE_STACK_UNWINDING 1

/** If set to 1, special OGRE debug-build asserts are compiled as exception
    throws on release builds
*/
#define OGRE_RELEASE_ASSERT 0

/** If set to 1, OGRE will always 'think' that the graphics card only has one
    texture unit. Very useful for testing multipass fallback.
*/
#define OGRE_TEST_MULTIPASS 0

/** If set to 1, Real is typedef'ed to double. Otherwise, Real is typedef'ed
    to float.
*/
#define OGRE_DOUBLE_PRECISION 0

/** If set to 1, the strings are transforned to Unicode, and char is replaced
    with wchar_t when having to do with strings of any kind.
*/
#define OGRE_WCHAR_T_STRINGS 0

/** Define number of texture coordinate sets allowed per vertex.
*/
#define OGRE_MAX_TEXTURE_COORD_SETS 6

/** Define max number of texture layers allowed per material. 
*/
#define OGRE_MAX_TEXTURE_LAYERS 8

/** Set this to zero if you want to link OGRE as a static lib. 
*/
#define OGRE_DYNAMIC_LINKAGE 1


/** Set this to 0 if you want to use the standard memory manager in Debug builds
    Release builds always use the standard memory manager
*/
#define OGRE_DEBUG_MEMORY_MANAGER 1

#endif
