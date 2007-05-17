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

#ifndef MEMPROFILER_H
#define MEMPROFILER_H

#include <cstddef>
#include <iostream>

#include "OgreMemProfilerBase.h"

namespace Ogre{

    template <typename T>
    class MemProfiler : public MemProfilerBase
    {
    public:
        /// define our types
        typedef T                   value_type;
        typedef value_type*         pointer;
        typedef const value_type*  const_pointer;
        typedef value_type&         reference;
        typedef const value_type&  const_reference;
        typedef std::size_t        size_type;
        typedef std::ptrdiff_t      difference_type;

        /// convert MemProfiler<T> to MemProfiler<U>
        template<typename U>
        struct rebind
        {
            typedef class MemProfiler<U> other;
        };

    public:
        explicit inline MemProfiler()
        { }

        inline ~MemProfiler()
        { }

        /// copy ctor
        inline explicit MemProfiler(MemProfiler const&)
        { }

        /// converstion
        template <typename U>
        inline explicit MemProfiler(MemProfiler<U> const&)
        { }

        /// Note information about an allocation
        inline void note_allocation(size_type sz,
                        typename std::allocator<void>::const_pointer ptr =0)
        {
            std::cout << "PROBE : alloc" << std::endl;
            mStats.numAllocations++;
            mStats.numBytesAllocated+=sz;
        }

        /// Note information about a deallocation
        inline void note_deallocation(pointer ptr, size_type sz)
        {
            std::cout << "PROBE : purge" << std::endl;
            mStats.numDeallocations++;
            mStats.numBytesDeallocated+=sz;
        }

    protected:
    private:
    };

}// namesoace Ogre

#endif // MEMPROFILER_H
