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

#ifndef OGREMEMOBJECTTRAITS_H
#define OGREMEMOBJECTTRAITS_H

namespace Ogre{

    /**
     * This class defines the required objects traits used by an allocator
     * the specific traits are defined by the ISO C++ standard
     */
    template <typename T>
    class ObjectTraits
    {

    public:
        /// convert ObjectTraits<T> to ObjectTrait<U>
        template<typename U>
        struct rebind
        {
            typedef ObjectTraits<U> other;
        };


        /// type conversion
        template <typename U>
        inline explicit ObjectTraits(ObjectTraits<U> const&)
        { }

        /// address
        inline T* address(T& r)
        {
            return &r;
        }

        /// const address
        inline T const* address(T const& r)
        {
            return &r;
        }

        /// construct (initialise) the object
        /// yes this is using an out variable, but it must.
        /// we have to meet the requirements for the STL
        inline void construct(T* p, const T& t)
        {
            new(p) T(t);
        }

        /// destroy an object
        inline void destroy(T* p)
        {
            p->~T();
        }

        inline explicit ObjectTraits()
        { }

        inline ~ObjectTraits()
        { }

    };

}// namesoace Ogre
#endif // OGREMEMOBJECTTRAITS_H
