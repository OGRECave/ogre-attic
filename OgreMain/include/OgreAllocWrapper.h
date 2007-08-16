/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef ALLOCWRAPPER_H
#define ALLOCWRAPPER_H

#include "OgreAllocator.h"

// defaults
#include "OgreObjectTraits.h"
#include "OgreStdAllocPolicy.h"
#include "OgreMemProfiler.h"

namespace Ogre{

    /**
     * This class is provided to form the base for a family of objects.
     * It provides an overloaded operator new and delete as well as
     * some utility functions to allow use of a custom allocator.
     * See OgreAllocator.h for more info
     *
     * It takes the following template parameters:-
     * Alloc      : the allocator type to wrap
     */
    template < typename Alloc >
    class AllocWrapper
    {
    public:
        /// this typedef hides the specicifc makup of the allocator
        typedef Alloc                             AllocatorType;
        typedef typename AllocatorType::size_type SizeType;
        typedef typename AllocatorType::pointer   PointerType;

        inline explicit AllocWrapper()
        { }

        virtual ~AllocWrapper()
        { }

        /// operator new
        static inline void* operator new( size_t sz )
        {
            return smAllocator.allocateBytes( sz );
        }

        /// placment operator new
        static inline void* operator new( size_t sz, void* ptr )
        {
            return smAllocator.allocateBytes( sz, ptr );
        }

        /// array operator new
        static inline void* operator new[] ( size_t sz, void* ptr )
        {
            return smAllocator.allocateBytes( sz, ptr );
        }

        /// operator delete
        static inline void operator delete( void* ptr, size_t sz )
        {
            smAllocator.deallocateBytes(
                static_cast<PointerType>( ptr ), sz );
        }

        /// array operator delete
        static inline void operator delete[] ( void* ptr, size_t sz )
        {
            smAllocator.deallocateBytes(
                static_cast<PointerType>( ptr ), sz );
        }

    private:
        static AllocatorType smAllocator;
    };

    // statics
    template<typename Alloc>
    typename AllocWrapper<Alloc>::AllocatorType
        AllocWrapper<Alloc>::smAllocator;

}// namesoace Ogre



#endif // ALLOCWRAPPER_H
