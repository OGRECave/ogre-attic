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
#ifndef __IteratorWrappers_H__
#define __IteratorWrappers_H__

#include "OgrePrerequisites.h"

namespace Ogre {

    /** Wraps iteration over a vector.
    @remarks
        This class is here just to allow clients to iterate over an internal
        vector of a class without having to have access to the vector itself
        (typically to iterate you need both the iterator and the end() iterator
        to test for the end condition, which is messy).
        No updates are allowed through this interface, it is purely for 
        iterating and reading.
    @par
        Note that like STL iterators, these iterators are only valid whilst no 
        updates are made to the underlying collection. You should not attempt to
        use this iterator if a change is made to the collection. In fact, treat this
        iterator as a transient object, do NOT store it and try to use it repeatedly.
    */
    template <class T>
    class VectorIterator
    {
    private:
        typename T::iterator mCurrent;
        typename T::iterator mEnd;
        /// Private constructor since only the parameterised constructor should be used
        VectorIterator() {};
    public:
        /** Constructor.
        @remarks
            Provide a start and end iterator to initialise.
        */
        VectorIterator(typename T::iterator start, typename T::iterator end)
            : mCurrent(start), mEnd(end)
        {
        }

        /** Returns true if there are more items in the collection. */
        bool hasMoreElements(void)
        {
            return mCurrent != mEnd;
        }

        /** Returns the next element in the collection, and advances to the next. */
        typename T::value_type getNext(void)
        {
            return *mCurrent++;
        }
        /** Returns the next element in the collection, without advancing to the next. */
        typename T::value_type peekNext(void)
        {
            return *mCurrent;
        }



    };

    /** Wraps iteration over a map.
    @remarks
        This class is here just to allow clients to iterate over an internal
        map of a class without having to have access to the map itself
        (typically to iterate you need both the iterator and the end() iterator
        to test for the end condition, which is messy).
        No updates are allowed through this interface, it is purely for 
        iterating and reading.
    @par
        Note that like STL iterators, these iterators are only valid whilst no 
        updates are made to the underlying collection. You should not attempt to
        use this iterator if a change is made to the collection. In fact, treat this
        iterator as a transient object, do NOT store it and try to use it repeatedly.
    */
    template <class T>
    class MapIterator
    {
    private:
        typename T::iterator mCurrent;
        typename T::iterator mEnd;
        /// Private constructor since only the parameterised constructor should be used
        MapIterator() {};
    public:
        /** Constructor.
        @remarks
            Provide a start and end iterator to initialise.
        */
        MapIterator(typename T::iterator start, typename T::iterator end)
            : mCurrent(start), mEnd(end)
        {
        }

        /** Returns true if there are more items in the collection. */
        bool hasMoreElements(void)
        {
            return mCurrent != mEnd;
        }

        /** Returns the next value element in the collection, and advances to the next. */
        typename T::mapped_type getNext(void)
        {
            return (mCurrent++)->second;
        }
        /** Returns the next value element in the collection, without advancing to the next. */
        typename T::mapped_type peekNextValue(void)
        {
            return mCurrent->second;
        }
        /** Returns the next key element in the collection, without advancing to the next. */
        typename T::key_type peekNextKey(void)
        {
            return mCurrent->first;
        }
        /** Required to overcome intermittent bug */
 	    MapIterator<T> & operator=( MapIterator<T> &rhs )
 	    {
 		    mCurrent = rhs.mCurrent;
 		    mEnd = rhs.mEnd;
 		    return *this;
 	    }



    };

}
#endif
