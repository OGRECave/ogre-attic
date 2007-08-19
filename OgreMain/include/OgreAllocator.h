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
Torus Knot Software Ltd
---------------------------------------------------------------------------
*/

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

// defaults
#include "OgreObjectTraits.h"
#include "OgreStdAllocPolicy.h"
#include "OgreMemProfiler.h"

namespace Ogre{


    /**
    * A memory allocator suitable for use with the STL containers.
    * This class acts as the host for several policies,it provides
    * a unified interface to the policy configured behaviour.
    *
    * It takes the following template parameters:-
    * T      : the type we will be allocating (see rebind later)
    * Policy : a memory allocator policy (optional)
    * Traits : a traits type (optional)
    * Profile: a memory profiling policy (optional)
    *
    * @note why have an element based allocation and a byte based
    * allocation when we know that we need to allocate objects of
    * type T and we know the size of T? Well the allocator is
    * used for both STL container allocations and direct allocations
    * with new and delete. The STL passes a size based on the number
    * of elements of type T to allocate, operator new/delete pass
    * a size in bytes. We have to handle both, the specifics are
    * handled in OgreAllocWrapper. We pass only a byte count to our
    * allocation policy.
    *
    */
    template
    <
        typename T,
        typename Policy = StdAllocPolicy<T>,
        typename Traits = ObjectTraits<T>,
        typename Profile = MemProfiler<T>
    >
    class Allocator :  public Policy,
                       public Traits,
                       public Profile
    {
    private :
        typedef Policy  AllocPolicy;
        typedef Traits  TypeTraits;
        typedef Profile ProfilePolicy;

    public :
        /// Propogate type info into our namespace
        typedef typename AllocPolicy::size_type         size_type;
        typedef typename AllocPolicy::difference_type   difference_type;
        typedef typename AllocPolicy::pointer           pointer;
        typedef typename AllocPolicy::const_pointer     const_pointer;
        typedef typename AllocPolicy::reference         reference;
        typedef typename AllocPolicy::const_reference   const_reference;
        typedef typename AllocPolicy::value_type        value_type;


        /// the rebind mechanism
        template<typename U>
        struct rebind
        {
            typedef Allocator
            <
                U,
                typename AllocPolicy::template   rebind<U>::other,
                typename TypeTraits::template    rebind<U>::other,
                typename ProfilePolicy::template rebind<U>::other
            >
            other;
        };

        /// ctor
        inline explicit Allocator()
        { }

        /// dtor
        inline ~Allocator()
        { }

        /// copy ctor - done component wise
        inline Allocator( Allocator const& rhs )
                : AllocPolicy( rhs )
                , TypeTraits( rhs )
                , ProfilePolicy( rhs )
        { }

        /// cast
        template <typename U>
        inline Allocator( Allocator<U> const& )
        { }

        /// conversion - done component wise
        template <typename U, typename P, typename Tt, typename Pp>
        inline Allocator( Allocator<U, P, Tt, Pp> const& rhs )
                : AllocPolicy( rhs )
                , TypeTraits( rhs )
                , ProfilePolicy( rhs )
        { }

        /// memory allocation (elements)
        // via delegation, so we can hook in the profile
        inline pointer allocate( size_type count,
                                 typename std::allocator<void>::const_pointer ptr = 0 )
        {
            register size_type sz = count*sizeof( T );
            pointer p  = AllocPolicy::allocate( sz, ptr );
            ProfilePolicy::note_allocation( AllocPolicy::block_size( p ), ptr );
            return p;
        }

        /// memory dealocation (elements)
        // via delegation, so we can hook in the profile
        inline void deallocate( pointer ptr, size_type count )
        {
            register size_type sz = count*sizeof( T );
            ProfilePolicy::note_deallocation( ptr, AllocPolicy::block_size( ptr ) );
            AllocPolicy::deallocate( ptr, sz );
        }

        /// memory allocation (bytes)
        // via delegation, so we can hook in the profile
        inline pointer allocateBytes( size_type count,
                                      typename std::allocator<void>::const_pointer ptr = 0 )
        {
            pointer p = AllocPolicy::allocate( count, ptr );
            ProfilePolicy::note_allocation( AllocPolicy::block_size( p ), ptr );
            return p;
        }

        /// memory dealocation (bytes)
        // via delegation, so we can hook in the profile
        inline void deallocateBytes( pointer ptr, size_type sz )
        {
            ProfilePolicy::note_deallocation( ptr, AllocPolicy::block_size( ptr ) );
            AllocPolicy::deallocate( ptr, sz );
        }
    };

}// namesoace Ogre

#endif // ALLOCATOR_H

