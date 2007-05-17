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

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

// defaults
#include "OgreObjectTraits.h"
#include "OgreStdAllocPolicy.h"
#include "OgreMemProfiler.h"

namespace Ogre{

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
                typename AllocPolicy::template rebind<U>::other,
                typename TypeTraits::template rebind<U>::other,
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
        inline Allocator(Allocator const& rhs)
                :Policy(rhs)
                ,Traits(rhs)
        { }

        /// cast
        template <typename U>
        inline Allocator(Allocator<U> const&)
        { }

        /// conversion - done component wise
        template <typename U, class P, class T2>
        inline Allocator(Allocator<U, P,T2> const& rhs)
                :Policy(rhs)
                ,Traits(rhs)
        { }

        /// memory allocation
        // via delegation, so we can hook in the profile
        inline pointer allocate(size_type count,
                                typename std::allocator<void>::const_pointer ptr =0)
        {
            ProfilePolicy::note_allocation(count,ptr);
            return AllocPolicy::allocate(count,ptr);
        }

        /// memory dealocation
        // via delegation, so we can hook in the profile
        inline void deallocate(pointer ptr, size_type sz)
        {
            ProfilePolicy::note_deallocation(ptr,sz);
            AllocPolicy::deallocate(ptr,sz);
        }
    };

}// namesoace Ogre
#endif // ALLOCATOR_H
