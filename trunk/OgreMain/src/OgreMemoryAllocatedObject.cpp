/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2008 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd
---------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreMemoryAllocatorConfig.h"

namespace Ogre
{
	/* 

	Ugh, I wish I didn't have to do this.

	The problem is that operator new/delete are *implicitly* static. We have to 
	instantiate them for each combination exactly once throughout all the compilation
	units that are linked together, and this appears to be the only way to do it. 

	At least I can do it via templates.

	*/

	/// operator new, with debug line info
	template <class Alloc>
	void* AllocatedObject<Alloc >::operator new(size_t sz, const char* file, int line, const char* func)
	{
		return Alloc::allocateBytes(sz, file, line, func);
	}

	template <class Alloc>
	void* AllocatedObject<Alloc >::operator new(size_t sz)
	{
		return Alloc::allocateBytes(sz);
	}

	/// placement operator new
	template <class Alloc>
	void* AllocatedObject<Alloc >::operator new(size_t sz, void* ptr)
	{
		return ptr;
	}

	/// array operator new, with debug line info
	template <class Alloc>
	void* AllocatedObject<Alloc >::operator new[] ( size_t sz, const char* file, int line, const char* func )
	{
		return Alloc::allocateBytes(sz, file, line, func);
	}

	template <class Alloc>
	void* AllocatedObject<Alloc >::operator new[] ( size_t sz )
	{
		return Alloc::allocateBytes(sz);
	}

	template <class Alloc>
	void AllocatedObject<Alloc >::operator delete( void* ptr )
	{
		Alloc::deallocateBytes(ptr);
	}

	// only called if there is an exception in corresponding 'new'
	template <class Alloc>
	void AllocatedObject<Alloc >::operator delete( void* ptr, const char* , int , const char*  )
	{
		Alloc::deallocateBytes(ptr);
	}

	template <class Alloc>
	void AllocatedObject<Alloc >::operator delete[] ( void* ptr )
	{
		Alloc::deallocateBytes(ptr);
	}

	template <class Alloc>
	void AllocatedObject<Alloc >::operator delete[] ( void* ptr, const char* , int , const char*  )
	{
		Alloc::deallocateBytes(ptr);
	}


	template class AllocatedObject<GeneralAllocPolicy>;
	template class AllocatedObject<GeometryAllocPolicy>;
	template class AllocatedObject<AnimationAllocPolicy>;
	template class AllocatedObject<SceneCtlAllocPolicy>;
	template class AllocatedObject<SceneObjAllocPolicy>;
	template class AllocatedObject<ResourceAllocPolicy>;
	template class AllocatedObject<ScriptingAllocPolicy>;
	template class AllocatedObject<RenderSysAllocPolicy>; 



}
